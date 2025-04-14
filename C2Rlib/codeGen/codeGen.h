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

#if !defined(CODE_GEN_H)
#define CODE_GEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <vector>
#include <string>
#include <tuple>

class CodeGen {
public:
    static void ind(FILE *fp, size_t icount, const char *space = "  ") {
        if (icount > 100) { printf("Something went wrong in CodeGen:::ind(icount = %d)\n", (int)icount); return; }
        for (size_t i = 0; i < icount; ++i) { fprintf(fp, "%s", space); }
    }
    FILE *fp;
    void emitDislaimer(const char *in_file_name, int line_num) {
        fprintf(fp, "/*\n"
            "Copyright 2016 New System Vision Research and Development Institute.\n"
            "\n"
            "Licensed under the Apache License, Version 2.0 (the \"License\");\n"
            "you may not use this file except in compliance with the License.\n"
            "You may obtain a copy of the License at\n"
            "\n"
            "http://www.apache.org/licenses/LICENSE-2.0\n"
            "\n"
            "Unless required by applicable law or agreed to in writing, software\n"
            "distributed under the License is distributed on an \"AS IS\" BASIS,\n"
            "WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
            "See the License for the specific language governing permissions and\n"
            "limitations under the License.\n"
            "*/\n\n");
        fprintf(fp, "/**** this is an auto-generated file by C2R_CodeGen utility *****/\n");
        fprintf(fp, "/**** (generated at : %s %s) *****/\n", __DATE__, __TIME__);// , in_file_name, line_num);
        fprintf(fp, "/**** (generated from : %s:L%d) *****/\n\n", in_file_name, line_num);
    }
    char extractFilePath(std::string &filePath) {
        for (int i = (int)filePath.size() - 1; i > 0; --i) {
            char c = filePath[i];
            if (c == '\\' || c == '/') {
                filePath.erase(filePath.begin() + i, filePath.end());
                return c;
            }
        }
        return 0;
    }
    CodeGen(const char *comp_name, const char *in_file_name, int line_num) : fp(nullptr) {
        std::string filePath = in_file_name;
        char sep = extractFilePath(filePath);
        if (!sep) { printf("ERROR in CodeGen!!! filePath is wrong... : %s\n", in_file_name); exit(-1); }
        std::string fname = filePath + sep + comp_name + ".hpp";
        fp = fopen(fname.c_str(), "w");
        if (!fp) { printf("ERROR in CodeGen!!! could not open file (%s)... bye-bye...\n", fname.c_str()); exit(-1); }
        printf("CodeGenRegIF_AXI4L : filename (%s)\n", fname.c_str());
        emitDislaimer(in_file_name, line_num);
    }
    ~CodeGen() { if (fp) { fclose(fp); } }
    class CodeGroup {
        FILE *fp;
        size_t &icount;
        const char *name;
		bool semicolonAtEnd;
		bool newLineAtEnd;
    public:
        CodeGroup(FILE *fp0, size_t &ic, const char *name0, bool noSemicolon, bool newLine = true) : 
			fp(fp0), icount(ic), name(name0), semicolonAtEnd(!noSemicolon), newLineAtEnd(newLine) {
            ind(fp, icount); fprintf(fp, "%s {\n", name); ++icount;
        }
		~CodeGroup() {
            ind(fp, --icount); fprintf(fp, "}%s /// %s\n%s", (semicolonAtEnd) ? ";" : "", name, (newLineAtEnd) ? "\n" : "");
		}
    };
    class MacroGroup {
        std::vector<std::tuple<std::string, std::string, std::string, std::string>> macroList;
    public:
        FILE *fp;
        size_t defLen;
        MacroGroup(FILE *fp0, size_t defaultLen) : fp(fp0), defLen(defaultLen) {}
        void addMacro(const char *mname, const char *mparam, const char *mdef, const char *comment = "") {
            macroList.emplace_back(mname, mparam, mdef, comment);
            fprintf(fp, "#define %*s%s  %s  ", (int)defLen, mname, mparam, mdef);
            if (comment[0]) { fprintf(fp, "/// %s", comment); }
            fprintf(fp, "\n");
        }
        std::tuple<std::string, std::string, std::string, std::string> &operator[](int idx) { return macroList[idx]; }
        size_t size() { return macroList.size(); }
        ~MacroGroup() {
            for (size_t i = 0, e = macroList.size(); i < e; ++i) {
                fprintf(fp, "#undef %*s ///%s %s %s\n", (int)defLen,
                    std::get<0>(macroList[i]).c_str(), std::get<1>(macroList[i]).c_str(),
                    std::get<2>(macroList[i]).c_str(), std::get<3>(macroList[i]).c_str());
            }
        }
    };
};

