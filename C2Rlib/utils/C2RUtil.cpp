
#include "ELFUtil.h"
#include "C2RUtil.h"
#include "C2RProcUtil.h"

char tmp_print_format[1000];

bool RVDisableProfilerFlag = 0;
bool RVDisableMsg = 0;
bool RVDisableTimerMsg = 0;

static char * print_format_double_string(double val)
{
    char s[1000];
    int digits, i, ii = 0;
    if (val > TOO_BIG_DOUBLE) { sprintf(tmp_print_format, "%.3f (too big to format...)", val); return tmp_print_format; }
    sprintf(s, "%.3f", val);
    digits = (int)strlen(s) - 4;
    for (i = 0; i < digits; i++) {
        tmp_print_format[ii++] = s[i];
        if ((digits - i) % 3 == 1 && i < digits - 1) { tmp_print_format[ii++] = ','; }
    }
    for (i = digits; i < digits + 4; i++) {
        tmp_print_format[ii++] = s[i];
    }
    tmp_print_format[ii] = 0;
    return tmp_print_format;
}

static char * print_format_int_string(long val)
{
    char s[20];
    int digits, i, ii = 0;
    sprintf(s, "%ld", val);
    digits = (int)strlen(s);
    for (i = 0; i < digits; i++) {
        tmp_print_format[ii++] = s[i];
        if ((digits - i) % 3 == 1 && i < digits - 1) { tmp_print_format[ii++] = ','; }
    }
    tmp_print_format[ii] = 0;
    return tmp_print_format;
}


std::stringstream & operator << (std::stringstream &ss, const C2R::SF &sf) { return sf.operator<<(ss); }

static C2R::ProcUtil::Profiler __prof(0);
C2R::ProcUtil::Profiler *C2R::ProcUtil::prof = &__prof;
FILE *C2R::ProcUtil::fp_prof = NULL;

C2R::ProcUtil::Profiler::Profiler(int idx) : /*profileCount(0), sp(0), extraCalls(0), skippedCycles(0),*/ id(idx) {
    if (!fp_prof) {
        fp_prof = fopen("profile.log", "w");
        C2R_ASSERT(fp_prof);
    }
    Reset();
}

void C2R::ProcUtil::Profiler::Reset() {
    //profileCount = 0;
    // sp = 0;
    //extraCalls = 0;
    totalErrorCount = 0;
    entryAddr = 0;
    unknownSymCount = 0;
    systemSymCount = 0;
    dbgPop = 0;
    skippedCycles = 0;
    extCycles = nullptr;
    gpr = nullptr; gprCount = 0;
    profileInfoMap.clear();
    specialSymbolMap.clear();
#if 1   /// 10/26/21
    curThreadPointer = 0;
    profileInfoContextMap.clear();
    profileInfoContextMap[curThreadPointer].Reset();
#elif 1   /// 10/26/21
    curThreadPointer = 0;
    ctxMap[curThreadPointer] = 0;   /// sp = 0;
    profileInfoContextStack.clear();
    profileInfoContextStack[curThreadPointer].clear();
#else
    profileInfoStack.clear();
#endif
}

#include <algorithm>

void C2R::ProcUtil::Profiler::InsertElfSymbols(ELF_Parser::Info &elfInfo)
{
    if (!entryAddr) {
        entryAddr = (unsigned int) elfInfo.elfHeader.ehdr.e_entry;
    }
    ProfileInfo * pi;
    auto &symTable = elfInfo.symbolTable;
    auto &sectionTable = elfInfo.sectionTable;
    auto *table = elfInfo.symbolTable.symTable;
    std::vector<ELF_Parser::Elf_Sym*> symbolList;
    for (size_t i = 0, e = symTable.symbolCount; i < e; ++i) {
        auto &sym = table[i];
        auto *shndxVal = ELF_Parser::shnumGroup.GetValue(sym.st_shndx);
        if (shndxVal || sym.st_name_str[0] == 0) { continue; }
        auto *shdr = &sectionTable.shdr[sym.st_shndx];
        if (shdr->field.executable) {
            symbolList.push_back(&sym);
        }
    }
    ELF_Parser::Elf_Sym *sym;
    sym = symbolList[0];
    if (profileInfoMap.empty() && sym->st_value != 0) {
        C2R_ASSERT(profileInfoMap[0] == nullptr);
        profileInfoMap[0] = new ProfileInfo;
        pi = profileInfoMap[0];
        /// 7/1/20
        pi->name = "_entry";
        pi->sectName = "_boot";
        pi->entryAddr = 0;
        pi->bindValue = sym->st_bind;
    }
    for (size_t i = 0, e = symbolList.size(); i < e; ++i) {
        sym = symbolList[i];
        auto *shdr = &sectionTable.shdr[sym->st_shndx];
        if (profileInfoMap[(unsigned)sym->st_value] == nullptr) {
            profileInfoMap[(unsigned)sym->st_value] = new ProfileInfo;
        }
        pi = profileInfoMap[(unsigned)sym->st_value];
        if (pi->bindValue < 0 || pi->bindValue > sym->st_bind ||
            (pi->name.size() > 1 && pi->name[0] == '$')) {
            pi->entryAddr = (unsigned)sym->st_value;
            pi->name = sym->st_name_str;
            pi->sectName = shdr->sh_name_str;
            pi->bindValue = sym->st_bind;
        }
        C2R_ASSERT(pi->entryAddr == (unsigned)sym->st_value);
        UpdateSpecialSymbol(pi);
    }
}

void C2R::ProcUtil::Profiler::UpdateSpecialSymbol(SpecialSymbolType st, ProfileInfo* pi) {
    auto& ssl = specialSymbolMap[st];
    pi->sstype = st;
    pi->ssidx = (int)ssl.size();
    ssl.push_back(pi);
}

void C2R::ProcUtil::Profiler::UpdateSpecialSymbol(ProfileInfo* pi) {
#define CASE_SPECIAL_SYM(sn)    case SS_##sn: if (pi->name == #sn) { UpdateSpecialSymbol(SS_##sn, pi); } break
    for (int i = 0; i < SS_count; ++i) {
        switch (i) {
            CASE_SPECIAL_SYM(do_exit);
            CASE_SPECIAL_SYM(_start);
        };
    }
#undef CASE_SPECIAL_SYM
}

