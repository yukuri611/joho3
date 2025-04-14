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

#if !defined(PROC_ARCH_BASE_H)
#define PROC_ARCH_BASE_H

struct CPUComp {
    struct Field {
        SINT32 msbPos, bitLength, codeOffset, isSigned;
    };
    struct PipeCtrl {
        BIT			stall;			//	merged stall signal (referenced by previous stages)
        ST_BIT		stall_fw;		//	latched stall signal (referenced by later stages)
        BIT			active;			// !stalled_flag --> referenced by previous stages)
        BIT			stalled_flag;	// <- wire(stalled | later_stages.stall_prev) : referenced by this stage
    };
    struct BranchStatus {
        BIT		active;
        UINT32	addr;
    };
    struct DstStatus {
        BIT		valid, ldm_f;
        UINT6	id;
        UINT32	out;
    };
    enum WritebackEnum  {
        W_null      = 0x0,
        W_ex_out    = 0x1,
        W_dout      = 0x2,
    };
    /// FE : fetch stage
    struct FESig {
        PipeCtrl	pctl;
    };
    struct FEReg  {
        UINT32	cur_pc, nxt_pc;
        BIT     start;  /// 11/20/16 : added
    } _TYPE(state);
    /// DC : decode stage
    struct DCSig {
        PipeCtrl		pctl;
        BranchStatus	br;
    };
    struct DCReg {
        UINT32	src[3], cur_pc;
        UINT6   op;
        UINT5   dst_id, src_id[2];
        BIT     dst_f, src_f[2];
    } _TYPE(state);
    /// EX : execute stage
    struct EXSig {
        PipeCtrl		pctl;
        DstStatus	    dst;
        UINT2	        wbflag;
        UINT6	        op;
    };
    struct EXReg {
        UINT32	out;
        UINT2	wbflag;
        /// 4/12/16
        BIT     dst_f;
        UINT6   dst_id;
        UINT6	op;
        UINT32	addr, st_data;
        UINT32	cur_pc;	///	dbg...
    } _TYPE(state);
    /// DM : data-memory stage
    struct DMSig {
        PipeCtrl		pctl;
        DstStatus	    dst;
        UINT2			wbflag;
    };
    struct DMReg {
        UINT32	out;
        UINT2	wbflag;
        /// 4/12/16
        BIT     dst_f;
        UINT6   dst_id;
        UINT32	cur_pc;	///	dbg...
    } _TYPE(state);
    /// WB : writeback stage
    struct WBSig {
        PipeCtrl		pctl;
        DstStatus	    dst;
        UINT32			dout;
    };
};


#if 0	///	2/6/15 : pipe stall control memo...
------------------(I - cache miss only)
FE	DC	EX	DM	WB
e	d	c	b	a
- I$	d	c	b
- I$ - d	c
- I$ - -d
uI	I$*	---(uI : update I - cache)
f	e - --
g	f	e - -
h	g	f	e -
i	h	g	f	e
.................
------------------(D - cache miss only)
FE	DC	EX	DM	WB
e	d	c	b	a
- --D$	b
- --D$ -
---D$ -
--uD	D$*	-(uD : update D - cache)
f	e	d	c -
g	f	e	d	c
h	g	f	e	d
i	h	g	f	e
.................
------------------(I - cache miss-- (1 cycle)--> D - cache miss)
FE	DC	EX	DM	WB
e	d	c	b	a
- I$	d	c	b
- I$ - D$	c
- I$ - D$ -
uI	I$*	-D$ -
--uD	D$*	-
f	e	d	d -
g	f	e	d	d
h	g	f	e	d
i	h	g	f	e
.................
------------------(I - cache miss & D - cache miss : same cycle)
FE	DC	EX	DM	WB
e	d	c	b	a
- I$ - D$	b
- I$ - D$ -
-I$ - D$ -
uI	I$*	uD	D$*	-
f	e	d	c -
g	f	e	d	c
h	g	f	e	d
i	h	g	f	e
.................
------------------(I - cache miss + div - stall@EX)
    FE	DC	EX	DM	WB
    e	d	c	b	a
    - I$ / c	b
    - I$ / -c
    - I$ / --
    uI	I$*	---
    f	e - --
    g	f	e - -
    h	g	f	e -
    i	h	g	f	e
    .................
    ------------------(ldm - stall@DC)
        FE	DC	EX	DM	WB
        e	LD	c	b	a
        - e!LD	c	b
        f	e - LD	c
        g	f	e - LD
        h	g	f	e -
        i	h	g	f	e
        .................
        ------------------(ldm - stall@DC --(1 cycle)--> D$ - stall)
        FE	DC	EX	DM	WB
        e	LD	c	b	a
        - e!LD	c	b
        - --D$	c
        - --D$	c
        - --D$	c
        - -uD	D$*	c
        f	e - LD	c
        g	f	e - LD
        h	g	f	e -
        i	h	g	f	e
        .................

        a: SP <-SP - 0x14;
