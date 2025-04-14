
#define TEST_VS
#if !defined(TEST_VS)
#define TEST_RV_BOOT_LOADER
#define USE_UART
#define UART_16550
#endif
#if defined(TEST_RV_BOOT_LOADER)

#if defined(UART_16550)	
#define	UART_RBR	0	// Receiver Buffer Regsiter
#define	UART_THR	0	// Transmitter Holding Regsiter
#define	UART_IER	1	// Interrupt Enable Regsiter
#define	UART_IIR	2	// Interrupt Identification Regsiter
#define	UART_FCR	2	// FIFO Control Regsiter
#define	UART_LCR	3	// Line Control Regsiter
#define	UART_MCR	4	// Modem Control Regsiter
#define	UART_LSR	5	// Line Status Regsiter
#define	UART_MSR	6	// Modem Status Regsiter
#define	UART_SCR	7	// Scratch Regsiter
#define	UART_DLL	0	// Divisor Latch (Least Significatn Byte) Regsiter
#define	UART_DLM	1	// Divisor Latch (Most Significatn Byte) Regsiter

static volatile unsigned int *uart = (volatile unsigned int *)0x64011000;
#elif defined(UART_XILINX)	
static volatile unsigned int *uart = (volatile unsigned int *)0x10000000;
#else
// UART status ((rx_error << 4) | (rx_empty << 3) | (rx_full << 2) | (tx_empty << 1) | (tx_full));
static volatile char *uart = (volatile char *)0x10000000;
#endif

static void io_putch(char ch);

static void __attribute__((constructor)) _io_init(void)
{
#if defined(USE_UART)
#if defined(UART_16550)	
    unsigned int divisor = 125000000 / (16 * 115200);

    uart[UART_IER] = 0x00;	// IER <= disable interrupts
    uart[UART_LCR] = 0x80;	// LCR <= Enable DLAB
    uart[UART_DLL] = (unsigned char)divisor; // DLL <= divisor low
    uart[UART_DLM] = (unsigned char)(divisor >> 8); // DLM <= divisor high
    uart[UART_LCR] = 0x03; // LCR <= 8 bits, no parity, one stop bit
    uart[UART_FCR] = 0xc7; // FCR <= Enable FIFO, clear, 14-byte thresh
#elif defined(UART_XILINX)	
    uart[3] = 0x3;	// Control: Rst tx, Rst rx.
#else
    //(activate) : baud_period(2), bit_count(8), stop_count(1), parity_type(3)
    *(unsigned short *)(uart + 4) = 1 | (15 << 1) | (3 << 5) | (0 << 7) | (3 << 8);
#endif
#endif
}

/// (uart + 4) uart_read_status : 
/// ((rx_error << 4) | (rx_empty << 3) | (rx_full << 2) | (tx_empty << 1) | (tx_full))

static void __attribute__((destructor)) _io_fini(void)
{
#if defined(USE_UART)
#if defined(UART_16550)	
    while ((uart[UART_LSR] & 0x40) == 0)
        ;
#elif defined(UART_XILINX)	
    while ((uart[2] & 4) == 0)  /// while (!(Status & Tx_Empty))
        ;
#else
    while ((*(unsigned int *)(uart + 4) & 2) == 0)  /// while (tx_empty == 0);
        ;
#endif
#endif
}

#if defined(USE_UART) || defined(USE_FIFO)
static void io_putch(char ch)
{
#if defined(USE_UART)
#if defined(UART_16550)	
    while ((uart[UART_LSR] & 0x20) == 0)
        ;
    uart[UART_THR] = ch;
#elif defined(UART_XILINX)	
    while (uart[2] & 8) /// while (Status & Tx_Full)
        ;
    uart[1] = ch;
#else
    while (*(unsigned int *)(uart + 4) & 1) /// while (tx_full == 1);
        ;
    *uart = ch;
#endif
#elif defined(USE_FIFO) /// 0x10000030; /// FIFO-char address
    *(char *)(fifo + 16) = ch;
#endif
}