void C2R::ProcUtil::Profiler::FinalizeProfileInfoMap(bool showSymbols) {
    C2R_ASSERT(curThreadPointer == 0);
    auto & profileInfoStack = profileInfoContextMap[curThreadPointer];
    profileInfoStack.stack.resize(profileInfoMap.size() + 1);
    ProfileInfo::maxProfileFuncNameLength = 0;
    for (auto i = profileInfoMap.begin(), e = profileInfoMap.end(); i != e; ++i) {
        auto* pi = i->second;
        size_t len = pi->name.size();
        if (ProfileInfo::maxProfileFuncNameLength < len) {
            ProfileInfo::maxProfileFuncNameLength = len;
        }
    }
    if (showSymbols) {
        for (auto i = profileInfoMap.begin(), e = profileInfoMap.end(); i != e; ++i) {
            auto* pi = i->second;
            printf("ElfProfile (%*s): addr = %08x : %s\n", (int)ProfileInfo::maxProfileFuncNameLength,
                pi->name.c_str(), pi->entryAddr, pi->sectName.c_str());
        }
    }
}

void C2R::ProcUtil::Profiler::PrepareSetupElfProfiler() {
    Reset();
    profileInfoMap.clear();
}

void C2R::ProcUtil::Profiler::SetupElfProfiler(ELF_Parser::Info &elfInfo)
{
    PrepareSetupElfProfiler();
    InsertElfSymbols(elfInfo);
    FinalizeProfileInfoMap(true);
    ProfilerStart();
}

size_t C2R::ProcUtil::ProfileInfo::maxProfileFuncNameLength = 0;
#define MAX_LINE 1000

int C2R::ProcUtil::check_code(const char * code, unsigned int * val)
{
    int v = 0;
    if (strlen(code) != 8) { return 0; }
    while (*code) {
        int d = *code - '0';
        int h = *code - 'a';
        v <<= 4;
        if (d >= 0 && d <= 9) { v += d; }
        else if (h >= 0 && h <= 6) { v += h + 10; }
        else { return 0; }
        code++;
    }
    *val = v;
    return 1;
}

bool C2R::ProcUtil::Profiler::PopPatternStartReentry(ProfileInfo* pi) {
    C2R_ASSERT(pi->sstype == SS__start && pi->ssidx >= 0);
    auto & pic = profileInfoContextMap[curThreadPointer];
    auto & profileInfoStack = pic.stack;
    auto& cic = pi->callInfoContext[curThreadPointer];
    C2R_ASSERT(!cic.empty());
    auto& ci = cic.back();
    C2R_ASSERT(pic.specialSymbolFlag[pi->sstype] == pi->ssidx);
    for (int i = pic.sp - 1; i > 0; --i) {
        auto& pii = pic.stack[i];
        if (pii.pi == pi) {
            ProfilerPopState pps(pi->entryAddr, ci.callerAddr, ci.addrRegID, curThreadPointer, ci.mode);
            PopMsg(pps, "(REWIND PROFILER STACK!!! <Start Reentry>)", pic.sp - i);
            while (pic.sp > i) {
                PopProfileInfoInstance();
            }
            C2R_ASSERT(pic.sp == i);
            return true;
        }
    }
    C2R_ASSERT(0 && "Error in PopPatternStartReentry...\n");
    return true;
}

void C2R::ProcUtil::Profiler::PushProfileInfoInstance(ProfileInfo * pi)
{
    auto & pic = profileInfoContextMap[curThreadPointer];
    auto & profileInfoStack = pic.stack;
    auto & cic = pi->callInfoContext[curThreadPointer];
    C2R_ASSERT(!cic.empty());
    auto & ci = cic.back();
    ci.sp = pic.sp;
    if (pic.sp == 0) {
        ci.callType = CT_entry;
    }
    if (pic.sp >= (int)profileInfoStack.size()) {
        size_t sz = profileInfoStack.size();
        C2R_ASSERT(sz);
        profileInfoStack.resize(sz + sz / 2);
        printf("profileInfoStack.size : %d --> %d\n", (int)sz, (int)(sz + sz / 2));
    }
    if (pi->sstype < SS_count) {
        if (0 && pi->sstype == SS__start && pic.specialSymbolFlag[pi->sstype] >= 0) {
            PopPatternStartReentry(pi);
        }
        pic.specialSymbolFlag[pi->sstype] = pi->ssidx;
    }
    curProfInfo = &profileInfoStack[pic.sp++];
    curProfInfo->pi = pi;
    curProfInfo->cycles = 0;
    curProfInfo->subCycles = 0;
    if (pic.sp == 1) {
        ci.prof.calls++;
    }
}

void C2R::ProcUtil::Profiler::PopProfileInfoInstance()
{
    auto & pic = profileInfoContextMap[curThreadPointer];
    auto & profileInfoStack = pic.stack;
    C2R_ASSERT(pic.sp > 0);
    ProfileInfoInstance * pii = &profileInfoStack[--pic.sp];
    pii->pi->prof.AddCycles(pii);
    pic.cycles += pii->cycles;
    auto& cic = pii->pi->callInfoContext[curThreadPointer];
    C2R_ASSERT(!cic.empty());
    cic.pop_back();
    if (pic.sp > 0) {
        curProfInfo = &profileInfoStack[pic.sp - 1];
        curProfInfo->subCycles += pii->cycles + pii->subCycles;
    }
    else if (curThreadPointer == 0) {
        curProfInfo = 0;
        pii->pi->prof.cycles--;
    }
}

void C2R::ProcUtil::Profiler::DBG_msg_indent(FILE *fp) {
    auto & sp = profileInfoContextMap[curThreadPointer].sp;
    for (int i = 0; i < (int)sp; ++i) {
        fprintf(fp, ":");
    }
}

#define MAX_CALL_DBG_COND           1000
#define MAX_CALL_DEPTH_DBG_COND     3//4
//#define DBG_THREAD                  (curThreadPointer == 0xfec5c000)
//#define DBG_THREAD                  (curThreadPointer == 0x002204a0)  /// (curThreadPointer == 0xfec5c000)    /// 1   ///(curThreadPointer == 0x00000000)
//#define DBG_THREAD                  (curThreadPointer == 0xfec90fc0)  /// (curThreadPointer == 0xfec5c000)    /// 1   ///(curThreadPointer == 0x00000000)
//#define DBG_THREAD                  ((curThreadPointer == 0x002204a0) || (curThreadPointer == 0xfec5c000))
#define DBG_THREAD                  (curThreadPointer == 0x002164a0)
//#define DBG_THREAD                  1//(curThreadPointer == 0x002164a0)
//fec90fc0
//002164a0
#define DBG_START_CYCLE             95622336    /// 98199093    /// 98198321    /// 95175651    /// 88340256    /// 119459014   /// 118732971   /// 118732959   /// 118504310   /// 118493135   /// 118504358
#define DBG_END_CYCLE               95890199 

