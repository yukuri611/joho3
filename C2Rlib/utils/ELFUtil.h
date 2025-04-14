#if !defined(ELF_UTILS_H)
#define ELF_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include "C2RUtil.h"


#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_ABIVERSION 8
#define EI_PAD 9
#define EI_NIDENT 16

///	e_ident[EI_CLASS]
#define ELFCLASSNONE	0		//  invalid class
#define ELFCLASS32		1		//  32-bit object
#define ELFCLASS64		2		//  64-bit object

///	e_ident[EI_DATA]
#define ELFDATANONE		0		//  invalid data encoding
#define ELFDATA2LSB		1		//  LSB-first (little endian)
#define ELFDATA2MSB		2		//	MSB-first (big endian)

///	e_type:
#define ET_COUNT	9

///	e_machine:
#define EM_COUNT		101

///	e_version:
#define EV_NONE	 0				//  invalid version
#define EV_CURRENT	 1			//  current version

#define ELFOSABI_COUNT	15
// 	64-255	Architecture-specific value range

#define SHT_COUNT	23
#define SHF_COUNT	12
#define SHN_COUNT	10

#define STB_COUNT	7
#define STT_COUNT	11
#define STV_COUNT	4

#define DT_COUNT	38

#define PT_COUNT	12
#define PF_COUNT	5

typedef unsigned long long  U64;
typedef unsigned int        U32;
typedef unsigned short      U16;
typedef unsigned char       U8;
typedef long long           S64;
typedef int                 S32;
typedef short               S16;
typedef char                S8;

typedef U32 Elf32_Addr;
typedef U16 Elf32_Half;
typedef U32 Elf32_Off;
typedef U32 Elf32_Sword;
typedef U32 Elf32_Word;

typedef U64 Elf64_Addr;
typedef U16 Elf64_Half;
typedef U64 Elf64_Off;
typedef U32 Elf64_Sword;
typedef U32 Elf64_Word;
typedef U64 Elf64_Xword;
typedef U64 Elf64_Sxword;

