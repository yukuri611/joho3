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


//#include "proc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/typedef.h"
#include "cache.h"

void CacheComp::PrintCacheStat(UINT32 cache_access, UINT32 cache_miss, int C_SIZE, int C_WAYS, int C_WORD_COUNT, int C_LINE_COUNT,
	const char *cname, int idx) {
	printf("----------- %s-cache[%d] stats ------------\n", cname, idx);
	printf("size = (%d total bytes, %d ways, %d bytes/line, %d lines)\n",
		C_SIZE * 4, C_WAYS, C_WORD_COUNT * 4, C_LINE_COUNT);

	int cache_hit = cache_access - cache_miss;
	printf("hit-rate = %f(%%)\n", (double)cache_hit / cache_access * 100);
	printf("# hits = %d, # misses = %d, # cache_access = %d\n\n", cache_hit, cache_miss, cache_access);
}

#if 0
printf("icache size = (%d total bytes, %d ways, %d bytes/line, %d lines)\n",
	ICACHE_SIZE * 4, ICACHE_WAYS, ICACHE_WORD_COUNT * 4, ICACHE_LINE_COUNT);
////printf("icache hitrate=%f(%%)\n",(double)icache_hit/(icache_hit+icache_miss)*100);
////printf("icache hit = %d, icache miss = %d\n",icache_hit,icache_miss);
{
	int icache_hit = icache_access - icache_miss;
	int dcache_hit = dcache_access - dcache_miss;
	printf("icache hitrate=%f(%%)\n", (double)icache_hit / icache_access * 100);
	printf("icache hit = %d, icache miss = %d\n", icache_hit, icache_miss);
	printf("div_cycle = %d\n", div_cycle);
	printf("hazard_count = %d\n", hazard_count);
	printf("dcache size = (%d total bytes, %d ways, %d bytes/line, %d lines)\n",
		DCACHE_SIZE * 4, DCACHE_WAYS, DCACHE_WORD_COUNT * 4, DCACHE_LINE_COUNT);
	printf("dcache hit = %d, dcache miss = %d\n", dcache_hit, dcache_miss);
	printf("dcache hitrate=%f(%%)\n", (double)dcache_hit / (dcache_access) * 100);
}
#endif

#if 0
void CacheInterface::fsm(RV_AXI4L *axim, AXI4L::CH *axi, CacheReg *creg) {
	axim->addr_out = addr;
	axim->fsm(0, io_op, 0, axi);
#if 1	///	10/6/17
	if (state == CIS_Pending) {
		if (axim->data_latched) {
			creg->word = axim->data_in;
		}
	}
	loaded = (state == CIS_Pending && axim->data_latched);
#else
	dout = axim->data_in;
	dout_valid = axim->data_latched;
	
#endif
}
#endif