#if 1
#define DBG_PUSH_SKIP_COND          (0)
#define DBG_POP_SKIP_COND           (DBG_PUSH_SKIP_COND)
#elif 1
#define DBG_PUSH_SKIP_COND          (!extCycles || *extCycles < DBG_START_CYCLE || *extCycles > DBG_END_CYCLE || !DBG_THREAD)
#define DBG_POP_SKIP_COND           (DBG_PUSH_SKIP_COND)
#elif 1
#define DBG_PUSH_SKIP_COND          ((pii.pi->calls > MAX_CALL_DBG_COND) || !extCycles || *extCycles < DBG_START_CYCLE || !DBG_THREAD)
#define DBG_POP_SKIP_COND           (DBG_PUSH_SKIP_COND)
#elif 1
#define DBG_PUSH_SKIP_COND          ((pii.pi->calls > MAX_CALL_DBG_COND || pic.sp - 1 > MAX_CALL_DEPTH_DBG_COND))
#define DBG_POP_SKIP_COND           (DBG_PUSH_SKIP_COND)
#elif 1
//#define DBG_PUSH_SKIP_COND          ((!DBG_THREAD) && (ps.pi->calls > MAX_CALL_DBG_COND || pic.sp - 1 > MAX_CALL_DEPTH_DBG_COND))
#define DBG_PUSH_SKIP_COND          ((!DBG_THREAD))
#define DBG_POP_SKIP_COND           ((!DBG_THREAD) && !cxtSwitch)
#else
#define DBG_PUSH_SKIP_COND          ((!dbgPop && !DBG_THREAD) && (ps.pi->calls > MAX_CALL_DBG_COND || pic.sp - 1 > MAX_CALL_DEPTH_DBG_COND))
#define DBG_POP_SKIP_COND           (!((dbgPop || cxtSwitch) && DBG_THREAD))
#endif

void C2R::ProcUtil::Profiler::DBG_CALL_msg(const char* msg, bool showStack) {
    auto & pic = profileInfoContextMap[curThreadPointer];
    auto & profileInfoStack = pic.stack;
    C2R_ASSERT(pic.sp < (int)profileInfoStack.size());
    auto &pii = profileInfoStack[pic.sp - 1];
    if (DBG_PUSH_SKIP_COND) {
        return;
    }
    if (extCycles) {
        fprintf(fp_prof, "[%10ld] ", *extCycles);
    }
    DBG_msg_indent(fp_prof);
    if (showStack) {
        if (pii.pi) {
            auto& cic = pii.pi->callInfoContext[curThreadPointer];
            C2R_ASSERT(!cic.empty());
            auto& ci = cic.back();
            fprintf(fp_prof, " (%s:%2d: %08x) (%08x --> %08x) : %s.%s [ar(%2d), %ld calls]",
                msg, pic.sp - 1, curThreadPointer, 
                ci.callerAddr, pii.pi->entryAddr, pii.pi->sectName.c_str(), pii.pi->name.c_str(), ci.addrRegID, pii.pi->prof.calls);
        }
        else {
            fprintf(fp_prof, " (%s:%2d) ", msg, pic.sp - 1);
        }
    }
    if (gpr && gprCount) {
        fprintf(fp_prof, " (a0-%d:", gprCount - 1);
        for (int i = 0; i < (int)gprCount; ++i) {
#if defined(PROC_64BIT)
            fprintf(fp_prof, "%08llx%s", gpr[10 + i], (i == gprCount - 1) ? ")" : " ");
#else
            fprintf(fp_prof, "%08x%s", gpr[10 + i], (i == gprCount - 1) ? ")" : " ");
#endif
        }
    }
    fprintf(fp_prof, "\n");
}

void C2R::ProcUtil::Profiler::DBG_JMP_msg(ProfilerPopState &pps, const char* msg, bool isJump) {
    auto & pic = profileInfoContextMap[curThreadPointer];
    if (extCycles) {
        fprintf(fp_prof, "[%10ld] ", *extCycles);
    }
    std::string sectName = "*", name = "*";
    if (isJump) {
        auto pim = profileInfoMap.find(pps.retAddr);
        if (pim != profileInfoMap.end()) {
            sectName = pim->second->sectName;
            name = pim->second->name;
        }
    }
    else {
        C2R_ASSERT(pic.sp < (int)pic.stack.size());
        auto &pii = pic.stack[pic.sp - 1];
        C2R_ASSERT(pii.pi);
        sectName = pii.pi->sectName;
        name = pii.pi->name;
    }
    DBG_msg_indent(fp_prof);
    if (isJump) {
        fprintf(fp_prof, " (%s:%2d: %08x) (%08x --> %08x) : %s.%s [ar(%2d)]\n",
            msg, pic.sp - 1, curThreadPointer,
            pps.cur_pc, pps.retAddr, sectName.c_str(), name.c_str(), pps.retAddrRegID);
    }
    else {
#if 1   /// 12/6/21
        fprintf(fp_prof, " (%s:%2d: %08x) (%08x <-- %08x) : %s.%s [ar(%2d)]",
            msg, pic.sp - 1, curThreadPointer,
            pps.retAddr, pps.cur_pc, sectName.c_str(), name.c_str(), pps.retAddrRegID);
        if (gpr && gprCount) {
            //fprintf(fp_prof, " (a0-2:%08x %08x %08x)", gpr[10], gpr[11], gpr[12]);
            fprintf(fp_prof, " (a0-%d:", gprCount - 1);
            for (int i = 0; i < (int)gprCount; ++i) {
#if defined(PROC_64BIT)
                fprintf(fp_prof, "%08llx%s", gpr[10 + i], (i == gprCount - 1) ? ")" : " ");
#else
                fprintf(fp_prof, "%08x%s", gpr[10 + i], (i == gprCount - 1) ? ")" : " ");
#endif
            }
        }
        fprintf(fp_prof, "\n");
#else
        fprintf(fp_prof, " (%s:%2d: %08x) (%08x <-- %08x) : %s.%s [ar(%2d)]\n",
            msg, pic.sp - 1, curThreadPointer,
            pps.retAddr, pps.cur_pc, sectName.c_str(), name.c_str(), pps.retAddrRegID);
#endif
    }
}