struct ELF_Parser {
    struct Info;
    static Info *curInfo;
    enum ErrorStatus {
#define ES_ENUM
#include "ElfStatus.h"
#undef ES_ENUM
        ES_COUNT,
    };
    static const char * errorStatusName[ES_COUNT];
    static const char * GetErrorStatus(int errorStatus) {
        C2R_ASSERT(errorStatus < 0 || errorStatus >= ES_COUNT);
        return errorStatusName[errorStatus];
    }
    struct Status {
        int errorFlag, pos, val;
        Status() { Reset(); }
        void Reset() {
            errorFlag = ES_NO_ERROR;
            pos = 0;
            val = 0;
        }
    };
    struct Value {
        const char * name;
        unsigned int value;
        const char * comment;
        int isLowRange, tmpValue;
        ///	9/30/15
        void RepairStrings(bool swapNameComment) {
            if (name == 0) { name = ""; }
            if (comment == 0) { comment = ""; }
            if (swapNameComment) {
                const char * tmp = name;
                name = comment;
                comment = tmp;
            }
        }
        void PrintInfo(std::stringstream &s, const char *t) {
            if (isLowRange) {
                s << C2R::SF("%12s = ", t) << C2R::SF("%3d [", tmpValue) << name << ":RANGE](" << comment << "):\n";
            }
            else {
                s << C2R::SF("%12s = ", t) << C2R::SF("%3d [", value) << name << ":RANGE](" << comment << "):\n";
            }
        }
    };
    static Value ehtypeArray[ET_COUNT], osabiArray[ELFOSABI_COUNT], machineArray[EM_COUNT];
    static Value shtypeArray[SHT_COUNT], shflagArray[SHF_COUNT], shnumArray[SHN_COUNT];
    static Value stbindArray[STB_COUNT], sttypeArray[STT_COUNT], stvisArray[STV_COUNT];
    static Value dtagArray[DT_COUNT], ptypeArray[PT_COUNT], pflagArray[PF_COUNT];
    struct ValueGroup {
        size_t count, maxNameLength;
        Value * value;
        ValueGroup(int c, Value * v, bool swapNameComment = false) {
            count = c;
            value = v;
            unsigned i;
            maxNameLength = 0;
            for (i = 0; i < count; i++) {
                value[i].RepairStrings(swapNameComment);
                size_t l = strlen(value[i].name);
                if (maxNameLength < l) { maxNameLength = l; }
            }
        }
        Value * GetValue(unsigned v) {
            unsigned i;
            for (i = 0; i < count; i++) {
                if (value[i].isLowRange) {
                    if (v >= value[i].value && v <= value[i + 1].value) { value[i].tmpValue = v; return &value[i]; }
                    i++;
                }
                else if (value[i].value == v) { return &value[i]; }
            }
            return 0;
        }
        Value * GetValue(const char * name) {
            unsigned i;
            for (i = 0; i < count; i++) { if (strcmp(value[i].name, name) == 0) { return &value[i]; } }
            printf("ValueGroup::GetValue(%s) not found...\n", name);
            std::stringstream ss;
            for (i = 0; i < count; i++) {
                value[i].PrintInfo(ss, "...");
            }
            std::cout << ss.str();
            C2R_ASSERT(0 && "Error in ValueGroup::GetValue!!!");
            return 0;
        }
        ///	10/1/15
        std::stringstream & GetValueMask(int v, std::stringstream & s, bool addSpacing = false) {
            s.str("");
            unsigned i;
            for (i = 0; i < count; i++) {
                if (value[i].value & v) {
                    s << value[i].name;
                    if (addSpacing) { s << " "; }
                }
            }
            return s;
        }
        int GetValueVector(const char * flags) {
            size_t len = strlen(flags), i, j, vector = 0;
            for (i = 0; i < count; i++) {
                for (j = 0; j < len; j++) { if (strcmp(value[i].name, flags + j) == 0) { vector |= value[i].value; } }
            }
            return (int)vector;
        }
    };
    static ValueGroup ehtypeGroup, osabiGroup, machineGroup;
    static ValueGroup shtypeGroup, shflagGroup, shnumGroup, stbindGroup, sttypeGroup, stvisGroup,
        dtagGroup, ptypeGroup, pflagGroup;
    struct Elf32_Ehdr {
        Elf32_Half e_type;
        Elf32_Half e_machine;
        Elf32_Word e_version;
        Elf32_Addr e_entry;
        Elf32_Off e_phoff;
        Elf32_Off e_shoff;
        Elf32_Word e_flags;
        Elf32_Half e_ehsize;
        Elf32_Half e_phentsize;
        Elf32_Half e_phnum;
        Elf32_Half e_shentsize;
        Elf32_Half e_shnum;
        Elf32_Half e_shstrndx;
    };
    struct Elf64_Ehdr {
        Elf64_Half e_type;
        Elf64_Half e_machine;
        Elf64_Word e_version;
        Elf64_Addr e_entry;
        Elf64_Off e_phoff;
        Elf64_Off e_shoff;
        Elf64_Word e_flags;
        Elf64_Half e_ehsize;
        Elf64_Half e_phentsize;
        Elf64_Half e_phnum;
        Elf64_Half e_shentsize;
        Elf64_Half e_shnum;
        Elf64_Half e_shstrndx;
    };
    struct Elf_Ehdr : Elf64_Ehdr {
#define ECPY(m)	e_##m = h->e_##m
#define ECPY_ALL	\
	ECPY(type);		ECPY(machine);		ECPY(version);	ECPY(entry);		ECPY(phoff);	ECPY(shoff); \
	ECPY(flags);	ECPY(ehsize);	ECPY(phentsize);	ECPY(phnum);	ECPY(shentsize);	ECPY(shnum); \
	ECPY(shstrndx);
        void Copy(Elf32_Ehdr * h) { ECPY_ALL }
        void Copy(Elf64_Ehdr * h) { ECPY_ALL }
#undef ECPY
#undef ECPY_ALL
    };
    struct Elf32_Shdr {
        Elf32_Word	sh_name;
        Elf32_Word	sh_type;
        Elf32_Word	sh_flags;
        Elf32_Addr	sh_addr;
        Elf32_Off	sh_offset;
        Elf32_Word	sh_size;
        Elf32_Word	sh_link;
        Elf32_Word	sh_info;
        Elf32_Word	sh_addralign;
        Elf32_Word	sh_entsize;
    };
    struct Elf64_Shdr {
        Elf64_Word	sh_name;
        Elf64_Word	sh_type;
        Elf64_Xword	sh_flags;
        Elf64_Addr	sh_addr;
        Elf64_Off	sh_offset;
        Elf64_Xword	sh_size;
        Elf64_Word	sh_link;
        Elf64_Word	sh_info;
        Elf64_Xword	sh_addralign;
        Elf64_Xword	sh_entsize;
    };
    struct Image {
        unsigned char * mem;
        unsigned int size, addr;
        Image(int sz, int ad) {
            addr = ad;
            size = ((sz + 3) >> 2) << 2;
            mem = new unsigned char[size];
            memset(mem, 0, size);
        }
        ~Image() { delete[] mem; }
        bool IsValidAddr(unsigned int addr0) {
            return (unsigned int)addr <= addr0 && (unsigned int)(addr + size) > addr0;
        }
    };
    struct Elf_Shdr : Elf64_Shdr {
        const char * sh_name_str;
        Image *image;
        struct Field {
            unsigned sh_alloc   : 1;
            unsigned executable : 1;
            Field() : sh_alloc(0), executable(0) { }
            void Reset() { sh_alloc = 0; executable = 0; }
        } field;
        Elf_Shdr() : sh_name_str(0), image(0) { }
        ~Elf_Shdr() { if (image) { delete image; } }

#define SCPY(m)	sh_##m = s->sh_##m
#define SCPY_ALL	\
	SCPY(name);	SCPY(type);	SCPY(flags);	SCPY(addr);			SCPY(offset);	\
	SCPY(size);	SCPY(link);	SCPY(info);		SCPY(addralign);	SCPY(entsize);	\
	sh_name_str = 0; image = 0; field.Reset(); 
        void Copy(Elf32_Shdr * s) { SCPY_ALL }
        void Copy(Elf64_Shdr * s) { SCPY_ALL }
        void PrintInfo(std::stringstream & s, int id, size_t maxStringLength);
        static size_t PrintHeader(std::stringstream & s, size_t maxStringLength);
        void PrintImage(FILE *fp, size_t maxNameLength);
#undef SCPY
#undef SCPY_ALL
    };
    struct Ehdr;
#if 1   /// 3/31/22
    struct ELF_Array {
        char* data;
        unsigned size;
        unsigned curPos;
//        ELF_Array(char* d, unsigned sz) : data(d), size(sz), curPos(0) {}
        ELF_Array() : data(0), size(0), curPos(0) {}
        ELF_Array(char* d, unsigned sz) { Set(d, sz); }
        void Set(char* d, unsigned sz) { data = d; size = sz; curPos = 0; }
        size_t Read(void *buf, size_t size, size_t n) {
            size_t sz = (unsigned)(size * n);
            memcpy(buf, &data[curPos], sz);
            curPos += (unsigned)sz;
            return sz;
        }
        void Seek(long pos) {
            curPos = pos;
        }
    };
    struct ELF_Data {
        FILE *fp;
        ELF_Array *elfArray;
        ELF_Data(FILE *fp0) : fp(fp0), elfArray(NULL) {}
        ELF_Data(ELF_Array *ea) : fp(NULL), elfArray(ea) {}
        size_t Read(void *buf, size_t size, size_t n) {
            if (fp) { return fread(buf, size, n, fp); }
            else { return elfArray->Read(buf, size, n); }
        }
        void Seek(long pos) {
            if (fp) { rewind(fp); fseek(fp, pos, SEEK_SET); }
            else { elfArray->Seek(pos); }
        }
    };
#endif
    struct ShdrTable {
        Status status;
        Elf_Shdr * shdr, *symbolSection, *dynSymSection, *dynSection;
        std::vector<Elf_Shdr*> relocSectionList, sectionList;
        std::string sectionStr, symbolStr, dynSymStr;
        size_t sectionCount, maxSectionStringLength;
        ShdrTable() {
            shdr = 0;
            Reset();
        }
        bool Parse(Ehdr * ehdr, ELF_Data elfData);
        bool Parse(Info * elfInfo, FILE * fp) { bool flag = Parse(&elfInfo->elfHeader, ELF_Data(fp)); EmitMessage(elfInfo); return flag; }
        bool Parse(Info * elfInfo, ELF_Array * ea) { return Parse(&elfInfo->elfHeader, ELF_Data(ea)); }
        void EmitMessage(Info * elfInfo);
        ~ShdrTable() { Reset(); }
        void Reset() {
            if (shdr) { delete[] shdr; }
            shdr = 0;
            symbolSection = 0;
            dynSymSection = 0;
            dynSection = 0;
            relocSectionList.clear();
            sectionList.clear();
            sectionStr = "";
            symbolStr = "";
            dynSymStr = "";
            sectionCount = 0;
            maxSectionStringLength = 0;
            status.Reset();
        }
        bool CreateSections(FILE *fp, FILE *fout);
    };

#define ELF_ST_BIND(i)			((i)>>4)
#define ELF_ST_TYPE(i)			((i)&0xf)
#define ELF_ST_INFO(b,t)		(((b)<<4)+((t)&0xf))
#define ELF_ST_VISIBILITY(o)	((o)&0x3)

