
#include "ELFUtil.h"

const char * ELF_Parser::errorStatusName[ES_COUNT] = {
#define ES_NAME
#include "ElfStatus.h"
#undef ES_NAME
};

ELF_Parser::Value ELF_Parser::ehtypeArray[ET_COUNT] = {
    { "NONE",	0,		"No file type" },
    { "REL",	1,		"Relocatable file" },
    { "EXEC",	2,		"Executable file" },
    { "DYN",	3,		"Shared object file" },
    { "CORE",	4,		"Core file" },
    { "LOOS",	0xfe00,	"Operating system-specific", 1 },
    { "HIOS",	0xfeff,	"Operating system-specific" },
    { "LOPROC",	0xff00,	"Processor-specific", 1 },
    { "HIPROC",	0xffff,	"Processor-specific" },
};
ELF_Parser::Value ELF_Parser::osabiArray[ELFOSABI_COUNT] = {
    { "NONE",	0,	"No extensions or unspecified" },
    { "HPUX",	1,	"Hewlett-Packard HP-UX" },
    { "NETBSD",	2,	"NetBSD" },
    { "LINUX",	3,	"Linux" },
    { "SOLARIS",6,	"Sun Solaris" },
    { "AIX",	7,	"AIX" },
    { "IRIX",	8,	"IRIX" },
    { "FREEBSD",9,	"FreeBSD" },
    { "TRU64",	10,	"Compaq TRU64 UNIX" },
    { "MODESTO",11,	"Novell Modesto" },
    { "OPENBSD",12,	"Open BSD" },
    { "OPENVMS",13,	"Open VMS" },
    { "NSK",	14,	"Hewlett-Packard Non-Stop Kernel" },
    { "LOPROC",	64,	"Processor-specific", 1 },
    { "HIPROC",	255,"Processor-specific" },
    //64-255	Architecture-specific value range
};
ELF_Parser::Value ELF_Parser::machineArray[EM_COUNT] = {
    { "NONE",		0,	"No machine" },
    { "M32",		1,	"AT&T WE 32100" },
    { "SPARC",		2,	"SPARC" },
    { "386",		3,	"Intel 80386" },
    { "68K",		4,	"Motorola 68000" },
    { "88K",		5,	"Motorola 88000" },
    //reserved	6	Reserved for future use (was {"486)
    { "860",		7,	"Intel 80860" },
    { "MIPS",		8,	"MIPS I Architecture" },
    { "S370",		9,	"IBM System/370 Processor" },
    { "MIPS_RS3_LE",10,	"MIPS RS3000 Little-endian" },
    //reserved	11-14	Reserved for future use
    { "PARISC",		15,	"Hewlett-Packard PA-RISC" },
    //reserved	16	Reserved for future use
    { "VPP500",		17,	"Fujitsu VPP500" },
    { "SPARC32PLUS",18,	"Enhanced instruction set SPARC" },
    { "960",		19,	"Intel 80960" },
    { "PPC",		20,	"PowerPC" },
    { "PPC64",		21,	"64-bit PowerPC" },
    { "S390",		22,	"IBM System/390 Processor" },
    //reserved	23-35	Reserved for future use
    { "V800",		36,	"NEC V800" },
    { "FR20",		37,	"Fujitsu FR20" },
    { "RH32",		38,	"TRW RH-32" },
    { "RCE",		39,	"Motorola RCE" },
    { "ARM",		40,	"Advanced RISC Machines ARM" },
    { "ALPHA",		41,	"Digital Alpha" },
    { "SH",			42,	"Hitachi SH" },
    { "SPARCV9",	43,	"SPARC Version 9" },
    { "TRICORE",	44,	"Siemens TriCore embedded processor" },
    { "ARC",		45,	"Argonaut RISC Core, Argonaut Technologies Inc." },
    { "H8_300",		46,	"Hitachi H8/300" },
    { "H8_300H",	47,	"Hitachi H8/300H" },
    { "H8S",		48,	"Hitachi H8S" },
    { "H8_500",		49,	"Hitachi H8/500" },
    { "IA_64",		50,	"Intel IA-64 processor architecture" },
    { "MIPS_X",		51,	"Stanford MIPS-X" },
    { "COLDFIRE",	52,	"Motorola ColdFire" },
    { "68HC12",		53,	"Motorola M68HC12" },
    { "MMA",		54,	"Fujitsu MMA Multimedia Accelerator" },
    { "PCP",		55,	"Siemens PCP" },
    { "NCPU",		56,	"Sony nCPU embedded RISC processor" },
    { "NDR1",		57,	"Denso NDR1 microprocessor" },
    { "STARCORE",	58,	"Motorola Star*Core processor" },
    { "ME16",		59,	"Toyota ME16 processor" },
    { "ST100",		60,	"STMicroelectronics ST100 processor" },
    { "TINYJ",		61,	"Advanced Logic Corp. TinyJ embedded processor family" },
    { "X86_64",		62,	"AMD x86-64 architecture" },
    { "PDSP",		63,	"Sony DSP Processor" },
    { "PDP10",		64,	"Digital Equipment Corp. PDP-10" },
    { "PDP11",		65,	"Digital Equipment Corp. PDP-11" },
    { "FX66",		66,	"Siemens FX66 microcontroller" },
    { "ST9PLUS",	67,	"STMicroelectronics ST9+ 8/16 bit microcontroller" },
    { "ST7",		68,	"STMicroelectronics ST7 8-bit microcontroller" },
    { "68HC16",		69,	"Motorola MC68HC16 Microcontroller" },
    { "68HC11",		70,	"Motorola MC68HC11 Microcontroller" },
    { "68HC08",		71,	"Motorola MC68HC08 Microcontroller" },
    { "68HC05",		72,	"Motorola MC68HC05 Microcontroller" },
    { "SVX",		73,	"Silicon Graphics SVx" },
    { "ST19",		74,	"STMicroelectronics ST19 8-bit microcontroller" },
    { "VAX",		75,	"Digital VAX" },
    { "CRIS",		76,	"Axis Communications 32-bit embedded processor" },
    { "JAVELIN",	77,	"Infineon Technologies 32-bit embedded processor" },
    { "FIREPATH",	78,	"Element 14 64-bit DSP Processor" },
    { "ZSP",		79,	"LSI Logic 16-bit DSP Processor" },
    { "MMIX",		80,	"Donald Knuth's educational 64-bit processor" },
    { "HUANY",		81,	"Harvard University machine-independent object files" },
    { "PRISM",		82,	"SiTera Prism" },
    { "AVR",		83,	"Atmel AVR 8-bit microcontroller" },
    { "FR30",		84,	"Fujitsu FR30" },
    { "D10V",		85,	"Mitsubishi D10V" },
    { "D30V",		86,	"Mitsubishi D30V" },
    { "V850",		87,	"NEC v850" },
    { "M32R",		88,	"Mitsubishi M32R" },
    { "MN10300",	89,	"Matsushita MN10300" },
    { "MN10200",	90,	"Matsushita MN10200" },
    { "PJ",			91,	"picoJava" },
    { "OPENRISC",	92,	"OpenRISC 32-bit embedded processor" },
    { "ARC_A5",		93,	"ARC Cores Tangent-A5" },
    { "XTENSA",		94,	"Tensilica Xtensa Architecture" },
    { "VIDEOCORE",	95,	"Alphamosaic VideoCore processor" },
    { "TMM_GPP",	96,	"Thompson Multimedia General Purpose Processor" },
    { "NS32K",		97,	"National Semiconductor 32000 series" },
    { "TPC",		98,	"Tenor Network TPC processor" },
    { "SNP1K",		99,	"Trebia SNP 1000 processor" },
    { "ST200",		100,"STMicroelectronics (www.st.com) ST200 microcontroller" },
    { "RISC_V",		243,"RISC-V" }, //// added 1/26/2022
    { "TCT",		49344, "Titech/Isshiki-Lab TCT Processor" },
    { "LOPROC",		101,"Unknown processor", 1 },
    { "HIPROC",		0xffff,	"Unknown processor" },
};
ELF_Parser::Value ELF_Parser::shtypeArray[SHT_COUNT] = {
    { "NULL",			0 },
    { "PROGBITS",		1 },
    { "SYMTAB",			2 },
    { "STRTAB",			3 },
    { "RELA",			4 },
    { "HASH",			5 },
    { "DYNAMIC",		6 },
    { "NOTE",			7 },
    { "NOBITS",			8 },
    { "REL",			9 },
    { "SHLIB",			10 },
    { "DYNSYM",			11 },
    { "INIT_ARRAY",		14 },
    { "FINI_ARRAY",		15 },
    { "PREINIT_ARRAY",	16 },
    { "GROUP",			17 },
    { "SYMTAB_SHNDX",	18 },
    { "LOOS",	0x60000000, 0, 1 },
    { "HIOS",	0x6fffffff },
    { "LOPROC",	0x70000000, 0, 1 },
    { "HIPROC",	0x7fffffff },
    { "LOUSER",	0x80000000, 0, 1 },
    { "HIUSER",	0xffffffff },
};
ELF_Parser::Value ELF_Parser::shflagArray[SHF_COUNT] = {
    { "W",			0x1,		"WRITE" },
    { "A",			0x2,		"ALLOC" },
    { "X",			0x4,		"EXECINSTR" },
    { "M",			0x10,		"MERGE" },
    { "S",			0x20,		"STRINGS" },
    { "I",			0x40,		"INFO_LINK" },
    { "L",			0x80,		"LINK_ORDER" },
    { "E",			0x100,		"OS_NONCONFORMING" },
    { "G",			0x200,		"GROUP" },
    { "T",			0x400,		"TLS" },
    { "MASKOS",		0x0ff00000, "MASKOS" },
    { "MASKPROC",	0xf0000000,	"MASKPROC" },
};
ELF_Parser::Value ELF_Parser::shnumArray[SHN_COUNT] = {
    { "UNDEF",		0 },
    { "LORESERVE",	0xff00 },
    { "LOPROC",		0xff00, 0, 1 },
    { "HIPROC",		0xff1f },
    { "LOOS",		0xff20, 0, 1 },
    { "HIOS",		0xff3f },
    { "ABS",			0xfff1 },
    { "COMMON",		0xfff2 },
    { "XINDEX",		0xffff },
    { "HIRESERVE",	0xffff },
};
ELF_Parser::Value ELF_Parser::stbindArray[STB_COUNT] = {
    { "LOCAL",	0 },
    { "GLOBAL",	1 },
    { "WEAK",	2 },
    { "LOOS",	10, 0, 1 },
    { "HIOS",	12 },
    { "LOPROC",	13, 0, 1 },
    { "HIPROC",	15 },
};
ELF_Parser::Value ELF_Parser::sttypeArray[STT_COUNT] = {
    { "NOTYPE",	0 },
    { "OBJECT",	1 },
    { "FUNC",	2 },
    { "SECTION",	3 },
    { "FILE",	4 },
    { "COMMON",	5 },
    { "TLS",		6 },
    { "LOOS",	10, 0, 1 },
    { "HIOS",	12 },
    { "LOPROC",	13, 0, 1 },
    { "HIPROC",	15 },
};
ELF_Parser::Value ELF_Parser::stvisArray[STV_COUNT] = {
    { "DEFAULT",		0 },
    { "INTERNAL",	1 },
    { "HIDDEN",		2 },
    { "PROTECTED",	3 },
};
ELF_Parser::Value ELF_Parser::dtagArray[DT_COUNT] = {
    { "NULL",	0,	"ignored" },//	mandatory	mandatory
    { "NEEDED",	1,	"val" },//	optional	optional
    { "PLTRELSZ",	2,	"val" },//	optional	optional
    { "PLTGOT",	3,	"ptr" },//	optional	optional
    { "HASH",	4,	"ptr" },//	mandatory	mandatory
    { "STRTAB",	5,	"ptr" },//	mandatory	mandatory
    { "SYMTAB",	6,	"ptr" },//	mandatory	mandatory
    { "RELA",	7,	"ptr" },//	mandatory	optional
    { "RELASZ",	8,	"val" },//	mandatory	optional
    { "RELAENT",	9,	"val" },//	mandatory	optional
    { "STRSZ",	10,	"val" },//	mandatory	mandatory
    { "SYMENT",	11,	"val" },//	mandatory	mandatory
    { "INIT",	12,	"ptr" },//	optional	optional
    { "FINI",	13,	"ptr" },//	optional	optional
    { "SONAME",	14,	"val" },//	ignored	optional
    { "RPATH*",	15,	"val" },//	optional	ignored
    { "SYMBOLIC*",	16,	"ignored" },//	ignored	optional
    { "REL",	17,	"ptr" },//	mandatory	optional
    { "RELSZ",	18,	"val" },//	mandatory	optional
    { "RELENT",	19,	"val" },//	mandatory	optional
    { "PLTREL",	20,	"val" },//	optional	optional
    { "DEBUG",	21,	"ptr" },//	optional	ignored
    { "TEXTREL*",	22,	"ignored" },	//optional	optional
    { "JMPREL",	23,	"ptr" },//	optional	optional
    { "BIND_NOW*",	24,	"ignored" },//	optional	optional
    { "INIT_ARRAY",	25,	"ptr" },//	optional	optional
    { "FINI_ARRAY",	26,	"ptr" },//	optional	optional
    { "INIT_ARRAYSZ",	27,	"val" },//	optional	optional
    { "FINI_ARRAYSZ",	28,	"val" },//	optional	optional
    { "RUNPATH",	29,	"val" },//	optional	optional
    { "FLAGS",	30,	"val" },//	optional	optional
    { "ENCODING",	31,	"unspecified" },//	unspecified	unspecified
    { "PREINIT_ARRAY",	32,	"ptr" },//optional	ignored
    { "PREINIT_ARRAYSZ",	33,	"val" },//	optional	ignored
    { "LOOS",	0x6000000D,	"unspecified", 1 },//	unspecified	unspecified
    { "HIOS",	0x6fffffff,	"unspecified" },	//unspecified	unspecified
    { "LOPROC",	0x70000000,	"unspecified", 1 },//	unspecified	unspecified
    { "HIPROC",	0x7fffffff,	"unspecified" },//	unspecified	unspecified
};
ELF_Parser::Value ELF_Parser::ptypeArray[PT_COUNT] = {
    { "NULL",	0 },
    { "LOAD",	1 },
    { "DYNAMIC",	2 },
    { "INTERP",	3 },
    { "NOTE",	4 },
    { "SHLIB",	5 },
    { "PHDR",	6 },
    { "TLS",		7 },
    { "LOOS",	0x60000000, 0, 1 },
    { "HIOS",	0x6fffffff },
    { "LOPROC",	0x70000000, 0, 1 },
    { "HIPROC",	0x7fffffff },
};
ELF_Parser::Value ELF_Parser::pflagArray[PF_COUNT] = {
    { "X",			0x1,		"Execute" },
    { "W",			0x2,		"Write" },
    { "R",			0x4,		"Read" },
    { "MASKOS",		0x0ff00000,	"Unspecified" },
    { "MASKPROC",	0xf0000000,	"Unspecified" },
};