void C2R::ProcUtil::Profiler::PopMsgError(FILE* fp, ProfilerPopState &pps, const char* msg) {
    fprintf(fp, "=============================================================\n");
    if (extCycles) {
        fprintf(fp, "[%10d] ", (unsigned int)*extCycles);
    }
    fprintf(fp, "ProfilerPop ERROR %s : tid(%08x), retAddr(%08x), curPC(%08x), retAddrRegID(%d)\n",
        msg, pps.threadID, pps.retAddr, pps.cur_pc, pps.retAddrRegID);
    PrintProfilerInfoStack(fp, curThreadPointer, "CURRENT THREAD", -1);
    for (auto i = profileInfoContextMap.begin(), e = profileInfoContextMap.end(); i != e; ++i) {
        if (i->first != curThreadPointer) {
            PrintProfilerInfoStack(fp, i->first, "PENDING THREAD", -1);
        }
    }
    fprintf(fp, "=============================================================\n");
}

int C2R::ProcUtil::ProfileInfo::GetCallInfoContextIdx(unsigned int threadID, int sp) {
    C2R_ASSERT(callInfoContext.find(threadID) != callInfoContext.end());
    auto& ci = callInfoContext[threadID];
    for (int i = (int)ci.size() - 1; i >= 0; --i) {
        if (ci[i].sp == sp) {
            return i;
        }
    }
    C2R_ASSERT(0 && "ERROR in ProfileInfo::GetCallInfoContextIdx...");
    return -1;
}

long C2R::ProcUtil::ProfileInfoContext::GetPendingCycles() {
    long pcycles = 0;
    C2R_ASSERT(sp < (int)stack.size());
    for (int i = 0; i < sp; ++i) {
        pcycles += stack[i].cycles;
    }
    return pcycles;
}

void C2R::ProcUtil::Profiler::PrintProfilerInfoStack(FILE *fp, unsigned int tid, const char* msg, int spCount) {
    auto & pic = profileInfoContextMap[tid];
    auto & profileInfoStack = pic.stack;
    fprintf(fp, "-------------------- (%s : %08x) [%13s cycles]---------------------\n", msg, tid,
        print_format_int_string(pic.cycles + pic.GetPendingCycles()));
    for (int i = 0; i < SS_count; ++i) {
        if (pic.specialSymbolFlag[i] >= 0) {
            auto& ssl = specialSymbolMap[(SpecialSymbolType)i];
            C2R_ASSERT(pic.specialSymbolFlag[i] < (int)ssl.size());
            auto* sspi = ssl[pic.specialSymbolFlag[i]];
            fprintf(fp, "***** special symbol (%s : %08x) called ****\n", sspi->name.c_str(), sspi->entryAddr);
        }
    }
    if (spCount < 0) {
        spCount = pic.sp;
    }
    for (int i = pic.sp - 1; i >= 0 && i >= pic.sp - spCount; --i) {
        auto* pii = &profileInfoStack[i];
        unsigned int callerAddr = 0, addrRegID = 0, mode = 0, callType = 0, timeStamp = 0;
        if (pii->pi->callInfoContext.find(tid) != pii->pi->callInfoContext.end()) {
            auto& cic = pii->pi->callInfoContext[tid];
            C2R_ASSERT(!cic.empty());
            if (!cic.empty()) {
                int idx = pii->pi->GetCallInfoContextIdx(tid, i);
                auto& ci = pii->pi->callInfoContext[tid][idx];
                callerAddr = ci.callerAddr;
                addrRegID = ci.addrRegID;
                callType = ci.callType;
                mode = ci.mode;
                timeStamp = ci.timeStamp;
            }
        }
        const char *ctName;
        switch (callType) {
        case CT_entry:  ctName = "ENTR"; break;
        case CT_ret:    ctName = "RET "; break;
        case CT_sret:   ctName = "SRET"; break;
        case CT_trap:   ctName = "TRAP"; break;
        case CT_cxt:    ctName = "CXT "; break;
        case CT_err:    ctName = "ERR "; break;
        case CT_pop:    ctName = "POP "; break;
        default:        ctName = "CALL"; break;
        }
        fprintf(fp, "PII(%2d) [%10d] : [tp:%08x] entry(%08x), caller(%08x), lkreg(%d), mode(%d) [%s] [%s.%s]\n",
            i, timeStamp, tid, pii->pi->entryAddr, callerAddr, addrRegID, mode,
            ctName, pii->pi->sectName.c_str(), pii->pi->name.c_str());
    }
}

void C2R::ProcUtil::Profiler::PopMsg(FILE* fp, ProfilerPopState &pps, const char* msg, int spCount) {
    auto & pic = profileInfoContextMap[curThreadPointer];
    auto & profileInfoStack = pic.stack;
    C2R_ASSERT(pic.sp > 0);
    auto pii = &profileInfoStack[pic.sp - 1];
    C2R_ASSERT(pii->pi);
    auto& cic = pii->pi->callInfoContext[curThreadPointer];
    C2R_ASSERT(!cic.empty());
    auto &ci = cic.back();
    fprintf(fp, "=============================================================\n");
    if (extCycles) {
        fprintf(fp, "[%10d] ", (unsigned int)*extCycles);
    }
    fprintf(fp, "ProfilerPop SKIPPED %s : tid(%08x), retAddr(%08x), curPC(%08x), retAddrRegID(%d), mode(%d)\n",
        msg, pps.threadID, pps.retAddr, pps.cur_pc, pps.retAddrRegID, pps.mode);
    if (spCount != 0) {
        PrintProfilerInfoStack(fp, curThreadPointer, "PENDING THREAD", spCount);
    }
    fprintf(fp, "=============================================================\n");
}