    struct Elf32_Sym {
        Elf32_Word	st_name;
        Elf32_Addr	st_value;
        Elf32_Word	st_size;
        unsigned char	st_info;
        unsigned char	st_other;
        Elf32_Half	st_shndx;
    };
    struct Elf64_Sym {
        Elf64_Word	st_name;
        unsigned char	st_info;
        unsigned char	st_other;
        Elf64_Half	st_shndx;
        Elf64_Addr	st_value;
        Elf64_Xword	st_size;
    };
    struct Elf_Sym : Elf64_Sym {
        const char *	st_name_str;
        unsigned char	st_bind;
        unsigned char	st_type;
        unsigned char	st_vis;

#define MCPY(m)	st_##m = s->st_##m
#define MCPY_ALL											\
	MCPY(name);		MCPY(value);	MCPY(size);	MCPY(info);	\
	MCPY(other);	MCPY(shndx);							\
	st_bind = ELF_ST_BIND(st_info);							\
	st_type = ELF_ST_TYPE(st_info);							\
	st_vis = ELF_ST_VISIBILITY(st_other);					\
	st_name_str = 0;
        void Copy(Elf32_Sym * s) { MCPY_ALL }
        void Copy(Elf64_Sym * s) { MCPY_ALL }
        void PrintInfo(std::stringstream & s, int id, size_t maxStringLength, ShdrTable * sectionTable);
        static size_t PrintHeader(std::stringstream & s, size_t maxStringLength, Elf_Shdr * symbolSection);
#undef MCPY
#undef MCPY_ALL
    };
    struct SymTable {
        Status status;
        Elf_Sym * symTable;
        Elf_Shdr * symbolSection;
        size_t symbolCount, maxSymbolStringLength;
        SymTable() { symTable = 0; Reset(); }
        bool Parse(Info * elfInfo, FILE * fp, Elf_Shdr * symSect, const char * symbolStringTable, bool showAll);
        ~SymTable() { Reset(); }
        void Reset() {
            if (symTable) { delete[] symTable; }
            symTable = 0;
            symbolCount = 0;
            symbolSection = 0;
            maxSymbolStringLength = 0;
            status.Reset();
        }
    };
    struct Elf32_Rel {
        Elf32_Addr	r_offset;
        Elf32_Word	r_info;
    };
    struct Elf32_Rela {
        Elf32_Addr	r_offset;
        Elf32_Word	r_info;
        Elf32_Sword	r_addend;
    };
    struct Elf64_Rel {
        Elf64_Addr	r_offset;
        Elf64_Xword	r_info;
    };
    struct Elf64_Rela {
        Elf64_Addr		r_offset;
        Elf64_Xword		r_info;
        Elf64_Sxword	r_addend;
    };
    struct Elf_Rel : Elf64_Rela {
        bool addend_enabled;
        U32 r_sym;
        U32 r_type;

#define ELF32_R_SYM(i)	  ((i)>>8)
#define ELF32_R_TYPE(i)   ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

#define ELF64_R_SYM(i)    ((i)>>32)
#define ELF64_R_TYPE(i)   ((i)&0xffffffffL)
#define ELF64_R_INFO(s,t) (((s)<<32)+((t)&0xffffffffL))