ELF_Parser::ValueGroup ELF_Parser::ehtypeGroup(ET_COUNT, ehtypeArray);
ELF_Parser::ValueGroup ELF_Parser::osabiGroup(ELFOSABI_COUNT, osabiArray);
ELF_Parser::ValueGroup ELF_Parser::machineGroup(EM_COUNT, machineArray);
ELF_Parser::ValueGroup ELF_Parser::shtypeGroup(SHT_COUNT, shtypeArray);
ELF_Parser::ValueGroup ELF_Parser::shflagGroup(SHF_COUNT, shflagArray);
ELF_Parser::ValueGroup ELF_Parser::shnumGroup(SHN_COUNT, shnumArray);
ELF_Parser::ValueGroup ELF_Parser::stbindGroup(STB_COUNT, stbindArray);
ELF_Parser::ValueGroup ELF_Parser::sttypeGroup(STT_COUNT, sttypeArray);
ELF_Parser::ValueGroup ELF_Parser::stvisGroup(STV_COUNT, stvisArray);
ELF_Parser::ValueGroup ELF_Parser::dtagGroup(DT_COUNT, dtagArray);
ELF_Parser::ValueGroup ELF_Parser::ptypeGroup(PT_COUNT, ptypeArray);
ELF_Parser::ValueGroup ELF_Parser::pflagGroup(PF_COUNT, pflagArray);


void ELF_Parser::Ehdr::PrintInfo(std::stringstream & s) {
    C2R_ASSERT(curInfo);
    s << C2R::SF("%12s = ", "File-Name") << curInfo->projectDescriptor << "\n";
    PrintMagicNumber(s);
    PrintClass(s);
    PrintByteOrder(s);
    PrintFileVersion(s);
    osabiGroup.GetValue(e_ident[EI_OSABI])->PrintInfo(s, "OSABI");
    PrintABIVersion(s);
    ehtypeGroup.GetValue(ehdr.e_type)->PrintInfo(s, "TYPE");
    machineGroup.GetValue(ehdr.e_machine)->PrintInfo(s, "MACHINE");

    PRINT_ELF_PARAM(s, e_version);
    PRINT_ELF_PARAM(s, e_entry);
    PRINT_ELF_PARAM(s, e_phoff);
    PRINT_ELF_PARAM(s, e_shoff);
    PRINT_ELF_PARAM(s, e_flags);
    PRINT_ELF_PARAM(s, e_ehsize);
    PRINT_ELF_PARAM(s, e_phentsize);
    PRINT_ELF_PARAM(s, e_phnum);
    PRINT_ELF_PARAM(s, e_shentsize);
    PRINT_ELF_PARAM(s, e_shnum);
    PRINT_ELF_PARAM(s, e_shstrndx);
}

#define MAKE_SILENT

size_t ELF_Parser::Elf_Shdr::PrintHeader(std::stringstream & s, size_t maxStringLength)
{
    s << "Section Headers:\n";
    size_t len = s.str().size();
    s << C2R::SF("|No.|%*s", maxStringLength, "Name   ");
    s << C2R::SF("|%*s", shtypeGroup.maxNameLength, "Type   ");
    s << "|Flag| Address| Offset |  Size  |Link|Info|Align|ESize|\n";
    len = s.str().size() - len;
    size_t ll = len;
    while (ll > 0) { s << "="; ll--; }
    s << "\n";
    return (int)len;
}

void ELF_Parser::Elf_Shdr::PrintInfo(std::stringstream & s, int id, size_t maxStringLength)
{
    std::stringstream f;
    shflagGroup.GetValueMask((int)sh_flags, f);
    s << C2R::SF("|%3d|", id) << C2R::SF("%-*s|", maxStringLength, sh_name_str)
        << C2R::SF("%-*s|", shtypeGroup.maxNameLength, shtypeGroup.GetValue(sh_type)->name)
        << C2R::SF("%-4s|", f.str()) << C2R::SF("%08x|", sh_addr) << C2R::SF("%08x|", sh_offset)
        << C2R::SF("%08x|", sh_size) << C2R::SF("%4d|", sh_link) << C2R::SF("%4d|", sh_info)
        << C2R::SF("%5d|", sh_addralign) << C2R::SF("%5d|\n", sh_entsize);
}

