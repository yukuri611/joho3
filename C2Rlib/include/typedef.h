/*!
*   @file typedef.h
*   @brief typedef for C2RTL
*   @author Isao Yamada
*/

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


#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

/*----------------------------------------------------------**
typedef
**----------------------------------------------------------*/

#if defined(__LLVM_C2RTL__)	///	new attribute annotation style on LLVM-C2RTL
#define _BW(N)		    __attribute__((C2RTL_bit_width(N)))
#define _BWT(N)		    __attribute__((C2RTL_bit_width(#N)))
#define _TYPE(N)		__attribute__((C2RTL_type(N)))
#define _C2R_FUNC(N)	__attribute__((C2RTL_function(N))) /// 5/29/17 : changed C2RTL_top_function --> C2RTL_function!!!
#define _C2R_FUNC_T(N)	__attribute__((C2RTL_function(#N))) /// 11/24/17 : accept template paramaeter for pipe-stage
#define _C2R_MODULE_    __attribute__((C2RTL_module))
#define _C2R_CLK_       __attribute__((C2RTL_clock(0)))
#define _C2R_CLK_SET_   __attribute__((C2RTL_clock(1)))
#else
#define _BW(N)


#define _BWT(N)
#define _TYPE(N)
#define _C2R_FUNC(N)    /// 5/29/17 : changed C2RTL_top_function --> C2RTL_function!!!
#define _C2R_FUNC_T(N)  
#define _C2R_MODULE_
#define _C2R_CLK_
#define _C2R_CLK_SET_
#endif


#include <stdint.h>


#if ! (defined(_STDINT) || defined(_STDINT_H))
typedef signed char		    int8_t;
typedef unsigned char	    uint8_t;
typedef signed short		int16_t;
typedef unsigned short	    uint16_t;
typedef signed int		    int32_t;
typedef unsigned int		uint32_t;
typedef signed long long	int64_t;
typedef unsigned long long	uint64_t;
#endif

//#ifndef __cplusplus
/*attribute of bit width for C2RTL*/
typedef uint8_t		BIT		_BW(1);		/**<1bit unsigned integer*/
typedef	int8_t		SINT2	_BW(2);		/**<3bit signed integer*/
typedef	uint8_t		UINT2	_BW(2);		/**<3bit unsigned integer*/
typedef	int8_t		SINT3	_BW(3);		/**<3bit signed integer*/
typedef	uint8_t		UINT3	_BW(3);		/**<3bit unsigned integer*/
typedef	int8_t		SINT4	_BW(4);		/**<4bit signed integer*/
typedef	uint8_t		UINT4	_BW(4);		/**<4bit unsigned integer*/
typedef	int8_t		SINT5	_BW(5);		/**<5bit signed integer*/
typedef	uint8_t		UINT5	_BW(5);		/**<5bit unsigned integer*/
typedef	int8_t		SINT6	_BW(6);		/**<6bit signed integer*/
typedef	uint8_t		UINT6	_BW(6);		/**<6bit unsigned integer*/
typedef	int8_t		SINT7	_BW(7);		/**<7bit signed integer*/
typedef	uint8_t		UINT7	_BW(7);		/**<7bit unsigned integer*/
typedef	int8_t		SINT8	_BW(8);		/**<7bit signed integer*/
typedef	uint8_t		UINT8	_BW(8);		/**<7bit unsigned integer*/
typedef	int16_t		SINT9	_BW(9);		/**<9bit signed integer*/
typedef	uint16_t	UINT9	_BW(9);		/**<9bit unsigned integer*/
typedef	int16_t		SINT10	_BW(10);	/**<10bit signed integer*/
typedef	uint16_t	UINT10	_BW(10);	/**<10bit unsigned integer*/
typedef	int16_t		SINT11	_BW(11);	/**<11bit signed integer*/
typedef	uint16_t	UINT11	_BW(11);	/**<11bit unsigned integer*/
typedef	int16_t		SINT12	_BW(12);	/**<12bit signed integer*/
typedef	uint16_t	UINT12	_BW(12);	/**<12bit unsigned integer*/
typedef	int16_t		SINT13	_BW(13);	/**<13bit signed integer*/
typedef	uint16_t	UINT13	_BW(13);	/**<13bit unsigned integer*/
typedef	int16_t		SINT14	_BW(14);	/**<14bit signed integer*/
typedef	uint16_t	UINT14	_BW(14);	/**<14bit unsigned integer*/
typedef	int16_t		SINT15	_BW(15);	/**<15bit signed integer*/
typedef	uint16_t	UINT15	_BW(15);	/**<15bit unsigned integer*/
typedef	int16_t		SINT16	_BW(16);	/**<16bit signed integer*/
typedef	uint16_t	UINT16	_BW(16);	/**<16bit unsigned integer*/
typedef	int32_t		SINT24	_BW(24);	/**<24bit signed integer*/
typedef	uint32_t	UINT24	_BW(24);	/**<24bit unsigned integer*/
typedef	int32_t		SINT32	_BW(32);	/**<32bit signed integer*/
typedef	uint32_t	UINT32	_BW(32);	/**<32bit unsigned integer*/
typedef	int64_t		SINT64	_BW(64);	/**<64bit signed integer*/
typedef	uint64_t	UINT64	_BW(64);	/**<64bit unsigned integer*/


