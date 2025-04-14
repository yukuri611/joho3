/*
Copyright 2016 New System Vision Research and Development Institute.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


#define C_WAYS			(1 << C_WAY_BITS)
#define C_WORD_COUNT	(1 << C_WORD_BITS)
#define C_LINE_BITS		(C_SIZE_BITS - C_WAY_BITS - C_WORD_BITS)
#define C_LINE_COUNT	(1 << C_LINE_BITS)
#define C_WAY_MASK		(C_WAYS - 1)
#define C_SIZE			(1 << C_SIZE_BITS)

#define DCACHE_EXT_WORD_RATIO_BIT	0	///	32 / 2^1 = 16
#define DCACHE_EXT_WORD_RATIO		(1 << (DCACHE_EXT_WORD_RATIO_BIT))

template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
void  Cache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::initialize_tag_only() {
    BIT last_init = (reg.lidx == C_LINE_COUNT - 1);
    int j;
    for (j = 0; j < C_WAYS; j++) {
        tagv[j][reg.lidx] = 0;
    }
    reg.lidx = (last_init) ? 0 : reg.lidx + 1;
    if (last_init) { reg.init_flag = 1; }
}

template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
void  Cache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::initialize_tag_and_word() {
    BIT last_init = (reg.lidx == C_LINE_COUNT - 1);
    int i, j;
    for (j = 0; j < C_WAYS; j++) {
        for (i = 0; i < C_WORD_COUNT; i++) { word[j][i][reg.lidx] = 0; }
        tagv[j][reg.lidx] = 0;
    }
    reg.lidx = (last_init) ? 0 : reg.lidx + 1;
    if (last_init) { reg.init_flag = 1; }
}

template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
UINT32  Cache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::read_tag_line(UINT32 lidx, UINT4 widx, UINT32 tag) {
    int j;
    BIT		hit_flag = 0, empty_flag = 0;
    UINT4	hit_way = 0, empty_way = 0;
    UINT32	hit_tagv = 0, cword = 0;
    for (j = 0; j < C_WAYS; j++) {
        UINT32 ctagv = tagv[j][lidx];
        BIT is_hit = (ctagv == tag);
        BIT is_empty = (ctagv & 1) == 0;
#if 1	///	10/13/17 : bug in C2RTL!!! FIXME!!! --> 10/16/17 : no, this is NOT A BUG!!!
        UINT32 cwtmp = word[j][widx][lidx];
        cword = (is_hit) ? cwtmp : cword;
#else
        ///	below expression implies that word[j][widx][lidx] is accessed ONLY IF (is_hit == 1)
        ///	where is_hit is the result from accessing tagv[j][lidx]
        ///	so there is a pipeline delay after accessing tagv[j][lidx] and before word[j][widx][lidx]
        ///	so below expression results in INCORRECT pipeline structure!!!!
        cword = (is_hit) ? word[j][widx][lidx] : cword;
#endif
        hit_way = (is_hit) ? j : hit_way;
        hit_tagv = (is_hit) ? ctagv : hit_tagv;
        hit_flag = is_hit | hit_flag;
        empty_flag = is_empty | empty_flag;
        empty_way = (is_empty) ? j : empty_way;
    }
    stt.next_way = (hit_flag) ? hit_way : (empty_flag) ? empty_way : reg.rand_way;
    stt.missed = !hit_flag;
    reg.rand_way = (reg.rand_way + 1) & C_WAY_MASK;
#if defined(DBG_CACHE_ACCESS_COUNT)
    reg.cache_access++;
#endif
    return cword;
}

template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
UINT32  Cache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::fetch_cache(UINT32 addr) {
    UINT32 tagv = ((addr >> (C_LINE_BITS + C_WORD_BITS)) << 1) | 1;	///	tag already includes valid bit
    UINT32 lidx = (addr >> C_WORD_BITS) & (C_LINE_COUNT - 1);
    UINT32 widx = addr & (C_WORD_COUNT - 1);

    reg.tag = tagv;
    reg.lidx = lidx;
    reg.widx = (UINT4)widx;
    reg.addr = addr & ~(C_WORD_COUNT - 1);	///	paddr is aligned to cache line size

    return read_tag_line(lidx, widx, tagv);
}

template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
void  Cache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::update_cache() {	///	refer to load_icache():
    ///	cpu.icache_stt.new_cache_line	: feedback signal from DC stage
    ///	cpu.icache_reg.line				: register (1-cycle delayed from cpu.icache_stt.new_cache_line)
    word[stt.way_idx][stt.next_update_widx][reg.lidx] = stt.next_word;
    tagv[stt.way_idx][reg.lidx] = reg.tag;
    if (dbg_cache_flag) {
        printf("update_cache : way(%d)), widx(%d), lidx(%d) <-- %08x\n",
            stt.way_idx, stt.next_update_widx, reg.lidx, stt.next_word);
    }
}

template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
void  DCache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::update_tag(int id) {	///	refer to load_icache():
	this->tagv[this->stt.way_idx][this->reg.lidx] = this->reg.tag;
	if (this->dbg_cache_flag) {
        if (dstt.update_tag_after_miss) {
            printf("[%d] update_dcache_tag(after miss) : way(%d)), lidx(%2d) <-- %08x\n",
                id, this->stt.way_idx, this->reg.lidx, this->reg.tag);
        }
        else {
            printf("[%d] update_dcache_tag(write hit ) : way(%d)), lidx(%2d) <-- %08x\n",
                id, this->stt.way_idx, this->reg.lidx, this->reg.tag);
        }
	}
}

template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
UINT3 Cache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::get_io_op(bool load_cache_active) {
    return (!load_cache_active || reg.state >= C_WORD_COUNT) ? IO_Idle : IO_ReadData;
}

template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
void Cache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::PrintStat(const char *cname, int idx) {
    CacheComp::PrintCacheStat(reg.cache_access, reg.cache_miss, C_SIZE, C_WAYS, C_WORD_COUNT, C_LINE_COUNT, cname, idx);
}

#define AVOID_LLVM_OPT_ON_BRANCHING_CALL

#if defined(TEST_ICACHE_AXI)
template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
bool Cache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::load_cache(AXI4L::CH *axi) {	///	@ DC-stage
    bool load_cache_active = (stt.missed || reg.load_state != CacheComp::CIS_Idle);
    if (load_cache_active) {
        if (dbg_cache_flag) {
            printf("load_cache : reg.state(%d), load_state(%d), miss(%d)\n", reg.state, reg.load_state, reg.cache_miss);
        }
        if (reg.state == 0 && reg.load_state == CacheComp::CIS_Idle) {
#if defined(DBG_CACHE_ACCESS_COUNT)
            reg.cache_miss++;
#endif
            reg.way_idx = stt.next_way;
        }
        else {	///	(cpu.icache_reg.state > 0)
            stt.way_idx = reg.way_idx;
            stt.next_word = reg.word;
        }
    }
    axi_ic.addr_out = (reg.addr + reg.state) | EXT_MEM_MASK;
    //		bool load_cache_active = (stt.missed || reg.load_state != CIS_Idle);
#if defined(AVOID_LLVM_OPT_ON_BRANCHING_CALL)
    ///	10/6/17 : watch out for llvm optimization --> axi_ic.fsm is duplicated on different control paths
    ///	10/6/17 : we don't want this....
    UINT3 io_op = get_io_op(load_cache_active);
#else
    UINT3 io_op;
    if (load_cache_active) {
        io_op = (reg.state < C_WORD_COUNT && reg.load_state != CIS_Pending) ? IO_ReadData : IO_Idle;
    }
    else { io_op = IO_Idle; }
#endif
    axi_ic.burst_length = C_WORD_COUNT - 1;
    axi_ic.fsm(0, io_op, 0, axi);
    if (load_cache_active) {
        if (reg.load_state == CacheComp::CIS_Active) {
            stt.update = 1;	//	update icache in FE-stage (at the SAME CYCLE)
            stt.next_update_widx = reg.update_widx;
            stt.update_word = (reg.update_widx == reg.widx);
        }
        if (reg.state < C_WORD_COUNT) {
            if (reg.load_state == CacheComp::CIS_Idle) { reg.load_state = CacheComp::CIS_Pending; }
            else {
                if (axi_ic.data_latched) {
                    reg.word = axi_ic.data_in;
                    reg.load_state = CacheComp::CIS_Active;
                    reg.update_widx = reg.state;
                    reg.state++;
                }
                else { reg.load_state = CacheComp::CIS_Pending; }
            }
        }
        else {	///	(cpu.icache_stt.state == 4) : last state
            reg.state = 0;	//	quit the state machine
            reg.load_state = CacheComp::CIS_Idle;
        }
    }
    return load_cache_active;
}
#else
template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
void Cache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::load_cache(UINT32 *mem) {	///	@ DC-stage
    if (reg.state == 0) {
#if defined(DBG_CACHE_ACCESS_COUNT)
        reg.cache_miss++;
#endif
        reg.word = 0;
        reg.way_idx = stt.next_way;
    }
    else {	///	(cpu.icache_reg.state > 0)
        UINT32 new_icache_word = stt.mem_data;	//	now 32bit
        reg.word = new_icache_word;	///	register: can be accessed at next cycle
        stt.next_word = new_icache_word;	///	feedback signal: can be accessed at the same cycle
        stt.way_idx = reg.way_idx;
    }
    if (reg.state != 0) {
        stt.update = 1;	//	update icache in FE-stage (at the SAME CYCLE)
        stt.next_update_widx = reg.update_widx;
        stt.update_word = (reg.update_widx == reg.widx);
    }
    if (reg.state < C_WORD_COUNT) {
        UINT32 maddr = reg.addr + reg.state;
        stt.mem_data = mem[maddr];
        reg.update_widx = reg.state;
        reg.state++;
    }
    else {	///	(cpu.icache_stt.state == 4) : last state
        reg.state = 0;	//	quit the state machine
    }
}
#endif

template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
void DCache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::read_tag(UINT32 lidx, UINT4 widx, UINT32 tag) {
	int j;
	BIT		hit_flag = 0, empty_flag = 0;
	UINT4	hit_way = 0, empty_way = 0, rand_way = this->reg.rand_way;
	UINT32	hit_tagv = 0, rand_tagv = 0, ctag;
	for (j = 0; j < C_WAYS; j++) {
		UINT32 ctagv = this->tagv[j][lidx];
		BIT is_hit = ((ctagv >> 1) == (tag >> 1));
		BIT is_empty = (ctagv & 2) == 0;
		hit_way = (is_hit) ? j : hit_way;
		hit_tagv = (is_hit) ? ctagv : hit_tagv;
		hit_flag = is_hit | hit_flag;
		empty_flag = is_empty | empty_flag;
		empty_way = (is_empty) ? j : empty_way;
		rand_tagv = (j == rand_way) ? ctagv : rand_tagv;
	}
    this->stt.next_way = (hit_flag) ? hit_way : (empty_flag) ? empty_way : this->reg.rand_way;
    this->stt.missed = !hit_flag;
	ctag = (hit_flag) ? hit_tagv : (!empty_flag) ? rand_tagv : 0;
    this->reg.rand_way = (rand_way + 1) & C_WAY_MASK;
	dstt.tagv = ctag;
	dstt.next_wflag = (ctag & 1) != 0;
	dstt.accessed = 1;
#if defined(DBG_CACHE_ACCESS_COUNT)
    this->reg.cache_access++;
#endif
}

template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
void DCache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::fetch_tag(UINT32 addr, BIT wflag) {
	UINT32 tagv = ((addr >> (C_LINE_BITS + C_WORD_BITS)) << 2) | 0x2 | wflag;	///	bit(1) : valid flag, bit(0) : wflag
	UINT32 lidx = (addr >> C_WORD_BITS) & (C_LINE_COUNT - 1);
	UINT32 widx = addr & (C_WORD_COUNT - 1);

	this->reg.tag = tagv;
    this->reg.lidx = lidx;
    this->reg.widx = (UINT4)widx;
    this->reg.addr = addr & ~(C_WORD_COUNT - 1);	///	paddr is aligned to cache line size

	read_tag(lidx, widx, tagv);
}

	/*****  : dcache writeback + load sequence: 32-bit external memory version
	///////////// initial state (0) /////////////
	0: (state, pending) == (0, 0) : --> (0, 1)[1] or (1, 2)[10]
		(tag.wflag == 1) : dcache.stt.next_word = read_dcache_line(..);  --> goto [1]
		(tag.wflag == 0) : dcache.stt.next_word = dmem32[daddr];  --> goto [6]
	///////////// writeback states (1, 2, 3, 4) /////////////
	1: (state, pending) == (1, 1) : --> (2, 1)[2]
		dmem32[wb_daddr] = dcache.stt.next_word; /// store 32 bits
		dcache.stt.next_word = read_dcache_line(..);
	2: (state, pending) == (2, 1) : --> (3, 1)[3]
		dmem32[wb_daddr] = dcache.stt.next_word; /// store 32 bits
		dcache.stt.next_word = read_dcache_line(..);
	3: (state, pending) == (3, 1) : --> (4, 1)[4]
		dmem32[wb_daddr] = dcache.stt.next_word; /// store 32 bits
		dcache.stt.next_word = read_dcache_line(..);
	4: (state, pending) == (4, 1) : --> (0, 2)[4]
		dmem32[wb_daddr] = dcache.stt.next_word; /// store 32 bits

	///////////// load states (5, 6, 7, 8, 9) /////////////
	5: (state, pending) == (0, 2) : --> (1, 2)[6]
		dcache.stt.next_word = dmem32[daddr]; /// load higher 16 bits
	6: (state, pending) == (1, 2) : --> (2, 2)[7]
		write_dcache_line(.., dcache.stt.next_word);
		dcache.stt.next_word = dmem32[daddr]; /// load lower 16 bits
	7: (state, pending) == (2, 2) : --> (3, 2)[7]
		write_dcache_line(.., dcache.stt.next_word);
		dcache.stt.next_word = dmem32[daddr]; /// load lower 16 bits
	8: (state, pending) == (3, 2) : --> (4, 2)[7]
		write_dcache_line(.., dcache.stt.next_word);
		dcache.stt.next_word = dmem32[daddr]; /// load lower 16 bits
	9: (state, pending) == (4, 2) : end
		write_dcache_line(.., dcache.stt.next_word);
	*/