size_t ELF_Parser::Elf_Sym::PrintHeader(std::stringstream & s, size_t maxStringLength, Elf_Shdr * symbolSection)
{
    s << C2R::SF("Symbol Table (%s):\n", symbolSection->sh_name_str);
    size_t len = s.str().size();
    s << C2R::SF("|No.|%*s| Address|  Size  |", maxStringLength, "Name   ")
        << C2R::SF("%*s|", stbindGroup.maxNameLength, "Bind ")
        << C2R::SF("%*s|", sttypeGroup.maxNameLength, "Type ")
        << C2R::SF("%*s|Section...\n", stvisGroup.maxNameLength, "Vis  ");
    len = s.str().size() - len;
    size_t ll = len;
    while (ll > 0) { s << "="; ll--; }
    s << "\n";
    return len;
}

void ELF_Parser::Elf_Sym::PrintInfo(std::stringstream & s, int id, size_t maxStringLength, ShdrTable * sectionTable)
{
    Value * shndxVal = shnumGroup.GetValue(st_shndx);
    Elf_Shdr * shdr = (shndxVal) ? 0 : &sectionTable->shdr[st_shndx];
    C2R_ASSERT(shndxVal || shdr);
    s << C2R::SF("|%3d|", id) << C2R::SF("%-*s|", maxStringLength, st_name_str)
        << C2R::SF("%08x|", st_value) << C2R::SF("%08x|", st_size)
        << C2R::SF("%-*s|", stbindGroup.maxNameLength, stbindGroup.GetValue(st_bind)->name)
        << C2R::SF("%-*s|", sttypeGroup.maxNameLength, sttypeGroup.GetValue(st_type)->name)
        << C2R::SF("%-*s|", stvisGroup.maxNameLength, stvisGroup.GetValue(st_vis)->name)
        << C2R::SF("(%s)\n", ((shndxVal) ? shndxVal->name : shdr->sh_name_str));
}

size_t ELF_Parser::Elf_Rel::PrintHeader(std::stringstream & s, Elf_Shdr * relocSection)
{
    s << "Relocation Table (" << relocSection->sh_name_str << "):\n";
    const char * reloc_type_name = shtypeGroup.GetValue(relocSection->sh_type)->name;
    int isRel = (strcmp(reloc_type_name, "REL") == 0);
    size_t len = s.str().size();
    if (isRel)  { s << "|No.| Offset |  Info  |Type|SymID| SymVal | SymName... \n"; }
    else        { s << "|No.| Offset |  Info  |Type|SymID| addend | SymVal | SymName... \n"; }
    len = s.str().size() - len;
    size_t ll = len;
    while (ll > 0) { s << "="; ll--; }
    s << "\n";
    return len;
}
void ELF_Parser::Elf_Rel::PrintInfo(std::stringstream & s, int id, ELF_Parser::SymTable * symTable)
{
#if 1   /// 5/16/23 : symTable->symbolCount == 0 can happen...
    if (symTable->symbolCount == 0) {
        if (addend_enabled) {
            s << C2R::SF("|%3d|", id) << C2R::SF("%08x|", r_offset) << C2R::SF("%08x|", r_info)
                << C2R::SF("%04x|", r_type) << C2R::SF("%5d|", r_sym) << C2R::SF("%08x|", r_addend)
                << " (Elf_Rel : symTable->symbolCount == 0)\n";
        }
        else {
            s << C2R::SF("|%3d|", id) << C2R::SF("%08x|", r_offset) << C2R::SF("%08x|", r_info)
                << C2R::SF("%04x|", r_type) << C2R::SF("%5d|", r_sym)
                << " (Elf_Rel : symTable->symbolCount == 0)\n";
        }
    }
    else {
        C2R_ASSERT(r_sym >= 0 && r_sym < (unsigned)symTable->symbolCount);
        Elf_Sym* sym = &symTable->symTable[r_sym];
        if (!sym) {   /// 11/2/21
            return;
        }
        if (addend_enabled) {
            s << C2R::SF("|%3d|", id) << C2R::SF("%08x|", r_offset) << C2R::SF("%08x|", r_info)
                << C2R::SF("%04x|", r_type) << C2R::SF("%5d|", r_sym) << C2R::SF("%08x|", r_addend)
                << C2R::SF("%08x|", sym->st_value) << C2R::SF("(%s)\n", sym->st_name_str);
        }
        else {
            s << C2R::SF("|%3d|", id) << C2R::SF("%08x|", r_offset) << C2R::SF("%08x|", r_info)
                << C2R::SF("%04x|", r_type) << C2R::SF("%5d|", r_sym)
                << C2R::SF("%08x|", sym->st_value) << C2R::SF("(%s)\n", sym->st_name_str);
        }
    }
#else
    C2R_ASSERT(r_sym >= 0 && r_sym >= (unsigned)symTable->symbolCount);
    Elf_Sym * sym = &symTable->symTable[r_sym];
#if 1   /// 11/2/21
    if (!sym) {
        return;
    }
#endif
    if (addend_enabled) {
        s << C2R::SF("|%3d|", id) << C2R::SF("%08x|", r_offset) << C2R::SF("%08x|", r_info)
            << C2R::SF("%04x|", r_type) << C2R::SF("%5d|", r_sym) << C2R::SF("%08x|", r_addend)
            << C2R::SF("%08x|", sym->st_value) << C2R::SF("(%s)\n", sym->st_name_str);
    }
    else {
        s << C2R::SF("|%3d|", id) << C2R::SF("%08x|", r_offset) << C2R::SF("%08x|", r_info)
            << C2R::SF("%04x|", r_type) << C2R::SF("%5d|", r_sym)
            << C2R::SF("%08x|", sym->st_value) << C2R::SF("(%s)\n", sym->st_name_str);
    }
#endif
}

size_t ELF_Parser::Elf_Dyn::PrintHeader(std::stringstream & s, Elf_Shdr * dynSection)
{
    s << "Dynamic Table (" << dynSection->sh_name_str << "):\n";
    const char * dyn_type_name = shtypeGroup.GetValue(dynSection->sh_type)->name;
    size_t len = s.str().size();
    s << C2R::SF("|No.|%*s|Name/Value|\n", dtagGroup.maxNameLength, "Tag   ");
    len = s.str().size() - len;
    size_t ll = len;
    while (ll > 0) { s << "="; ll--; }
    s << "\n";
    return len;
}
void ELF_Parser::Elf_Dyn::PrintInfo(std::stringstream & s, int id)
{
    Value * dtag = dtagGroup.GetValue((int)d_tag);
    s << C2R::SF("|%3d|", id) << C2R::SF("%*s|", dtagGroup.maxNameLength, dtag->name);
    if (dtag->comment[0] == 'v')      { s << C2R::SF("[V] %08x", d_un.d_val); }
    else if (dtag->comment[0] == 'p') { s << C2R::SF("[P] %08x", d_un.d_ptr); }
    else                              { s << "[IGNORE]"; }
    if (name) { s << C2R::SF(" (%s)", name); }
    s << "\n";
}

#define ELF_ERROR(n)	{ status.errorFlag = ES_##n; return false;}

bool ELF_Parser::Info::dontPrintParseLog = false;
#if 1   /// 11/18/24
ELF_Parser::HostSymbols ELF_Parser::Info::hostSym = { 0 };
#else
Elf64_Addr ELF_Parser::Info::isatest_tohost = 0;    /// 6/4/24
#endif

ELF_Parser::Info::Info() {
    fout = 0;
    C2R_ASSERT(!curInfo);
    curInfo = this;
    OpenOutputFile();
#if 1   /// 11/18/24
    hostSym.Reset();
#else
    isatest_tohost = 0; /// 6/4/24
#endif
}
ELF_Parser::Info::~Info() {
    CloseOutputFile();
    C2R_ASSERT(curInfo == this);
    curInfo = 0;
}

/// 1/14/21
bool ELF_Parser::Ehdr::Parse(ELF_Data elfData)
{
    size_t frval;
    if ((frval = elfData.Read(e_ident, 1, EI_NIDENT)) != EI_NIDENT) { ELF_ERROR(IDENT) }

    /// 1. check magic number : 0x7f 'E' 'L' 'F'
    if (e_ident[0] != 0x7f || e_ident[1] != 'E' || e_ident[2] != 'L' || e_ident[3] != 'F') { ELF_ERROR(MAGIC_NUM) }
    if (IsElf32()) {
        Elf32_Ehdr h32;
        if ((frval = elfData.Read(&h32, 1, sizeof(Elf32_Ehdr))) != sizeof(Elf32_Ehdr)) { ELF_ERROR(ELF32) }
        ehdr.Copy(&h32);
    }
    else if (IsElf64()) {
        Elf64_Ehdr h64;
        if ((frval = elfData.Read(&h64, 1, sizeof(Elf64_Ehdr))) != sizeof(Elf64_Ehdr)) { ELF_ERROR(ELF64) }
        ehdr.Copy(&h64);
    }
    else { ELF_ERROR(CLASS) }
    if (e_ident[EI_DATA] <= ELFDATANONE || e_ident[EI_DATA] > ELFDATA2MSB) { ELF_ERROR(DATA) }
    if (e_ident[EI_VERSION] != EV_CURRENT) { ELF_ERROR(VERSION) }
    if (osabiGroup.GetValue(e_ident[EI_OSABI]) == 0) { ELF_ERROR(OSABI) }

    if (ehtypeGroup.GetValue(ehdr.e_type) == 0) { ELF_ERROR(EHTYPE) }
    if (machineGroup.GetValue(ehdr.e_machine) == 0) { ELF_ERROR(MACHINE) }
    if (ehdr.e_ehsize != ((IsElf32()) ? sizeof(Elf32_Ehdr) : sizeof(Elf64_Ehdr)) + EI_NIDENT) { ELF_ERROR(EHSIZE) }
    if (ehdr.e_shentsize != ((IsElf32()) ? sizeof(Elf32_Shdr) : sizeof(Elf64_Shdr))) { ELF_ERROR(SHENTSIZE) }
    if (ehdr.e_phentsize != ((IsElf32()) ? sizeof(Elf32_Phdr) : sizeof(Elf64_Phdr))) { ELF_ERROR(PHENTSIZE) }

    return true;
}

