
#if defined(ES_ENUM)
#define ES(n)	ES_##n,
#elif defined(ES_NAME)
#define ES(n)	#n,
#elif defined(ES_CASE)  /// 4/1/22
#define ES(n)	case ES_##n : return #n;
#else
#define ES(n)
#endif

ES(NO_ERROR)
ES(IDENT)	ES(MAGIC_NUM)	ES(CLASS)	ES(DATA)	ES(VERSION)	ES(OSABI)
ES(ELF32)	ES(ELF64)	ES(EHSIZE)	ES(SHENTSIZE)	ES(PHENTSIZE)	ES(EHTYPE)	ES(MACHINE)
ES(SH32)	ES(SH64)	ES(SHTYPE)	ES(STRTAB_0)	ES(STRTAB_1)	ES(STRTAB_2)
ES(SYM_0)	ES(SYM_1)	ES(SYM_2)	ES(SYM_3)	ES(SYM_4)
ES(REL_0)	ES(REL_1)	ES(REL_2)	ES(REL_3)	ES(REL_4)
ES(DYN_0)	ES(DYN_1)	ES(DYN_2)	ES(DYN_3)
ES(PH32)	ES(PH64)	ES(PHTYPE)	ES(PH_INTERP)

ES(X_0)		ES(X_1)		ES(X_2)		ES(X_3)		ES(X_4)		ES(X_5)		ES(X_6)

ES(LNK_0)	ES(LNK_1)	ES(LNK_2)	ES(LNK_3)	ES(LNK_4)	ES(LNK_5)

#undef ES
