#pragma region String Ops

GEN_DEF_INLINE const char* gen_char_first_occurence( const char* str, char c );
GEN_DEF_INLINE const char* gen_char_last_occurence( const char* str, char c );

GEN_DEF_INLINE gen_b32 gen_char_is_alpha( char c );
GEN_DEF_INLINE gen_b32 gen_char_is_alphanumeric( char c );
GEN_DEF_INLINE gen_b32 gen_char_is_digit( char c );
GEN_DEF_INLINE gen_b32 gen_char_is_hex_digit( char c );
GEN_DEF_INLINE gen_b32 gen_char_is_space( char c );
GEN_DEF_INLINE char    gen_char_to_lower( char c );
GEN_DEF_INLINE char    gen_char_to_upper( char c );

GEN_DEF_INLINE gen_s32 gen_digit_to_int( char c );
GEN_DEF_INLINE gen_s32 gen_hex_digit_to_int( char c );

GEN_DEF_INLINE gen_s32     gen_str_cmp( const char* s1, const char* s2 );
GEN_DEF_INLINE gen_s32     gen_str_cmp_len( const char* s1, const char* s2, gen_sw len );
GEN_DEF_INLINE char*       gen_str_copy( char* dest, const char* source );
GEN_DEF_INLINE char*       gen_str_copy_cap( char* dest, const char* source, gen_sw len );
GEN_DEF_INLINE gen_sw      gen_str_copy_nulpad( char* dest, const char* source, gen_sw len );
GEN_DEF_INLINE gen_sw      gen_str_len( const char* str );
GEN_DEF_INLINE gen_sw      gen_str_len_cap( const char* str, gen_sw max_len );
GEN_DEF_INLINE char*       gen_str_reverse( char* str );    // NOTE: ASCII only
GEN_DEF_INLINE char const* gen_str_skip( char const* str, char c );
GEN_DEF_INLINE char const* gen_str_skip_any( char const* str, char const* char_list );
GEN_DEF_INLINE char const* gen_str_trim( char const* str, gen_b32 catch_newline );

GEN_DEF_INLINE void gen_str_to_lower( char* str );
GEN_DEF_INLINE void gen_str_to_upper( char* str );

gen_s64 gen_str_to_i64( const char* str, char** end_ptr, gen_s32 base );
gen_f64 gen_str_to_f64( const char* str, char** end_ptr );
void    gen_i64_to_str( gen_s64 value, char* string, gen_s32 base );
void    gen_u64_to_str( gen_u64 value, char* string, gen_s32 base );

/* inlines */

GEN_IMPL_INLINE const char* gen_char_first_occurence( const char* s, char c )
{
	char ch = c;
	for ( ; *s != ch; s++ )
	{
		if ( *s == '\0' )
			return NULL;
	}
	return s;
}

GEN_IMPL_INLINE const char* gen_char_last_occurence( const char* s, char c )
{
	char* result = ( char* )NULL;
	do
	{
		if ( *s == c )
			result = ( char* )s;
	} while ( *s++ );

	return result;
}

GEN_IMPL_INLINE gen_b32 gen_char_is_alpha( char c )
{
	if ( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) )
		return true;
	return false;
}

GEN_IMPL_INLINE gen_b32 gen_char_is_alphanumeric( char c )
{
	return gen_char_is_alpha( c ) || gen_char_is_digit( c );
}

GEN_IMPL_INLINE gen_b32 gen_char_is_digit( char c )
{
	if ( c >= '0' && c <= '9' )
		return true;
	return false;
}

GEN_IMPL_INLINE gen_b32 gen_char_is_hex_digit( char c )
{
	if ( gen_char_is_digit( c ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ) )
		return true;
	return false;
}