/*attribute of state,memory for C2RTL*/
///typedef	BIT			ST_BIT	  _TYPE(state), M_BIT	 _TYPE(memory), D_BIT    _TYPE(direct_signal);

#define C2R_ST_M_D(T)    typedef T  ST_##T _TYPE(state), M_##T _TYPE(memory), D_##T _TYPE(direct_signal)
#define C2R_ST_M_D_S_US(T)  C2R_ST_M_D(S##T); C2R_ST_M_D(U##T)

C2R_ST_M_D(BIT);        /// ST_BIT, M_BIT, D_BIT;
C2R_ST_M_D_S_US(INT2);  /// ST_SINT2, M_SINT2, D_SINT2, ST_UINT2, M_UINT2, D_UINT2;
C2R_ST_M_D_S_US(INT3);  /// ST_SINT3, M_SINT3, D_SINT3, ST_UINT3, M_UINT3, D_UINT3;
C2R_ST_M_D_S_US(INT4);
C2R_ST_M_D_S_US(INT5);
C2R_ST_M_D_S_US(INT6);
C2R_ST_M_D_S_US(INT7);
C2R_ST_M_D_S_US(INT8);
C2R_ST_M_D_S_US(INT9);
C2R_ST_M_D_S_US(INT10);
C2R_ST_M_D_S_US(INT11);
C2R_ST_M_D_S_US(INT12);
C2R_ST_M_D_S_US(INT13);
C2R_ST_M_D_S_US(INT14);
C2R_ST_M_D_S_US(INT15);
C2R_ST_M_D_S_US(INT16);
C2R_ST_M_D_S_US(INT32);
C2R_ST_M_D_S_US(INT64);

#undef C2R_ST_M_D
#undef C2R_ST_M_D_S_US

typedef	UINT32		MI_UINT32 _TYPE(memory_init);