void ELF_Parser::Ehdr::EmitMessage() {
    std::stringstream msg;
    PrintInfo(msg);
    //printf("PrintInfo(msg) ok\n");
    if (!Info::dontPrintParseLog) {
        std::cout << msg.str();
    }
    fprintf(fout, "%s", msg.str().c_str());
}


#define ELF_ERROR_POS(n, i, data)	{ status.errorFlag = ES_##n; status.pos = i; status.val = data;	return false;}

/// 3/31/22
bool ELF_Parser::ShdrTable::Parse(Ehdr * ehdr, ELF_Data elfData)
{
    C2R_ASSERT(!shdr && !symbolSection && !dynSymSection && !dynSection && relocSectionList.empty());
    C2R_ASSERT(ehdr->ehdr.e_shnum);
    sectionCount = ehdr->ehdr.e_shnum;
    shdr = new Elf_Shdr[sectionCount];
    int i;
    elfData.Seek((long)ehdr->ehdr.e_shoff);
    size_t frval;
    Elf32_Shdr s32;
    Elf64_Shdr s64;
    int allocVec = shflagGroup.GetValueVector("A");
    for (i = 0; i < sectionCount; i++) {
        if (ehdr->IsElf32()) {
            if ((frval = elfData.Read(&s32, 1, sizeof(Elf32_Shdr))) != sizeof(Elf32_Shdr)) { ELF_ERROR(SH32) }
            shdr[i].Copy(&s32);
        }
        else {
            if ((frval = elfData.Read(&s64, 1, sizeof(Elf64_Shdr))) != sizeof(Elf64_Shdr)) { ELF_ERROR(SH64) }
            shdr[i].Copy(&s64);
        }
        if ((shtypeGroup.GetValue(shdr[i].sh_type)) == 0) { ELF_ERROR_POS(SHTYPE, i, shdr[i].sh_type) }
        if (shdr[i].sh_flags & allocVec) { shdr[i].field.sh_alloc = 1; }
    }
    if (ehdr->ehdr.e_shstrndx >= sectionCount) { ELF_ERROR(STRTAB_0) }
    Elf64_Shdr * shStrTabSection = &shdr[ehdr->ehdr.e_shstrndx];
    sectionStr.resize((size_t)shStrTabSection->sh_size);
    char * sectionStringTable = &sectionStr[0];
    elfData.Seek((long)shStrTabSection->sh_offset);

    if (elfData.Read(sectionStringTable, 1, (int)shStrTabSection->sh_size) != shStrTabSection->sh_size) {
        ELF_ERROR(STRTAB_1)
    }
    maxSectionStringLength = 0;
    Elf_Shdr * symStrTabSection = 0, *dynSymStrTabSection = 0;
    for (i = 0; i < sectionCount; i++) {
        const char * sectionName = sectionStringTable + shdr[i].sh_name;
        shdr[i].sh_name_str = sectionName;
        size_t len = strlen(sectionName);
        if (maxSectionStringLength < len) { maxSectionStringLength = len; }
        const char * sh_type_name = shtypeGroup.GetValue(shdr[i].sh_type)->name;
        if (strcmp(sectionName, ".symtab") == 0 && strcmp(sh_type_name, "SYMTAB") == 0) {
            symbolSection = &shdr[i];
        }
        else if (strcmp(sectionName, ".strtab") == 0 && strcmp(sh_type_name, "STRTAB") == 0) {
            symStrTabSection = &shdr[i];
        }
        else if (strcmp(sectionName, ".dynsym") == 0 && strcmp(sh_type_name, "DYNSYM") == 0) {
            dynSymSection = &shdr[i];
        }
        else if (strcmp(sectionName, ".dynamic") == 0 && strcmp(sh_type_name, "DYNAMIC") == 0) {
            dynSection = &shdr[i];
        }
        else if (strcmp(sectionName, ".dynstr") == 0 && strcmp(sh_type_name, "STRTAB") == 0) {
            dynSymStrTabSection = &shdr[i];
        }
        else if (strcmp(sh_type_name, "REL") == 0 || strcmp(sh_type_name, "RELA") == 0) {
            relocSectionList.push_back(&shdr[i]);
        }
    }
    C2R_ASSERT(!symbolSection || symStrTabSection);
    if (symStrTabSection) {
        symbolStr.resize((size_t)symStrTabSection->sh_size);
        char * symbolStringTable = &symbolStr[0];
        elfData.Seek((long)symStrTabSection->sh_offset);
        if (elfData.Read(symbolStringTable, 1, (int)symStrTabSection->sh_size) != symStrTabSection->sh_size) {
            ELF_ERROR(STRTAB_2)
        }
    }
    C2R_ASSERT(!dynSymSection || dynSymStrTabSection);
    if (dynSymStrTabSection) {
        dynSymStr.resize((size_t)dynSymStrTabSection->sh_size);
        char * dynSymStringTable = &dynSymStr[0];
        elfData.Seek((long)dynSymStrTabSection->sh_offset);
        if (elfData.Read(dynSymStringTable, 1, (int)dynSymStrTabSection->sh_size) != dynSymStrTabSection->sh_size) {
            ELF_ERROR(STRTAB_2)
        }
    }
    return true;
}
void ELF_Parser::ShdrTable::EmitMessage(Info * elfInfo) {
    std::stringstream msg;
    size_t len = Elf_Shdr::PrintHeader(msg, maxSectionStringLength);
    if (!Info::dontPrintParseLog) {
        std::cout << msg.str();
    }
    fprintf(fout, "%s", msg.str().c_str());
    for (int i = 0; i < sectionCount; i++) {
        msg.str("");
        shdr[i].PrintInfo(msg, i, maxSectionStringLength);
        if (!Info::dontPrintParseLog) {
            std::cout << msg.str();
        }
        fprintf(fout, "%s", msg.str().c_str());
    }
    msg.str("");
    PrintTail(msg, len);
    if (!Info::dontPrintParseLog) {
        std::cout << msg.str();
    }
    fprintf(fout, "%s", msg.str().c_str());
}

bool ELF_Parser::SymTable::Parse(ELF_Parser::Info * elfInfo, FILE * fp, Elf_Shdr * symSect, const char * stringTable,
    bool showAll)
{
    Ehdr * ehdr = &elfInfo->elfHeader;
    symbolSection = symSect;
    C2R_ASSERT(!symTable && symbolSection && stringTable);
    int symSize = (ehdr->IsElf32()) ? sizeof(Elf32_Sym) : sizeof(Elf64_Sym);
    if (symbolSection->sh_entsize != symSize || symbolSection->sh_size % symSize != 0) { ELF_ERROR(SYM_0) }
    symbolCount = (int)symbolSection->sh_size / symSize;
    symTable = new Elf_Sym[symbolCount];

    int i;
    rewind(fp);
    fseek(fp, (long)symbolSection->sh_offset, SEEK_SET);
    size_t frval;
    Elf32_Sym s32;
    Elf64_Sym s64;
    maxSymbolStringLength = 0;
    for (i = 0; i < symbolCount; i++) {
        if (ehdr->IsElf32()) {
            if ((frval = fread(&s32, 1, sizeof(Elf32_Sym), fp)) != sizeof(Elf32_Sym)) { ELF_ERROR_POS(SYM_1, i, 0) }
            symTable[i].Copy(&s32);
        }
        else {
            if ((frval = fread(&s64, 1, sizeof(Elf64_Sym), fp)) != sizeof(Elf64_Sym)) { ELF_ERROR_POS(SYM_2, i, 0) }
            symTable[i].Copy(&s64);
        }
        if (stbindGroup.GetValue(symTable[i].st_bind) == 0) { ELF_ERROR_POS(SYM_3, i, symTable[i].st_bind) }
        if (sttypeGroup.GetValue(symTable[i].st_type) == 0) { ELF_ERROR_POS(SYM_4, i, symTable[i].st_name) }
        const char * sym_name = stringTable + symTable[i].st_name;
        symTable[i].st_name_str = sym_name;
        size_t len = strlen(sym_name);
        if (maxSymbolStringLength < len) { maxSymbolStringLength = len; }
    }
    std::stringstream msg;
    size_t len = Elf_Sym::PrintHeader(msg, maxSymbolStringLength, symbolSection);
#if !defined(MAKE_SILENT)
    std::cout << msg.str();
#endif
    fprintf(fout, "%s", msg.str().c_str());
    Value * typeFunc = sttypeGroup.GetValue("FUNC");
    Value * typeSect = sttypeGroup.GetValue("SECTION");
    Value * typeObj = sttypeGroup.GetValue("OBJECT");
    for (i = 0; i < symbolCount; i++) {
        msg.str("");
        Elf_Sym * sym = &symTable[i];	///	4/1/14
        Value * val = sttypeGroup.GetValue(sym->st_type);
        sym->PrintInfo(msg, i, maxSymbolStringLength, &elfInfo->sectionTable);
#if !defined(MAKE_SILENT)
        if (showAll || (val == typeFunc || val == typeSect || val == typeObj)) { MESSAGE(msg); }
#endif
        fprintf(fout, "%s", msg.str().c_str());
    }
    msg.str("");
    PrintTail(msg, len);
#if !defined(MAKE_SILENT)
    std::cout << msg.str();
#endif
    fprintf(fout, "%s", msg.str().c_str());
    return true;
}