void C2R::ProcUtil::Profiler::ContextSwitchMsg(FILE* fp, ProfilerPopState &pps, int sp) {
    if (extCycles) {
        fprintf(fp, "[%10d] ", (unsigned int)*extCycles);
    }
    fprintf(fp, "+++++++++++++++++ context switch @ %08x : (%08x) --> (%08x) [sp = %2d] +++++++++++++++\n",
        pps.cur_pc, curThreadPointer, pps.threadID, sp);
}

bool C2R::ProcUtil::Profiler::ContextSwitch(ProfilerPopState &pps) {
    auto & pic = profileInfoContextMap[pps.threadID];
    C2R_ASSERT((pic.sp == 0) == (pic.stack.empty()));
    ContextSwitchMsg(pps, pic.sp);
    curThreadPointer = pps.threadID;
    if (pic.stack.empty()) {
        pic.stack.resize(profileInfoMap.size() + 1);
    }
    /// check temporal context switch
    if (pps.retAddr == pps.cur_pc && pps.retAddrRegID == 0 && pic.sp > 0) {
        auto & profileInfoStack = pic.stack;
        auto* pii1 = (pic.sp >= 1) ? &profileInfoStack[pic.sp - 1] : NULL;
        C2R_ASSERT(!pii1 || pii1->pi);
        auto* cic1 = (pii1) ? &pii1->pi->callInfoContext[curThreadPointer] : NULL;
        C2R_ASSERT(!cic1 || !cic1->empty());
        auto* ci1 = (cic1) ? &cic1->back() : NULL;
        long timeStamp = (extCycles) ? *extCycles : 0;
        C2R_ASSERT(ci1);
        if (ci1 && ci1->callerAddr == pii1->pi->entryAddr && ci1->addrRegID == 0) {
            /// temporal context switch repeated...
            ci1->timeStamp = timeStamp;
            curProfInfo = pii1;
            return true;
        }
        ProfilerPush(pps.retAddr, pps.cur_pc, pps.retAddrRegID, pps.threadID, pps.mode, CT_cxt);
        return true;
    }
    return false;
}

///     enum ProfileCommand { P_NULL = 0, P_CALL = 1, P_RET = 2 };

void C2R::ProcUtil::Profiler::ProfilerAddTraceRTL(unsigned com, unsigned regID, unsigned mode, unsigned tpc, unsigned cpc, unsigned tid) {
    if (com) { pTraceRTL.emplace_back(com, regID, mode, tpc, cpc, tid); }
}

void C2R::ProcUtil::Profiler::ProfilerPush(unsigned int tgtAddr, unsigned int cur_pc,
    unsigned int tgtAddrRegID, unsigned int threadID, unsigned int mode, ProfileCallType callType)
{
#if defined(RECORD_PROFILER_TRACE)  /// 3/28/23
    pTrace.emplace_back(1, tgtAddrRegID, mode, tgtAddr, cur_pc, threadID);
#endif
    C2R_ASSERT(curThreadPointer == threadID);
    if (tgtAddrRegID == 0 && callType == CT_call) {
        callType = CT_trap;
    }
    ProfileInfo * piTarget = profileInfoMap[tgtAddr];
    if (piTarget == nullptr) {
        piTarget = new ProfileInfo;
        profileInfoMap[tgtAddr] = piTarget;
        piTarget->entryAddr = tgtAddr;
        if (callType == CT_trap) {
            piTarget->name = "trap_" + std::to_string(systemSymCount++);
            piTarget->sectName = ".system";
        }
        else if (callType == CT_sret) {
            piTarget->name = "sret_" + std::to_string(systemSymCount++);
            piTarget->sectName = ".system";
        }
        else if (callType == CT_ret) {
            piTarget->name = "ret_" + std::to_string(systemSymCount++);
            piTarget->sectName = ".system";
        }
        else if (callType == CT_cxt) {
            piTarget->name = "switch_" + std::to_string(systemSymCount++);
            piTarget->sectName = ".context";
        }
        else {
            piTarget->name = "sym_" + std::to_string(unknownSymCount++);
            piTarget->sectName = ".unknown";
        }
    }
    else {
        C2R_ASSERT(piTarget->entryAddr == tgtAddr);
    }
    piTarget->prof.calls++;
    long timeStamp = (extCycles) ? *extCycles : 0;
    piTarget->callInfoContext[threadID].emplace_back(cur_pc, tgtAddrRegID, mode, callType, timeStamp);
    PushProfileInfoInstance(piTarget);  /// 7/1/20
#if defined(DBG_PROFILER)
    DBG_CALL_msg("CALL", true);
#endif
}

