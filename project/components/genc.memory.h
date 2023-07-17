#pragma region Memory
//! Checks if value is power of 2.
GEN_DEF_INLINE gen_b32 gen_is_power_of_two( gen_sw x );

//! Aligns address to specified alignment.
GEN_DEF_INLINE void* gen_align_forward( void* ptr, gen_sw alignment );

//! Aligns value to a specified alignment.
GEN_DEF_INLINE gen_s64 gen_align_forward_i64( gen_s64 value, gen_sw alignment );

//! Aligns value to a specified alignment.
GEN_DEF_INLINE gen_u64 gen_align_forward_u64( gen_u64 value, gen_uw alignment );

//! Moves pointer forward by bytes.
GEN_DEF_INLINE void* gen_pointer_add( void* ptr, gen_sw bytes );

//! Moves pointer backward by bytes.
GEN_DEF_INLINE void* gen_pointer_sub( void* ptr, gen_sw bytes );

//! Moves pointer forward by bytes.
GEN_DEF_INLINE void const* gen_pointer_add_const( void const* ptr, gen_sw bytes );

//! Moves pointer backward by bytes.
GEN_DEF_INLINE void const* gen_pointer_sub_const( void const* ptr, gen_sw bytes );

//! Calculates difference between two addresses.
GEN_DEF_INLINE gen_sw gen_pointer_diff( void const* begin, void const* end );

#define gen_ptr_add       gen_pointer_add
#define gen_ptr_sub       gen_pointer_sub
#define gen_ptr_add_const gen_pointer_add_const
#define gen_ptr_sub_const gen_pointer_sub_const
#define gen_ptr_diff      gen_pointer_diff

//! Clears up memory at location by specified size.

//! @param ptr Memory location to clear up.
//! @param size The size to clear up with.
GEN_DEF_INLINE void gen_zero_size( void* ptr, gen_sw size );

#ifndef gen_zero_item
//! Clears up an item.
#	define gen_zero_item( t ) gen_zero_size( ( t ), size_of( *( t ) ) )    // NOTE: Pass pointer of struct

//! Clears up an array.
#	define gen_zero_array( a, count ) gen_zero_size( ( a ), size_of( *( a ) ) * count )
#endif

//! Copy memory from source to destination.
GEN_DEF_INLINE void* gen_mem_move( void* dest, void const* source, gen_sw size );

//! Set constant value at memory location with specified size.
GEN_DEF_INLINE void* gen_mem_set( void* data, gen_u8 byte_value, gen_sw size );

//! Compare two memory locations with specified size.
GEN_DEF_INLINE gen_s32 gen_memcompare( void const* s1, void const* s2, gen_sw size );

//! Swap memory contents between 2 locations with size.
void gen_mem_swap( void* i, void* j, gen_sw size );

//! Search for a constant value within the size limit at memory location.
void const* gen_mem_find( void const* data, gen_u8 byte_value, gen_sw size );

//! Search for a constant value within the size limit at memory location in backwards.
void const* gen_memrchr( void const* data, gen_u8 byte_value, gen_sw size );

//! Copy non-overlapping memory from source to destination.
void* gen_mem_copy( void* dest, void const* source, gen_sw size );

#ifndef gen_memcopy_array

//! Copy non-overlapping array.
#	define gen_memcopy_array( dst, src, count ) gen_mem_copy( ( dst ), ( src ), size_of( *( dst ) ) * ( count ) )
#endif

//! Copy an array.
#ifndef gen_memmove_array
#	define gen_memmove_array( dst, src, count ) gen_mem_move( ( dst ), ( src ), size_of( *( dst ) ) * ( count ) )
#endif

#ifndef GEN_BIT_CAST
#	define GEN_BIT_CAST( dest, source )                                                                                            \
		do                                                                                                                          \
		{                                                                                                                           \
			GEN_STATIC_ASSERT( size_of( *( dest ) ) <= size_of( source ), "size_of(*(dest)) !<= size_of(source)" ); \
			gen_mem_copy( ( dest ), &( source ), size_of( *dest ) );                                                  \
		} while ( 0 )
#endif