static void putline(const char *s)
{
    char ch;

    while (ch = *s++)
        io_putch(ch);
}

int puts(const char *s)
{
    putline(s);
    io_putch('\n');

    return 1;
}

/// 11/18/21 : added
int putchar(int c)
{
    io_putch((char)c);
    return c;
}
#endif
#endif

#include <stdarg.h>

#if !defined(TEST_RV_BOOT_LOADER)
#include <stdio.h>
void mcall_console_putchar(int c) {
    putchar(c);
}
#endif

void ELputstring(const char* s)
{
    while (*s)
        mcall_console_putchar(*s++);
}

struct ELvsnprintf {
    int format, longarg, leftJustify, fillZero, numDigits;
    size_t pos;
    char tmp[256];
    void ResetFlags() { format = 0; longarg = 0; leftJustify = 0; fillZero = 0; numDigits = 0; }
    void SetOutput(char* out, size_t n, char tmp[], int len) {
        while (numDigits > len && !leftJustify) {
            if (++pos < n) { out[pos - 1] = (fillZero) ? '0' : ' '; }
            numDigits--;
        }
        for (int i = len - 1; i >= 0; --i) {
            if (++pos < n) { out[pos - 1] = tmp[i]; }
        }
        while (numDigits > len && leftJustify) {
            if (++pos < n) { out[pos - 1] = ' '; }
            numDigits--;
        }
    }
    ELvsnprintf(char* out, size_t n, const char* s, va_list vl) {
        /// sflg : signed or unsigned
#define FETCH_ARGVAL(sflg)   (longarg == 2) ? va_arg(vl, sflg long long) : (longarg == 1) ? va_arg(vl, sflg long) : va_arg(vl, sflg int)
        ResetFlags();
        pos = 0;
        for (; *s; s++) {
            if (format) {
                switch (*s) {
                case 'l':
                    longarg = (*(s + 1) == 'l') ? 2 : 1;    /// ll (long long) --> 2, l (long) --> 1
                    if (longarg == 2) { s++; }
                    break;
                case 'x':
                {
                    auto num = FETCH_ARGVAL(unsigned);
                    int i = 0;
                    while (num) {
                        int d = (num % 16);
                        num /= 16;
                        if ((++i) < 256) { tmp[i - 1] = (d < 10) ? '0' + d : 'a' + d - 10; }  /// tmp[] : reverse order
                    }
                    SetOutput(out, n, tmp, i);
                    ResetFlags();
                    break;
                }
                case 'd':
                {
                    auto num = FETCH_ARGVAL(signed);
                    int i = 0, negFlag = (num < 0);
                    if (negFlag) { num = -num; }
                    if (num == 0) { if ((++i) < 256) { tmp[i - 1] = '0'; } }
                    while (num) {
                        int d = (num % 10);
                        num /= 10;
                        if ((++i) < 256) { tmp[i - 1] = '0' + d; }  /// tmp[] : reverse order
                    }
                    if (negFlag && (++i) < 256) { tmp[i - 1] = '-'; }  /// tmp[] : reverse order
                    SetOutput(out, n, tmp, i);
                    ResetFlags();
                    break;
                }
                case 'u':
                {
                    auto num = FETCH_ARGVAL(unsigned);
                    int i = 0;
                    if (num == 0) { if ((++i) < 256) { tmp[i - 1] = '0'; } }
                    while (num) {
                        int d = (num % 10);
                        num /= 10;
                        if ((++i) < 256) { tmp[i - 1] = '0' + d; }  /// tmp[] : reverse order
                    }
                    SetOutput(out, n, tmp, i);
                    ResetFlags();
                    break;
                }
                case 's':
                {
                    const char* s2 = va_arg(vl, const char*);
                    auto* p = s2;
                    while (*p) { p++; }
                    int i = 0;
                    while ((--p) >= s2) {
                        if ((++i) < 256) { tmp[i - 1] = *p; }  /// tmp[] : reverse order
                    }
                    SetOutput(out, n, tmp, i);
                    ResetFlags();
                    break;
                }
                case 'c':
                {
                    if (++pos < n) out[pos - 1] = (char)va_arg(vl, int);
                    ResetFlags();
                    break;
                }
                /// numDigits
                case '0':
                    if (numDigits == 0) {   /// ex: %08x --> fill-0
                        fillZero = 1;
                        break;
                    }   /// if (numDigits != 0) { fall through }
                case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                    numDigits = numDigits * 10 + ((*s) - '0');
                    break;
                case '-':
                    leftJustify = 1;
                    break;
                case '*':   /// numDigits specified in arg
                    numDigits = (int)va_arg(vl, int);
                    break;
                default:
                    break;
                }
            }
            else if (*s == '%')
                format = 1;
            else
                if (++pos < n) out[pos - 1] = *s;
        }
        if (pos < n)
            out[pos] = 0;
        else if (n)
            out[n - 1] = 0;
    }
};