bool ELF_Parser::RelocTable::Parse(ELF_Parser::Info * elfInfo, FILE * fp, Elf_Shdr * relSect)
{
    Ehdr * ehdr = &elfInfo->elfHeader;
    relocSection = relSect;
    symTable = &elfInfo->dynSymTable;
    C2R_ASSERT(!relocTable && relocSection && symTable);
    const char * reloc_type_name = shtypeGroup.GetValue(relocSection->sh_type)->name;
    int isRel = (strcmp(reloc_type_name, "REL") == 0);
    int isRela = (strcmp(reloc_type_name, "RELA") == 0);
    C2R_ASSERT(isRel + isRela == 1);
    int relSize = (ehdr->IsElf32()) ? ((isRel) ? sizeof(Elf32_Rel) : sizeof(Elf32_Rela))
        : ((isRel) ? sizeof(Elf64_Rel) : sizeof(Elf64_Rela));

    if (relocSection->sh_entsize != relSize || relocSection->sh_size % relSize != 0) { ELF_ERROR(REL_0) }
    relocCount = (int)relocSection->sh_size / relSize;
    relocTable = new Elf_Rel[relocCount];

    int i;
    rewind(fp);
    fseek(fp, (long)relocSection->sh_offset, SEEK_SET);
    size_t frval;
    Elf32_Rel r32;
    Elf64_Rel r64;
    Elf32_Rela r32a;
    Elf64_Rela r64a;
    for (i = 0; i < relocCount; i++) {
        if (ehdr->IsElf32()) {
            if (isRel) {
                if ((frval = fread(&r32, 1, sizeof(Elf32_Rel), fp)) != sizeof(Elf32_Rel)) {
                    ELF_ERROR_POS(REL_1, i, 0)
                }
                relocTable[i].Copy(&r32);
            }
            else {
                if ((frval = fread(&r32a, 1, sizeof(Elf32_Rela), fp)) != sizeof(Elf32_Rela)) {
                    ELF_ERROR_POS(REL_2, i, 0)
                }
                relocTable[i].Copy(&r32a);
            }
        }
        else {
            if (isRel) {
                if ((frval = fread(&r64, 1, sizeof(Elf64_Rel), fp)) != sizeof(Elf64_Rel)) {
                    ELF_ERROR_POS(REL_3, i, 0)
                }
                relocTable[i].Copy(&r64);
            }
            else {
                if ((frval = fread(&r64a, 1, sizeof(Elf64_Rela), fp)) != sizeof(Elf64_Rela)) {
                    ELF_ERROR_POS(REL_4, i, 0)
                }
                relocTable[i].Copy(&r64a);
            }
        }
    }
    std::stringstream msg;
    size_t len = Elf_Rel::PrintHeader(msg, relocSection);
#if !defined(MAKE_SILENT)
    MESSAGE(msg);
#endif
    fprintf(fout, "%s", msg.str().c_str());
    for (i = 0; i < relocCount; i++) {
        msg.str("");
        relocTable[i].PrintInfo(msg, i, symTable);
#if !defined(MAKE_SILENT)
        std::cout << msg.str();
#endif
        fprintf(fout, "%s", msg.str().c_str());
    }
    msg.str("");
    PrintTail(msg, len);
#if !defined(MAKE_SILENT)
    std::cout << msg.str();
#endif
    fprintf(fout, "%s", msg.str().c_str());
    return true;
}

bool ELF_Parser::DynTable::Parse(ELF_Parser::Info * elfInfo, FILE * fp, Elf_Shdr * dynSect, const char * stringTable)
{
    Ehdr * ehdr = &elfInfo->elfHeader;
    dynSection = dynSect;
    C2R_ASSERT(!dynTable && dynSection && stringTable);
    int dynSize = (ehdr->IsElf32()) ? sizeof(Elf32_Dyn) : sizeof(Elf64_Dyn);

    if (dynSection->sh_entsize != dynSize || dynSection->sh_size % dynSize != 0) { ELF_ERROR(DYN_0) }
    dynCount = (int)dynSection->sh_size / dynSize;
    dynTable = new Elf_Dyn[dynCount];

    int i;
    rewind(fp);
    fseek(fp, (long)dynSection->sh_offset, SEEK_SET);
    size_t frval;
    Elf32_Dyn d32;
    Elf64_Dyn d64;
    for (i = 0; i < dynCount; i++) {
        if (ehdr->IsElf32()) {
            if ((frval = fread(&d32, 1, sizeof(Elf32_Dyn), fp)) != sizeof(Elf32_Dyn)) { ELF_ERROR_POS(DYN_1, i, 0) }
                dynTable[i].Copy(&d32);
        }
        else {
            if ((frval = fread(&d64, 1, sizeof(Elf64_Dyn), fp)) != sizeof(Elf64_Dyn)) { ELF_ERROR_POS(DYN_2, i, 0) }
                dynTable[i].Copy(&d64);
        }
        Value * tag = dtagGroup.GetValue((int)dynTable[i].d_tag);
        if (tag == 0) { ELF_ERROR_POS(DYN_3, i, 0) }
            if (strcmp(tag->name, "NEEDED") == 0) { dynTable[i].name = stringTable + dynTable[i].d_un.d_val; }
    }
    std::stringstream msg;
    size_t len = Elf_Dyn::PrintHeader(msg, dynSection);
#if !defined(MAKE_SILENT)
    std::cout << msg.str();
#endif
    fprintf(fout, "%s", msg.str().c_str());
    for (i = 0; i < dynCount; i++) {
        msg.str("");
        dynTable[i].PrintInfo(msg, i);
#if !defined(MAKE_SILENT)
        std::cout << msg.str();
#endif
        fprintf(fout, "%s", msg.str().c_str());
    }
    msg.str("");
    PrintTail(msg, len);
#if !defined(MAKE_SILENT)
    std::cout << msg.str();
#endif
    fprintf(fout, "%s", msg.str().c_str());
    return true;
}

/// 1/14/21
bool ELF_Parser::PhdrTable::Parse(Ehdr * ehdr, ELF_Data elfData)
{
    C2R_ASSERT(!phdr && !interpName);
    if (ehdr->ehdr.e_phnum == 0) { return true; }
    segmentCount = ehdr->ehdr.e_phnum;
    phdr = new Elf_Phdr[segmentCount];
    int i;
    elfData.Seek((long)ehdr->ehdr.e_phoff);
    size_t frval;
    Elf32_Phdr p32;
    Elf64_Phdr p64;
    for (i = 0; i < segmentCount; i++) {
        if (ehdr->IsElf32()) {
            if ((frval = elfData.Read(&p32, 1, sizeof(Elf32_Phdr))) != sizeof(Elf32_Phdr)) { ELF_ERROR_POS(PH32, i, 0) }
            phdr[i].Copy(&p32);
        }
        else {
            if ((frval = elfData.Read(&p64, 1, sizeof(Elf64_Phdr))) != sizeof(Elf64_Phdr)) { ELF_ERROR_POS(PH64, i, 0) }
            phdr[i].Copy(&p64);
        }
        Value * type = ptypeGroup.GetValue(phdr[i].p_type);
        if ((type) == 0) { ELF_ERROR_POS(PHTYPE, i, phdr[i].p_type) }
        if (strcmp(type->name, "INTERP") == 0) { segInterp = &phdr[i]; }
    }
    if (segInterp) {
        elfData.Seek((long)segInterp->p_offset);
        C2R_ASSERT(segInterp->p_filesz <= segInterp->p_memsz);
        interpName = new char[(int)segInterp->p_memsz];
        if ((frval = elfData.Read(interpName, 1, (int)segInterp->p_filesz)) != segInterp->p_filesz) {
            ELF_ERROR(PH_INTERP)
        }
        U64 i;
        for (i = segInterp->p_filesz; i < segInterp->p_memsz; i++) { interpName[i] = 0; }
    }
    return true;
}

void ELF_Parser::PhdrTable::EmitMessage(Info * elfInfo) {
    std::stringstream msg;
    size_t len = Elf_Phdr::PrintHeader(msg);
    if (!Info::dontPrintParseLog) {
        std::cout << msg.str();
    }
    fprintf(fout, "%s", msg.str().c_str());
    ShdrTable *stable = (elfInfo) ? &elfInfo->sectionTable : NULL;
    for (int i = 0; i < segmentCount; i++) {
        msg.str("");
        phdr[i].PrintInfo(msg, i, stable);
        if (&phdr[i] == segInterp) { msg << "(interp = " << interpName << ")\n"; }
        if (!Info::dontPrintParseLog) {
            std::cout << msg.str();
        }
        fprintf(fout, "%s", msg.str().c_str());
    }
    msg.str("");
    PrintTail(msg, len);
    if (!Info::dontPrintParseLog) {
        std::cout << msg.str();
    }
    fprintf(fout, "%s", msg.str().c_str());
}

size_t ELF_Parser::Elf_Phdr::PrintHeader(std::stringstream & s)
{
    s << "Program Headers:\n";
    size_t len = s.str().size();
    s << "|No.|Type   | Offset | Vaddr  | Paddr  |Filesize|Memsize |Flags|Algn|Sections...\n";
    len = s.str().size() - len;
    size_t ll = len;
    while (ll > 0) { s << "="; ll--; }
    s << "\n";
    return len;
}

