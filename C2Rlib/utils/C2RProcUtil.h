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

#if !defined(C2R_PROC_UTIL_H)
#define C2R_PROC_UTIL_H

#include "ELFUtil.h"
#include <vector>
#include <map>

#define DBG_WB_BRANCH
#define DBG_PROFILER
//#define DISABLE_PROFILE
//#define ENABLE_PROFILE

#define RECORD_PROFILER_TRACE

namespace C2R {
    struct ProcUtil {
        static const char* elfFileName;
        static int check_code(const char* code, unsigned int* val);
        enum ProfileCallType {
            CT_call, CT_entry, CT_trap, CT_ret, CT_sret, CT_cxt, CT_pop, CT_err,
        };
        enum SpecialSymbolType {
            SS_do_exit,
            SS__start,
            SS_count,
        };
        struct ProfileInfo;
        struct ProfileInfoInstance {
            long cycles, subCycles;
            ProfileInfo * pi;
        };
        struct ProfileInfoContext {
            int sp;
            long cycles;
            int specialSymbolFlag[SS_count];
            std::vector<ProfileInfoInstance> stack;
            void ClearSpecialSymbolFlag() { for (int i = 0; i < SS_count; ++i) { specialSymbolFlag[i] = -1; } }
            void Reset() { sp = 0; cycles = 0; stack.clear(); ClearSpecialSymbolFlag(); }
            ProfileInfoContext() { Reset(); }
            long GetPendingCycles();
        };
        struct ProfileData {
            long cycles, subCycles, calls;
            ProfileData() { Reset(); }
            void Reset() { cycles = 0; subCycles = 0; calls = 0; }
            void AddCycles(ProfileInfoInstance* pii) {
                cycles += pii->cycles;
                subCycles += pii->subCycles;
            }
            void MaxCycles(ProfileData& prof) {
                if (cycles < prof.cycles) {
                    cycles = prof.cycles;
                }
                if (subCycles < prof.subCycles) {
                    subCycles = prof.subCycles;
                }
                if (calls < prof.calls) {
                    calls = prof.calls;
                }
            }
            void GetFormatLengths() {
                cycles = GetFormatLength(cycles);
                subCycles = GetFormatLength(subCycles);
                calls = GetFormatLength(calls);
            }
            static long GetFormatLength(long d);
        };
        struct CallContext {
            int sp;
            unsigned int callerAddr, addrRegID, mode;
            ProfileCallType callType;
            long timeStamp;
            ProfileData prof;
            CallContext() : sp(0), callerAddr(0), addrRegID(0), mode(0), callType(CT_call), timeStamp(0) {}
            CallContext(unsigned int ca, unsigned int ar, unsigned int md, ProfileCallType ct, long tm) { Set(ca, ar, md, ct, tm); }
            void Set(unsigned int ca, unsigned int ar, unsigned int md, ProfileCallType ct, long tm) {
                sp = 0; callerAddr = ca; addrRegID = ar; mode = md; callType = ct; timeStamp = tm;
            }
        };
        struct ProfileInfo {
            unsigned int entryAddr;
            int bindValue;
            std::map<unsigned int/*threadID*/, std::vector<CallContext>> callInfoContext;
            ProfileData prof;
            SpecialSymbolType sstype;
            int ssidx;
            std::string name, sectName;
            ProfileInfo() { entryAddr = 0; bindValue = -1; sstype = SS_count, ssidx = 0; }
            static size_t maxProfileFuncNameLength;
            static bool CompareProfile(ProfileInfo *a, ProfileInfo *b);
            void PrintProfileInfo(FILE * fp, double totalCycles, int maxNameLength, ProfileData& pd);
            int GetCallInfoContextIdx(unsigned int threadID, int sp);
        };
        struct SpecialSymbolInfo {
            SpecialSymbolType sstype;
            const char* ssname;
            SpecialSymbolInfo(SpecialSymbolType st, const char* sn) : sstype(st), ssname(sn) {}
        };
        struct ProfilerPopState {
            unsigned int retAddr, cur_pc, retAddrRegID, threadID, mode;
            ProfilerPopState(unsigned int ra, unsigned int pc, unsigned int raID, unsigned int tID, unsigned int md) {
                retAddr = ra; cur_pc = pc; retAddrRegID = raID; threadID = tID; mode = md;
            }
        };
#if defined(RECORD_PROFILER_TRACE)  /// 3/28/23
        struct ProfilerTrace {
            unsigned char com, regID, mode;
            unsigned tgt_pc, cur_pc, tid;
            ProfilerTrace(unsigned c, unsigned r, unsigned m, unsigned tpc, unsigned cpc, unsigned tid0) :
                com(c), regID(r), mode(m), tgt_pc(tpc), cur_pc(cpc), tid(tid0) {}
            static bool match(ProfilerTrace &p0, ProfilerTrace &p1);
        };
#endif
        struct Profiler {
            std::map<unsigned/*addr*/, ProfileInfo*> profileInfoMap;
            std::map<SpecialSymbolType, std::vector<ProfileInfo*>> specialSymbolMap;
            unsigned curThreadPointer, entryAddr;
            int unknownSymCount, systemSymCount, dbgPop;
            std::map<unsigned/*threadID*/, ProfileInfoContext> profileInfoContextMap;
            ProfileInfoInstance * curProfInfo;
            long skippedCycles;
            int id;
            int totalErrorCount;
            long *extCycles;
            unsigned gprCount;
#if defined(PROC_64BIT)
            unsigned long long *gpr;
#else
            unsigned *gpr;
#endif
#if defined(RECORD_PROFILER_TRACE)  /// 3/28/23
            std::vector<ProfilerTrace> pTrace, pTraceRTL;
#endif
            Profiler(int idx);
            ~Profiler() {
                for (auto i = profileInfoMap.begin(), e = profileInfoMap.end(); i != e; ++i) {
                    C2R_ASSERT(i->second); delete i->second;
                }
                if (fp_prof) {
                    fclose(fp_prof);
                    fp_prof = NULL;
                }
            }
            void Reset();
            void SetupProfiler(const char * prog_file_name);
            void PrepareSetupElfProfiler();
            void SetupElfProfiler(ELF_Parser::Info &elfInfo);
            void InsertElfSymbols(ELF_Parser::Info &elfInfo);
            void UpdateSpecialSymbol(ProfileInfo* pi);
            void UpdateSpecialSymbol(SpecialSymbolType st, ProfileInfo* pi);
            void FinalizeProfileInfoMap(bool showSymbols);
            void ProfilerPush(unsigned int tgtAddr, unsigned int cur_pc = (unsigned)(-1),
                unsigned int tgtAddrRegID = (unsigned)(-1), unsigned int threadID = 0,
                unsigned int mode = 0, ProfileCallType callType = CT_call);
            void ProfilerPop();
            void ProfilerPop(unsigned int retAddr, unsigned int cur_pc, unsigned int retAddrRegID, unsigned int threadID, unsigned int mode);
            void ProfilerUpdate();
            void ProfilerSkip();
            void ProfilerStart();
            void InitializeProfileInfoInstance();
            void ProfilerPrint();
#if defined(RECORD_PROFILER_TRACE)  /// 3/28/23
            void ProfilerTracePrint();
            void ProfilerTraceRTLPrint();
            void ProfilerAddTraceRTL(unsigned com, unsigned regID, unsigned mode, unsigned tpc, unsigned cpc, unsigned tid);
#endif
            void PrintProfileCycles(FILE* fp, double totalCycles, double activeCycles, double skippedCycles1);
            void PrintContextStacks(FILE* fp);
            void PushProfileInfoInstance(ProfileInfo * pi);
            void PopProfileInfoInstance();
            void DBG_msg_indent(FILE *fp);
            void DBG_CALL_msg(const char* msg, bool showStack = false);
            void DBG_JMP_msg(ProfilerPopState &pps, const char* msg, bool isJump);
            void PopMsgError(FILE* fp, ProfilerPopState &pps, const char* msg);
            void PopMsgError(ProfilerPopState &pps, const char* msg) {
                PopMsgError(stdout, pps, msg);
                PopMsgError(fp_prof, pps, msg);
                if (fp_prof) {
                    fclose(fp_prof);
                }
                C2R_ASSERT(0 && "PopMsgError");
            }
#define SKIP_STDOUT_MSG
            void PopMsg(FILE* fp, ProfilerPopState &pps, const char *msg, int spCount);
            void PopMsg(ProfilerPopState &pps, const char *msg, int spCount) {
#if !defined(SKIP_STDOUT_MSG)
                PopMsg(stdout, pps, msg, spCount);
#endif
                PopMsg(fp_prof, pps, msg, spCount);
            }
            void PrintProfilerInfoStack(FILE* fp, unsigned tid, const char* msg, int spCount);
            void ContextSwitchMsg(FILE* fp, ProfilerPopState &pps, int sp);
            void ContextSwitchMsg(ProfilerPopState &pps, int sp) {
#if !defined(SKIP_STDOUT_MSG)
                ContextSwitchMsg(stdout, pps, sp);
#endif
                ContextSwitchMsg(fp_prof, pps, sp);
            }
            bool ContextSwitch(ProfilerPopState &pps);
            bool PopPatternReturnFromException(ProfilerPopState &pps);
            bool PopPatternReturnFromKernelThread(ProfilerPopState &pps);
            bool PopPatternReturnFromSystemTask(ProfilerPopState &pps);
            bool PopPatternDoExit(ProfilerPopState &pps);
            bool PopPatternStartReentry(ProfileInfo* pi);
            bool PopPatternRewindStack(ProfilerPopState &pps);
        };
        struct ProfilerGroup {
            std::vector<Profiler> prof;
            Profiler *curProf;
            ProfilerGroup() : curProf(NULL) {}
            void ProfilerPush(unsigned int targetAddr) { if (curProf) { curProf->ProfilerPush(targetAddr); } }
            void ProfilerPop() { if (curProf) { curProf->ProfilerPop(); } }
            void ProfilerUpdate() { if (curProf) { curProf->ProfilerUpdate(); } }
            void ProfilerSkip() { if (curProf) { curProf->ProfilerSkip(); } }
            void ProfilerPrint() { if (curProf) { curProf->ProfilerPrint(); } }
            void ProfilerPrint(int idx) {
                if (idx < (int)prof.size()) { prof[idx].ProfilerPrint(); }
            }
            void SetupProfiler(const char * prog_file_name) {
                prof.emplace_back((int)prof.size());
                curProf = &prof.back();
                curProf->SetupProfiler(prog_file_name);
                for (size_t i = 0, e = prof.size(); i < e; ++i) { prof[i].InitializeProfileInfoInstance(); }
            }
            void SetupElfProfiler(ELF_Parser::Info &elfInfo) {
                prof.emplace_back((int)prof.size());
                curProf = &prof.back();
                curProf->SetupElfProfiler(elfInfo);
                for (size_t i = 0, e = prof.size(); i < e; ++i) { prof[i].InitializeProfileInfoInstance(); }
            }
            void SetProfiler(int idx) {
                if (idx < (int)prof.size()) { curProf = &prof[idx]; }
                else { curProf = NULL; }
            }
        };
        static Profiler *prof;
        static FILE *fp_prof;
    };
    struct ElfSymExtractor {
        enum {
#if defined(PROC_64BIT)
            ProcWordBytePosBW = 3,
#else
            ProcWordBytePosBW = 2,
#endif
            ProcWordByteMask = (1 << ProcWordBytePosBW) - 1,
        };
        struct Symbol {
            std::string symName;
            U64 size, memSize, memAddr, memAddrAligned, byteOffset;
            std::vector<U64> cacheHitFlag, value;
            Symbol() : symName(""), size(0), memSize(0), memAddr(0), memAddrAligned(0) {}
            void CreateStorage(U64 addr, U64 sz) {
                memAddr = addr;
                size = sz;
                byteOffset = memAddr & ProcWordByteMask; /// 0x7
                memSize = size + byteOffset;
                memAddrAligned = memAddr & (~ProcWordByteMask);
                auto wordCount = memSize >> ProcWordBytePosBW;
                auto fsz = wordCount / 64 + ((wordCount % 64) != 0);
                if (!fsz) { fsz = 1; }
                cacheHitFlag.resize(fsz);
                auto valueWordCount = (memSize / 8) + ((memSize % 8) != 0);
                value.resize(valueWordCount);
                ClearCacheHitFlag();
            }
            void* GetValueAddr() { return &value[0]; }
            template<typename T> T &GetRawValue(int idx = 0) {
                char *byteAddr = ((char *)GetValueAddr());
                return *(((T*)byteAddr) + idx);
            }
            template<typename T> T &GetReadValue(int idx = 0) {
                char *byteAddr = ((char *)GetValueAddr()) + byteOffset;
                return *(((T*)byteAddr) + idx);
            }
            template<typename T> size_t GetSize() { return value.size() * sizeof(U64) / sizeof(T); }
            void ClearCacheHitFlag() {
                for (int i = 0; i < (int)cacheHitFlag.size(); ++i) {
                    cacheHitFlag[i] = 0ULL;
                }
            }
            bool IsCached(U64 addr) {
                auto widx = addr >> ProcWordBytePosBW;
                auto mwidx = widx / 64;
                auto mbpos = widx % 64;
                C2R_ASSERT(mwidx < cacheHitFlag.size());
                return ((cacheHitFlag[mwidx] >> mbpos) & 1ULL) != 0ULL;
            }
            void SetCacheHit(U64 addr) {
                auto widx = addr >> ProcWordBytePosBW;
                auto mwidx = widx / 64;
                auto mbpos = widx % 64;
                C2R_ASSERT(mwidx < cacheHitFlag.size());
                cacheHitFlag[mwidx] |= (1ULL << mbpos);
            }
            void printInfo() {
                printf("symName(%-10s) : memAddr(%08llx), size(%3llx)\n", symName.c_str(), memAddr, size);
            }
        };
        struct List {
            std::map<std::string, Symbol> smap;
            void insertSymbol(const char* symName) {
                auto si = smap.find(symName);
                if (si == smap.end()) {
                    smap[symName].symName = symName;
                }
            }
            void analyze(ELF_Parser::Info &elfInfo) {
                for (auto i = smap.begin(), e = smap.end(); i != e; ++i) {
                    auto &si = i->second;
                    if (auto* sym = elfInfo.GetSymbolAddress(si.symName.c_str())) {
                        si.CreateStorage(sym->st_value, sym->st_size);
                    }
                }
            }
        };
    };
};