#ifndef gen_kilobytes
#	define gen_kilobytes( x ) ( ( x ) * ( gen_s64 )( 1024 ) )
#	define gen_megabytes( x ) ( gen_kilobytes( x ) * ( gen_s64 )( 1024 ) )
#	define gen_gigabytes( x ) ( gen_megabytes( x ) * ( gen_s64 )( 1024 ) )
#	define gen_terabytes( x ) ( gen_gigabytes( x ) * ( gen_s64 )( 1024 ) )
#endif


/* inlines */

#define GEN__ONES          ( zpl_cast( gen_uw ) - 1 / GEN_U8_MAX )
#define GEN__HIGHS         ( GEN__ONES * ( GEN_U8_MAX / 2 + 1 ) )
#define GEN__HAS_ZERO( x ) ( ( ( x )-GEN__ONES ) & ~( x )&GEN__HIGHS )

GEN_IMPL_INLINE void* gen_align_forward( void* ptr, gen_sw alignment )
{
	gen_uptr p;

	GEN_ASSERT( gen_is_power_of_two( alignment ) );

	p = zpl_cast( gen_uptr ) ptr;
	return zpl_cast( void* )( ( p + ( alignment - 1 ) ) & ~( alignment - 1 ) );
}

GEN_IMPL_INLINE gen_s64 gen_align_forward_i64( gen_s64 value, gen_sw alignment )
{
	return value + ( alignment - value % alignment ) % alignment;
}

GEN_IMPL_INLINE gen_u64 gen_align_forward_u64( gen_u64 value, gen_uw alignment )
{
	return value + ( alignment - value % alignment ) % alignment;
}

GEN_IMPL_INLINE void* gen_pointer_add( void* ptr, gen_sw bytes )
{
	return zpl_cast( void* )( zpl_cast( gen_u8* ) ptr + bytes );
}

GEN_IMPL_INLINE void* gen_pointer_sub( void* ptr, gen_sw bytes )
{
	return zpl_cast( void* )( zpl_cast( gen_u8* ) ptr - bytes );
}

GEN_IMPL_INLINE void const* gen_pointer_add_const( void const* ptr, gen_sw bytes )
{
	return zpl_cast( void const* )( zpl_cast( gen_u8 const* ) ptr + bytes );
}

GEN_IMPL_INLINE void const* gen_pointer_sub_const( void const* ptr, gen_sw bytes )
{
	return zpl_cast( void const* )( zpl_cast( gen_u8 const* ) ptr - bytes );
}

GEN_IMPL_INLINE gen_sw gen_pointer_diff( void const* begin, void const* end )
{
	return zpl_cast( gen_sw )( zpl_cast( gen_u8 const* ) end - zpl_cast( gen_u8 const* ) begin );
}

GEN_IMPL_INLINE void gen_zero_size( void* ptr, gen_sw size )
{
	gen_mem_set( ptr, 0, size );
}

#if defined( _MSC_VER ) && ! defined( __clang__ )
#	pragma intrinsic( __movsb )
#endif

GEN_IMPL_INLINE void* gen_mem_move( void* dest, void const* source, gen_sw n )
{
	if ( dest == NULL )
	{
		return NULL;
	}

	gen_u8*       d = zpl_cast( gen_u8* ) dest;
	gen_u8 const* s = zpl_cast( gen_u8 const* ) source;

	if ( d == s )
		return d;
	if ( s + n <= d || d + n <= s )    // NOTE: Non-overlapping
		return gen_mem_copy( d, s, n );

	if ( d < s )
	{
		if ( zpl_cast( gen_uptr ) s % size_of( gen_sw ) == zpl_cast( gen_uptr ) d % size_of( gen_sw ) )
		{
			while ( zpl_cast( gen_uptr ) d % size_of( gen_sw ) )
			{
				if ( ! n-- )
					return dest;
				*d++ = *s++;
			}
			while ( n >= size_of( gen_sw ) )
			{
				*zpl_cast( gen_sw* ) d  = *zpl_cast( gen_sw* ) s;
				n                      -= size_of( gen_sw );
				d                      += size_of( gen_sw );
				s                      += size_of( gen_sw );
			}
		}
		for ( ; n; n-- )
			*d++ = *s++;
	}
	else
	{
		if ( ( zpl_cast( gen_uptr ) s % size_of( gen_sw ) ) == ( zpl_cast( gen_uptr ) d % size_of( gen_sw ) ) )
		{
			while ( zpl_cast( gen_uptr )( d + n ) % size_of( gen_sw ) )
			{
				if ( ! n-- )
					return dest;
				d[ n ] = s[ n ];
			}
			while ( n >= size_of( gen_sw ) )
			{
				n                             -= size_of( gen_sw );
				*zpl_cast( gen_sw* )( d + n )  = *zpl_cast( gen_sw* )( s + n );
			}
		}
		while ( n )
			n--, d[ n ] = s[ n ];
	}

	return dest;
}