        void Copy(Elf32_Rel * r) {
            r_sym = ELF32_R_SYM(r->r_info);
            r_type = ELF32_R_TYPE(r->r_info);
            r_info = r->r_info;
            r_offset = r->r_offset;
            addend_enabled = false;
        }
        void Copy(Elf32_Rela * r) {
            r_sym = ELF32_R_SYM(r->r_info);
            r_type = ELF32_R_TYPE(r->r_info);
            r_info = r->r_info;
            r_offset = r->r_offset;
            r_addend = r->r_addend;
            addend_enabled = true;
        }
        void Copy(Elf64_Rel * r) {
            r_sym = ELF64_R_SYM(r->r_info);
            r_type = ELF64_R_TYPE(r->r_info);
            r_info = r->r_info;
            r_offset = r->r_offset;
            addend_enabled = false;
        }
        void Copy(Elf64_Rela * r) {
            r_sym = ELF64_R_SYM(r->r_info);
            r_type = ELF64_R_TYPE(r->r_info);
            r_info = r->r_info;
            r_offset = r->r_offset;
            r_addend = r->r_addend;
            addend_enabled = true;
        }
        void PrintInfo(std::stringstream & s, int id, SymTable * symTable);
        static size_t PrintHeader(std::stringstream & s, Elf_Shdr * relocSection);
    };
    struct RelocTable {
        Status status;
        Elf_Rel * relocTable;
        Elf_Shdr * relocSection;
        SymTable * symTable;
        size_t relocCount;
        bool Parse(Info * elfInfo, FILE * fp, Elf_Shdr * relSect);
        RelocTable() { relocTable = 0; Reset(); }
        void Reset() {
            if (relocTable) { delete[] relocTable; }
            relocTable = 0;
            relocSection = 0;
            relocCount = 0;
            symTable = 0;
            status.Reset();
        }
        ~RelocTable() { Reset(); }
    };
    struct Elf32_Dyn {
        Elf32_Sword	d_tag;
        union {
            Elf32_Word	d_val;
            Elf32_Addr	d_ptr;
        } d_un;
    };
    struct Elf64_Dyn {
        Elf64_Sxword	d_tag;
        union {
            Elf64_Xword	d_val;
            Elf64_Addr	d_ptr;
        } d_un;
    };
    struct Elf_Dyn : Elf64_Dyn {
        const char * name;
        void Copy(Elf32_Dyn * d) {
            d_tag = d->d_tag;
            d_un.d_val = d->d_un.d_val;
            name = 0;
        }
        void Copy(Elf64_Dyn * d) {
            d_tag = d->d_tag;
            d_un.d_val = d->d_un.d_val;
            name = 0;
        }
        void PrintInfo(std::stringstream & s, int id);
        static size_t PrintHeader(std::stringstream & s, Elf_Shdr * dynSection);
    };
    struct DynTable {
        Status status;
        Elf_Dyn * dynTable;
        Elf_Shdr * dynSection;
        int dynCount;
        bool Parse(Info * elfInfo, FILE * fp, Elf_Shdr * dynSect, const char * symbolStringTable);
        DynTable() { dynTable = 0; Reset(); }
        void Reset() {
            if (dynTable) { delete dynTable; }
            dynTable = 0;
            dynSection = 0;
            dynCount = 0;
            status.Reset();
        }
        ~DynTable() { Reset(); }
    };
    struct Elf32_Phdr {
        Elf32_Word	p_type;
        Elf32_Off	p_offset;
        Elf32_Addr	p_vaddr;
        Elf32_Addr	p_paddr;
        Elf32_Word	p_filesz;
        Elf32_Word	p_memsz;
        Elf32_Word	p_flags;
        Elf32_Word	p_align;
    };
    struct Elf64_Phdr {
        Elf64_Word	p_type;
        Elf64_Word	p_flags;
        Elf64_Off	p_offset;
        Elf64_Addr	p_vaddr;
        Elf64_Addr	p_paddr;
        Elf64_Xword	p_filesz;
        Elf64_Xword	p_memsz;
        Elf64_Xword	p_align;
    };
    struct Elf_Phdr : Elf64_Phdr {
#define PCPY(m)	p_##m = p->p_##m
#define PCPY_ALL	\
			PCPY(type);		PCPY(flags);	PCPY(offset);	PCPY(vaddr);	\
			PCPY(paddr);	PCPY(filesz);	PCPY(memsz);	PCPY(align);
        void Copy(Elf32_Phdr * p) { PCPY_ALL }
        void Copy(Elf64_Phdr * p) { PCPY_ALL }
        void PrintInfo(std::stringstream & s, int id, ShdrTable * sectionTable);
        static size_t PrintHeader(std::stringstream & s);
#undef PCPY
#undef PCPY_ALL
    };
    struct PhdrTable {
        Status status;
        Elf_Phdr * phdr;
        size_t segmentCount;
        char * interpName;
        Elf_Phdr * segInterp;
        PhdrTable() { phdr = 0; interpName = 0; segInterp = 0; Reset(); }
        /// 11/14/21
        bool Parse(Ehdr * ehdr, ELF_Data elfData);
        bool Parse(Info * elfInfo, FILE * fp) { bool flag = Parse(&elfInfo->elfHeader, ELF_Data(fp)); EmitMessage(elfInfo); return flag; }
        bool Parse(Info * elfInfo, ELF_Array * ea) { return Parse(&elfInfo->elfHeader, ELF_Data(ea)); }
        void EmitMessage(Info * elfInfo);
        ~PhdrTable() { Reset(); }
        void Reset() {
            if (phdr) { delete[] phdr; }
            if (interpName) { delete[] interpName; }
            phdr = 0;
            interpName = 0;
            segmentCount = 0;
            status.Reset();
        }
    };
    struct Ehdr {
        Status status;
        unsigned char e_ident[EI_NIDENT];
        Elf_Ehdr ehdr;
        bool IsElf32() { return e_ident[EI_CLASS] == ELFCLASS32; }
        bool IsElf64() { return e_ident[EI_CLASS] == ELFCLASS64; }
        bool IsElfNone() { return e_ident[EI_CLASS] == ELFCLASSNONE; }
        void EmitMessage();
        bool Parse(FILE * fp) { bool flag = Parse(ELF_Data(fp)); EmitMessage(); return flag; }
        bool Parse(ELF_Array *ea) { return Parse(ELF_Data(ea)); }
        bool Parse(ELF_Data elfData);
        void PrintMagicNumber(std::stringstream & s) {
            s << C2R::SF("%12s = ", "Magic");
            for (int i = 0; i < EI_NIDENT; i++) { s << C2R::SF("%02x ", e_ident[i]); }
            s << "\n";
        }
        void PrintClass(std::stringstream & s) {
            s << C2R::SF("%12s = ", "Class");
            if (IsElf32())      { s << "ELF32\n"; }
            else if (IsElf64()) { s << "ELF64\n"; }
            else                { s << "<unknown-ELF-class>\n"; }
        }
        void PrintByteOrder(std::stringstream & s) {
            s << C2R::SF("%12s = ", "Byte-Order");
            if (e_ident[EI_DATA] == ELFDATA2LSB)      { s << "Little-endian\n"; }
            else if (e_ident[EI_DATA] == ELFDATA2MSB) { s << "Big-endian\n"; }
            else                                      { s << "<unknown-endian>\n"; }
        }
        void PrintFileVersion(std::stringstream & s) {
            s << C2R::SF("%12s = ", "File-Version");
            if (e_ident[EI_VERSION] == EV_CURRENT)  { s << "Current-version:\n"; }
            else                                    { s << "<invalid-version>:\n"; }
        }
        void PrintABIVersion(std::stringstream & s) {
            s << C2R::SF("%12s = ", "ABI-Version") << C2R::SF("%3d\n", e_ident[EI_ABIVERSION]);
        }
        void PrintInfo(std::stringstream & s);
        void Reset() { status.Reset(); }
        ~Ehdr() { Reset(); }
#define PRINT_ELF_PARAM(s, n)		\
    s << C2R::SF("%12s = ", #n) << C2R::SF("%10d (", ehdr.n) << C2R::SF("%08x):\n", ehdr.n)
    };
    /// 4/11/22
    struct MemoryInfo {
        unsigned char *mem;
        size_t wordSize, wordByteNum, wordByteNumBits;
        size_t printfAddrWidth, printfDataWidth, segmentCount;
        size_t installedSectionEndAddr; /// 1/12/24
        size_t addrOffset;  /// 11/17/24
        FILE *fp[8];
        size_t getTotalByteNum() { return wordSize << wordByteNumBits; }
        MemoryInfo() {
            mem = 0; wordSize = 0; wordByteNum = 0; wordByteNumBits = 0; printfAddrWidth = 0; printfDataWidth = 0; segmentCount = 0;
            installedSectionEndAddr = 0;
            addrOffset = 0;
            for (unsigned i = 0; i < 8; ++i) { fp[i] = 0; }
        }
        void Set(unsigned char *m, unsigned sz, int aw, int dw, int sc, FILE *fp0[], int wordByteCountBits = 2) {
            mem = m; wordSize = sz; printfAddrWidth = aw; printfDataWidth = dw; segmentCount = sc;
            wordByteNumBits = wordByteCountBits;
            wordByteNum = (size_t)1 << wordByteNumBits;
#if 1	///	4/14/22 : debugging...
		    if (!(printfDataWidth == wordByteNum * 2 || printfDataWidth == 2)) {
			    printf("printfDataWidth = %d, wordByteNum = %d, segmentCount = %d, wordSize = %d\n",
                    (unsigned)printfDataWidth, (unsigned)wordByteNum, (unsigned)segmentCount, (unsigned) wordSize);
		    }
#endif
            C2R_ASSERT(printfDataWidth == wordByteNum * 2 || printfDataWidth == 2);   /// full word or char
            C2R_ASSERT(segmentCount == 1 || segmentCount == wordByteNum);
            int i;
            for (i = 0; i < wordByteNum; ++i) { fp[i] = fp0[i]; }
        }
        void Close() {
            int i;
            for (i = 0; i < (int)segmentCount; ++i) {
                if (fp[i]) { fclose(fp[i]); }
            }
        }
        template<typename T> void DumpFile(U64 minAddr, U64 sh_size) {
            C2R_ASSERT(sizeof(T) == wordByteNum);
            C2R_ASSERT((minAddr & (wordByteNum - 1)) == 0);
            T *pval = (T *)&mem[minAddr];
            T addr = ((T)minAddr) >> wordByteNumBits;
            const char* fstr = (wordByteNum == 8) ? "@%0*llx %0*llx\n" : "@%0*x %0*x\n";
            for (size_t i = 0; i < sh_size; i += wordByteNum, ++pval, ++addr) {
                T val = *pval;
                for (size_t j = 0; j < segmentCount; j++) {
                    T v = (printfDataWidth == wordByteNum * 2) ?
                        val : (val >> (j << 3)) & ((1 << (printfDataWidth << 2)) - 1);
                    if (fp[j]) {
                        fprintf(fp[j], fstr, (int)printfAddrWidth, addr, (int)printfDataWidth, v);
                    }
                }
            }
        }
    };
    struct HostSymbols {    /// 11/18/24
        Elf64_Addr tohost, fromhost;
        void Reset() { tohost = 0; fromhost = 0; }
    };
    struct Info {
        Status status;
        MemoryInfo dmemInfo, pmemInfo;
        std::string projectDescriptor;
        Ehdr elfHeader;
        ShdrTable sectionTable;
        PhdrTable segmentTable;
        SymTable symbolTable, dynSymTable;
        DynTable dynTable;
        std::vector<RelocTable> relocTableList;
        static bool dontPrintParseLog;
#if 1   /// 11/18/24
        static HostSymbols hostSym;
#else
        static Elf64_Addr isatest_tohost;   /// 6/4/24
#endif
        /// 11/4/21
        Info();// { fout = 0; C2R_ASSERT(!curInfo); curInfo = this; }
        ~Info();// { if (fout) { fclose(fout); fout = 0; } C2R_ASSERT(curInfo == this); curInfo = 0; }
        bool Parse(FILE * fp);
        bool ParseProgramHeader(FILE *fp);
        bool InstallMemory(int memDumpFlag, void(*p)(unsigned ir, unsigned addr) = nullptr);
        void printDissassembler(void(*p)(unsigned ir, unsigned addr), unsigned char* image, unsigned addr, unsigned sz);
        bool InstallSection(Elf_Shdr * section, MemoryInfo *mi);
        void Reset() {
            elfHeader.Reset();
            sectionTable.Reset();
            segmentTable.Reset();
            symbolTable.Reset();
            dynSymTable.Reset();
            dynTable.Reset();
            relocTableList.clear();
        }
        void ShowErrorStatus(std::stringstream & s) {
            s << "error status: info(" << GetErrorStatus(status.errorFlag) << "), ehdr("
                << GetErrorStatus(elfHeader.status.errorFlag) << "), shdr("
                << GetErrorStatus(sectionTable.status.errorFlag) << ", pos = "
                << sectionTable.status.pos << ", val = " << sectionTable.status.val << "("
                << std::hex << sectionTable.status.val << "))\n";
            s << "error status: sym(" << GetErrorStatus(symbolTable.status.errorFlag) << ", pos = "
                << symbolTable.status.pos << ", val = " << symbolTable.status.val << "("
                << std::hex << symbolTable.status.val << ")), dynSym("
                << GetErrorStatus(dynSymTable.status.errorFlag) << ", pos = "
                << dynSymTable.status.pos << ", val = " << dynSymTable.status.val << "("
                << std::hex << dynSymTable.status.val << ")), dyn(" << GetErrorStatus(dynTable.status.errorFlag)
                << ", pos = " << dynTable.status.pos << ", val = " << dynSymTable.status.val << "("
                << std::hex << dynSymTable.status.val << "))\n";
            if (fout) { fprintf(fout, "%s", s.str().c_str()); }
        }
        static bool CompareUnsigned(unsigned int a, unsigned b) { return a > b; }
        Elf_Sym* GetSymbolAddress(const char *symbolName);
    };
    static bool CompareSection(Elf_Shdr * a, Elf_Shdr * b) { return a->sh_addr < b->sh_addr; }
    static bool CompareSymbol(Elf_Sym * a, Elf_Sym * b) { return a->st_value < b->st_value; }
    static FILE * fout;
    static void OpenOutputFile();
    static void CloseOutputFile();
    static void PrintTail(std::stringstream &s, size_t len) {
        size_t ll = len;
        while (ll > 0) { s << "="; ll--; }
        s << "\n";
    }
    struct Loader {
        Status status;
        ELF_Array elfArray;
        Ehdr elfHeader;
        PhdrTable segmentTable;
        Elf64_Addr maxAllocAddr;
        Loader(char* edata, unsigned esize) { Create(edata, esize); }
        Loader() {}
        void Create(char* edata, unsigned esize);
        void Reset() {
            elfHeader.Reset();
            segmentTable.Reset();
        }
        bool ComputeMaxAllocAddr();
    };
    struct CharArray {
        unsigned size;
        char * data;
        CharArray() : size(0), data(NULL) {}
        CharArray(unsigned sz) { Create(sz); }
        CharArray(FILE* fp) : size(0), data(NULL) {
            Create(fp);
        }
        void Create(FILE* fp) {
            C2R_ASSERT(!size && !data && fp);
            rewind(fp);
            fseek(fp, 0L, SEEK_END);
            Create((unsigned)ftell(fp));
            rewind(fp);
            fread(data, 1, size, fp);
        }
        void Create(unsigned sz) {
            size = sz;
            data = new char[size];
        }
        ~CharArray() { delete[] data; }
    };