GEN_IMPL_INLINE gen_b32 gen_char_is_space( char c )
{
	if ( c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v' )
		return true;
	return false;
}

GEN_IMPL_INLINE gen_sw gen_str_len( const char* str )
{
	if ( str == NULL )
	{
		return 0;
	}
	const char* p = str;
	while ( *str )
		str++;
	return str - p;
}

GEN_IMPL_INLINE gen_sw gen_str_len( const char* str, gen_sw max_len )
{
	const char* end = zpl_cast( const char* ) gen_mem_find( str, 0, max_len );
	if ( end )
		return end - str;
	return max_len;
}

GEN_IMPL_INLINE char gen_char_to_lower( char c )
{
	if ( c >= 'A' && c <= 'Z' )
		return 'a' + ( c - 'A' );
	return c;
}

GEN_IMPL_INLINE char gen_char_to_upper( char c )
{
	if ( c >= 'a' && c <= 'z' )
		return 'A' + ( c - 'a' );
	return c;
}

GEN_IMPL_INLINE gen_s32 gen_digit_to_int( char c )
{
	return gen_char_is_digit( c ) ? c - '0' : c - 'W';
}

GEN_IMPL_INLINE gen_s32 gen_hex_digit_to_int( char c )
{
	if ( gen_char_is_digit( c ) )
		return gen_digit_to_int( c );
	else if ( is_between( c, 'a', 'f' ) )
		return c - 'a' + 10;
	else if ( is_between( c, 'A', 'F' ) )
		return c - 'A' + 10;
	return -1;
}

GEN_IMPL_INLINE gen_s32 gen_str_cmp( const char* s1, const char* s2 )
{
	while ( *s1 && ( *s1 == *s2 ) )
	{
		s1++, s2++;
	}
	return *( gen_u8* )s1 - *( gen_u8* )s2;
}

GEN_IMPL_INLINE gen_s32 gen_str_cmp_len( const char* s1, const char* s2, gen_sw len )
{
	for ( ; len > 0; s1++, s2++, len-- )
	{
		if ( *s1 != *s2 )
			return ( ( s1 < s2 ) ? -1 : +1 );
		else if ( *s1 == '\0' )
			return 0;
	}
	return 0;
}

GEN_IMPL_INLINE char* gen_str_copy( char* dest, const char* source )
{
	GEN_ASSERT_NOT_NULL( dest );
	if ( source )
	{
		char* str = dest;
		while ( *source )
			*str++ = *source++;
	}
	return dest;
}

GEN_IMPL_INLINE char* gen_str_copy_cap( char* dest, const char* source, gen_sw len )
{
	GEN_ASSERT_NOT_NULL( dest );
	if ( source )
	{
		char* str = dest;
		while ( len > 0 && *source )
		{
			*str++ = *source++;
			len--;
		}
		while ( len > 0 )
		{
			*str++ = '\0';
			len--;
		}
	}
	return dest;
}

GEN_IMPL_INLINE gen_sw gen_str_copy_nulpad( char* dest, const char* source, gen_sw len )
{
	gen_sw result = 0;
	GEN_ASSERT_NOT_NULL( dest );
	if ( source )
	{
		const char* source_start = source;
		char*       str          = dest;
		while ( len > 0 && *source )
		{
			*str++ = *source++;
			len--;
		}
		while ( len > 0 )
		{
			*str++ = '\0';
			len--;
		}

		result = source - source_start;
	}
	return result;
}

GEN_IMPL_INLINE char* gen_str_reverse( char* str )
{
	gen_sw len  = gen_str_len( str );
	char*  a    = str + 0;
	char*  b    = str + len - 1;
	len        /= 2;
	while ( len-- )
	{
		swap( char, *a, *b );
		a++, b--;
	}
	return str;
}

GEN_IMPL_INLINE char const* gen_str_skip( char const* str, char c )
{
	while ( *str && *str != c )
	{
		++str;
	}
	return str;
}

GEN_IMPL_INLINE char const* gen_str_skip_any( char const* str, char const* char_list )
{
	char const* closest_ptr     = zpl_cast( char const* ) gen_pointer_add( ( void* )str, gen_str_len( str ) );
	gen_sw      char_list_count = gen_str_len( char_list );
	for ( gen_sw i = 0; i < char_list_count; i++ )
	{
		char const* p = gen_str_skip( str, char_list[ i ] );
		closest_ptr   = min( closest_ptr, p );
	}
	return closest_ptr;
}

GEN_IMPL_INLINE char const* gen_str_trim( char const* str, gen_b32 catch_newline )
{
	while ( *str && gen_char_is_space( *str ) && ( ! catch_newline || ( catch_newline && *str != '\n' ) ) )
	{
		++str;
	}
	return str;
}

GEN_IMPL_INLINE void gen_str_to_lower( char* str )
{
	if ( ! str )
		return;
	while ( *str )
	{
		*str = gen_char_to_lower( *str );
		str++;
	}
}

GEN_IMPL_INLINE void gen_str_to_upper( char* str )
{
	if ( ! str )
		return;
	while ( *str )
	{
		*str = gen_char_to_upper( *str );
		str++;
	}
}

#pragma endregion String Ops