void C2R::ProcUtil::Profiler::ProfilerPop(unsigned int retAddr, unsigned int cur_pc,
    unsigned int retAddrRegID, unsigned int threadID, unsigned int mode)
{
#if defined(RECORD_PROFILER_TRACE)  /// 3/28/23
    pTrace.emplace_back(2, retAddrRegID, mode, retAddr, cur_pc, threadID);
#endif
    ProfilerPopState pps(retAddr, cur_pc, retAddrRegID, threadID, mode);
    bool cxtSwitch = false;
    if (curThreadPointer != threadID) {
        if (ContextSwitch(pps)) {
            return;
        }
        cxtSwitch = true;
    }
    auto & pic = profileInfoContextMap[curThreadPointer];
    auto & profileInfoStack = pic.stack;
    if (pic.sp == 0) {
        if (cxtSwitch) {
            ProfilerPush(retAddr, cur_pc, retAddrRegID, threadID, mode, CT_entry);
            return;
        }
        PopMsgError(pps, "(sp == 0)");
    }
    C2R_ASSERT(pic.sp > 0);
    auto &pii = profileInfoStack[pic.sp - 1];
    C2R_ASSERT(pii.pi);
    auto& cic = pii.pi->callInfoContext[curThreadPointer];
    auto &ci = cic.back();
    auto callerAddr = ci.callerAddr;
    auto addrRegID = ci.addrRegID;
    if (addrRegID == retAddrRegID) {
        if (!((callerAddr + 4 == retAddr) || (callerAddr == retAddr && addrRegID == 0))) {   /// return address does not match with caller address
            if (retAddrRegID == 0) {
                if (PopPatternReturnFromSystemTask(pps)) {
                    return;
                }
                PopMsg(pps, "(callerAddr does not match)", 1);
                ProfilerPush(retAddr, cur_pc, retAddrRegID, threadID, mode, CT_sret);
                return;
            }
            else if (PopPatternReturnFromKernelThread(pps) || PopPatternReturnFromException(pps)) {
                return;
            }
            else if (pic.specialSymbolFlag[SS_do_exit] >= 0 && PopPatternDoExit(pps)) {
                return;
            }
            else if (PopPatternRewindStack(pps)) {
                return;
            }
#define MAX_ERROR_COUNT 100//20//10
            else if (++totalErrorCount < MAX_ERROR_COUNT) {
                printf("[%10ld] totalErrorCount = %d (curThreadPointer = %08x)\n", *extCycles, totalErrorCount, curThreadPointer);
                fprintf(fp_prof, "[%10ld] totalErrorCount = %d (curThreadPointer = %08x)\n", *extCycles, totalErrorCount, curThreadPointer);
                PopMsg(pps, "ERROR!!! (callerAddr does not match)", 0);
                ProfilerPush(retAddr, cur_pc, retAddrRegID, threadID, mode, CT_err);
                return;
            }
            else {
                PopMsgError(pps, "(callerAddr does not match)");
            }
        }
        C2R_ASSERT((callerAddr + 4 == retAddr) || (callerAddr == retAddr));
#if defined(DBG_PROFILER)
        if (!DBG_POP_SKIP_COND) {
            DBG_JMP_msg(pps, (cxtSwitch) ? "RETX" : "RET ", false/*isJump*/);
        }
#endif
        PopProfileInfoInstance();
    }
    else if (addrRegID == 1 && retAddrRegID == 5 && callerAddr + 4 == retAddr) {
        C2R_ASSERT(!cxtSwitch);
#if defined(DBG_PROFILER)
        if (!DBG_POP_SKIP_COND) {
            DBG_JMP_msg(pps, "RET ", false/*isJump*/);
        }
#endif
        PopProfileInfoInstance();
    }
    else if ((addrRegID == 1 && retAddrRegID == 0) || (addrRegID == 0 && retAddrRegID == 1)) {
        if (PopPatternReturnFromException(pps)) {
            return;
        }
        PopMsg(pps, "(addrRegID does not match)", 1);
        ProfilerPush(retAddr, cur_pc, retAddrRegID, threadID, mode, (retAddrRegID == 0) ? CT_sret : CT_ret);
    }
    else if (!DBG_POP_SKIP_COND) {
        DBG_JMP_msg(pps, "JUMP", true/*isJump*/);
    }
}

bool C2R::ProcUtil::Profiler::PopPatternReturnFromException(ProfilerPopState &pps) {
    auto & pic = profileInfoContextMap[curThreadPointer];
    auto & profileInfoStack = pic.stack;
    if (pic.sp == 0) {
        return false;
    }
    auto& pii1 = profileInfoStack[pic.sp - 1];
    C2R_ASSERT(pii1.pi);
    auto& cic1 = pii1.pi->callInfoContext[curThreadPointer];
    C2R_ASSERT(!cic1.empty());
    auto &ci1 = cic1.back();
    if (pps.retAddrRegID == ci1.addrRegID) {
        if (pii1.pi->entryAddr == pps.retAddr && ci1.addrRegID == 1) {
            PopMsg(pps, "(most likely return_from_exception [direct-REENTRANT])", 1);
            PopProfileInfoInstance();
            ProfilerPush(pps.retAddr, pps.cur_pc, pps.retAddrRegID, pps.threadID, pps.mode, CT_ret);
            return true;
        }
    }
    else {
        /// pattern A:
        if (pps.retAddrRegID == 0) {
            if (pic.sp >= 3) {
                auto& pii2 = profileInfoStack[pic.sp - 2];
                auto& pii3 = profileInfoStack[pic.sp - 3];
                C2R_ASSERT(pii2.pi && pii3.pi);
                auto& cic2 = pii2.pi->callInfoContext[curThreadPointer];
                auto& cic3 = pii3.pi->callInfoContext[curThreadPointer];
                C2R_ASSERT(!cic2.empty() && !cic3.empty());
                auto &ci2 = cic2.back();
                auto &ci3 = cic3.back();
                bool cond2 = (ci2.callerAddr == pii2.pi->entryAddr) && (ci2.callType == CT_cxt);   /// context switch
                bool cond3 = (ci3.addrRegID == 0 && ci3.callerAddr == pps.retAddr);     /// callerAddr/retAddr match!
                if (cond2 && cond3) {
                    /// return-from-exception : 
                    PopMsg(pps, "(most likely return_from_exception)", 3);
                    PopProfileInfoInstance();
                    PopProfileInfoInstance();
                    PopProfileInfoInstance();   /// pop three times....
                    return true;
                }
            }
        }
        else if (pps.retAddrRegID == 1) {
            if (pic.sp >= 2) {
                auto& pii2 = profileInfoStack[pic.sp - 2];
                C2R_ASSERT(pii1.pi && pii2.pi);
                auto& cic2 = pii2.pi->callInfoContext[curThreadPointer];
                C2R_ASSERT(!cic1.empty() && !cic2.empty());
                auto &ci2 = cic2.back();
                bool cond1 = (ci1.callerAddr == pii1.pi->entryAddr) && (ci1.callType == CT_cxt);   /// context switch
                bool cond2 = (ci2.addrRegID == 1 && pii2.pi->entryAddr == pps.retAddr && ci2.callType == CT_ret);   /// reentrance
                if (cond1 && cond2) {
                    /// return-from-exception : 
                    PopMsg(pps, "(most likely return_from_exception [cxt-REENTRANT])", 2);
                    PopProfileInfoInstance();
                    PopProfileInfoInstance();   /// pop two times....
                    return true;
                }
            }
        }
    }
    return false;
}

bool C2R::ProcUtil::Profiler::PopPatternReturnFromSystemTask(ProfilerPopState &pps) {
    auto & pic = profileInfoContextMap[curThreadPointer];
    auto & profileInfoStack = pic.stack;
    /// pattern A:
    if (pps.retAddrRegID == 0) {
        if (pic.sp >= 2) {
            auto& pii1 = profileInfoStack[pic.sp - 1];
            auto& pii2 = profileInfoStack[pic.sp - 2];
            C2R_ASSERT(pii1.pi && pii2.pi);
            auto& cic1 = pii1.pi->callInfoContext[curThreadPointer];
            auto& cic2 = pii2.pi->callInfoContext[curThreadPointer];
            C2R_ASSERT(!cic1.empty() && !cic2.empty());
            auto &ci1 = cic1.back();
            auto &ci2 = cic2.back();
            bool cond1 = (ci1.callerAddr == pii1.pi->entryAddr) && (ci1.callType == CT_cxt);   /// context switch
            bool cond2 = (ci2.addrRegID == 0 && (ci2.callerAddr == pps.retAddr || ci2.callerAddr + 4 == pps.retAddr));
            if (cond1 && cond2) {
                /// return-from-exception : 
                PopMsg(pps, "(most likely return_from_system_task)", 2);
                PopProfileInfoInstance();
                PopProfileInfoInstance();   /// pop two times....
                return true;
            }
        }
    }
    return false;
}

