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


#if !defined(CACHE_H)
#define CACHE_H

#define DBG_CACHE_ACCESS_COUNT
#define DBG_CACHE_MSG

////////////// I/D cache parameters ////////////////

struct CacheComp {
    struct State {
        BIT		missed;
        BIT		update, update_word;
        UINT4	next_update_widx;
        UINT32	next_word;
        UINT4	next_way, way_idx;	///	up to 16-ways

        UINT32	mem_data;
    };
    struct Reg {
        BIT		init_flag;
        UINT32	tag;
        UINT32	lidx;
        UINT4	widx, update_widx;
        UINT32	word;
        UINT5	state, load_state;
        BIT		word_loaded;

#if defined(DBG_CACHE_ACCESS_COUNT)
        ///	for debugging...
        UINT32	cache_access, cache_miss;
#endif
        UINT4	way_idx, rand_way;	///	up to 16-ways
        UINT32	addr;	///	UINT16 --> UINT32
    };
    struct DState { /// for d-cache
        BIT	accessed, next_wflag , wflag;
        BIT update_tag_after_miss, update_tag_after_write_hit;
        UINT32	tagv;
    };
    struct DReg {   /// for d-cache
        UINT32	wb_daddr;
        UINT2	pending;
    };
    enum CacheInterfaceState {
        CIS_Idle, CIS_Pending, CIS_Active,
    };
    static void PrintCacheStat(UINT32 cache_access, UINT32 cache_miss, int c_size, int c_ways, int c_word_count, int c_line_count,
        const char *cname, int idx);
};

#define C_WAYS			(1 << C_WAY_BITS)
#define C_WORD_COUNT	(1 << C_WORD_BITS)
#define C_LINE_BITS		(C_SIZE_BITS - C_WAY_BITS - C_WORD_BITS)
#define C_LINE_COUNT	(1 << C_LINE_BITS)
#define C_WAY_MASK		(C_WAYS - 1)
#define C_SIZE			(1 << C_SIZE_BITS)

struct CPU;

template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS> struct Cache
{
    ///	cache size		: 2^SIZE_BITS	words
    ///	cache ways		: 2^WAY_BITS	ways
    ///	cache line size	: 2^WORD_BITS	words
    ///	cache line count: 2^(SIZE_BITS - WAY_BITS - WORD_BITS)	lines
    CacheComp::State	stt;
    CacheComp::Reg	    reg _TYPE(state);
    M_UINT32	word[C_WAYS][C_WORD_COUNT][C_LINE_COUNT];
    M_UINT32	tagv[C_WAYS][C_LINE_COUNT];
#if defined(TEST_ICACHE_AXI)
    RV_AXI4L axi_ic;
#endif
#if defined(DBG_CACHE_MSG)
    BIT dbg_cache_flag;
#endif

    void initialize_tag_only();
    void initialize_tag_and_word();
    UINT32 read_tag_line(UINT32 lidx, UINT4 widx, UINT32 tag);
    UINT32 fetch_cache(UINT32 addr);
    void update_cache();
    UINT3 get_io_op(bool load_cache_active);
    ///	10/14/17
#if defined(TEST_ICACHE_AXI)
    bool load_cache(AXI4L::CH *axi);
#else
    void load_cache(UINT32 *mem);
#endif
    void PrintStat(const char *cname, int idx);
};

template <int C_SIZE_BITS, int C_WAY_BITS, int C_WORD_BITS> struct DCache : Cache<C_SIZE_BITS, C_WAY_BITS, C_WORD_BITS>
{
    CacheComp::DState dstt;
    CacheComp::DReg	dreg _TYPE(state);
    void read_tag(UINT32 lidx, UINT4 widx, UINT32 tag);
    void fetch_tag(UINT32 addr, BIT wflag);
    void update_tag(int id);
#if defined(TEST_DCACHE_AXI)
    UINT3 get_io_op(bool rflag, bool wflag);
    void process_dcache(CPU *cpu, AXI4L::CH *axi);
#else
    void process_dcache(CPU *cpu);
#endif
};


#undef C_WAYS
#undef C_WORD_COUNT
#undef C_LINE_BITS
#undef C_LINE_COUNT
#undef C_WAY_MASK
#undef C_SIZE

#endif///#if !defined(CACHE_H)