b: M[SP + 0x14] <-R1; (STM)
    c : M[SP - 0x4] <-LKR, LKR <-PC + 4, PC <-call_addr(CALL)

    FE	DC	EX	DM	WB
    b	a - --
    c	b	a - -SP -= 0x14
    d	c	b	a - (SP : EX <-DM)
    e	d	c	b	a(SP: EX <-WB)

    FE	DC	EX	DM	WB
    b	a - --
    c	b	a - -SP -= 0x14
    d	c	b	a - (SP: EX <-DM)
    - --D$	a	D$ - miss on b : STM
    - --D$ -
    ---D$ -
    --uD	D$*	-
    e	d	c	b - (SP : EX <-WB) <= == this is not possible!!!need to change data - forwarding paths!!!

    FE	DC	EX	DM	WB
    b	a - --
    c	b	a - -SP -= 0x14
    d	c	b	a - (SP: EX <-DM)(SP: DC <-DM) <-- - data - forwarding changed
    e	d	c	b	a

    FE	DC	EX	DM	WB
    b	a - --
    c	b	a - -SP -= 0x14
    d	c	b	a - (SP: EX <-DM)(SP: DC <-DM) <-- - data - forwarding changed
    - --D$	a	D$ - miss on b : STM
    - --D$ -
    ---D$ -
    --uD	D$*	-
    e	d	c	b -
#endif

#if 0 //: data forwarding paths
    DC : operand fetch
    R0(branch / indirect jump) : <--EX | DM | WB
    R0 - hazard condition : LDM R0 @ EX | DM
    LKR(return LDM) : <--WB(LKR is modified only by RTN insn)
    LKR - hazard condition : LDM LKR @ EX <= == changed to : LDM LKR @ EX | DM
    operand Rx : <--WB(cur_insn - 3) <= == change this to : <--DM(non - LDM) | WB(cur_insn - 2(non - LDM) | 3)
    Rx - hazard condition : LDM Rx @ EX(previous insn is LDM Rx)
    EX : operand fetch
    operand Rx : <--DM | WB(LDM) (cur_insn - 1 | 2(LDM))

#endif

    ///	2/5/15
#define PST(pipe)		pipe##_sig.pctl.stall
#define PST_FW(pipe)	pipe##_sig.pctl.stall_fw

    ///	pipe-stages which generates stalls : 
    ///	- DC (I$-miss, br/ld hazards)
    ///	- EX (multi-cycle divider)
    ///	- DM (D$-miss)
    ///	FE.stall = WB.stall = 0; (constant)

#if defined(ADD_DM_STAGE)   /// 5-stage
    ///	backward stall signals
#define DM_STALL_BW		(PST(dm))					///	(dm_sig.pctl.stall)
#define EX_STALL_BW		(PST(ex) | DM_STALL_BW)		///	(ex_sig.pctl.stall | dm_sig.pctl.stall)
#define DC_STALL_BW		(PST(dc) | EX_STALL_BW)		///	(dc_sig.pctl.stall | ex_sig.pctl.stall | dm_sig.pctl.stall)
    ///	forward stall signals
#define DC_STALL_FW		(PST_FW(dc))				///	(dc_sig.pctl.stall_fw)
#define EX_STALL_FW		(DC_STALL_FW | PST_FW(ex))	///	(ex_sig.pctl.stall_fw | dc_sig.pctl.stall_fw)
#define DM_STALL_FW		(EX_STALL_FW | PST_FW(dm))	///	(dm_sig.pctl.stall_fw | ex_sig.pctl.stall_fw | dc_sig.pctl.stall_fw)