////
/////* min max value */
////#if ! (defined(_STDINT) || defined(_STDINT_H))
////#ifdef _MSC_VER				//   
////#define	SINT8_MAX			127i8
////#define	SINT8_MIN			(-127i8 - 1)
////#define UINT8_MAX			0xffui8
////#define	UINT8_MIN			0
////#define	SINT16_MAX			32767i16
////#define	SINT16_MIN			(-32767i16 - 1)
////#define	UINT16_MAX			0xffffui16
////#define	UINT16_MIN			0
////#define	SINT32_MAX			-2147483647i32
////#define	SINT32_MIN			(-2147483647i32 - 1)
////#define	UINT32_MAX			0xffffffffui32
////#define	UINT32_MIN			0
////#define	SINT64_MAX			9223372036854775807i64
////#define	SINT64_MIN			(-9223372036854775807i64 - 1)
////#define	UINT64_MAX			0xffffffffffffffffui64
////#define	UINT64_MIN			0
////#else
////#define	SINT8_MAX			127
////#define	SINT8_MIN			(-127 - 1)
////#define UINT8_MAX			0xff
////#define	UINT8_MIN			0
////#define	SINT16_MAX			32767
////#define	SINT16_MIN			(-32767 - 1)
////#define	UINT16_MAX			0xffff
////#define	UINT16_MIN			0
////#define	SINT32_MAX			-2147483647L
////#define	SINT32_MIN			(-2147483647L - 1)
////#define	UINT32_MAX			0xffffffffUL
////#define	UINT32_MIN			0
////#define	SINT64_MAX			9223372036854775807LL
////#define	SINT64_MIN			(-9223372036854775807LL - 1)
////#define	UINT64_MAX			0xffffffffffffffffULL
////#define	UINT64_MIN			0
////#endif
////#else
////#define	SINT8_MAX			INT8_MAX
////#define	SINT8_MIN			INT8_MIN
////#define	SINT16_MAX			INT16_MAX
////#define	SINT16_MIN			INT16_MIN
////#define	SINT32_MAX			INT32_MAX
////#define	SINT32_MIN			INT32_MIN
////#define	SINT64_MAX			INT64_MAX
////#define	SINT64_MIN			INT64_MIN
////#define	UINT8_MIN			0
////#define	UINT16_MIN			0
////#define	UINT32_MIN			0
////#define	UINT64_MIN			0
////#endif
////
////#define	SINTxx_MAX(xx)		((1<<((xx)-1))-1)
////#define	SINTxx_MIN(xx)		(-(1<<((xx)-1)))
////#define	UINTxx_MAX(xx)		((1<<(xx))-1)
////#define	UINTxx_MIN(xx)		(0)
////
////#define	SINT3_MAX			SINTxx_MAX(3)
////#define	SINT3_MIN			SINTxx_MIN(3)
////#define	UINT3_MAX			UINTxx_MAX(3)
////#define	UINT3_MIN			UINTxx_MIN(3)
////#define	SINT4_MAX			SINTxx_MAX(4)
////#define	SINT4_MIN			SINTxx_MIN(4)
////#define	UINT4_MAX			UINTxx_MAX(4)
////#define	UINT4_MIN			UINTxx_MIN(4)
////#define	SINT5_MAX			SINTxx_MAX(5)
////#define	SINT5_MIN			SINTxx_MIN(5)
////#define	UINT5_MAX			UINTxx_MAX(5)
////#define	UINT5_MIN			UINTxx_MIN(5)
////#define	SINT6_MAX			SINTxx_MAX(6)
////#define	SINT6_MIN			SINTxx_MIN(6)
////#define	UINT6_MAX			UINTxx_MAX(6)
////#define	UINT6_MIN			UINTxx_MIN(6)
////#define	SINT7_MAX			SINTxx_MAX(7)
////#define	SINT7_MIN			SINTxx_MIN(7)
////#define	UINT7_MAX			UINTxx_MAX(7)
////#define	UINT7_MIN			UINTxx_MIN(7)
////
////#define	SINT9_MAX			SINTxx_MAX(9)
////#define	SINT9_MIN			SINTxx_MIN(9)
////#define	UINT9_MAX			UINTxx_MAX(9)
////#define	UINT9_MIN			UINTxx_MIN(9)
////#define	SINT10_MAX			SINTxx_MAX(10)
////#define	SINT10_MIN			SINTxx_MIN(10)
////#define	UINT10_MAX			UINTxx_MAX(10)
////#define	UINT10_MIN			UINTxx_MIN(10)
////#define	SINT11_MAX			SINTxx_MAX(11)
////#define	SINT11_MIN			SINTxx_MIN(11)
////#define	UINT11_MAX			UINTxx_MAX(11)
////#define	UINT11_MIN			UINTxx_MIN(11)
////#define	SINT12_MAX			SINTxx_MAX(12)
////#define	SINT12_MIN			SINTxx_MIN(12)
////#define	UINT12_MAX			UINTxx_MAX(12)
////#define	UINT12_MIN			UINTxx_MIN(12)
////#define	SINT13_MAX			SINTxx_MAX(13)
////#define	SINT13_MIN			SINTxx_MIN(13)
////#define	UINT13_MAX			UINTxx_MAX(13)
////#define	UINT13_MIN			UINTxx_MIN(13)
////#define	SINT14_MAX			SINTxx_MAX(14)
////#define	SINT14_MIN			SINTxx_MIN(14)
////#define	UINT14_MAX			UINTxx_MAX(14)
////#define	UINT14_MIN			UINTxx_MIN(14)
////#define	SINT15_MAX			SINTxx_MAX(15)
////#define	SINT15_MIN			SINTxx_MIN(15)
////#define	UINT15_MAX			UINTxx_MAX(15)
////#define	UINT15_MIN			UINTxx_MIN(15)
////



#endif /*_TYPEDEF_H_*/
