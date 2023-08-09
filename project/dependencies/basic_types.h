#pragma region Basic Types

#define GEN_U8_MIN 0u
#define GEN_U8_MAX 0xffu
#define GEN_I8_MIN ( -0x7f - 1 )
#define GEN_I8_MAX 0x7f

#define GEN_U16_MIN 0u
#define GEN_U16_MAX 0xffffu
#define GEN_I16_MIN ( -0x7fff - 1 )
#define GEN_I16_MAX 0x7fff

#define GEN_U32_MIN 0u
#define GEN_U32_MAX 0xffffffffu
#define GEN_I32_MIN ( -0x7fffffff - 1 )
#define GEN_I32_MAX 0x7fffffff

#define GEN_U64_MIN 0ull
#define GEN_U64_MAX 0xffffffffffffffffull
#define GEN_I64_MIN ( -0x7fffffffffffffffll - 1 )
#define GEN_I64_MAX 0x7fffffffffffffffll

#if defined( GEN_ARCH_32_BIT )
#	define GEN_USIZE_MIN GEN_U32_MIN
#	define GEN_USIZE_MAX GEN_U32_MAX
#	define GEN_ISIZE_MIN GEN_S32_MIN
#	define GEN_ISIZE_MAX GEN_S32_MAX
#elif defined( GEN_ARCH_64_BIT )
#	define GEN_USIZE_MIN GEN_U64_MIN
#	define GEN_USIZE_MAX GEN_U64_MAX
#	define GEN_ISIZE_MIN GEN_I64_MIN
#	define GEN_ISIZE_MAX GEN_I64_MAX
#else
#	error Unknown architecture size. This library only supports 32 bit and 64 bit architectures.
#endif

#define GEN_F32_MIN 1.17549435e-38f
#define GEN_F32_MAX 3.40282347e+38f
#define GEN_F64_MIN 2.2250738585072014e-308
#define GEN_F64_MAX 1.7976931348623157e+308

#if defined( GEN_COMPILER_MSVC )
#	if _MSC_VER < 1300
typedef unsigned char  gen_u8;
typedef signed char    gen_s8;
typedef unsigned short gen_u16;
typedef signed short   gen_s16;
typedef unsigned int   gen_u32;
typedef signed int     gen_s32;
#    else
typedef unsigned __int8  gen_u8;
typedef signed __int8    gen_s8;
typedef unsigned __int16 gen_u16;
typedef signed __int16   gen_s16;
typedef unsigned __int32 gen_u32;
typedef signed __int32   gen_s32;
#    endif
typedef unsigned __int64 gen_u64;
typedef signed __int64   gen_s64;
#else
#	include <stdint.h>

typedef uint8_t  gen_u8;
typedef int8_t   gen_s8;
typedef uint16_t gen_u16;
typedef int16_t  gen_s16;
typedef uint32_t gen_u32;
typedef int32_t  gen_s32;
typedef uint64_t gen_u64;
typedef int64_t  gen_s64;
#endif

static_assert( sizeof( gen_u8 ) == sizeof( gen_s8 ), "sizeof(u8) != sizeof(s8)" );
static_assert( sizeof( gen_u16 ) == sizeof( gen_s16 ), "sizeof(u16) != sizeof(s16)" );
static_assert( sizeof( gen_u32 ) == sizeof( gen_s32 ), "sizeof(u32) != sizeof(s32)" );
static_assert( sizeof( gen_u64 ) == sizeof( gen_s64 ), "sizeof(u64) != sizeof(s64)" );

static_assert( sizeof( gen_u8 ) == 1, "sizeof(u8) != 1" );
static_assert( sizeof( gen_u16 ) == 2, "sizeof(u16) != 2" );
static_assert( sizeof( gen_u32 ) == 4, "sizeof(u32) != 4" );
static_assert( sizeof( gen_u64 ) == 8, "sizeof(u64) != 8" );

typedef size_t    gen_uw;
typedef ptrdiff_t gen_sw;

static_assert( sizeof( gen_uw ) == sizeof( gen_sw ), "sizeof(uw) != sizeof(sw)" );

// NOTE: (u)zpl_intptr is only here for semantic reasons really as this library will only support 32/64 bit OSes.
#if defined( _WIN64 )
typedef signed __int64   gen_sptr;
typedef unsigned __int64 gen_uptr;
#elif defined( _WIN32 )
// NOTE; To mark types changing their size, e.g. zpl_intptr
#	ifndef _W64
#		if ! defined( __midl ) && ( defined( _X86_ ) || defined( _M_IX86 ) ) && _MSC_VER >= 1300
#			define _W64 __w64
#		else
#			define _W64
#		endif
#	endif
typedef _W64 signed int   gen_sptr;
typedef _W64 unsigned int gen_uptr;
#else
typedef uintptr_t gen_uptr;
typedef intptr_t  gen_sptr;
#endif

static_assert( sizeof( gen_uptr ) == sizeof( gen_sptr ), "sizeof(uptr) != sizeof(sptr)" );

typedef float  gen_f32;
typedef double gen_f64;

static_assert( sizeof( gen_f32 ) == 4, "sizeof(f32) != 4" );
static_assert( sizeof( gen_f64 ) == 8, "sizeof(f64) != 8" );

typedef gen_s8  gen_b8;
typedef gen_s16 gen_b16;
typedef gen_s32 gen_b32;

#if ! defined( __cplusplus )
#	if ( defined( _MSC_VER ) && _MSC_VER < 1800 ) || ( ! defined( _MSC_VER ) && ! defined( __STDC_VERSION__ ) )
#		ifndef true
#			define true( 0 == 0 )
#		endif
#		ifndef false
#			define false( 0 != 0 )
#		endif
typedef gen_b8 bool;
#	else
#		include <stdbool.h>
#	endif
#endif

#pragma endregion Basic Types

