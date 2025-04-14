#if !defined(ELF_LOADER_H)
#define ELF_LOADER_H

//#define TEST_RV_BOOT_LOADER

#if 1
#include <string.h>
#include <stdlib.h>
#if !defined(TEST_RV_BOOT_LOADER)
#include <stdio.h>
#endif
#else
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
#endif

#define ELF_ASSERT_ERROR(n)         ELF_Loader::ErrorMessage(n, __FILE__, __FUNCTION__, __LINE__)
#define ELF_ASSERT(n)               do { if (!(n)) { ELF_ASSERT_ERROR(#n); } } while(0)

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

#if 1//defined(TEST_RV_BOOT_LOADER)
void ELprintf(const char* s, ...);
#endif

struct ELF_Loader {
    static int ErrorMessage(const char *n, const char *pathName, const char *funcName, int lineNum) {
        ELprintf("C2R::ERROR!!! %s\n%s, %s:L.%d (aborting...)\n", n, pathName, funcName, lineNum);
        //getchar();
        exit(-1);
        return 0;
    }
#if 0
    struct Info;
    static Info *curInfo;
#endif
    enum ErrorStatus {
#define ES_ENUM
#include "ElfStatus.h"
#undef ES_ENUM
        ES_COUNT,
    };
    static const char * errorStatusName[ES_COUNT];
    static const char * GetErrorStatus(int errorStatus) {
        ELF_ASSERT(errorStatus < 0 || errorStatus >= ES_COUNT);
#if 0
        switch (errorStatus) {
#define ES_CASE
#include "ElfStatus.h"
#undef ES_CASE
        default: return "???";
        }
#else
        return errorStatusName[errorStatus];
#endif
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
#define Value_ERROR (unsigned)(-1)
    struct Value {
        const char * name;
        unsigned int value;
        const char * comment;
#if 0
        void Set(const char* n, unsigned int v, const char* c = "") { name = n; value = v; comment = c; }
        void PrintInfo(const char *t) {
            ELprintf("%12s = %3d [%s:RANGE](%s):\n", t, value, name, comment);
        }
#else
        ///	9/30/15
        int isLowRange, tmpValue;
        void RepairStrings(bool swapNameComment) {
            if (name == 0) { name = ""; }
            if (comment == 0) { comment = ""; }
            if (swapNameComment) {
                const char * tmp = name;
                name = comment;
                comment = tmp;
            }
        }
        void PrintInfo(const char *t) {
            ELprintf("%12s = %3d [%s:RANGE](%s):\n", t, (isLowRange) ? tmpValue : value, name, comment);
        }
#endif
    };
#if 0
    static Value ehtypeArray[ET_COUNT], osabiArray[ELFOSABI_COUNT], machineArray[EM_COUNT];
    static Value shtypeArray[SHT_COUNT], shflagArray[SHF_COUNT], shnumArray[SHN_COUNT];
    static Value stbindArray[STB_COUNT], sttypeArray[STT_COUNT], stvisArray[STV_COUNT];
    static Value dtagArray[DT_COUNT], ptypeArray[PT_COUNT], pflagArray[PF_COUNT];
#endif
#if 1
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
            ELprintf("ValueGroup::GetValue(%s) not found...\n", name);
            for (i = 0; i < count; i++) {
                value[i].PrintInfo("...");
            }
            ELF_ASSERT(0 && "Error in ValueGroup::GetValue!!!");
            return 0;
        }
        void PrintValueMask(int v, unsigned numdigits, bool addSpacing = false) {
            unsigned i, vcount = 0;
            for (i = 0; i < count; i++) {
                if (value[i].value & v) {
                    vcount += 1 + addSpacing;
                }
            }
            for (; vcount < numdigits; vcount++) {
                ELprintf(" ");
            }
            for (i = 0; i < count; i++) {
                if (value[i].value & v) {
                    ELprintf("%s", value[i].name);
                    if (addSpacing) { printf(" "); }
                }
            }
        }
        int GetValueVector(const char * flags) {
            size_t len = strlen(flags), i, j, vector = 0;
            for (i = 0; i < count; i++) {
                for (j = 0; j < len; j++) { if (strcmp(value[i].name, flags + j) == 0) { vector |= value[i].value; } }
            }
            return (int)vector;
        }
    };