GEN_IMPL_INLINE void* gen_mem_set( void* dest, gen_u8 c, gen_sw n )
{
	if ( dest == NULL )
	{
		return NULL;
	}

	gen_u8* s = zpl_cast( gen_u8* ) dest;
	gen_sw  k;
	gen_u32 c32 = ( ( gen_u32 )-1 ) / 255 * c;

	if ( n == 0 )
		return dest;
	s[ 0 ] = s[ n - 1 ] = c;
	if ( n < 3 )
		return dest;
	s[ 1 ] = s[ n - 2 ] = c;
	s[ 2 ] = s[ n - 3 ] = c;
	if ( n < 7 )
		return dest;
	s[ 3 ] = s[ n - 4 ] = c;
	if ( n < 9 )
		return dest;

	k  = -zpl_cast( gen_sptr ) s & 3;
	s += k;
	n -= k;
	n &= -4;

	*zpl_cast( gen_u32* )( s + 0 )     = c32;
	*zpl_cast( gen_u32* )( s + n - 4 ) = c32;
	if ( n < 9 )
		return dest;
	*zpl_cast( gen_u32* )( s + 4 )      = c32;
	*zpl_cast( gen_u32* )( s + 8 )      = c32;
	*zpl_cast( gen_u32* )( s + n - 12 ) = c32;
	*zpl_cast( gen_u32* )( s + n - 8 )  = c32;
	if ( n < 25 )
		return dest;
	*zpl_cast( gen_u32* )( s + 12 )     = c32;
	*zpl_cast( gen_u32* )( s + 16 )     = c32;
	*zpl_cast( gen_u32* )( s + 20 )     = c32;
	*zpl_cast( gen_u32* )( s + 24 )     = c32;
	*zpl_cast( gen_u32* )( s + n - 28 ) = c32;
	*zpl_cast( gen_u32* )( s + n - 24 ) = c32;
	*zpl_cast( gen_u32* )( s + n - 20 ) = c32;
	*zpl_cast( gen_u32* )( s + n - 16 ) = c32;

	k  = 24 + ( zpl_cast( gen_uptr ) s & 4 );
	s += k;
	n -= k;

	{
		gen_u64 c64 = ( zpl_cast( gen_u64 ) c32 << 32 ) | c32;
		while ( n > 31 )
		{
			*zpl_cast( gen_u64* )( s + 0 )  = c64;
			*zpl_cast( gen_u64* )( s + 8 )  = c64;
			*zpl_cast( gen_u64* )( s + 16 ) = c64;
			*zpl_cast( gen_u64* )( s + 24 ) = c64;

			n -= 32;
			s += 32;
		}
	}

	return dest;
}

GEN_IMPL_INLINE gen_s32 gen_memcompare( void const* s1, void const* s2, gen_sw size )
{
	gen_u8 const* s1p8 = zpl_cast( gen_u8 const* ) s1;
	gen_u8 const* s2p8 = zpl_cast( gen_u8 const* ) s2;

	if ( s1 == NULL || s2 == NULL )
	{
		return 0;
	}

	while ( size-- )
	{
		gen_sw d;
		if ( ( d = ( *s1p8++ - *s2p8++ ) ) != 0 )
			return zpl_cast( gen_s32 ) d;
	}
	return 0;
}

GEN_IMPL_INLINE gen_b32 gen_is_power_of_two( gen_sw x )
{
	if ( x <= 0 )
		return false;
	return ! ( x & ( x - 1 ) );
}
#pragma endregion Memory
