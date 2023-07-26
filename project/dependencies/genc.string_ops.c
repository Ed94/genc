#pragma region String Ops

gen_internal gen_sw gen__scan_zpl_i64( const char* text, gen_s32 base, gen_s64* value )
{
	const char* text_begin = text;
	gen_s64     result     = 0;
	gen_b32     negative   = false;

	if ( *text == '-' )
	{
		negative = true;
		text++;
	}

	if ( base == 16 && gen_str_compare( text, "0x", 2 ) == 0 )
		text += 2;

	for ( ;; )
	{
		gen_s64 v;
		if ( gen_char_is_digit( *text ) )
			v = *text - '0';
		else if ( base == 16 && gen_char_is_hex_digit( *text ) )
			v = gen_hex_digit_to_int( *text );
		else
			break;

		result *= base;
		result += v;
		text++;
	}

	if ( value )
	{
		if ( negative )
			result = -result;
		*value = result;
	}

	return ( text - text_begin );
}

// TODO : Are these good enough for characters?
global const char _num_to_char_table[] =
	"0123456789"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"@$";

gen_s64 gen_str_to_i64( const char* str, char** end_ptr, gen_s32 base )
{
	gen_sw  len;
	gen_s64 value;

	if ( ! base )
	{
		if ( ( gen_str_len( str ) > 2 ) && ( gen_str_compare( str, "0x", 2 ) == 0 ) )
			base = 16;
		else
			base = 10;
	}

	len = gen__scan_zpl_i64( str, base, &value );
	if ( end_ptr )
		*end_ptr = ( char* )str + len;
	return value;
}

gen_f64 gen_str_to_f64( const char* str, char** end_ptr )
{
	gen_f64 result, value, sign, scale;
	gen_s32 frac;

	while ( gen_char_is_space( *str ) )
	{
		str++;
	}

	sign = 1.0;
	if ( *str == '-' )
	{
		sign = -1.0;
		str++;
	}
	else if ( *str == '+' )
	{
		str++;
	}

	for ( value = 0.0; gen_char_is_digit( *str ); str++ )
	{
		value = value * 10.0 + ( *str - '0' );
	}

	if ( *str == '.' )
	{
		gen_f64 pow10 = 10.0;
		str++;
		while ( gen_char_is_digit( *str ) )
		{
			value += ( *str - '0' ) / pow10;
			pow10 *= 10.0;
			str++;
		}
	}

	frac  = 0;
	scale = 1.0;
	if ( ( *str == 'e' ) || ( *str == 'E' ) )
	{
		gen_u32 exp;

		str++;
		if ( *str == '-' )
		{
			frac = 1;
			str++;
		}
		else if ( *str == '+' )
		{
			str++;
		}

		for ( exp = 0; gen_char_is_digit( *str ); str++ )
		{
			exp = exp * 10 + ( *str - '0' );
		}
		if ( exp > 308 )
			exp = 308;

		while ( exp >= 50 )
		{
			scale *= 1e50;
			exp   -= 50;
		}
		while ( exp >= 8 )
		{
			scale *= 1e8;
			exp   -= 8;
		}
		while ( exp > 0 )
		{
			scale *= 10.0;
			exp   -= 1;
		}
	}

	result = sign * ( frac ? ( value / scale ) : ( value * scale ) );

	if ( end_ptr )
		*end_ptr = zpl_cast( char* ) str;

	return result;
}

void gen_i64_to_str( gen_s64 value, char* string, gen_s32 base )
{
	char*   buf      = string;
	gen_b32 negative = false;
	gen_u64 v;

	if ( value < 0 )
	{
		negative = true;
		value    = -value;
	}

	v = zpl_cast( gen_u64 ) value;
	if ( v != 0 )
	{
		while ( v > 0 )
		{
			*buf++  = gen__num_to_char_table[ v % base ];
			v      /= base;
		}
	}
	else
	{
		*buf++ = '0';
	}
	if ( negative )
		*buf++ = '-';
	*buf = '\0';
	gen_str_reverse( string );
}

void gen_u64_to_str( gen_u64 value, char* string, gen_s32 base )
{
	char* buf = string;

	if ( value )
	{
		while ( value > 0 )
		{
			*buf++  = gen__num_to_char_table[ value % base ];
			value  /= base;
		}
	}
	else
	{
		*buf++ = '0';
	}
	*buf = '\0';

	gen_str_reverse( string );
}

#pragma endregion String Ops