void ELF_Parser::Elf_Phdr::PrintInfo(std::stringstream & s, int id, ShdrTable * sectionTable)
{
    std::stringstream f;
    pflagGroup.GetValueMask((int)p_flags, f);
    s << C2R::SF("|%3d|", id)
        << C2R::SF("%-*s|", ptypeGroup.maxNameLength, ptypeGroup.GetValue(p_type)->name)
        << C2R::SF("%08x|", p_offset) << C2R::SF("%08x|", p_vaddr) << C2R::SF("%08x|", p_paddr)
        << C2R::SF("%08x|", p_filesz) << C2R::SF("%08x|", p_memsz) << C2R::SF("%5s|", f.str())
        << C2R::SF("%04x|", p_align);

    if (sectionTable) {
        int i;
        for (i = 0; i < sectionTable->sectionCount; i++) {
            Elf64_Addr sh_addr = sectionTable->shdr[i].sh_addr;
            if (sh_addr >= p_vaddr && sh_addr < p_vaddr + p_memsz) {
                s << C2R::SF("(%s)", &sectionTable->sectionStr[0] + sectionTable->shdr[i].sh_name);
            }
        }
    }
    s << "\n";
}

FILE * ELF_Parser::fout = NULL;

ELF_Parser::Info *ELF_Parser::curInfo = NULL;

void ELF_Parser::OpenOutputFile() {
    if (!fout) {
        fout = fopen("elf_info.log", "w");
    }
    C2R_ASSERT(fout);
}

void ELF_Parser::CloseOutputFile() {
    if (fout) { fclose(fout); }
    fout = NULL;
}

bool ELF_Parser::Info::Parse(FILE * fp)
{
    Reset();
    if (!dontPrintParseLog) {
        std::cout << "------------ ELF_Parser::Info::Parser() ----------------\n";
    }
    if (!elfHeader.Parse(fp)) { return false; }
    if (elfHeader.ehdr.e_phnum) { if (!segmentTable.Parse(this, fp)) { return false; } }
    if (elfHeader.ehdr.e_shnum) {
        if (!sectionTable.Parse(this, fp)) { return false; }
        if (sectionTable.symbolSection) {
            C2R_ASSERT(!sectionTable.symbolStr.empty());
            if (!symbolTable.Parse(this, fp, sectionTable.symbolSection, sectionTable.symbolStr.c_str(), false)) {
                return false;
            }
        }
        if (sectionTable.dynSymSection) {
            C2R_ASSERT(!sectionTable.dynSymStr.empty());
            if (!dynSymTable.Parse(this, fp, sectionTable.dynSymSection, sectionTable.dynSymStr.c_str(), true)) {
                return false;
            }
        }
        if (sectionTable.dynSection) {
            C2R_ASSERT(!sectionTable.dynSymStr.empty());
            if (!dynTable.Parse(this, fp, sectionTable.dynSection, sectionTable.dynSymStr.c_str())) {
                return false;
            }
        }
        for (size_t i = 0, e = sectionTable.relocSectionList.size(); i < e; ++i) {
            Elf_Shdr * rsh = sectionTable.relocSectionList[i];
            relocTableList.emplace_back();
            relocTableList.back().Parse(this, fp, rsh);
        }
    }
    if (!sectionTable.CreateSections(fp, fout)) { return false; }
    CloseOutputFile();
    return true;
}

bool ELF_Parser::ShdrTable::CreateSections(FILE *fp, FILE *fout) {
    size_t i, e;
    size_t frval;
    Elf_Shdr * section;
    size_t maxNameLength = 0;
    for (i = 0; i < sectionCount; i++) {
        if (shdr[i].field.sh_alloc) {
            sectionList.push_back(&shdr[i]);
            size_t l = strlen(shdr[i].sh_name_str);
            if (maxNameLength < l) { maxNameLength = l; }
        }
    }
    std::sort(sectionList.begin(), sectionList.end(), CompareSection);
    int execVector = shflagGroup.GetValueVector("X");
    for (i = 0, e = sectionList.size(); i < e; ++i) {
        section = sectionList[i];
#if !defined(PROC_64BIT)
        if (section->sh_addr + section->sh_size >= 0x100000000LL) { ELF_ERROR(X_3) }
#endif
        int sz = (int)section->sh_size;
        int addr = (int)section->sh_addr;
        if (strcmp(shtypeGroup.GetValue(section->sh_type)->name, "NOBITS") != 0) {
            section->image = new Image(sz, addr);
            rewind(fp);
            fseek(fp, (long)section->sh_offset, SEEK_SET);
            if ((frval = fread(section->image->mem, 1, sz, fp)) != sz) { ELF_ERROR(X_4) }
        }
        section->field.executable = (execVector & section->sh_flags) != 0;
        if (fout) { section->PrintImage(fout, maxNameLength); }
    }
    return true;
}

#if 1   /// 1/17/25
void ELF_Parser::Info::printDissassembler(void(*p)(unsigned ir, unsigned addr), unsigned char* image, unsigned addr, unsigned sz) {
    unsigned end = addr + sz;
    for (; addr < end; addr += 4) {
        p(*(unsigned*)(image + addr), addr);
    }
}
#endif

struct BinFileWriter {
    FILE* fp;
    unsigned wordCount, checkSum;
    ELF_Parser::Info &info;
    void AddWord(unsigned val, bool noUpdate = false) {
        fwrite(&val, 4, 1, fp);
        if (!noUpdate) {
            wordCount++;
            checkSum += val;
        }
    }
#if 0   /// debugging....
BinFileWriter(FILE* fp0, ELF_Parser::Info& info0) : fp(fp0), wordCount(0), checkSum(0), info(info0) {
    if (!fp) { return; }
    AddWord((unsigned)info.elfHeader.ehdr.e_entry); /// entry_point
    auto& sectionList = info.sectionTable.sectionList;
    AddWord((unsigned)sectionList.size());          /// section count
    int count = 0;
    printf("%3d: %8x\n", count++, (unsigned)info.elfHeader.ehdr.e_entry);
    printf("%3d: %8x\n", count++, (unsigned)sectionList.size());
    for (unsigned i = 0, e = (unsigned)sectionList.size(); i < e; ++i) {
        auto* section = sectionList[i];
        unsigned minAddr = (unsigned)section->sh_addr;
        unsigned size = (section->image) ? (unsigned)section->sh_size : 0;
        AddWord(minAddr);
        AddWord(size);
        printf("%3d: %8x\n", count++, minAddr);
        printf("%3d: %8x\n", count++, size);
    }
}
#else
    BinFileWriter(FILE* fp0, ELF_Parser::Info& info0) : fp(fp0), wordCount(0), checkSum(0), info(info0) {
        if (!fp) { return; }
        AddWord((unsigned)info.elfHeader.ehdr.e_entry); /// entry_point
        auto& sectionList = info.sectionTable.sectionList;
        AddWord((unsigned)sectionList.size());          /// section count
        for (unsigned i = 0, e = (unsigned)sectionList.size(); i < e; ++i) {
            auto* section = sectionList[i];
            unsigned minAddr = (unsigned)section->sh_addr;
            unsigned size = (section->image) ? (unsigned)section->sh_size : 0;
            AddWord(minAddr);
            AddWord(size);
            for (unsigned j = 0; j < size; j += 4) {
                AddWord(*(unsigned*)(section->image->mem + j));
            }    
        }
        AddWord(wordCount, true/*noUpdate*/);
        AddWord(checkSum, true/*noUpdate*/);
        fclose(fp);
        printf("entry_point = 0x%x\n", (unsigned)info.elfHeader.ehdr.e_entry);
        printf("section_count = %d\n", (unsigned)sectionList.size());
        printf("word_count = %d\n", wordCount);
        printf("check_sum = 0x%x\n", checkSum);
        printf("total bytes = %d\n", (wordCount + 2) << 2);
    }
#endif
};