class CodeGenRegIF : public CodeGen {
public:
    CodeGenRegIF(const char *comp_name, const char *in_file_name, int line_num) : CodeGen(comp_name, in_file_name, line_num) {}
    class RegElement {
    public:
        const char *name;
        /// 1/30/18 : accessType (isReadOnly/isWriteOnly removed)
        int size, elementCount, sign, accessType, structType, bitWidth;   /// accessType = 3 (hidden) : 11/8/18
        /// size : in bytes, structType : true if the register-group is a structure
        const char *size_str;
        const char *elementCount_str;
        const char *exp_str;
        const char *bitWidth_str;
        RegElement(const char *n, int sz, int ec, int isSigned, int at, int bw, const char *expr = nullptr)
        {
            initialize(n, nullptr, sz, nullptr, ec, isSigned, at, nullptr, bw, expr, 0);
        }
        RegElement(const char *n, int sz, int ec, int isSigned, int at, const char *expr = nullptr,
            const char *bw = nullptr, int st = 0)
        {
            initialize(n, nullptr, sz, nullptr, ec, isSigned, at, bw, 0, expr, st);
        }
        RegElement(const char *n, int sz, const char *ec_str, int max_ec, int isSigned, int at,
            const char *bw = nullptr, int st = 0)
        {
            initialize(n, nullptr, sz, ec_str, max_ec, isSigned, at, bw, 0, nullptr, st);
        }
        RegElement(const char *n, const char *sz_str, int max_sz, int ec, int isSigned, int at,
            const char *bw = nullptr, int st = 0)
        {
            initialize(n, sz_str, max_sz, nullptr, ec, isSigned, at, bw, 0, nullptr, st);
        }
        RegElement(const char *n, const char *sz_str, int max_sz, const char *ec_str, int max_ec, int isSigned, int at,
            const char *bw = nullptr, int st = 0)
        {
            initialize(n, sz_str, max_sz, ec_str, max_ec, isSigned, at, bw, 0, nullptr, st);
        }
        void initialize(const char *n, const char *sz_str, int sz, const char *ec_str, int ec, int isSigned, int at,
            const char *bw_str, int bw, const char *expr, int st) {
            name = n; size_str = sz_str; size = sz; elementCount_str = ec_str; elementCount = ec; sign = isSigned;
            accessType = at; bitWidth_str = bw_str, bitWidth = bw; exp_str = expr; structType = st;
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
    void emitSimpleResourceDefinition(std::vector<RegElement> &regs, size_t &icount, size_t maxNameLength) {
        /////////////// declare registers //////////////
        ind(fp, icount); fprintf(fp, "/// register reclaration...\n\n");
        size_t rsz = regs.size(), i;
        for (i = 0; i < rsz; ++i) {
            if (regs[i].accessType == 3) { continue; }
            ind(fp, icount); fprintf(fp, "%-6s %s", regs[i].GetTypeName(), regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
            const char *remarks = (regs[i].accessType == 1) ? " /// (read-only)" : (regs[i].accessType == 2) ? " /// (write-only)" : "";
            if (regs[i].bitWidth_str) {
                fprintf(fp, " _BWT(%s);%s\n", regs[i].bitWidth_str, remarks);
            }
            else if (regs[i].bitWidth) {
                fprintf(fp, " _BW(%d);%s\n", regs[i].bitWidth, remarks);
            }
            else {
                fprintf(fp, " ;%s\n", remarks);
            }
        }
        fprintf(fp, "\n");
    }
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
            else if (regs[i].bitWidth) {
                fprintf(fp, " _TYPE(static_state) _BW(%d);%s\n", regs[i].bitWidth, remarks);
            }
            else {
                fprintf(fp, " _TYPE(static_state);%s\n", remarks);
            }
        }
        fprintf(fp, "\n");
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
                if (i) { fprintf(fp, " = REG_OFS(%s),\n", regs[i - 1].name); }
                else { fprintf(fp, " = 0,\n"); }
            }
        }
        size_t maxTotalBytes = 0;
        for (i = 0; i < rsz; ++i) {
            if (regs[i].accessType == 3) { continue; }
#if 1
            int bytes = regs[i].size * regs[i].elementCount;
            int alignedBytes = (bytes & ~(0x3)) + (((bytes & (0x3)) != 0) * 4);
            maxTotalBytes += alignedBytes;
#else
            int bytes = regs[i].size * regs[i].elementCount;
            maxTotalBytes += bytes + ((bytes % 4) != 0);
#endif
        }
        size_t addrBits = 0, bytes = maxTotalBytes - 1;
        while (bytes) { ++addrBits; bytes >>= 1; }
        ind(fp, icount); fprintf(fp, "/// maxTotalBytes = %d, addrBits = %d\n\n", (int)maxTotalBytes, (int)addrBits);
        return addrBits;
    }
    void emitHeader(std::vector<RegElement> &regs, size_t &icount, const char *comp_name) {
        fprintf(fp, "#if !defined(__LLVM_C2RTL__)\n");
        fprintf(fp, "#define EMIT_HEADER	emit_header(ID)\n");
        fprintf(fp, "#else\n");
        fprintf(fp, "#define EMIT_HEADER\n");
        fprintf(fp, "#endif\n");
        ind(fp, icount); fprintf(fp, "%s() { }\n", comp_name);
        ind(fp, icount); fprintf(fp, "%s(int ID) { EMIT_HEADER; }\n", comp_name);
        size_t rsz = regs.size(), i;
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
    }
};