    struct ElfMemoryInstaller {
        const char *elfFileName, * memModuleName, *RTL_V_MemInitDir_Name, *dmemName, *pmemName;
        unsigned wordByteNum;
        void* dmem, *pmem;
        unsigned dmemAddrBitNum, pmemAddrBitNum;
        void* pc, *sp;
        int memDumpFlag, pc_offset;
        unsigned dmemAddrOffset, pmemAddrOffset;
        void(*printRVInst)(unsigned ir, unsigned addr);
        void(*addSymbolMap)(unsigned addr, const char* symName);
        ElfMemoryInstaller(const char* fname, const char* memName, const char* vdir, const char* dmName, const char* pmName,
            unsigned byteNum, void* dm, void* pm, unsigned dabits, unsigned pabits,
            void* pc0, void* sp0, int mdumpFlag, int pc_ofs)
            : elfFileName(fname), memModuleName(memName), RTL_V_MemInitDir_Name(vdir), dmemName(dmName), pmemName(pmName),
            wordByteNum(byteNum), dmem(dm), pmem(pm), dmemAddrBitNum(dabits), pmemAddrBitNum(pabits), pc(pc0), sp(sp0),
            memDumpFlag(mdumpFlag), pc_offset(pc_ofs), pmemAddrOffset(0), dmemAddrOffset(0), printRVInst(nullptr),
            addSymbolMap(nullptr) {}    /// 1/6/2023
        ElfMemoryInstaller(const char* fname, const char* memName, const char* vdir, const char* mName,
            unsigned byteNum, void* m, unsigned abits,
            void* pc0, void* sp0, int mdumpFlag, int pc_ofs, void(*p)(unsigned ir, unsigned addr) = nullptr,
            void(*addSymMap)(unsigned addr, const char* symName) = nullptr)
            : elfFileName(fname), memModuleName(memName), RTL_V_MemInitDir_Name(vdir), dmemName(mName), pmemName(""),
            wordByteNum(byteNum), dmem(m), pmem(nullptr), dmemAddrBitNum(abits), pmemAddrBitNum(0), pc(pc0), sp(sp0),
            memDumpFlag(mdumpFlag), pc_offset(pc_ofs), pmemAddrOffset(0), dmemAddrOffset(0), printRVInst(p),
            addSymbolMap(addSymMap) {
        }    /// 1/6/2023
        void createMemInitFile();
        void createRV32BinFile(ELF_Parser::Info& elfInfo);
        void setup(const char* fname, const char* memName, const char* vdir, const char* mName,
            unsigned byteNum, void* m, unsigned abits,
            void* pc0, void* sp0, int mdumpFlag, int pc_ofs, void(*p)(unsigned ir, unsigned addr) = nullptr,
            void(*addSymMap)(unsigned addr, const char* symName) = nullptr) {
            elfFileName = fname; memModuleName = memName; RTL_V_MemInitDir_Name = vdir; dmemName = mName; pmemName = "";
            wordByteNum = byteNum; dmem = m; pmem = nullptr; dmemAddrBitNum = abits; pmemAddrBitNum = 0; pc = pc0; sp = sp0;
            memDumpFlag = mdumpFlag; pc_offset = pc_ofs; pmemAddrOffset = 0; dmemAddrOffset = 0; printRVInst = p;
            addSymbolMap = addSymMap;
        }
        void openMemDumpFile(int type, FILE **fp, int *segmentCount, int *printfAddrWidth, int *printfDataWidth, int doOpen);
        void InstallElf();
    };
};

#endif  // ELF_UTILS_H