void ELvprintm(const char* s, va_list vl)
{
    char buf[256];
    ELvsnprintf(buf, sizeof buf, s, vl);
    ELputstring(buf);
}

void ELprintf(const char* s, ...)
{
    va_list vl;
    va_start(vl, s);
    ELvprintm(s, vl);
    va_end(vl);
}

#include "ELFLoader.h"

#if 1
const char * ELF_Loader::errorStatusName[ES_COUNT] = {
#define ES_NAME
#include "ElfStatus.h"
#undef ES_NAME
};

ELF_Loader::Value ELF_Loader::ehtypeArray[ET_COUNT] = {
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
ELF_Loader::Value ELF_Loader::osabiArray[ELFOSABI_COUNT] = {
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
ELF_Loader::Value ELF_Loader::machineArray[EM_COUNT] = {
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
ELF_Loader::Value ELF_Loader::ptypeArray[PT_COUNT] = {
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
ELF_Loader::Value ELF_Loader::pflagArray[PF_COUNT] = {
    { "X",			0x1,		"Execute" },
    { "W",			0x2,		"Write" },
    { "R",			0x4,		"Read" },
    { "MASKOS",		0x0ff00000,	"Unspecified" },
    { "MASKPROC",	0xf0000000,	"Unspecified" },
};
ELF_Loader::ValueGroup ELF_Loader::ehtypeGroup(ET_COUNT, ehtypeArray);
ELF_Loader::ValueGroup ELF_Loader::osabiGroup(ELFOSABI_COUNT, osabiArray);
ELF_Loader::ValueGroup ELF_Loader::machineGroup(EM_COUNT, machineArray);
ELF_Loader::ValueGroup ELF_Loader::ptypeGroup(PT_COUNT, ptypeArray);
ELF_Loader::ValueGroup ELF_Loader::pflagGroup(PF_COUNT, pflagArray);

#endif

void ELF_Loader::Ehdr::PrintInfo() {
    PrintMagicNumber();
    PrintClass();
    PrintByteOrder();
    PrintFileVersion();
    osabiGroup.GetValue(e_ident[EI_OSABI])->PrintInfo("OSABI");
    PrintABIVersion();
    ehtypeGroup.GetValue(ehdr.e_type)->PrintInfo("TYPE");
    machineGroup.GetValue(ehdr.e_machine)->PrintInfo("MACHINE");

#define PRINT_ELF_PARAM(n)	ELprintf("%12s = %10d (%08x):\n", #n, (unsigned)ehdr.n, (unsigned)ehdr.n);

    PRINT_ELF_PARAM(e_version);
    PRINT_ELF_PARAM(e_entry);
    PRINT_ELF_PARAM(e_phoff);
    PRINT_ELF_PARAM(e_shoff);
    PRINT_ELF_PARAM(e_flags);
    PRINT_ELF_PARAM(e_ehsize);
    PRINT_ELF_PARAM(e_phentsize);
    PRINT_ELF_PARAM(e_phnum);
    PRINT_ELF_PARAM(e_shentsize);
    PRINT_ELF_PARAM(e_shnum);
    PRINT_ELF_PARAM(e_shstrndx);

#undef PRINT_ELF_PARAM
}

#define MAKE_SILENT

#define ELF_ERROR(n)	{ status.errorFlag = ES_##n; return false;}

bool ELF_Loader::Ehdr::Parse(ELF_Data &elfData)
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

#define ELF_ERROR_POS(n, i, data)	{ status.errorFlag = ES_##n; status.pos = i; status.val = data;	return false;}

bool ELF_Loader::PhdrTable::Parse(Ehdr * ehdr, ELF_Data &elfData, int command)
{
    if (ehdr->ehdr.e_phnum == 0) { return true; }
    segmentCount = ehdr->ehdr.e_phnum;
    int i;
    elfData.Seek((long)ehdr->ehdr.e_phoff);
    size_t frval;
    Elf32_Phdr p32;
    Elf64_Phdr p64;
    Elf_Phdr phdr;
    if (command == 1) { Elf_Phdr::PrintHeader(); }
    for (i = 0; i < segmentCount; i++) {
        if (ehdr->IsElf32()) {
            if ((frval = elfData.Read(&p32, 1, sizeof(Elf32_Phdr))) != sizeof(Elf32_Phdr)) { ELF_ERROR_POS(PH32, i, 0) }
            phdr.Copy(&p32);
        }
        else {
            if ((frval = elfData.Read(&p64, 1, sizeof(Elf64_Phdr))) != sizeof(Elf64_Phdr)) { ELF_ERROR_POS(PH64, i, 0) }
            phdr.Copy(&p64);
        }
        if (ptypeGroup.GetValue(phdr.p_type) == 0) { ELF_ERROR_POS(PHTYPE, i, phdr.p_type) }
        if (command == 1) {
            phdr.PrintInfo(i);
        }
        else if (command == 2) {
            phdr.CopySegment(i);
        }
    }
    return true;
}

void ELF_Loader::Elf_Phdr::PrintHeader()
{
    ELprintf("Program Headers:\n");
    ELprintf("|No.|Type   | Offset | Vaddr  | Paddr  |Filesize|Memsize |Flags|Algn|Sections...\n");
    ELprintf("================================================================================\n");
}

void ELF_Loader::Elf_Phdr::PrintInfo(int id)
{
    ELprintf("|%3d|%-7s|%08x|%08x|%08x|%08x|%08x|", id, 
        ptypeGroup.GetValue(p_type)->name, (unsigned)p_offset, (unsigned)p_vaddr,
        (unsigned)p_paddr, (unsigned)p_filesz, (unsigned)p_memsz);
    pflagGroup.PrintValueMask((int)p_flags, 5);
    ELprintf("|%04x|\n", (unsigned)p_align);
}

#define BOOT_ROM_OFFSET 0x00010000

void ELF_Loader::Elf_Phdr::CopySegment(int id)
{
    if (p_type == 1) {  /// LOAD
        ELprintf("Copy segment (%d : %-7s) : p_offset(%08x), p_filesz(%08x) --> p_paddr(%08x)\n",
            id, ptypeGroup.GetValue(p_type)->name, (unsigned)p_offset, (unsigned)p_filesz, (unsigned)p_paddr);
#if 0
        char* ddr = (char*)p_paddr;
        const char* boot_rom = ((const char*)BOOT_ROM) + p_offset;
        memcpy(ddr, boot_rom, (size_t)p_filesz);
#endif
    }
}

ELF_Loader::Loader::Loader(char* edata, unsigned esize) : elfData(edata, esize) {
    Reset();
    ELprintf("------------ ELF_Loader::Loader::Loader() ----------------\n");
    if (elfHeader.Parse(elfData) && elfHeader.ehdr.e_phnum && segmentTable.Parse(&elfHeader, elfData, false)) {
        elfHeader.PrintInfo();
        segmentTable.Parse(&elfHeader, elfData, true);
    }
}