class MyCodeGenRegIF : public CodeGenRegIF {
public:
    MyCodeGenRegIF(const char *comp_name, const char *comp_header, std::vector<RegElement> &regs,
        const char *in_file_name, int line_num) : CodeGenRegIF(comp_name, in_file_name, line_num) {
        size_t rsz = regs.size(), maxNameLength = 0, i, len;
        for (i = 0; i < rsz; ++i) { len = strlen(regs[i].name); if (maxNameLength < len) { maxNameLength = len; } }
        size_t icount = 0;
        {
            std::string cm = comp_header; cm += "struct "; cm += comp_name;
            CodeGroup cg1(fp, icount, cm.c_str(), false);

            /// 9/27/18
            MacroGroup mg(fp, 20);
            emitSimpleResourceDefinition(regs, icount, maxNameLength);

            emitRegSize(regs, icount, maxNameLength);
            emitRegCount(regs, icount, maxNameLength);
            mg.addMacro("REG_SZ", "(rn)", "(RSZ_##rn * RCNT_##rn)", "reg size");
            size_t addrBits = emitRegAddr(regs, icount, maxNameLength, &mg);

            emitHeader(regs, icount, comp_name);
            /////////////// BIT devReset() { ... } //////////////
            {
                CodeGroup gc(fp, icount, "BIT devReset()", true);
                for (i = 0; i < rsz; ++i) {
                    if (regs[i].accessType == 3) { continue; }
                    if (regs[i].accessType == 1) {
                        ind(fp, icount); fprintf(fp, "/// %s: skipped... (isReadOnly)\n", regs[i].name);
                    }
                    else if (regs[i].elementCount > 1 || regs[i].elementCount_str) {
                        ind(fp, icount);
                        fprintf(fp, "for(int i = 0; i < ");
                        if (regs[i].elementCount_str) { fprintf(fp, "%s", regs[i].elementCount_str); }
                        else { fprintf(fp, "%d", regs[i].elementCount); }
                        fprintf(fp, "; i++) { %s[i] = 0; }\n", regs[i].name);
                    }
                    else {
                        ind(fp, icount);
                        fprintf(fp, "%s", regs[i].name);
                        ind(fp, maxNameLength - strlen(regs[i].name), " ");
                        fprintf(fp, " = 0;\n");
                    }
                }
                ind(fp, icount); fprintf(fp, "return 1;\n");
            }
            /////////////// BIT devRead(UINT32 *data, UINT32 addr) { ... } //////////////
            {
                CodeGroup gc(fp, icount, "BIT devRead(unsigned *data, unsigned addr)", true);
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
                ind(fp, icount); fprintf(fp, "return -1; /// readFlag = 0;\n");
                if (flag) {
                    ind(fp, --icount); fprintf(fp, "}\n");
                    ind(fp, icount); fprintf(fp, "break;\n");
                }
                --icount;
                ind(fp, --icount); fprintf(fp, "}\n");
//                ind(fp, icount); fprintf(fp, "readFlag = 1;\n");
                ind(fp, icount); fprintf(fp, "return 1;\n");
            }
            /////////////// BIT devWrite(UINT32 data, UINT32 addr) { ... } //////////////
            {
                CodeGroup cg(fp, icount, "BIT devWrite(unsigned data, unsigned addr)", true);
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
                        fprintf(fp, " : %s", regs[i].name); ind(fp, maxNameLength - strlen(regs[i].name), " ");
#if 1
                        if (regs[i].bitWidth || regs[i].bitWidth_str) {
                            fprintf(fp, " = (%s)(data & ((1 << ", regs[i].GetTypeName());
                            if (regs[i].bitWidth)   { fprintf(fp, "%d) - 1)); break;\n", regs[i].bitWidth); }
                            else                    { fprintf(fp, "(%s)) - 1)); break;\n", regs[i].bitWidth_str); }
                        }
                        else {
                            fprintf(fp, " = (%s)data ; break;\n", regs[i].GetTypeName());
                        }
#else
                        fprintf(fp, " = (%s)data ; break;\n", regs[i].GetTypeName());
#endif
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
                ind(fp, icount); fprintf(fp, "return -1; /// writeFlag = 0; \n");
                if (flag) {
                    ind(fp, --icount); fprintf(fp, "}\n");
                    ind(fp, icount); fprintf(fp, "break;\n");
                }
                --icount;
                ind(fp, --icount); fprintf(fp, "}\n");
//                ind(fp, icount); fprintf(fp, "writeFlag = 1;\n");
                ind(fp, icount); fprintf(fp, "return 1;\n");
            }
#if 0
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
#endif
        }
    }
};


#define C2R_CODE_GEN(module, vname, name, header, elements)         module vname(name, header, elements, __FILE__, __LINE__)
#define C2R_CODE_GEN_noFO(module, vname, name, header, elements)    module vname(0, 0, name, header, elements, __FILE__, __LINE__)
#define C2R_CODE_GEN_noFO_macro(module, vname, name, header, elements, macros)    module vname(0, 0, name, header, elements, macros, __FILE__, __LINE__)
#define C2R_CODE_GEN_FO(module, vname, name, header, elements)      module vname(0, 1, name, header, elements, __FILE__, __LINE__)
#define C2R_CODE_GEN_NI_noFO(module, vname, name, header, elements)   module vname(1, 0, name, header, elements, __FILE__, __LINE__)
#define C2R_CODE_GEN_BF_noFO(module, vname, name, header, elements)   module vname(2, 0, name, header, elements, __FILE__, __LINE__)

#endif  // CODE_GEN_H