#if 1   /// 11/17/24 : use address offset, don't install executable sections in dmem
bool ELF_Parser::Info::InstallMemory(int memDumpFlag, void(*p)(unsigned ir, unsigned addr))
{
    Elf_Shdr* section;
    auto& sectionList = sectionTable.sectionList;

    bool flag = true;
    /// 1/17/25
#if 1
    bool dmemUsed = dmemInfo.mem != 0;
    bool pmemUsed = pmemInfo.mem != 0;
#else
    bool dmemOpened = (dmemInfo.fp[0] != 0);
    bool pmemOpened = (pmemInfo.fp[0] != 0);
#endif
    C2R_ASSERT(dmemUsed || pmemUsed);
    bool unifiedMemoryFlag = (!dmemUsed || !pmemUsed);

    for (size_t i = 0, e = sectionList.size(); i < e; ++i) {
        section = sectionList[i];
        Elf64_Addr minAddr = section->sh_addr, maxAddr = section->sh_addr + section->sh_size;
#if 1   /// 1/17/25
        if (pmemUsed && section->field.executable) {
#else
        if (section->field.executable) {
#endif
            minAddr -= pmemInfo.addrOffset;
            maxAddr -= pmemInfo.addrOffset;
            C2R_ASSERT(section->image->mem);
            if ((size_t)maxAddr >= pmemInfo.getTotalByteNum()) {
                printf("section (%s : %08llx - %08llx) cannot fit into pmem(size = %08x)...\n",
                    section->sh_name_str, minAddr, maxAddr, (unsigned)pmemInfo.getTotalByteNum());
                flag = false;
            }
            else { memcpy(pmemInfo.mem + minAddr, section->image->mem, (size_t)section->sh_size); }
        }
#if 1   /// 1/17/25
        if ((!section->field.executable || !pmemUsed) || !dmemInfo.addrOffset) {  /// !executable
#else
        if (!section->field.executable || !dmemInfo.addrOffset) {  /// !executable
#endif
            minAddr -= dmemInfo.addrOffset;
            maxAddr -= dmemInfo.addrOffset;
            if ((size_t)maxAddr >= dmemInfo.getTotalByteNum()) {
                printf("section (%s : %08llx - %08llx) cannot fit into dmem(size = %08x)...\n",
                    section->sh_name_str, minAddr, maxAddr, (unsigned)dmemInfo.getTotalByteNum());
                flag = false;
            }
            else if (!section->image) { memset(dmemInfo.mem + minAddr, 0, (size_t)section->sh_size); }
            else { memcpy(dmemInfo.mem + minAddr, section->image->mem, (size_t)section->sh_size); }
        }
#if 1   /// 1/17/25
        if (section->field.executable && p) {
            printDissassembler(p, dmemInfo.mem, (unsigned)minAddr, (unsigned)section->sh_size);
        }
#endif
    }

    if (!flag || !memDumpFlag) { return flag; }
    for (size_t i = 0, e = sectionList.size(); i < e; ++i) {
        section = sectionList[i];
#if 1   /// 1/17/25
        MemoryInfo* mi;
        if (pmemUsed && dmemUsed) {
            mi = (section->field.executable) ? &pmemInfo : &dmemInfo;
        }
        else {
            mi = (pmemUsed) ? &pmemInfo : &dmemInfo;
        }
#else
        MemoryInfo* mi = (section->field.executable) ? &pmemInfo : &dmemInfo;
#endif
        InstallSection(section, mi);
    }
    pmemInfo.Close();
    dmemInfo.Close();
    return flag;
}
#else
/// 4/11/22
bool ELF_Parser::Info::InstallMemory(int memDumpFlag)
{
    Elf_Shdr * section;
    auto &sectionList = sectionTable.sectionList;

    bool flag = true;

    for (size_t i = 0, e = sectionList.size(); i < e; ++i) {
        section = sectionList[i];
        Elf64_Addr minAddr = section->sh_addr, maxAddr = section->sh_addr + section->sh_size;
        if (section->field.executable) {
            C2R_ASSERT(section->image->mem);
            if ((size_t)maxAddr >= pmemInfo.getTotalByteNum()) {
                printf("section (%s : %08llx - %08llx) cannot fit into pmem(size = %08llx)...\n",
                    section->sh_name_str, minAddr, maxAddr, pmemInfo.getTotalByteNum());
                flag = false;
            }
            else { memcpy(pmemInfo.mem + minAddr, section->image->mem, (size_t)section->sh_size); }
        }
        if ((size_t)maxAddr >= dmemInfo.getTotalByteNum()) {
            printf("section (%s : %08llx - %08llx) cannot fit into dmem(size = %08llx)...\n",
                section->sh_name_str, minAddr, maxAddr, dmemInfo.getTotalByteNum());
            flag = false;
        }
        else if (!section->image) { memset(dmemInfo.mem + minAddr, 0, (size_t)section->sh_size); }
        else { memcpy(dmemInfo.mem + minAddr, section->image->mem, (size_t)section->sh_size); }
    }

    if (!flag || !memDumpFlag) { return flag; }
    for (size_t i = 0, e = sectionList.size(); i < e; ++i) {
        section = sectionList[i];
        MemoryInfo *mi = (section->field.executable) ? &pmemInfo : &dmemInfo;
        InstallSection(section, mi);
        if (section->field.executable) { InstallSection(section, &dmemInfo); }
    }
    pmemInfo.Close();
    dmemInfo.Close();
    return flag;
}
#endif

template<typename T, unsigned wordByteNum> void TMPInstallSection();

bool ELF_Parser::Info::InstallSection(Elf_Shdr * section, MemoryInfo *mi) {
    auto minAddr = section->sh_addr, maxAddr = section->sh_addr + section->sh_size;
    auto minAddr2 = minAddr - mi->addrOffset, maxAddr2 = maxAddr - mi->addrOffset;
    C2R_ASSERT(mi->wordByteNum == 4 || mi->wordByteNum == 8);
    if (!dontPrintParseLog) {   /// 11/17/24
        printf("InstallSection(%s) : %08x(%08x) : %08x(%08x)\n", section->sh_name_str,
            (unsigned)minAddr, (unsigned)minAddr2, (unsigned)maxAddr, (unsigned)maxAddr2);
    }
    auto addrMask = mi->wordByteNum - 1;
    /// 11/17/24
    auto alignedStartAddr = (minAddr2 & (~addrMask));
    auto alignedEndAddr = (maxAddr2 & (~addrMask)) + ((maxAddr2 & addrMask) ? mi->wordByteNum : 0);
    if (alignedStartAddr < mi->installedSectionEndAddr) {
        alignedStartAddr = mi->installedSectionEndAddr;
    }
    mi->installedSectionEndAddr = alignedEndAddr;
    auto sectionSize = alignedEndAddr - alignedStartAddr;
    if (mi->wordByteNum == 8) { mi->DumpFile<unsigned long long>(alignedStartAddr, sectionSize); }
    else { mi->DumpFile<unsigned>(alignedStartAddr, sectionSize); }
    return true;
}

void ELF_Parser::Elf_Shdr::PrintImage(FILE *fp, size_t maxNameLength)
{
    std::stringstream f;
    shflagGroup.GetValueMask((int)sh_flags, f);
    C2R_ASSERT(fp);
    fprintf(fp, "-------------------------------------------------------\n");
    fprintf(fp, "Image(Section: %*s) : addr(%08llx - %08llx) [%4s]\n",
        (int)maxNameLength, sh_name_str, sh_addr, sh_addr + sh_size, f.str().c_str());
    if (image == 0) { fprintf(fp, "ALL ZEROES...\n"); return; }
    unsigned char * mem = image->mem;
    /// 8 words (32 bytes) per line
    Elf64_Addr addr0 = sh_addr & ~0x1f, maxAddr = sh_addr + sh_size;
    unsigned i;
    if (sh_addr & 0x1f) {
        fprintf(fp, "%08llx:", addr0);
        for (i = 0; i < 32; ++i) {
            fprintf(fp, "%s", (i & 0x3) ? " " : "|");
            if (addr0 + i < sh_addr) { fprintf(fp, "--"); }
            else fprintf(fp, "%02x", mem[addr0 + i - sh_addr]);
        }
        fprintf(fp, "\n");
        addr0 += 32;
    }
    int zeroRows = 0;
    while (addr0 < maxAddr) {
        bool nonZero = false;
        for (i = 0; i < 32; ++i) {
            if (addr0 + i < maxAddr && mem[addr0 + i - sh_addr]) { nonZero = true; break; }
        }
        if (nonZero) {
            zeroRows = 0;
            fprintf(fp, "%08llx:", addr0);
            for (i = 0; i < 32; ++i) {
                fprintf(fp, "%s", (i & 0x3) ? " " : "|");
                if (addr0 + i >= maxAddr) { fprintf(fp, "--"); }
                else fprintf(fp, "%02x", mem[addr0 + i - sh_addr]);
            }
            fprintf(fp, "\n");
        }
        else {
            if (!zeroRows) { fprintf(fp, "%08llx: ... all 0s ...\n", addr0); }
            ++zeroRows;
        }
        addr0 += 32;
    }
}

ELF_Parser::Elf_Sym* ELF_Parser::Info::GetSymbolAddress(const char* symbolName) {
    auto* symTable = symbolTable.symTable;
    for (size_t i = 0, e = symbolTable.symbolCount; i < e; ++i) {
        auto& sym = symTable[i];
        if (strcmp(sym.st_name_str, symbolName) == 0) {
            return &sym;
        }
    }
    return NULL;
}

bool ELF_Parser::Info::ParseProgramHeader(FILE* fp) {
    Reset();
    if (!elfHeader.Parse(fp)) { return false; }
    if (elfHeader.ehdr.e_phnum) {
        if (!segmentTable.Parse(this, fp)) { return false; }
    }
    return true;
}

void ELF_Parser::Loader::Create(char* edata, unsigned esize) {// : elfArray(edata, esize) {
    elfArray.Set(edata, esize);
    Reset();
    if (elfHeader.Parse(&elfArray) && elfHeader.ehdr.e_phnum && segmentTable.Parse(&elfHeader, &elfArray)) {
        if (curInfo) {
            printf("------------ ELF_Parser::Loader::Loader() ----------------\n");
            elfHeader.EmitMessage();
            segmentTable.EmitMessage(NULL);
        }
        ComputeMaxAllocAddr();
    }
}

bool ELF_Parser::Loader::ComputeMaxAllocAddr() {
    auto* ehdr = &elfHeader;
    auto sectionCount = ehdr->ehdr.e_shnum;
    C2R_ASSERT(sectionCount);
    elfArray.Seek((long)ehdr->ehdr.e_shoff);
    int allocVec = shflagGroup.GetValueVector("A");
    int i;
    size_t frval;
    Elf32_Shdr s32;
    Elf64_Shdr s64;
    Elf_Shdr shdr;
    maxAllocAddr = 0;
    for (i = 0; i < sectionCount; i++) {
        if (ehdr->IsElf32()) {
            if ((frval = elfArray.Read(&s32, 1, sizeof(Elf32_Shdr))) != sizeof(Elf32_Shdr)) { ELF_ERROR(SH32) }
            shdr.Copy(&s32);
        }
        else {
            if ((frval = elfArray.Read(&s64, 1, sizeof(Elf64_Shdr))) != sizeof(Elf64_Shdr)) { ELF_ERROR(SH64) }
            shdr.Copy(&s64);
        }
        if (shdr.sh_flags & allocVec) {
            Elf64_Addr addr = (shdr.sh_addr + shdr.sh_size);
            if (addr > maxAllocAddr)
                maxAllocAddr = addr;
        }
    }
    return true;
}

#include <string>

#define PRINTF_ADDR_SIZE(addr_width)	(((addr_width) / 4) + (((addr_width) % 4) != 0))

void ELF_Parser::ElfMemoryInstaller::openMemDumpFile(int type, FILE **fp, int *segmentCount, int *printfAddrWidth,
    int *printfDataWidth, int doOpen) {
    for (unsigned i = 0; i < 8; ++i) { fp[i] = 0; }
    if (type == 0) {
        *segmentCount = (int)wordByteNum;	///	separate byte-enable
        std::string fnamePrefix = RTL_V_MemInitDir_Name; fnamePrefix += dmemName; fnamePrefix += "_";
        for (unsigned i = 0; i < wordByteNum; ++i) {
            std::string fname = fnamePrefix;
            fname += std::to_string(i) + ".log";
		    fp[i] = (doOpen) ? fopen(fname.c_str(), "w") : 0;
            if (fp[i]) {
                fprintf(fp[i], "/// elfFileName : %s, Data Memory(byte #%d)\n", elfFileName, i);
            }
            if (doOpen && !Info::dontPrintParseLog) {
                printf("fopen (%s) : %s\n", fname.c_str(), (fp[i]) ? "OK" : "NG");
            }
        }
        *printfAddrWidth = PRINTF_ADDR_SIZE(dmemAddrBitNum);
        *printfDataWidth = 2;
    }
    else {
        *segmentCount = 1;	///	single word
        std::string fname = RTL_V_MemInitDir_Name; fname += pmemName; fname += ".log";
        fp[0] = (doOpen) ? fopen(fname.c_str(), "w") : 0;
        if (fp[0]) {
            fprintf(fp[0], "/// elfFileName : %s, Program Memory\n", elfFileName);
        }
        if (doOpen && !Info::dontPrintParseLog) {
            printf("fopen (%s) : %s\n", fname.c_str(), (fp[0]) ? "OK" : "NG");
        }
        *printfAddrWidth = PRINTF_ADDR_SIZE(pmemAddrBitNum);
        *printfDataWidth = wordByteNum * 2;// 8;
    }
}

#ifdef _WIN32  /// 11/26/21 : moved from bottom...
#include <direct.h>
#define getcwd          _getcwd
#define chdir           _chdir
#define mkdir(d, mode)  _mkdir(d)
#define stricmp         _stricmp
#define PATH_SEPARATOR  "\\"
#else
#include <unistd.h>
#include <sys/stat.h>
#define PATH_SEPARATOR  "/"
#endif

void ELF_Parser::ElfMemoryInstaller::createMemInitFile() {
    std::string dir = RTL_V_MemInitDir_Name, localdir;
    int cd_count = 0;
    while (!dir.empty()) {
        auto p1 = dir.find_first_of('/');
        if (p1 == 0) {
            return; /// don't create directory at root
        }
        if (p1 != std::string::npos && p1) {
            localdir = dir.substr(0, p1 + 1);
            dir = dir.substr(p1 + 1);
            const char* chname = localdir.c_str();
            int flag = chdir(chname);
            if (flag) {
                printf("chdir \"%s\" failed...\n", chname);
                if (mkdir(chname, 0777)) {
                    printf("mkdir \"%s\" failed...\n", chname);
                    break;
                }
                else {
                    printf("mkdir \"%s\" succeeded...\n", chname);
                    flag = chdir(chname);
                    printf("chdir \"%s\" %s...\n", chname, (flag) ? "failed" : "succeeded");
                }
            }
            cd_count++;
        }
    }
    bool memInitFileCreated = false;
    while (cd_count) {
        chdir("../");
        cd_count--;
        if (!memInitFileCreated) {
            memInitFileCreated = true;
            std::string fn = memModuleName;
            fn += "_MemInitFileName.v";
            FILE* fp = fopen(fn.c_str(), "w");
            if (fp) {
                if (*dmemName) {
                    for (unsigned i = 0; i < wordByteNum; ++i) {
                        fprintf(fp, "`define MEM_INIT_%s_%d \"%s%s_%d.log\"\n", dmemName, i, localdir.c_str(), dmemName, i);
                    }
                }
                if (*pmemName) {
                    fprintf(fp, "`define MEM_INIT_%s \"%s%s.log\"\n", pmemName, localdir.c_str(), pmemName);
                }
                fclose(fp);
            }
        }
    }
}

void ELF_Parser::ElfMemoryInstaller::createRV32BinFile(ELF_Parser::Info& elfInfo) {
    std::string fn = elfFileName;
    auto p1 = fn.find_last_of('/');
    if (p1 != std::string::npos) {
        fn = fn.substr(p1 + 1);
    }
    p1 = fn.find_last_of('.');
    if (p1 != std::string::npos) {
        fn = fn.substr(0, p1);
    }
    fn += ".bin";
    FILE* fp_bin = fopen(fn.c_str(), "wb");
    BinFileWriter bfw(fp_bin, elfInfo);
}

void ELF_Parser::ElfMemoryInstaller::InstallElf() {
    if (wordByteNum != 4 && wordByteNum != 8) {
        printf("ElfMemoryInstaller : wordByteNum (%d) should be 4 or 8\n", wordByteNum);
        exit(-1);
    }
    unsigned wordByteNumBits = (wordByteNum == 4) ? 2 : 3;
    FILE *fp = fopen(elfFileName, "rb");
    if (fp) {
        ELF_Parser::Info elfInfo;
        elfInfo.Parse(fp);
#if 1   /// 1/31/25
        if (addSymbolMap) {
            addSymbolMap((unsigned)elfInfo.elfHeader.ehdr.e_entry, "_start");
            auto& symTable = elfInfo.symbolTable;
            auto* typeFunc = ELF_Parser::sttypeGroup.GetValue("FUNC");
            for (int i = 0, e = (int)symTable.symbolCount; i < e; ++i) {
                auto* sym = &symTable.symTable[i];
                auto* val = ELF_Parser::sttypeGroup.GetValue(sym->st_type);
                if (val == typeFunc) {
                    addSymbolMap((unsigned)sym->st_value, sym->st_name_str);
                }
            }
        }
#endif
#if 1   /// 3/2/25
        createMemInitFile();
#endif
        FILE *fp_out[8] = { 0 };
        /// 4/12/22
        unsigned dmem_word_size = 1 << dmemAddrBitNum;
        unsigned pmem_word_size = 1 << pmemAddrBitNum;
        int segmentCount = 0, printfAddrWidth, printfDataWidth;
        if (dmem) {
            openMemDumpFile(0, fp_out, &segmentCount, &printfAddrWidth, &printfDataWidth, memDumpFlag);
            elfInfo.dmemInfo.Set((unsigned char*)dmem, dmem_word_size, printfAddrWidth, printfDataWidth, segmentCount, fp_out, wordByteNumBits);
            elfInfo.dmemInfo.addrOffset = dmemAddrOffset;   /// 11/17/24
        }
        if (pmem) {
            openMemDumpFile(1, fp_out, &segmentCount, &printfAddrWidth, &printfDataWidth, memDumpFlag);
            elfInfo.pmemInfo.Set((unsigned char*)pmem, pmem_word_size, printfAddrWidth, printfDataWidth, segmentCount, fp_out, wordByteNumBits);
            elfInfo.pmemInfo.addrOffset = pmemAddrOffset;   /// 11/17/24
        }
        bool flag = true;
        if (dmem || pmem) {
            flag = elfInfo.InstallMemory(memDumpFlag, printRVInst);
#if 1
            createRV32BinFile(elfInfo);
#else
            std::string binFileName = dmemName; binFileName += ".bin";
            FILE *fp_bin = fopen(binFileName.c_str(), "wb");
            BinFileWriter bfw(fp_bin, elfInfo);
#endif
        }
        if (pc) {
            switch (wordByteNum) {
            case 4:
                *(unsigned*)pc = (unsigned)elfInfo.elfHeader.ehdr.e_entry + pc_offset;
                if (!Info::dontPrintParseLog) {
                    printf("pc : %08x\n", *(unsigned*)pc);
                }
                break;
            case 8:
                *(unsigned long long*)pc = (unsigned long long)elfInfo.elfHeader.ehdr.e_entry + pc_offset;
                if (!Info::dontPrintParseLog) {
                    printf("pc : %08llx\n", *(unsigned long long*)pc);
                }
                break;
            }
        }
#define STACK_MARGIN	0x100
        if (sp) {
            unsigned dmem_size = 1 << (dmemAddrBitNum + wordByteNumBits);
            switch (wordByteNum) {
            case 4:
                *(unsigned*)sp = (unsigned)(dmem_size - STACK_MARGIN);
                if (!Info::dontPrintParseLog) {
                    printf("sp : %08x\n", *(unsigned*)sp);
                }
                break;
            case 8:
                *(unsigned long long*)sp = (unsigned long long)(dmem_size - STACK_MARGIN);
                if (!Info::dontPrintParseLog) {
                    printf("sp : %08llx\n", *(unsigned long long*)sp);
                }
                break;
            }
        }
        elfInfo.hostSym.Reset();
        if (auto* sym = elfInfo.GetSymbolAddress("tohost")) {
            Info::hostSym.tohost = sym->st_value;
            C2R_ASSERT((Info::hostSym.tohost & 0x7) == 0);  /// make sure tohost is aligned to 8 bytes
        }
        if (auto* sym = elfInfo.GetSymbolAddress("fromhost")) {
            Info::hostSym.fromhost = sym->st_value;
            C2R_ASSERT((Info::hostSym.fromhost & 0x7) == 0);  /// make sure fromhost is aligned to 8 bytes
        }
        fclose(fp);
    }
}
