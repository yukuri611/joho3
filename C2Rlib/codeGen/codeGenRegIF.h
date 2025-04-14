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

#if !defined(CODE_GEN_REG_IF_H)
#define CODE_GEN_REG_IF_H

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <vector>
#include <string>
#endif

#include "codeGen.h"

#if 0   /// moved to "codeGen.h"
class CodeGenRegIF : public CodeGen {
public:
    CodeGenRegIF(const char *comp_name, const char *in_file_name, int line_num) : CodeGen(comp_name, in_file_name, line_num) {}
    class RegElement {
    public:
        const char *name;
        /// 1/30/18 : accessType (isReadOnly/isWriteOnly removed)
        int size, elementCount, sign, accessType, structType;   /// accessType = 3 (hidden) : 11/8/18
        /// size : in bytes, structType : true if the register-group is a structure
        const char *size_str;
        const char *elementCount_str;
        const char *exp_str;
        const char *bitWidth_str;
        RegElement(const char *n, int sz, int ec, int isSigned, int at, const char *expr = nullptr,
            const char *bw = nullptr, int st = 0)
        {
            initialize(n, nullptr, sz, nullptr, ec, isSigned, at, bw, expr, st);
        }
        RegElement(const char *n, int sz, const char *ec_str, int max_ec, int isSigned, int at,
            const char *bw = nullptr, int st = 0)
        {
            initialize(n, nullptr, sz, ec_str, max_ec, isSigned, at, bw, nullptr, st);
        }
        RegElement(const char *n, const char *sz_str, int max_sz, int ec, int isSigned, int at,
            const char *bw = nullptr, int st = 0)
        {
            initialize(n, sz_str, max_sz, nullptr, ec, isSigned, at, bw, nullptr, st);
        }
        RegElement(const char *n, const char *sz_str, int max_sz, const char *ec_str, int max_ec, int isSigned, int at,
            const char *bw = nullptr, int st = 0)
        {
            initialize(n, sz_str, max_sz, ec_str, max_ec, isSigned, at, bw, nullptr, st);
        }
        void initialize(const char *n, const char *sz_str, int sz, const char *ec_str, int ec, int isSigned, int at,
            const char *bw_str, const char *expr, int st) {
            name = n; size_str = sz_str; size = sz; elementCount_str = ec_str; elementCount = ec; sign = isSigned;
            accessType = at; bitWidth_str = bw_str, exp_str = expr; structType = st;
        }
        bool isArray() { return elementCount > 1 || elementCount_str; }
        /// 1/18/19 : don't use SINT8, SINT16, etc. --> these already have bit-width attributes...
        const char *GetTypeName() {
            if (size_str) { return size_str; }
            else {
                switch (size) {
                case 1: return (sign) ? "int8_t " : "uint8_t ";
                case 2: return (sign) ? "int16_t" : "uint16_t";
                case 4: return (sign) ? "int32_t" : "uint32_t";
                default: return "<unknown type>";
                }
            }
        }
    };
    void emitResourceDefinition(std::vector<RegElement> &regs, size_t &icount, size_t maxNameLength, const char *arrayAttr,
        MacroGroup *mg = nullptr, unsigned mgIdx = 0) {
        /////////////// declare registers //////////////
        ind(fp, icount); fprintf(fp, "/// register reclaration...\n\n");
        ind(fp, icount); fprintf(fp, "private:\n");
        size_t rsz = regs.size(), i;
        for (i = 0; i < rsz; ++i) {
            if (regs[i].isArray() || regs[i].accessType == 3) { continue; }
            ind(fp, icount); fprintf(fp, "%-6s r_%s", regs[i].GetTypeName(), regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
            const char *remarks = (regs[i].accessType == 1) ? " /// (read-only)" : (regs[i].accessType == 2) ? " /// (write-only)" : "";
            if (regs[i].bitWidth_str) {
                fprintf(fp, " _TYPE(static_state) _BWT(%s);%s\n", regs[i].bitWidth_str, remarks);
            }
            else {
                fprintf(fp, " _TYPE(static_state);%s\n", remarks);
            }
        }
        ind(fp, icount); fprintf(fp, "public:\n");
        for (i = 0; i < rsz; ++i) {
            ind(fp, icount); fprintf(fp, "%-6s %s", regs[i].GetTypeName(), regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
            if (regs[i].elementCount_str) {
                if (mg) {
                    fprintf(fp, "[%s]", std::get<0>((*mg)[mgIdx + 1]).c_str());
                    fprintf(fp, "[%s]", std::get<0>((*mg)[mgIdx]).c_str());
                    mgIdx += 2;
                }
                else { fprintf(fp, "[%s]", regs[i].elementCount_str); }
            }
            else if (regs[i].elementCount > 1) { fprintf(fp, "[%d]", regs[i].elementCount); }
            int at = regs[i].accessType;
            const char *remarks = (at == 1) ? " /// (read-only)" :
                (at == 2) ? " /// (write-only)" : (at == 3) ? " /// (hidden wire)" : "";
            if (regs[i].isArray() && regs[i].bitWidth_str) {
                fprintf(fp, " _TYPE(%s) _BWT(%s);%s\n", arrayAttr, regs[i].bitWidth_str, remarks);
            }
            else {
                fprintf(fp, " %s;%s\n", (regs[i].isArray()) ? "_TYPE(static_state)" : "", remarks);
            }
        }
        fprintf(fp, "\n");
    }
    void emitRegSize(std::vector<RegElement> &regs, size_t &icount, size_t maxNameLength) {
        size_t rsz = regs.size(), i;
        /////////////// enum REG_SIZE //////////////
        CodeGroup cg(fp, icount, "enum REG_SIZE", false);
        for (i = 0; i < rsz; ++i) {
            if (regs[i].accessType == 3) { continue; }
            ind(fp, icount); fprintf(fp, "RSZ_%s", regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
            if (regs[i].size_str) { fprintf(fp, " = sizeof(%s), /// max(%d)\n", regs[i].size_str, regs[i].size); }
            else { fprintf(fp, " = %2d,\n", regs[i].size); }
        }
    }
    void emitRegCount(std::vector<RegElement> &regs, size_t &icount, size_t maxNameLength,
        MacroGroup *mg = nullptr, unsigned mgIdx = 0) {
        size_t rsz = regs.size(), i;
        /////////////// enum REG_COUNT //////////////
        CodeGroup cg(fp, icount, "enum REG_COUNT", false);
        for (i = 0; i < rsz; ++i) {
            if (regs[i].accessType == 3) { continue; }
            ind(fp, icount); fprintf(fp, "RCNT_%s", regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
            if (regs[i].elementCount_str) {
                if (mg) {
                    fprintf(fp, " = %s*", std::get<0>((*mg)[mgIdx++]).c_str());
                    fprintf(fp, "%s, /// max(%d)\n", std::get<0>((*mg)[mgIdx++]).c_str(), regs[i].elementCount);
                }
                else fprintf(fp, " = %s, /// max(%d)\n", regs[i].elementCount_str, regs[i].elementCount);
            }
            else { fprintf(fp, " = %2d,\n", regs[i].elementCount); }
        }
    }
    size_t emitRegAddr(std::vector<RegElement> &regs, size_t &icount, size_t maxNameLength, MacroGroup *mg) {
        size_t rsz = regs.size(), i;
        mg->addMacro("REG_SZ_ALIGNED", "(rn)", "((REG_SZ(rn) & ~(0x3)) + (((REG_SZ(rn) & (0x3)) != 0) * 4))", "aligned reg size");
        mg->addMacro("REG_OFS", "(rn)", "(RADDR_##rn + REG_SZ_ALIGNED(rn))", "reg offset");
        /////////////// enum REG_ADDR //////////////
        {
            CodeGroup cg(fp, icount, "enum REG_ADDR", false);
            for (i = 0; i < rsz; ++i) {
                if (regs[i].accessType == 3) { continue; }
                ind(fp, icount); fprintf(fp, "RADDR_%s", regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
                if (i)  { fprintf(fp, " = REG_OFS(%s),\n", regs[i - 1].name); }
                else    { fprintf(fp, " = 0,\n"); }
            }
        }
        size_t maxTotalBytes = 0;
        for (i = 0; i < rsz; ++i) {
            if (regs[i].accessType == 3) { continue; }
            int bytes = regs[i].size * regs[i].elementCount;
            maxTotalBytes += bytes + ((bytes % 4) != 0);
        }
        size_t addrBits = 0, bytes = maxTotalBytes - 1;
        while (bytes) { ++addrBits; bytes >>= 1; }
        ind(fp, icount); fprintf(fp, "/// maxTotalBytes = %d, addrBits = %d\n\n", (int)maxTotalBytes, (int)addrBits);
        return addrBits;
    }
};
#endif

class CodeGenRegIF_AXI4L : public CodeGenRegIF {
public:
    CodeGenRegIF_AXI4L(const char *comp_name, const char *comp_header, std::vector<RegElement> &regs,
        const char *in_file_name, int line_num) : CodeGenRegIF(comp_name, in_file_name, line_num) {
        size_t rsz = regs.size(), maxNameLength = 0, i, len;
        for (i = 0; i < rsz; ++i) { len = strlen(regs[i].name); if (maxNameLength < len) { maxNameLength = len; } }
        size_t icount = 0;
        {
            std::string cm = comp_header; cm += "struct "; cm += comp_name; cm += " : AXI4L::SlaveFSM";
            CodeGroup cg1(fp, icount, cm.c_str(), false);

            /// 9/27/18
            MacroGroup mg(fp, 20);
            emitResourceDefinition  (regs, icount, maxNameLength, "static_state");
            emitRegSize             (regs, icount, maxNameLength);
            emitRegCount            (regs, icount, maxNameLength);
            mg.addMacro("REG_SZ", "(rn)", "(RSZ_##rn * RCNT_##rn)", "reg size");
            size_t addrBits = emitRegAddr(regs, icount, maxNameLength, &mg);

            /////////////// BIT devRead(UINT32 *data, UINT32 addr) { ... } //////////////
            {
                CodeGroup gc(fp, icount, "BIT devRead(UINT32 *data, UINT32 addr)", true);
                ind(fp, icount); fprintf(fp, "switch (UINT32 idx = addr & 0x%x) {\n", (1 << addrBits) - 1); ++icount;
                for (i = 0; i < rsz; ++i) {
                    if (regs[i].accessType == 3) { continue; }
                    if (regs[i].accessType == 2) {
                        fprintf(fp, "/// case RADDR_%s: skipped... (isWriteOnly)\n", regs[i].name);
                    }
                    else {
                        if (regs[i].elementCount > 1 || regs[i].elementCount_str) {
                            ind(fp, icount); fprintf(fp, "/// case RADDR_%s: skipped... (handled in default:)\n", regs[i].name);
                        }
                        else {
                            ind(fp, icount); fprintf(fp, "case RADDR_%s", regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
                            fprintf(fp, " : *data = %s", regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
                            fprintf(fp, " ; break;\n");
                        }
                    }
                }
                ind(fp, icount); fprintf(fp, "default:\n"); ++icount;
                bool flag = false;
                for (i = 0; i < rsz; ++i) {
                    if (regs[i].accessType == 3) { continue; }
                    if (regs[i].accessType != 2 && (regs[i].size_str || regs[i].elementCount_str)) {
                        ind(fp, icount); fprintf(fp, "%sif (idx >= RADDR_%s && idx < REG_OFS(%s)) {\n",
                            (flag) ? "else " : "", regs[i].name, regs[i].name); ++icount;
                        ind(fp, icount); fprintf(fp, "*data = %s[(idx - RADDR_%s)/RSZ_%s];\n", regs[i].name, regs[i].name, regs[i].name);
                        ind(fp, --icount); fprintf(fp, "}\n");
                        flag = true;
                    }
                }
                if (flag) { ind(fp, icount); fprintf(fp, "else {\n"); ++icount; }
                ind(fp, icount); fprintf(fp, "*data = 0;\n");
                ind(fp, icount); fprintf(fp, "printf(\"WARNING on %s::devRead()!!! address (0x%%8x) is invalid!!!\\n\", addr);\n", comp_name);
                ind(fp, icount); fprintf(fp, "return 1; /// readFlag = 0;\n");
                if (flag) { ind(fp, --icount); fprintf(fp, "}\n"); }
                ind(fp, icount); fprintf(fp, "break;\n");
                --icount;
                ind(fp, --icount); fprintf(fp, "}\n");
                ind(fp, icount); fprintf(fp, "readFlag = 1;\n");
                ind(fp, icount); fprintf(fp, "return 1;\n");
            }
            /////////////// BIT devWrite(UINT32 data, UINT32 addr) { ... } //////////////
            {
                CodeGroup cg(fp, icount, "BIT devWrite(UINT32 data, UINT32 addr)", true);
                ind(fp, icount); fprintf(fp, "switch (UINT32 idx = addr & 0x%x) {\n", (1 << addrBits) - 1); ++icount;
                for (i = 0; i < rsz; ++i) {
                    if (regs[i].accessType == 3) { continue; }
                    ind(fp, icount);
                    if (regs[i].accessType == 1) {
                        fprintf(fp, "/// case RADDR_%s: skipped... (isReadOnly)\n", regs[i].name);
                    }
                    else if (regs[i].elementCount > 1 || regs[i].elementCount_str) {
                        fprintf(fp, "/// case RADDR_%s: skipped... (handled in default:)\n", regs[i].name);
                    }
                    else {
                        fprintf(fp, "case RADDR_%s", regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
                        fprintf(fp, " : r_%s", regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
                        fprintf(fp, " = (%s)data ; break;\n", regs[i].GetTypeName());
                    }
                }
                ind(fp, icount); fprintf(fp, "default:\n"); ++icount;
                bool flag = false;
                for (i = 0; i < rsz; ++i) {
                    if (regs[i].accessType != 1 && (regs[i].size_str || regs[i].elementCount_str)) {
                        ind(fp, icount); fprintf(fp, "%sif (idx >= RADDR_%s && idx < REG_OFS(%s)) {\n",
                            (flag) ? "else " : "", regs[i].name, regs[i].name); ++icount;
                        ind(fp, icount); fprintf(fp, "%s[(idx - RADDR_%s)/RSZ_%s] = ", regs[i].name, regs[i].name, regs[i].name);
                        fprintf(fp, " (%s)data ;\n", regs[i].GetTypeName());
                        ind(fp, --icount); fprintf(fp, "}\n");
                        flag = true;
                    }
                }
                if (flag) {
                    ind(fp, icount); fprintf(fp, "else {\n"); ++icount;
                }
                ind(fp, icount); fprintf(fp, "printf(\"WARNING on %s::devWrite()!!! address (0x%%8x) is invalid!!!\\n\", addr);\n", comp_name);
                ind(fp, icount); fprintf(fp, "return 1; /// writeFlag = 0; \n");
                if (flag) {
                    ind(fp, --icount); fprintf(fp, "}\n");
                }
                ind(fp, icount); fprintf(fp, "break;\n");
                --icount;
                ind(fp, --icount); fprintf(fp, "}\n");
                ind(fp, icount); fprintf(fp, "writeFlag = 1;\n");
                ind(fp, icount); fprintf(fp, "return 1;\n");
            }
            /////////////// void fsmUser() { ... } //////////////
            {
                CodeGroup cg(fp, icount, "void fsmUser()", true);
                for (i = 0; i < rsz; ++i) {
                    if (regs[i].exp_str) {
                        ind(fp, icount);
                        fprintf(fp, "r_%s = %s;\n", regs[i].name, regs[i].exp_str);
                    }
                }
                for (i = 0; i < rsz; ++i) {
                    if (regs[i].accessType == 3) { continue; }
                    if (regs[i].isArray()) { continue; }
                    ind(fp, icount);
                    fprintf(fp, "%s = r_%s;\n", regs[i].name, regs[i].name);
                }
            }
        }
    }
};

template<int P> /// 11/19/18 : FO = fanout
class CodeGenRegIF_C2RI : public CodeGenRegIF {
    void EmitFloatQuantCode(size_t i, size_t &icount, FILE *fp, std::vector<RegElement> &regs) {
        fprintf(fp, "#if defined(EFFECTIVE_%s_REG_BITS)\n", regs[i].name);
        ind(fp, icount);
        /// 1/17/19
        fprintf(fp, "d = QUANT_INT_%s(d);\n", regs[i].name);
        fprintf(fp, "#endif\n");
    }
public:
#if 1
    CodeGenRegIF_C2RI(int nonIntegralType, int FO, const char *comp_name, const char *comp_header,
        std::vector<RegElement> &regs,
        std::vector<std::tuple<std::string, std::string, std::string, std::string>> &macroList,
        const char *in_file_name, int line_num) : CodeGenRegIF(comp_name, in_file_name, line_num) {
        Create(nonIntegralType, FO, comp_name, comp_header, regs, &macroList);
    }
#endif
    CodeGenRegIF_C2RI(int nonIntegralType, int FO, const char *comp_name, const char *comp_header,
        std::vector<RegElement> &regs,
        const char *in_file_name, int line_num) : CodeGenRegIF(comp_name, in_file_name, line_num) {
        Create(nonIntegralType, FO, comp_name, comp_header, regs, nullptr);
    }
    void Create(int nonIntegralType, int FO, const char *comp_name, const char *comp_header,
        std::vector<RegElement> &regs,
        std::vector<std::tuple<std::string, std::string, std::string, std::string>> *macroList) {
		size_t rsz = regs.size(), maxNameLength = 0, i, len;
		for (i = 0; i < rsz; ++i) { len = strlen(regs[i].name); if (maxNameLength < len) { maxNameLength = len; } }
		size_t icount = 0;
		{
            std::string cm = comp_header; cm += "struct "; cm += comp_name; cm += " : C2RI::SlaveFSM<C2RI_T>";
			CodeGroup cg1(fp, icount, cm.c_str(), false);

           /// 9/27/18
            MacroGroup mg(fp, 20);
            unsigned mgIdx = 0;
            if (P > 1) {
#if 1
                if (macroList) {
                    size_t msz = macroList->size();
                    for (i = 0; i < msz; i++) {
                        auto &m = (*macroList)[i];
                        mg.addMacro(std::get<0>(m).c_str(), std::get<1>(m).c_str(),
                            std::get<2>(m).c_str(), std::get<3>(m).c_str());
                    }
                }
                else {
                    mg.addMacro("OSZ_P", "", "DIV_CEIL(OSZ, P)", "ceil(OSZ / P)");
                    mg.addMacro("ISZ_P", "", "DIV_CEIL(ISZ, P)", "ceil(ISZ / P)");
                    mgIdx = (int)mg.size();
                    mg.addMacro("WSZ_1", "", "((OSZ == 1) ? ISZ_P*(FSZ*FSZ) : (ISZ*FSZ*FSZ)*OSZ_P)", "weight dim[1]");
                    mg.addMacro("WSZ_0", "", "((OSZ == 1 && ISZ == 1) ? 1 : P)", "weight dim[0]");
                    mg.addMacro("BSZ_1", "", "OSZ_P", "bias dim[1]");
                    mg.addMacro("BSZ_0", "", "((OSZ == 1) ? 1 : P)", "bias dim[0]");
                    if (FO == 1) {   /// 11/19/18
                        mg.addMacro("PHW_SZ", "", "DIV_CEIL(WSZ_1, FO)", "phase array size");
                        mg.addMacro("PHB_SZ", "", "DIV_CEIL(BSZ_1, FO)", "phase array size");
                        mg.addMacro("PH_IDX", "(idx)", "((idx) / FO)", "phase FAN-OUT");
                    }
                }
#else
                mg.addMacro("OSZ_P", "", "DIV_CEIL(OSZ, P)", "ceil(OSZ / P)");
                mg.addMacro("ISZ_P", "", "DIV_CEIL(ISZ, P)", "ceil(ISZ / P)");
                mgIdx = (int)mg.size();
                mg.addMacro("WSZ_1", "", "((OSZ == 1) ? ISZ_P*(FSZ*FSZ) : (ISZ*FSZ*FSZ)*OSZ_P)", "weight dim[1]");
                mg.addMacro("WSZ_0", "", "((OSZ == 1 && ISZ == 1) ? 1 : P)", "weight dim[0]");
                mg.addMacro("BSZ_1", "", "OSZ_P", "bias dim[1]");
                mg.addMacro("BSZ_0", "", "((OSZ == 1) ? 1 : P)", "bias dim[0]");
                if (FO == 1) {   /// 11/19/18
                    mg.addMacro("PHW_SZ", "", "DIV_CEIL(WSZ_1, FO)", "phase array size");
                    mg.addMacro("PHB_SZ", "", "DIV_CEIL(BSZ_1, FO)", "phase array size");
                    mg.addMacro("PH_IDX", "(idx)", "((idx) / FO)", "phase FAN-OUT");
                }
#endif
                emitResourceDefinition(regs, icount, maxNameLength, "state", &mg, mgIdx);
                /// 11/19/18
                if (FO == 1) {
                    ind(fp, icount); fprintf(fp, "UINT16 phase_w[PHW_SZ] _TYPE(state), phase_b[PHB_SZ] _TYPE(state), rw_count _TYPE(state); /// internal states\n");
                }
                else {
                    ind(fp, icount); fprintf(fp, "UINT16 phase _TYPE(state), rw_count _TYPE(state); /// internal states\n");
                }
                ind(fp, icount); fprintf(fp, "UINT16 max_phase, max_rw_count; \n");
                ind(fp, icount); fprintf(fp, "BIT update_phase; \n\n");
                emitRegSize(regs, icount, maxNameLength);
                emitRegCount(regs, icount, maxNameLength, &mg, mgIdx);
            }
            else {
                emitResourceDefinition(regs, icount, maxNameLength, "static_state");
                emitRegSize(regs, icount, maxNameLength);
                emitRegCount(regs, icount, maxNameLength);
            }
            mg.addMacro("GET_FLOAT_EXPONENT", "(d)", "(((d) >> 23) & 0xff)", "float exponent");
            mg.addMacro("REG_SZ", "(rn)", "(RSZ_##rn)", "reg size");
            size_t addrBits = emitRegAddr(regs, icount, maxNameLength, &mg);
            ind(fp, icount); fprintf(fp, "public:\n");
#if 1
            emitHeader(regs, icount, comp_name);
#else
            fprintf(fp, "#if !defined(__LLVM_C2RTL__)\n");
            fprintf(fp, "#define EMIT_HEADER	emit_header(ID)\n");
            fprintf(fp, "#else\n");
            fprintf(fp, "#define EMIT_HEADER\n");
            fprintf(fp, "#endif\n");
            ind(fp, icount); fprintf(fp, "%s(int ID) { EMIT_HEADER; }\n", comp_name);
			////////////////// header_flag() ///////////
			{
				CodeGroup cg(fp, icount, "const char * header_flag(int ID)", true);
				ind(fp, icount); fprintf(fp, "return (ID == 0) ? \"w\" : \"a\";\n");
			}
			////////////////// emit_header() ///////////
			{
				CodeGroup cg(fp, icount, "void emit_header(int ID)", true);
				ind(fp, icount); fprintf(fp, "FILE *fp = fopen(\"%s.log\", header_flag(ID));\n", comp_name);
				ind(fp, icount); fprintf(fp, "if (fp == NULL) { printf(\"ERROR in %s::emit_header()...\\n\"); return; }\n", comp_name);
				ind(fp, icount); fprintf(fp, "fprintf(fp, \"/****** Layer-%%d REG-INTERFACE ******/\\n\", ID);\n");
				for (i = 0; i < rsz; ++i) {
                    if (regs[i].accessType == 3) { continue; }
                    ind(fp, icount); fprintf(fp, "fprintf(fp, \"#define ADDR_%%d_%s %%d\\n\", ID, RADDR_%s);\n", regs[i].name, regs[i].name);
				}
				ind(fp, icount); fprintf(fp, "\n");
				for (i = 0; i < rsz; ++i) {
                    if (regs[i].accessType == 3) { continue; }
                    ind(fp, icount); fprintf(fp, "fprintf(fp, \"#define CNT_%%d_%s %%d\\n\", ID, RCNT_%s);\n", regs[i].name, regs[i].name);
				}
				ind(fp, icount); fprintf(fp, "fclose(fp);\n");
			}
#endif
            ////////////////// write_register_array_by_shifting() ///////////
            /// 11/19/18
            unsigned idx = mgIdx;
            if (FO == 1) {
                for (i = 0; i < rsz; ++i) {
                    if (regs[i].accessType == 3) { continue; }
                    if (regs[i].elementCount_str || regs[i].elementCount > 1) {
                        std::string funcName = "void write_";
                        funcName += regs[i].name;
                        funcName += "(C2RI_T d)";   /// 12/2/18
                        char type = regs[i].name[0];
                        CodeGroup cg(fp, icount, funcName.c_str(), true);
                        ind(fp, icount);
                        if (P > 1) { fprintf(fp, "const int SZ = %s;\n", std::get<0>(mg[idx++]).c_str()); }
                        else { fprintf(fp, "const int SZ = RCNT_%s;\n", regs[i].name); }
                        {
                            const char *upperDim0 = (P > 1) ? (type == 'w') ? "[phase_w[PH_IDX(i)]]" : "[phase_b[PH_IDX(i)]]" : "";
                            const char *upperDim1 = (P > 1) ? (type == 'w') ? "[phase_w[PH_IDX(i+1)]]" : "[phase_b[PH_IDX(i+1)]]" : "";
                            CodeGroup cg(fp, icount, "for (int i = 0; i < SZ - 1; i++)", true, false);
                            ind(fp, icount);
                            fprintf(fp, "%s%s[i] = %s%s[i + 1];\n", regs[i].name, upperDim0, regs[i].name, upperDim1);
                        }
                        const char *upperDim0 = (P > 1) ? (type == 'w') ? "[phase_w[PH_IDX(SZ - 1)]]" : "[phase_b[PH_IDX(SZ - 1)]]" : "";
                        /// 12/2/18
                        if (nonIntegralType && regs[i].size_str) {
                            EmitFloatQuantCode(i, icount, fp, regs);
                        }
                        ind(fp, icount);
                        if (nonIntegralType && regs[i].size_str) {
                            fprintf(fp, "%s%s[SZ - 1] = *(%s*)&d;\n", regs[i].name, upperDim0, regs[i].GetTypeName());
                        }
                        else {
                            fprintf(fp, "%s%s[SZ - 1] = (%s)d;\n", regs[i].name, upperDim0, regs[i].GetTypeName());
                        }
                        if (P > 1) {
                            ind(fp, icount); fprintf(fp, "max_phase = %s - 1;\n", std::get<0>(mg[idx++]).c_str());
                            ind(fp, icount); fprintf(fp, "max_rw_count = SZ - 1;\n");
                            ind(fp, icount); fprintf(fp, "update_phase = 1;\n");
                        }
                    }
                }
            }
            else {
                const char *upperDim = (P > 1) ? "[phase]" : "";
                for (i = 0; i < rsz; ++i) {
                    if (regs[i].accessType == 3) { continue; }
                    if (regs[i].elementCount_str || regs[i].elementCount > 1) {
                        std::string funcName = "void write_";
                        funcName += regs[i].name;
                        funcName += "(C2RI_T d)";   /// 12/2/18
                        CodeGroup cg(fp, icount, funcName.c_str(), true);
                        ind(fp, icount);
                        if (P > 1) { fprintf(fp, "const int SZ = %s;\n", std::get<0>(mg[idx++]).c_str()); }
                        else { fprintf(fp, "const int SZ = RCNT_%s;\n", regs[i].name); }
                        {
                            CodeGroup cg(fp, icount, "for (int i = 0; i < SZ - 1; i++)", true, false);
                            ind(fp, icount);
                            fprintf(fp, "%s%s[i] = %s%s[i + 1];\n", regs[i].name, upperDim, regs[i].name, upperDim);
                        }
                        /// 12/2/18
                        if (nonIntegralType && regs[i].size_str) {
                            EmitFloatQuantCode(i, icount, fp, regs);
                        }
                        ind(fp, icount);
                        if (nonIntegralType && regs[i].size_str) {
                            fprintf(fp, "%s%s[SZ - 1] = *(%s*)&d;\n", regs[i].name, upperDim, regs[i].GetTypeName());
                        }
                        else {
                            fprintf(fp, "%s%s[SZ - 1] = (%s)d;\n", regs[i].name, upperDim, regs[i].GetTypeName());
                        }
                        if (P > 1) {
                            ind(fp, icount); fprintf(fp, "max_phase = %s - 1;\n", std::get<0>(mg[idx++]).c_str());
                            ind(fp, icount); fprintf(fp, "max_rw_count = SZ - 1;\n");
                            ind(fp, icount); fprintf(fp, "update_phase = 1;\n");
                        }
                    }
                }
            }
            ////////////////// read_register_array_by_shifting() ///////////
            idx = mgIdx;
            for (i = 0; i < rsz; ++i) {
                if (regs[i].accessType == 3) { continue; }
                if (regs[i].elementCount_str || regs[i].elementCount > 1) {
                    std::string funcName = "C2RI_T read_";   /// 12/2/18
					funcName += regs[i].name;
					funcName += "()";
                    char type = regs[i].name[0];
                    const char *upperDim = (P > 1) ? (FO == 1) ? (type == 'w') ? "[phase_w[0]]" : "[phase_b[0]]" : "[phase]" : "";
					CodeGroup cg(fp, icount, funcName.c_str(), true);
                    /// 12/2/18
                    ind(fp, icount);
                    if (nonIntegralType && regs[i].size_str) {
                        fprintf(fp, "C2RI_T d = *(C2RI_T*)&%s%s[0];\n", regs[i].name, upperDim);
                    }
                    else {
                        fprintf(fp, "C2RI_T d = (C2RI_T)%s%s[0];\n", regs[i].name, upperDim);
                    }
                    ind(fp, icount); fprintf(fp, "write_%s(d);\n", regs[i].name);
					ind(fp, icount); fprintf(fp, "return d;\n");
				}
			}
            if (P > 1) {
                CodeGroup cg(fp, icount, "void update_state()", true);
                {
                    CodeGroup cg1(fp, icount, "if (update_phase == 0)", true, false);
                    ind(fp, icount); fprintf(fp, "rw_count = 0;\n");
                    /// 11/19/18
                    if (FO == 1) {
                        ind(fp, icount); fprintf(fp, "for (int i = 0; i < PHW_SZ; i++) { phase_w[i] = 0; }\n");
                        ind(fp, icount); fprintf(fp, "for (int i = 0; i < PHB_SZ; i++) { phase_b[i] = 0; }\n");
                    }
                    else {
                        ind(fp, icount); fprintf(fp, "phase = 0;\n");
                    }
                }
                {
                    CodeGroup cg1(fp, icount, "else if (rw_count == max_rw_count)", true, false);
                    ind(fp, icount); fprintf(fp, "rw_count = 0;\n");
                    /// 11/19/18
                    if (FO == 1) {
                        ind(fp, icount); fprintf(fp, "for (int i = 0; i < PHW_SZ; i++) { phase_w[i] = (phase_w[i] == max_phase) ? 0 : phase_w[i] + 1; }\n");
                        ind(fp, icount); fprintf(fp, "for (int i = 0; i < PHB_SZ; i++) { phase_b[i] = (phase_b[i] == max_phase) ? 0 : phase_b[i] + 1; }\n");
                    }
                    else {
                        ind(fp, icount); fprintf(fp, "phase = (phase == max_phase) ? 0 : phase + 1;\n");
                    }
                }
                ind(fp, icount); fprintf(fp, "else { rw_count++; }\n");
            }
            /////////////// BIT devRead(UINT32 *data, UINT32 addr) { ... } //////////////
			{
                /// 12/2/18
                CodeGroup gc(fp, icount, "BIT devRead(C2RI_T *data, UINT32 addr)", true);
				ind(fp, icount); fprintf(fp, "switch (UINT32 idx = addr & 0x%x) {\n", (1 << addrBits) - 1); ++icount;
				for (i = 0; i < rsz; ++i) {
                    if (regs[i].accessType == 3) { continue; }
                    if (regs[i].accessType == 2) {
						fprintf(fp, "/// case RADDR_%s: skipped... (isWriteOnly)\n", regs[i].name);
					}
					else if (regs[i].elementCount > 1 || regs[i].elementCount_str) {
						ind(fp, icount); fprintf(fp, "case RADDR_%s", regs[i].name);
						ind(fp, maxNameLength - strlen(regs[i].name), " ");
						fprintf(fp, " : *data = read_%s", regs[i].name);
						fprintf(fp, "() ; break;\n");
					}
					else {
						ind(fp, icount); fprintf(fp, "case RADDR_%s", regs[i].name);
						ind(fp, maxNameLength - strlen(regs[i].name), " ");
                        /// 12/2/18
                        if (nonIntegralType && regs[i].size_str) {
                            fprintf(fp, " : *data = *(C2RI_T*)&%s", regs[i].name);
                        }
                        else {
                            fprintf(fp, " : *data = (C2RI_T)%s", regs[i].name);
                        }
                        ind(fp, maxNameLength - strlen(regs[i].name), " ");
						fprintf(fp, " ; break;\n");
					}
				}
				ind(fp, icount); fprintf(fp, "default:\n"); ++icount;
                ind(fp, icount); fprintf(fp, "*data = 0;\n");
				ind(fp, icount); fprintf(fp, "printf(\"WARNING on %s::devRead()!!! address (0x%%8x) is invalid!!!\\n\", addr);\n", comp_name);
				ind(fp, icount); fprintf(fp, "return 1; /// readFlag = 0;\n");
				ind(fp, icount); fprintf(fp, "break;\n");
				--icount;
				ind(fp, --icount); fprintf(fp, "}\n");
				ind(fp, icount); fprintf(fp, "this->readFlag = 1;\n");
				ind(fp, icount); fprintf(fp, "return 1;\n");
			}
			/////////////// BIT devWrite(UINT32 data, UINT32 addr) { ... } //////////////
			{
                /// 12/2/18
                CodeGroup cg(fp, icount, "BIT devWrite(C2RI_T data, UINT32 addr)", true);
                ind(fp, icount); fprintf(fp, "switch (UINT32 idx = addr & 0x%x) {\n", (1 << addrBits) - 1); ++icount;
				for (i = 0; i < rsz; ++i) {
                    if (regs[i].accessType == 3) { continue; }
                    ind(fp, icount);
					if (regs[i].accessType == 1) {
						fprintf(fp, "/// case RADDR_%s: skipped... (isReadOnly)\n", regs[i].name);
					}
					else if (regs[i].elementCount > 1 || regs[i].elementCount_str) {
						fprintf(fp, "case RADDR_%s", regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
                        fprintf(fp, " : write_%s(data); break;\n", regs[i].name);
					}
					else {
						fprintf(fp, "case RADDR_%s", regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
						fprintf(fp, " : r_%s", regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
                        /// 12/2/18
                        if (nonIntegralType && regs[i].size_str) {
                            fprintf(fp, " = *(%s*)&data ; break;\n", regs[i].GetTypeName());
                        }
                        else {
                            fprintf(fp, " = (%s)data ; break;\n", regs[i].GetTypeName());
                        }
					}
				}
				ind(fp, icount); fprintf(fp, "default:\n"); ++icount;
				ind(fp, icount); fprintf(fp, "printf(\"WARNING on %s::devWrite()!!! address (0x%%8x) is invalid!!!\\n\", addr);\n", comp_name);
				ind(fp, icount); fprintf(fp, "return 1; /// writeFlag = 0; \n");
				ind(fp, icount); fprintf(fp, "break;\n");
				--icount;
				ind(fp, --icount); fprintf(fp, "}\n");
				ind(fp, icount); fprintf(fp, "this->writeFlag = 1;\n");
				ind(fp, icount); fprintf(fp, "return 1;\n");
			}
            /////////////// void preFsmUser() { ... } //////////////
            {
                CodeGroup cg(fp, icount, "void preFsmUser()", true);
                for (i = 0; i < rsz; ++i) {
                    if (regs[i].accessType == 3) { continue; }
                    if (regs[i].isArray()) { continue; }
                    ind(fp, icount); 
                    fprintf(fp, "%s", regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
                    if (regs[i].accessType == 3) { fprintf(fp, " = 0;\n"); }
                    else { fprintf(fp, " = r_%s;\n", regs[i].name); }
                }
                if (P > 1) { ind(fp, icount); fprintf(fp, "max_phase = 0; max_rw_count = 0; update_phase = 0;\n"); }
                //#define DBG_C2R
#if defined(DBG_C2R)
                if (P > 1) { ind(fp, icount); fprintf(fp, "if (this->readFlag || this->writeFlag) { update_state(); }\n"); }
#endif
            }
            /////////////// void fsmUser() { ... } //////////////
			{
				CodeGroup cg(fp, icount, "void fsmUser()", true);
				for (i = 0; i < rsz; ++i) {
					if (regs[i].exp_str) {
						ind(fp, icount);
                        fprintf(fp, "r_%s", regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
                        fprintf(fp, " = %s;\n", regs[i].exp_str);
					}
				}
#if !defined(DBG_C2R)
                if (P > 1) { ind(fp, icount); fprintf(fp, "if (this->readFlag || this->writeFlag) { update_state(); }\n"); }
#endif
            }
			ind(fp, icount); fprintf(fp, "void fsmIntr(); /// need to define this later....\n");

            fprintf(fp, "#if defined(ENABLE_C2R_CHECK_POINT)\n");
            {
                CodeGroup cg(fp, icount, "void C2R_SetCheckPoint(C2R_CheckPoint *C2R_CP)", true);
                for (i = 0; i < rsz; ++i) {
                    ind(fp, icount);
                    if (regs[i].elementCount > 1 || regs[i].elementCount_str) {
                        fprintf(fp, "C2R_CP_ADD_ARRAY(%s);\n", regs[i].name);
                    }
                    else {
                        fprintf(fp, "C2R_CP_ADD(r_%s);\n", regs[i].name);
                    }
                }
            }
            fprintf(fp, "#endif\n");
		}
	}
};


#endif  // CODE_GEN_REG_IF_H