#define FE_STALL		(DC_STALL_BW)				///	(dc_sig.pctl.stall | ex_sig.pctl.stall | dm_sig.pctl.stall)
#define DC_STALL		(EX_STALL_BW)				///	(ex_sig.pctl.stall | dm_sig.pctl.stall)
#define EX_STALL		(DC_STALL_FW | DM_STALL_BW)	///	(dc_sig.pctl.stall_fw | dm_sig.pctl.stall)
#define DM_STALL		(EX_STALL_FW)				///	(ex_sig.pctl.stall_fw | dc_sig.pctl.stall_fw)
#define WB_STALL		(DM_STALL_FW)				///	(dm_sig.pctl.stall_fw | ex_sig.pctl.stall_fw | dc_sig.pctl.stall_fw)
#else	///	4-stage...
    ///	backward stall signals
#define DM_STALL_BW		0
#define EX_STALL_BW		(PST(ex) | DM_STALL_BW)
#define DC_STALL_BW		(PST(dc) | EX_STALL_BW)
    ///	forward stall signals
#define DC_STALL_FW		(PST_FW(dc))
#define EX_STALL_FW		(DC_STALL_FW | PST_FW(ex))

#define FE_STALL		(DC_STALL_BW)
#define DC_STALL		(EX_STALL_BW)
#define EX_STALL		(DC_STALL_FW | DM_STALL_BW)
#define WB_STALL		(EX_STALL_FW)
#endif

#define INIT_PIPE_CTRL(pctl, stall_val)		pctl.stall = 0; pctl.stalled_flag = stall_val
#define POST_PIPE_CTRL(pctl, stall_bw)		pctl.stall_fw = pctl.stall & !stall_bw
#define SET_DST_FB(PIPE, PIPE_REF, DST_MASK, OUT)												\
	PIPE##_stt.dst.dst_f	= (PIPE##_stt.pctl.ready && PIPE_REF##_reg.dst_f && (DST_MASK));	\
	PIPE##_stt.dst.dst_id	= PIPE_REF##_reg.dst_id;											\
	PIPE##_stt.dst.ldm_f	= (PIPE##_stt.pctl.ready && PIPE_REF##_reg.ldm_f);					\
	PIPE##_stt.dst.out	= OUT


#define INV7(val)		(0x7f & ~(val))

#define GET_BITS(data, msbPos, bitLength)	(((data) >> ((msbPos) - (bitLength) + 1)) & ((1 << (bitLength)) - 1))
#define GET_FIELD(fid)						(GET_BITS(ir, FLD[fid].msbPos, FLD[fid].bitLength) + FLD[fid].codeOffset)
#define DEC_FLD(fid)						insn.fid = GET_FIELD(F_##fid)
#define DEC_FLD_ADD(fid)					insn.fid = GET_FIELD(F_##fid##a)
    /// RISC-V
#define RV_GET_FIELD(fid)					(GET_BITS(ir, RVFLD[fid].msbPos, RVFLD[fid].bitLength) + RVFLD[fid].codeOffset)
#define RV_DEC_FLD(fid)						insn.fid = RV_GET_FIELD(RVF_##fid)
#define SET_RV_INSN_FLAG(fs1, fs2, fd)		insn.rs1 = fs1; insn.rs2 = fs2; insn.rd_f = fd

#define SIGN_EXT(data, len0, len1)			(((FC_SINT32)((data) << ((len1) - (len0)))) >> ((len1) - (len0)))
#define GET_IMM(fid)						(SIGN_EXT(insn.fid, FLD[F_##fid].bitLength, 32))
#define SET_IMM(fid)						DEC_FLD(fid); insn.imm = GET_IMM(fid)

#define SET_INSN(n, m)						insn.n = insn.m

#define SET_INSN_FLAG(fs0, fs1, fd)			insn.src_f[0] = fs0; insn.src_f[1] = fs1; insn.dst_f = fd

#define RV_BYPASS_DC(PIPE, ID)		(PIPE##_stt.dst.id == ID) ? PIPE##_stt.dst.out
#define RV_LDM_PENDING(PIPE, ID)	(PIPE##_stt.dst.id == ID && PIPE##_stt.dst.ldm_f)

#define BYPASS_DC(PIPE, ID)		(PIPE##_sig.dst.valid && PIPE##_sig.dst.id == ID) ? PIPE##_sig.dst.out
#define LDM_PENDING(PIPE, ID)	(PIPE##_sig.dst.valid && PIPE##_sig.dst.id == ID && PIPE##_sig.dst.ldm_f)
#define BYPASS_EX(PIPE, IDX)		\
(PIPE##_sig.dst.valid && dc_reg.src_f[IDX] &&	PIPE##_sig.dst.id == dc_reg.src_id[IDX]) ? PIPE##_sig.dst.out




#endif  // !defined(PROC_ARCH_BASE_H)