//#define DBG_PIPE_COND(cycle)	(cycle >= 80400 && cycle <= 80860)

#if defined(TEST_DCACHE_AXI)
template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
UINT3 DCache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::get_io_op(bool rflag, bool wflag) {
    return (rflag) ? IO_ReadData : (wflag) ? IO_WriteData : IO_Idle;
}

/// 11/14/17 : redo this!!!
template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
void DCache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::process_dcache(CPU *cpu, AXI4L::CH *axi) {
    BIT dcache_load_pending = this->stt.missed || dreg.pending;
    BIT writeOp = (cpu->ex_reg.op == RVO_st), readOp = (cpu->ex_reg.op == RVO_ld);
    BIT wb_start = dcache_load_pending && (dreg.pending == 0) && dstt.next_wflag;
    UINT32 dcache_word = 0, dmword = 0;
    ///	dcache accesses are merged here...
    UINT2 byte_pos = (cpu->ex_reg.addr & 0x3);
    BIT dcache_read_op, dcache_write_op, dcache_read_WB, dcache_write_LD;

    this->stt.way_idx = (dstt.accessed) ? this->stt.next_way : this->reg.way_idx;
    if (dstt.accessed) { this->reg.way_idx = this->stt.next_way; }
    dstt.wflag = !dstt.accessed || dstt.next_wflag;
    if (dcache_load_pending && dreg.pending == 0) {
#if defined(DBG_CACHE_ACCESS_COUNT)
        this->reg.cache_miss++;
#endif
        if (this->dbg_cache_flag) {
            printf("[%d] dcache_miss : way(%d), widx(%d), lidx(%2d), tag(%08x), addr(%08x:%x), wb(%d), pc(%08x) wc(%6d) [%8d]\n",
                cpu->pid_dbg, this->stt.way_idx, this->reg.widx, this->reg.lidx, this->reg.tag, cpu->ex_reg.addr >> 2, byte_pos,
                dstt.next_wflag, cpu->ex_reg.cur_pc, cpu->dm_sig.wcount, cpu->cycle);
#if !defined(__LLVM_C2RTL__)
            fprintf(cpu->fp_dcache, "dcache_miss : way(%d), widx(%d), lidx(%2d), tag(%08x), addr(%08x:%x), wb(%d)\n",
                this->stt.way_idx, this->reg.widx, this->reg.lidx, this->reg.tag, cpu->ex_reg.addr >> 2, byte_pos, dstt.next_wflag);
#endif
        }
    }
    bool axi_w = dcache_load_pending && (dreg.pending == 1);
    bool axi_r = dcache_load_pending && (dreg.pending != 1) && !wb_start && (this->reg.state < C_WORD_COUNT);
    this->axi_ic.addr_out = ((axi_r) ? this->reg.addr : dreg.wb_daddr) | EXT_MEM_MASK;
    this->axi_ic.burst_length = C_WORD_COUNT - 1;
    this->axi_ic.fsm(0, get_io_op(axi_r, axi_w), this->stt.next_word, axi);

    int read_WB_state = this->reg.state + this->axi_ic.dout_ready;
    this->reg.word = this->axi_ic.data_in;
    if (dcache_load_pending) {
        dcache_read_op = 0;
        dcache_write_op = 0;
        dcache_read_WB = wb_start || (dreg.pending == 1 && read_WB_state < C_WORD_COUNT);
        dcache_write_LD = (dreg.pending == 2) && (this->reg.state != 0) && (this->reg.load_state == CacheComp::CIS_Active);
    }
    else {
        dcache_read_op = readOp;// !wflag;
        dcache_write_op = writeOp;// wflag;
        dcache_read_WB = 0;
        dcache_write_LD = 0;
    }
    if (dcache_read_op || dcache_read_WB) {			///	dcache_read or dcache_WB (writeback)
        int widx = (dcache_read_op) ? this->reg.widx : read_WB_state;
        dcache_word = this->word[this->stt.way_idx][widx][this->reg.lidx];
        UINT2 bpos = (dcache_read_op) ? byte_pos : 0;
        if (this->dbg_cache_flag && dcache_read_op) {
            printf("[%d] read_dcache : word[%2d][%2d][%2d](%d) --> %08x : (%5d, %5d)\n",
                cpu->pid_dbg, this->stt.way_idx, widx, this->reg.lidx, bpos, dcache_word, cpu->dm_sig.rcount, cpu->dm_sig.wcount);
        }
        cpu->wb_sig.dout = cpu->format_read_data(dcache_word, bpos);
    }
    else if (dcache_write_op || dcache_write_LD) {	///	dcache_write or dcache_LD (load dcache-line)
        UINT32 din = (dcache_write_op) ? format_write_data(cpu->ex_reg.st_data, byte_pos) : this->stt.next_word;
        UINT4 byte_en = (dcache_write_op) ? cpu->byte_enable(byte_pos) : 0xf;
        int widx = (dcache_write_op) ? this->reg.widx : this->reg.update_widx;
        ///	10/10/17 : no its NOT ok!!! ---> 10/16/17 : C2RTL is FIXED!!!!
        write_mem(&this->word[this->stt.way_idx][widx][this->reg.lidx], byte_en, din);
    }
    if (dcache_load_pending) {	///	only for dcache-miss...
#if 0   /// this part is replaced with axi
        if (dreg.pending == 1) {	///	writeback
            cpu->dmem[dreg.wb_daddr + this->reg.state - 1] = this->stt.next_word;		///	store 16-bits to external memory
        }
        else if (!wb_start && this->reg.state < C_WORD_COUNT) {	///	load
            dmword = cpu->dmem[this->reg.addr + this->reg.state];
        }
#endif
        if (dreg.pending == 0) {	///	initial state
            dreg.wb_daddr = ((dstt.tagv >> 2) << (C_LINE_BITS + C_WORD_BITS)) | (this->reg.lidx << C_WORD_BITS);
        }
        dstt.update_tag_after_miss = (this->reg.state == C_WORD_COUNT) && (dreg.pending == 2);
        if (wb_start || dreg.pending == 1) {	///	writeback : load dcache_line and write to external memory
            this->stt.next_word = dcache_word;	///	dcache_word	= read_dcache_line(lidx, widx);
        }
        else {	///	load dcache-line : load external memory and write to dcache
            this->stt.next_word = this->reg.word;   /// should not be axi_ic.data_in;
        }
        if (dreg.pending) {
            if (this->reg.state == C_WORD_COUNT) {
                this->reg.load_state = CacheComp::CIS_Idle;
                this->reg.state = 0;	//	quit the state machine if (dreg.pending == 2)
                dreg.pending = (dreg.pending == 1) ? 2 : 0;
            }
            else if (this->axi_ic.data_latched | this->axi_ic.dout_ready) {
                this->reg.load_state = CacheComp::CIS_Active;
                this->reg.update_widx = this->reg.state;
                this->reg.state++;
            }
            else { this->reg.load_state = CacheComp::CIS_Pending; }
        }
        else {
            dreg.pending = (wb_start) ? 1 : 2;
            this->reg.state = 0;
            this->reg.update_widx = 0;
        }
        cpu->dm_sig.pctl.stall = 1;
    }
}
#else
template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS>
void DCache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>::process_dcache(CPU *cpu) {
    BIT dcache_load_pending = this->stt.missed || dreg.pending;
	BIT writeOp = (cpu->ex_reg.op == RVO_st), readOp = (cpu->ex_reg.op == RVO_ld);
	BIT wb_start = dcache_load_pending && (dreg.pending == 0) && dstt.next_wflag;
	UINT32 dcache_word = 0, dmword = 0;
	///	dcache accesses are merged here...
	UINT2 byte_pos = (cpu->ex_reg.addr & 0x3);
	BIT dcache_read_op, dcache_write_op, dcache_read_WB, dcache_write_LD;

    /// 11/14/17 : be careful with state assignment dependence!!! this->reg.way_idx
    this->stt.way_idx = (dstt.accessed) ? this->stt.next_way : this->reg.way_idx;
    if (dstt.accessed) { this->reg.way_idx = this->stt.next_way; }  /// <-- reg.way_idx should be referred AFTER it's assignment
    dstt.wflag = !dstt.accessed || dstt.next_wflag;

    if (dcache_load_pending && dreg.pending == 0) {
#if defined(DBG_CACHE_ACCESS_COUNT)
        this->reg.cache_miss++;
#endif
		if (this->dbg_cache_flag) {
			printf("dcache_miss : way(%d), widx(%d), lidx(%d), tag(%08x), addr(%08x:%x), wb(%d)\n",
                this->stt.way_idx, this->reg.widx, this->reg.lidx, this->reg.tag, cpu->ex_reg.addr >> 2, byte_pos, dstt.next_wflag);
		}
	}
	if (dcache_load_pending) {
		dcache_read_op = 0;
		dcache_write_op = 0;
		dcache_read_WB = wb_start || (dreg.pending == 1 && this->reg.state < C_WORD_COUNT);
		dcache_write_LD = dreg.pending == 2 && this->reg.state != 0;
	}
	else {
        dcache_read_op = readOp;// !wflag;
        dcache_write_op = writeOp;// wflag;
		dcache_read_WB = 0;
		dcache_write_LD = 0;
	}
	if (dcache_read_op || dcache_read_WB) {			///	dcache_read or dcache_WB (writeback)
		int widx = (dcache_read_op) ? this->reg.widx : this->reg.state;
		dcache_word = this->word[this->stt.way_idx][widx][this->reg.lidx];
#if 0   /// 11/13/17 : debugging...
        if (((int)dcache_word < -1 || (int)dcache_word > 1) && dcache_read_WB && cpu->dm_sig.wcount < 10510) {
            printf("dcache_read_WB : word[%2d][%2d][%2d] --> %08x : %d [%8d]\n",
                stt.way_idx, widx, reg.lidx, dcache_word, cpu->dm_sig.wcount, cpu->cycle);
        }
#endif
        UINT2 bpos = (dcache_read_op) ? byte_pos : 0;
		if (this->dbg_cache_flag && dcache_read_op) {
            printf("read_dcache : word[%2d][%2d][%2d](%d) --> %08x\n",
                this->stt.way_idx, widx, this->reg.lidx, bpos, dcache_word);
		}
		cpu->wb_sig.dout = cpu->format_read_data(dcache_word, bpos);
	}
	else if (dcache_write_op || dcache_write_LD) {	///	dcache_write or dcache_LD (load dcache-line)
		UINT32 din = (dcache_write_op) ? format_write_data(cpu->ex_reg.st_data, byte_pos) : this->stt.next_word;
		UINT4 byte_en = (dcache_write_op) ? cpu->byte_enable(byte_pos) : 0xf;
		int widx = (dcache_write_op) ? this->reg.widx : this->reg.update_widx;
    	///	10/10/17 : no its NOT ok!!! ---> 10/16/17 : C2RTL is FIXED!!!!
		write_mem(&this->word[this->stt.way_idx][widx][this->reg.lidx], byte_en, din);
	}
	if (dcache_load_pending) {	///	only for dcache-miss...
		if (dreg.pending == 1) {	///	writeback
			cpu->dmem[dreg.wb_daddr + this->reg.state - 1] = this->stt.next_word;		///	store 16-bits to external memory
		}
		else if (!wb_start && this->reg.state < C_WORD_COUNT) {	///	load
			dmword = cpu->dmem[this->reg.addr + this->reg.state];
		}
		if (dreg.pending == 0) {	///	initial state
			dreg.wb_daddr = ((dstt.tagv >> 2) << (C_LINE_BITS + C_WORD_BITS)) | (this->reg.lidx << C_WORD_BITS);
		}
		dstt.update_tag_after_miss = (this->reg.state == C_WORD_COUNT) && (dreg.pending == 2);
		if (wb_start || dreg.pending == 1) {	///	writeback : load dcache_line and write to external memory
            this->stt.next_word = dcache_word;	///	dcache_word	= read_dcache_line(lidx, widx);
		}
		else {	///	load dcache-line : load external memory and write to dcache
            this->stt.next_word = dmword;		///	dmword = cpu.dmem16[dmaddr];
		}
		if (dreg.pending) {
			if (this->reg.state == C_WORD_COUNT) {
                this->reg.state = 0;	//	quit the state machine
				dreg.pending = (dreg.pending == 1) ? 2 : 0;
			}
			else {
                this->reg.update_widx = this->reg.state;
                this->reg.state++;
			}
		}
		else {
			dreg.pending = (wb_start) ? 1 : 2;
            this->reg.state = 1;
            this->reg.update_widx = 0;
		}
        cpu->dm_sig.pctl.stall = 1;
    }
}
#endif

#undef C_WAYS
#undef C_WORD_COUNT
#undef C_LINE_BITS
#undef C_LINE_COUNT
#undef C_WAY_MASK
#undef C_SIZE