#if 1
    static Value ehtypeArray[ET_COUNT], osabiArray[ELFOSABI_COUNT], machineArray[EM_COUNT];
    static Value ptypeArray[PT_COUNT], pflagArray[PF_COUNT];
    static ValueGroup ehtypeGroup, osabiGroup, machineGroup, ptypeGroup, pflagGroup;
#else
    static ValueGroup ehtypeGroup, osabiGroup, machineGroup;
    static ValueGroup shtypeGroup, shflagGroup, shnumGroup, stbindGroup, sttypeGroup, stvisGroup,
        dtagGroup, ptypeGroup, pflagGroup;
#endif
#endif
#if 0
    static Value osabiGroup_GetValue(unsigned v);
    static Value ehtypeGroup_GetValue(unsigned v);
    static Value machineGroup_GetValue(unsigned v);
    static Value ptypeGroup_GetValue(unsigned v);
    static unsigned ptypeGroup_maxNameLength() { return 7; }
    static Value pflagGroup_GetValue(unsigned v);
    static void pflagGroup_PrintValueMask(unsigned v, unsigned numdigits);
#endif
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
    struct Ehdr;
    struct ELF_Data {
        char* data;
        unsigned size;
        unsigned curPos;
        ELF_Data(char* d, unsigned sz) : data(d), size(sz), curPos(0) {}
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
        void PrintInfo(int id);
        void CopySegment(int id);
        static void PrintHeader();
#undef PCPY
#undef PCPY_ALL
    };
    struct PhdrTable {
        Status status;
        size_t segmentCount;
        PhdrTable() { Reset(); }
        bool Parse(Ehdr * ehdr, ELF_Data &elfData, int command);
        ~PhdrTable() { Reset(); }
        void Reset() {
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
        bool Parse(ELF_Data &elfData);
        void PrintMagicNumber() {
            ELprintf("%12s = ", "Magic");
            for (int i = 0; i < EI_NIDENT; i++) { ELprintf("%02x ", e_ident[i]); }
            ELprintf("\n");
        }
        void PrintClass() {
            ELprintf("%12s = ", "Class");
            if (IsElf32())      { ELprintf("ELF32\n"); }
            else if (IsElf64()) { ELprintf("ELF64\n"); }
            else                { ELprintf("<unknown-ELF-class>\n"); }
        }
        void PrintByteOrder() {
            ELprintf("%12s = ", "Byte-Order");
            if (e_ident[EI_DATA] == ELFDATA2LSB)        { ELprintf("Little-endian\n"); }
            else if (e_ident[EI_DATA] == ELFDATA2MSB)   { ELprintf("Big-endian\n"); }
            else                                        { ELprintf("<unknown-endian>\n"); }
        }
        void PrintFileVersion() {
            ELprintf("%12s = ", "File-Version");
            if (e_ident[EI_VERSION] == EV_CURRENT)      { ELprintf("Current-version:\n"); }
            else                                        { ELprintf("<invalid-version>:\n"); }
        }
        void PrintABIVersion() {
            ELprintf("%12s = ", "ABI-Version");
            ELprintf("%3d\n", e_ident[EI_ABIVERSION]);
        }
        void PrintInfo();
        void Reset() { status.Reset(); }
        ~Ehdr() { Reset(); }
    };
    struct Loader {
        Status status;
        ELF_Data elfData;
        Ehdr elfHeader;
        PhdrTable segmentTable;
        Loader(char* edata, unsigned esize);
        void Reset() {
            elfHeader.Reset();
            segmentTable.Reset();
        }
    };
    struct CharArray {
        unsigned size;
        char * data;
        CharArray(unsigned sz) { Create(sz); }
        CharArray(FILE* fp) {
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

};

#endif  // ELF_LOADER_H