#define USE_ELF_FILE

#if !defined(__LLVM_C2RTL__) && defined(ENABLE_PROFILE)
#define PROFILER_CALL(pc)	                            C2R::ProcUtil::prof->ProfilerPush(pc);
#define PROFILER_CALL_5(tgt_pc, cur_pc, tgt_reg_id, tid, mode)     \
    C2R::ProcUtil::prof->ProfilerPush((unsigned)tgt_pc, (unsigned)cur_pc, tgt_reg_id, (unsigned)tid, mode);
#define PROFILER_CALL_COND(pc, cond)	                do { if (cond) C2R::ProcUtil::prof->ProfilerPush((unsigned)pc); } while(0)
#define PROFILER_RETURN		                            C2R::ProcUtil::prof->ProfilerPop();
#define PROFILER_RETURN_5(ret_pc, cur_pc, ret_reg_id, tid, mode)   \
    C2R::ProcUtil::prof->ProfilerPop((unsigned)ret_pc, (unsigned)cur_pc, ret_reg_id, (unsigned)tid, mode);
#define PROFILER_UPDATE		                            C2R::ProcUtil::prof->ProfilerUpdate();
#define PROFILER_SKIP		                            C2R::ProcUtil::prof->ProfilerSkip(); // for sleep support through wait instruction
#define PROFILER_PRINT		                            C2R::ProcUtil::prof->ProfilerPrint();
#else
#define PROFILER_CALL(pc)
#define PROFILER_CALL_5(tgt_pc, cur_pc, tgt_reg_id, tid, mode)
#define PROFILER_CALL_COND(pc, cond)
#define PROFILER_RETURN
#define PROFILER_RETURN_5(ret_pc, cur_pc, ret_reg_id, tid, mode)
#define PROFILER_UPDATE
#define PROFILER_START
#define PROFILER_PRINT
#endif
#if defined(RECORD_PROFILER_TRACE) && !defined(__LLVM_C2RTL__) && defined(ENABLE_PROFILE)  /// 3/28/23
#define PROFILER_TRACE_PRINT		                    C2R::ProcUtil::prof->ProfilerTracePrint();
#define PROFILER_TRACE_PRINT_RTL		                C2R::ProcUtil::prof->ProfilerTraceRTLPrint();
#define PROFILER_ADD_TRACE_RTL(com, regID, mode, tgt_pc, cur_pc, tid)     \
    C2R::ProcUtil::prof->ProfilerAddTraceRTL(com, regID, mode, tgt_pc, cur_pc, tid)
#else
#define PROFILER_TRACE_PRINT
#define PROFILER_TRACE_PRINT_RTL
#define PROFILER_ADD_TRACE_RTL(com, regID, mode, tgt_pc, cur_pc, tid)   
#endif


#endif //#if !defined(C2R_PROC_UTIL_H)