bool C2R::ProcUtil::Profiler::PopPatternReturnFromKernelThread(ProfilerPopState &pps) {
    if (pps.retAddrRegID == 1) {
        auto & pic = profileInfoContextMap[curThreadPointer];
        auto & profileInfoStack = pic.stack;
        if (pic.sp == 1) {
            auto* pii1 = &profileInfoStack[pic.sp - 1];
            C2R_ASSERT(pii1->pi);
            auto& cic1 = pii1->pi->callInfoContext[curThreadPointer];
            auto &ci1 = cic1.back();
            if (ci1.callType == CT_entry && ci1.addrRegID == 1) {
                PopMsg(pps, "(most likely return_from_kernel_thread)", 3);
                ProfilerPush(pps.retAddr, pps.cur_pc, pps.retAddrRegID, pps.threadID, pps.mode, CT_ret);
                return true;
            }
        }
    }
    return false;
}

bool C2R::ProcUtil::Profiler::PopPatternDoExit(ProfilerPopState &pps) {
    auto & pic = profileInfoContextMap[curThreadPointer];
    C2R_ASSERT(pic.specialSymbolFlag[SS_do_exit] >= 0);
    PopMsg(pps, "(most likely reentrant_after_do_exit)", pic.sp);
    while (pic.sp > 1) {
        PopProfileInfoInstance();
    }
    C2R_ASSERT(pic.sp == 1);
    pic.ClearSpecialSymbolFlag();
    return true;
}

bool C2R::ProcUtil::Profiler::PopPatternRewindStack(ProfilerPopState &pps) {
    if (pps.retAddrRegID == 1) {
        auto & pic = profileInfoContextMap[curThreadPointer];
        auto & profileInfoStack = pic.stack;
        C2R_ASSERT(pic.sp < (int)profileInfoStack.size());
        for (int i = pic.sp - 1; i > 0; --i) {
            auto &pii = profileInfoStack[i];
            if (!(pii.pi && !pii.pi->callInfoContext.empty())) {
                printf("ERROR in PopPatternRewindStack : sp = %d, i = %d, pii.pi = %08x", pic.sp, i, (unsigned)((long long)pii.pi));
                if (pii.pi) {
                    printf("pii.pi->callInfoContext.size() = %d", (int)pii.pi->callInfoContext.size());
                }
                printf("\n");
            }
            C2R_ASSERT(pii.pi && !pii.pi->callInfoContext.empty());
            auto& cic = pii.pi->callInfoContext[curThreadPointer].back();
            if (cic.callerAddr + 4 == pps.retAddr && cic.addrRegID == 1) {
                C2R_ASSERT(i != pic.sp - 1 && i > 0);
                PopMsg(pps, "(REWIND PROFILER STACK!!!)", pic.sp - i);
                while (pic.sp > i) {
                    PopProfileInfoInstance();
                }
                C2R_ASSERT(pic.sp == i);
                return true;
            }
        }
    }
    return false;
}

void C2R::ProcUtil::Profiler::ProfilerPop()
{
    PopProfileInfoInstance();
}

void C2R::ProcUtil::Profiler::ProfilerUpdate()
{
    if (curProfInfo) { curProfInfo->cycles++; }
}

void C2R::ProcUtil::Profiler::ProfilerSkip()
{
    skippedCycles++;
}

void C2R::ProcUtil::Profiler::ProfilerStart()
{
    C2R_ASSERT(profileInfoMap.size() > 0);
    ProfileInfo::maxProfileFuncNameLength = 0;
    for (auto i = profileInfoMap.begin(), e = profileInfoMap.end(); i != e; ++i) {
        auto* pi = i->second;
        size_t len = pi->name.size();
        if (ProfileInfo::maxProfileFuncNameLength < len) { ProfileInfo::maxProfileFuncNameLength = len; }
    }
    for (auto i = profileInfoMap.begin(), e = profileInfoMap.end(); i != e; ++i) {
        auto* pi = i->second;
        pi->prof.Reset();
    }
    InitializeProfileInfoInstance();
}
void C2R::ProcUtil::Profiler::InitializeProfileInfoInstance() {
    C2R_ASSERT(curThreadPointer == 0);
    auto & sp = profileInfoContextMap[curThreadPointer].sp;
    sp = 0;
    auto* pi0 = profileInfoMap[0];
    C2R_ASSERT(pi0);
    pi0->callInfoContext[curThreadPointer].emplace_back(entryAddr, 0, 0, CT_trap, 0);
    PushProfileInfoInstance(pi0);
}

void C2R::ProcUtil::ProfileInfo::PrintProfileInfo(FILE * fp, double totalCycles, int maxNameLength, ProfileData& pd)
{
    fprintf(fp, "%*s [%08x]: calls = ", maxNameLength, name.c_str(), entryAddr);
    fprintf(fp, "%*s", (int)pd.calls, print_format_int_string(prof.calls));
    fprintf(fp, ", clks(acc) = ");
    fprintf(fp, "%*s", (int)pd.subCycles, print_format_int_string(prof.cycles + prof.subCycles));
    fprintf(fp, " [%7.3f%%], clks(local) = ", (prof.cycles + prof.subCycles) * 100.0 / totalCycles);
    fprintf(fp, "%*s", (int)pd.cycles, print_format_int_string(prof.cycles));
    fprintf(fp, " [%7.3f%%]", (prof.cycles) * 100.0 / totalCycles);
    fprintf(fp, ", clks/call = %*s\n", (int)(pd.cycles + 4),
        (prof.calls != 0) ? print_format_double_string((double)prof.cycles / prof.calls) : "");
}

bool C2R::ProcUtil::ProfileInfo::CompareProfile(ProfileInfo *a, ProfileInfo *b) {
    return a->prof.cycles + a->prof.subCycles > b->prof.cycles + b->prof.subCycles;
}

long C2R::ProcUtil::ProfileData::GetFormatLength(long d) {
    long l = 0;
    while (d > 0) {
        l++;
        d /= 10;
    }
    if (l == 0) {
        l = 1;
    }
    l += l / 3; /// commas
    return l;
}

void C2R::ProcUtil::Profiler::PrintProfileCycles(FILE* fp, double totalCycles, double activeCycles, double skippedCycles1) {
    double active_percent = (activeCycles / totalCycles) * 100;
    fprintf(fp, "total cycles = %13s", print_format_double_string(totalCycles));
    fprintf(fp, " (active: %s, ", print_format_double_string(activeCycles));
    fprintf(fp, "sleep: %s) ", print_format_double_string(skippedCycles1));
    fprintf(fp, "(active: %3.2f%%, sleep: %3.2f%%) \n\n", active_percent, 100 - active_percent);
}

void C2R::ProcUtil::Profiler::PrintContextStacks(FILE* fp) {
    PrintProfilerInfoStack(fp, curThreadPointer, "CURRENT THREAD", -1);
    for (auto i = profileInfoContextMap.begin(), e = profileInfoContextMap.end(); i != e; ++i) {
        if (i->first != curThreadPointer) {
            PrintProfilerInfoStack(fp, i->first, "PENDING THREAD", -1);
        }
    }
    fprintf(fp, "=============================================================\n");
}

#if defined(RECORD_PROFILER_TRACE)  /// 3/28/23
void C2R::ProcUtil::Profiler::ProfilerTracePrint() {
    printf("# profilerTraces = %d\n", (unsigned)pTrace.size());
}

bool C2R::ProcUtil::ProfilerTrace::match(ProfilerTrace &p0, ProfilerTrace &p1) {
    return (p0.com == p1.com && p0.regID == p1.regID && p0.mode == p1.mode &&
        p0.tgt_pc == p1.tgt_pc && p0.cur_pc == p1.cur_pc && p0.tid == p1.tid);
}

void C2R::ProcUtil::Profiler::ProfilerTraceRTLPrint() {
    printf("# profilerTraces(CPP-Dataflow) = %d\n", (unsigned)pTrace.size());
    printf("# profilerTraces(RTL-C)        = %d\n", (unsigned)pTraceRTL.size());
    if (pTrace.size() == pTraceRTL.size()) {
        printf("pTrace size matched...\n");
        unsigned matchedCount = 0, unmatchedIdx = 0;
        for (unsigned i = 0, e = (unsigned)pTrace.size(); i < e; ++i) {
            auto p0 = pTrace[i], p1 = pTraceRTL[i];
            if (ProfilerTrace::match(p0, p1)) {
                matchedCount++;
            }
            else if (!unmatchedIdx) {
                printf("pTrace unmatched at i = %d\n", i);
                unmatchedIdx = i + 1;
            }
        }
        if (!unmatchedIdx) { printf("pTrace all matched!!\n"); }
        else {
            printf("pTrace match-count = (%d / %d)\n", matchedCount, (unsigned)pTrace.size());
            unsigned e = unmatchedIdx + 100;
            if (e >= (unsigned)pTrace.size()) { e = (unsigned)pTrace.size(); }
            for (unsigned i = 0; i < e; ++i) {
                auto& p0 = pTrace[i], &p1 = pTraceRTL[i];
                printf("%s %4d : ( %d %2d %d %08x %08x %08x ) ( %d %2d %d %08x %08x %08x )\n",
                    (ProfilerTrace::match(p0, p1)) ? " " : "*", i,
                    p0.com, p0.regID, p0.mode, p0.tgt_pc, p0.cur_pc, p0.tid,
                    p1.com, p1.regID, p1.mode, p1.tgt_pc, p1.cur_pc, p1.tid);
            }
        }
    }
    else {
        printf("pTrace size not matched (%d, %d)\n", (unsigned)pTrace.size(), (unsigned)pTraceRTL.size());
    }
}
#endif

void C2R::ProcUtil::Profiler::ProfilerPrint()
{
    if (profileInfoContextMap.empty()) {
        return;
    }
    {
        PrintContextStacks(fp_prof);
    }
    for (auto i = profileInfoContextMap.begin(), e = profileInfoContextMap.end(); i != e; ++i) {
        auto &pic = i->second;
        curThreadPointer = i->first;
        while (pic.sp > 0) {
            PopProfileInfoInstance();
        }
    }
    double activeCycles = 0;
    double totalCycles = 0;
    double skippedCycles1 = 0;
    printf("Profiler result(%d):\n", id);
    for (auto i = profileInfoMap.begin(), e = profileInfoMap.end(); i != e; ++i) {
        activeCycles += i->second->prof.cycles;
    }
    skippedCycles1 = (double)skippedCycles;
    totalCycles = activeCycles + skippedCycles1;
    if (activeCycles == 0.0) { activeCycles = 1.0; } /// avoid divide-by-zero

    PrintProfileCycles(stdout, totalCycles, activeCycles, skippedCycles1);
    if (fp_prof) {
        PrintProfileCycles(fp_prof, totalCycles, activeCycles, skippedCycles1);
    }
    std::vector<ProfileInfo*> profList;
    ProfileData pd;
    int maxNameLength = 0;
    for (auto i = profileInfoMap.begin(), e = profileInfoMap.end(); i != e; ++i) {
        if (i->second->prof.calls) {
            profList.push_back(i->second);
            pd.MaxCycles(i->second->prof);
            if (maxNameLength < i->second->name.length()) {
                maxNameLength = (int)i->second->name.length();
            }
        }
    }
    pd.subCycles += pd.cycles;
    pd.GetFormatLengths();
    std::sort(profList.begin(), profList.end(), ProfileInfo::CompareProfile);
    for (int i = 0, e = (int)profList.size(); i < e; ++i) {
        {//if (profileInfoContextMap.size() == 1) {
            profList[i]->PrintProfileInfo(stdout, activeCycles, maxNameLength, pd);
        }
        if (fp_prof) {
            profList[i]->PrintProfileInfo(fp_prof, activeCycles, maxNameLength, pd);
        }
    }
}

