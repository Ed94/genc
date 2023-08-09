#pragma region Printing

gen_sw gen_str_fmt_out_va( char const* fmt, va_list va )
{
	return gen_str_fmt_file_va( gen_file_get_standard( gen_EFileStandard_OUTPUT ), fmt, va );
}

gen_sw gen_str_fmt_out_err_va( char const* fmt, va_list va )
{
	return gen_str_fmt_file_va( gen_file_get_standard( gen_EFileStandard_ERROR ), fmt, va );
}

gen_sw gen_str_fmt_file_va( struct gen_FileInfo* f, char const* fmt, va_list va )
{
	local_persist zpl_thread_local char buf[ GEN_PRINTF_MAXLEN ];
	gen_sw                                  len = gen_str_fmt_va( buf, gen_size_of( buf ), fmt, va );
	gen_b32                                 res = gen_file_write( f, buf, len - 1 );    // NOTE: prevent extra whitespace
	return res ? len : -1;
}

char* gen_str_fmt_buf_va( char const* fmt, va_list va )
{
	local_persist zpl_thread_local char buffer[ GEN_PRINTF_MAXLEN ];
	gen_str_fmt_va( buffer, gen_size_of( buffer ), fmt, va );
	return buffer;
}

gen_sw gen_str_fmt_alloc_va( gen_AllocatorInfo allocator, char** buffer, char const* fmt, va_list va )
{
	local_persist zpl_thread_local char tmp[ GEN_PRINTF_MAXLEN ];
	GEN_ASSERT_NOT_NULL( buffer );
	gen_sw res;
	res     = gen_str_fmt_va( tmp, gen_size_of( tmp ), fmt, va );
	*buffer = gen_alloc_str( allocator, tmp );
	return res;
}

gen_sw gen_str_fmt_out( char const* fmt, ... )
{
	gen_sw  res;
	va_list va;
	va_start( va, fmt );
	res = gen_str_fmt_out_va( fmt, va );
	va_end( va );
	return res;
}

gen_sw gen_str_fmt_out_err( char const* fmt, ... )
{
	gen_sw  res;
	va_list va;
	va_start( va, fmt );
	res = gen_str_fmt_out_err_va( fmt, va );
	va_end( va );
	return res;
}

gen_sw gen_str_fmt_file( struct gen_FileInfo* f, char const* fmt, ... )
{
	gen_sw  res;
	va_list va;
	va_start( va, fmt );
	res = gen_str_fmt_file_va( f, fmt, va );
	va_end( va );
	return res;
}

char* gen_str_fmt_buf( char const* fmt, ... )
{
	va_list va;
	char*   str;
	va_start( va, fmt );
	str = gen_str_fmt_buf_va( fmt, va );
	va_end( va );
	return str;
}

gen_sw gen_str_fmt_alloc( gen_AllocatorInfo allocator, char** buffer, char const* fmt, ... )
{
	gen_sw  res;
	va_list va;
	va_start( va, fmt );
	res = gen_str_fmt_alloc_va( allocator, buffer, fmt, va );
	va_end( va );
	return res;
}

gen_sw gen_str_fmt( char* str, gen_sw n, char const* fmt, ... )
{
	gen_sw  res;
	va_list va;
	va_start( va, fmt );
	res = gen_str_fmt_va( str, n, fmt, va );
	va_end( va );
	return res;
}

enum
{
	GEN_FMT_MINUS = GEN_BIT( 0 ),
	GEN_FMT_PLUS  = GEN_BIT( 1 ),
	GEN_FMT_ALT   = GEN_BIT( 2 ),
	GEN_FMT_SPACE = GEN_BIT( 3 ),
	GEN_FMT_ZERO  = GEN_BIT( 4 ),

	GEN_FMT_CHAR   = GEN_BIT( 5 ),
	GEN_FMT_SHORT  = GEN_BIT( 6 ),
	GEN_FMT_INT    = GEN_BIT( 7 ),
	GEN_FMT_LONG   = GEN_BIT( 8 ),
	GEN_FMT_LLONG  = GEN_BIT( 9 ),
	GEN_FMT_SIZE   = GEN_BIT( 10 ),
	GEN_FMT_INTPTR = GEN_BIT( 11 ),

	GEN_FMT_UNSIGNED = GEN_BIT( 12 ),
	GEN_FMT_LOWER    = GEN_BIT( 13 ),
	GEN_FMT_UPPER    = GEN_BIT( 14 ),
	GEN_FMT_WIDTH    = GEN_BIT( 15 ),

	GEN_FMT_DONE = GEN_BIT( 30 ),

	GEN_FMT_INTS = GEN_FMT_CHAR | GEN_FMT_SHORT | GEN_FMT_INT | GEN_FMT_LONG | GEN_FMT_LLONG | GEN_FMT_SIZE | GEN_FMT_INTPTR
};

typedef struct
{
	gen_s32 base;
	gen_s32 flags;
	gen_s32 width;
	gen_s32 precision;
} gen__format_info;

internal gen_sw gen__print_string( char* text, gen_sw max_len, gen__format_info* info, char const* str )
{
	gen_sw res = 0, len = 0;
	gen_sw remaining = max_len;
	char*  begin     = text;

	if ( str == NULL && max_len >= 6 )
	{
		res += gen_str_copy_nulpad( text, "(null)", 6 );
		return res;
	}

	if ( info && info->precision >= 0 )
	{
		// Made the design decision for this library that precision is the length of the string.
		len = info->precision;
	}
	else
		len = gen_str_len( str );

	if ( info && ( info->width == 0 && info->flags & GEN_FMT_WIDTH ) )
	{
		return res;
	}

	if ( info && ( info->width == 0 || info->flags & GEN_FMT_MINUS ) )
	{
		if ( info->precision > 0 )
			len = info->precision < len ? info->precision : len;
		if ( res + len > max_len )
			return res;
		res  += gen_str_copy_nulpad( text, str, len );
		text += res;

		if ( info->width > res )
		{
			gen_sw padding = info->width - len;

			char pad = ( info->flags & GEN_FMT_ZERO ) ? '0' : ' ';
			while ( padding-- > 0 && remaining-- > 0 )
				*text++ = pad, res++;
		}
	}
	else
	{
		if ( info && ( info->width > res ) )
		{
			gen_sw padding = info->width - len;
			char   pad     = ( info->flags & GEN_FMT_ZERO ) ? '0' : ' ';
			while ( padding-- > 0 && remaining-- > 0 )
				*text++ = pad, res++;
		}

		if ( res + len > max_len )
			return res;
		res += gen_str_copy_nulpad( text, str, len );
	}

	if ( info )
	{
		if ( info->flags & GEN_FMT_UPPER )
			gen_str_to_upper( begin );
		else if ( info->flags & GEN_FMT_LOWER )
			gen_str_to_lower( begin );
	}

	return res;
}

internal gen_sw gen__print_char( char* text, gen_sw max_len, gen__format_info* info, char arg )
{
	char str[ 2 ] = "";
	str[ 0 ]      = arg;
	return gen__print_string( text, max_len, info, str );
}

internal gen_sw gen__print_repeated_char( char* text, gen_sw max_len, gen__format_info* info, char arg )
{
	gen_sw  res = 0;
	gen_s32 rem = ( info ) ? ( info->width > 0 ) ? info->width : 1 : 1;
	res         = rem;
	while ( rem-- > 0 )
		*text++ = arg;

	return res;
}

internal gen_sw gen__print_i64( char* text, gen_sw max_len, gen__format_info* info, gen_s64 value )
{
	char num[ 130 ];
	gen_i64_to_str( value, num, info ? info->base : 10 );
	return gen__print_string( text, max_len, info, num );
}

internal gen_sw gen__print_u64( char* text, gen_sw max_len, gen__format_info* info, gen_u64 value )
{
	char num[ 130 ];
	gen_u64_to_str( value, num, info ? info->base : 10 );
	return gen__print_string( text, max_len, info, num );
}

internal gen_sw gen__print_f64( char* text, gen_sw max_len, gen__format_info* info, gen_b32 is_hexadecimal, gen_f64 arg )
{
	// TODO: Handle exponent notation
	gen_sw width, len, remaining = max_len;
	char*  text_begin = text;

	if ( arg )
	{
		gen_u64 value;
		if ( arg < 0 )
		{
			if ( remaining > 1 )
				*text = '-', remaining--;
			text++;
			arg = -arg;
		}
		else if ( info->flags & GEN_FMT_MINUS )
		{
			if ( remaining > 1 )
				*text = '+', remaining--;
			text++;
		}

		value  = zpl_cast( gen_u64 ) arg;
		len    = gen__print_u64( text, remaining, NULL, value );
		text  += len;

		if ( len >= remaining )
			remaining = gen_min( remaining, 1 );
		else
			remaining -= len;
		arg -= value;

		if ( info->precision < 0 )
			info->precision = 6;

		if ( ( info->flags & GEN_FMT_ALT ) || info->precision > 0 )
		{
			gen_s64 mult = 10;
			if ( remaining > 1 )
				*text = '.', remaining--;
			text++;
			while ( info->precision-- > 0 )
			{
				value  = zpl_cast( gen_u64 )( arg * mult );
				len    = gen__print_u64( text, remaining, NULL, value );
				text  += len;
				if ( len >= remaining )
					remaining = gen_min( remaining, 1 );
				else
					remaining -= len;
				arg  -= zpl_cast( gen_f64 ) value / mult;
				mult *= 10;
			}
		}
	}
	else
	{
		if ( remaining > 1 )
			*text = '0', remaining--;
		text++;
		if ( info->flags & GEN_FMT_ALT )
		{
			if ( remaining > 1 )
				*text = '.', remaining--;
			text++;
		}
	}

	width = info->width - ( text - text_begin );
	if ( width > 0 )
	{
		char  fill = ( info->flags & GEN_FMT_ZERO ) ? '0' : ' ';
		char* end  = text + remaining - 1;
		len        = ( text - text_begin );

		for ( len = ( text - text_begin ); len--; )
		{
			if ( ( text_begin + len + width ) < end )
				*( text_begin + len + width ) = *( text_begin + len );
		}

		len   = width;
		text += len;
		if ( len >= remaining )
			remaining = gen_min( remaining, 1 );
		else
			remaining -= len;

		while ( len-- )
		{
			if ( text_begin + len < end )
				text_begin[ len ] = fill;
		}
	}

	return ( text - text_begin );
}

GEN_NEVER_INLINE gen_sw gen_str_fmt_va( char* text, gen_sw max_len, char const* fmt, va_list va )
{
	char const* text_begin = text;
	gen_sw      remaining  = max_len, res;

	while ( *fmt )
	{
		gen__format_info info = { 0 };
		gen_sw           len  = 0;
		info.precision        = -1;

		while ( *fmt && *fmt != '%' && remaining )
			*text++ = *fmt++;

		if ( *fmt == '%' )
		{
			do
			{
				switch ( *++fmt )
				{
					case '-' :
						{
							info.flags |= GEN_FMT_MINUS;
							break;
						}
					case '+' :
						{
							info.flags |= GEN_FMT_PLUS;
							break;
						}
					case '#' :
						{
							info.flags |= GEN_FMT_ALT;
							break;
						}
					case ' ' :
						{
							info.flags |= GEN_FMT_SPACE;
							break;
						}
					case '0' :
						{
							info.flags |= ( GEN_FMT_ZERO | GEN_FMT_WIDTH );
							break;
						}
					default :
						{
							info.flags |= GEN_FMT_DONE;
							break;
						}
				}
			} while ( ! ( info.flags & GEN_FMT_DONE ) );
		}

		// NOTE: Optional Width
		if ( *fmt == '*' )
		{
			int width = va_arg( va, int );
			if ( width < 0 )
			{
				info.flags |= GEN_FMT_MINUS;
				info.width  = -width;
			}
			else
			{
				info.width = width;
			}
			info.flags |= GEN_FMT_WIDTH;
			fmt++;
		}
		else
		{
			info.width = zpl_cast( gen_s32 ) gen_str_to_i64( fmt, zpl_cast( char** ) & fmt, 10 );
			if ( info.width != 0 )
			{
				info.flags |= GEN_FMT_WIDTH;
			}
		}

		// NOTE: Optional Precision
		if ( *fmt == '.' )
		{
			fmt++;
			if ( *fmt == '*' )
			{
				info.precision = va_arg( va, int );
				fmt++;
			}
			else
			{
				info.precision = zpl_cast( gen_s32 ) gen_str_to_i64( fmt, zpl_cast( char** ) & fmt, 10 );
			}
			info.flags &= ~GEN_FMT_ZERO;
		}

		switch ( *fmt++ )
		{
			case 'h' :
				if ( *fmt == 'h' )
				{    // hh => char
					info.flags |= GEN_FMT_CHAR;
					fmt++;
				}
				else
				{    // h => short
					info.flags |= GEN_FMT_SHORT;
				}
				break;

			case 'l' :
				if ( *fmt == 'l' )
				{    // ll => long long
					info.flags |= GEN_FMT_LLONG;
					fmt++;
				}
				else
				{    // l => long
					info.flags |= GEN_FMT_LONG;
				}
				break;

				break;

			case 'z' :    // NOTE: zpl_usize
				info.flags |= GEN_FMT_UNSIGNED;
				// fallthrough
			case 't' :    // NOTE: zpl_isize
				info.flags |= GEN_FMT_SIZE;
				break;

			default :
				fmt--;
				break;
		}

		switch ( *fmt )
		{
			case 'u' :
				info.flags |= GEN_FMT_UNSIGNED;
				// fallthrough
			case 'd' :
			case 'i' :
				info.base = 10;
				break;

			case 'o' :
				info.base = 8;
				break;

			case 'x' :
				info.base   = 16;
				info.flags |= ( GEN_FMT_UNSIGNED | GEN_FMT_LOWER );
				break;

			case 'X' :
				info.base   = 16;
				info.flags |= ( GEN_FMT_UNSIGNED | GEN_FMT_UPPER );
				break;

			case 'f' :
			case 'F' :
			case 'g' :
			case 'G' :
				len = gen__print_f64( text, remaining, &info, 0, va_arg( va, gen_f64 ) );
				break;

			case 'a' :
			case 'A' :
				len = gen__print_f64( text, remaining, &info, 1, va_arg( va, gen_f64 ) );
				break;

			case 'c' :
				len = gen__print_char( text, remaining, &info, zpl_cast( char ) va_arg( va, int ) );
				break;

			case 's' :
				len = gen__print_string( text, remaining, &info, va_arg( va, char* ) );
				break;

			case 'r' :
				len = gen__print_repeated_char( text, remaining, &info, va_arg( va, int ) );
				break;

			case 'p' :
				info.base   = 16;
				info.flags |= ( GEN_FMT_LOWER | GEN_FMT_UNSIGNED | GEN_FMT_ALT | GEN_FMT_INTPTR );
				break;

			case '%' :
				len = gen__print_char( text, remaining, &info, '%' );
				break;

			default :
				fmt--;
				break;
		}

		fmt++;

		if ( info.base != 0 )
		{
			if ( info.flags & GEN_FMT_UNSIGNED )
			{
				gen_u64 value = 0;
				switch ( info.flags & GEN_FMT_INTS )
				{
					case GEN_FMT_CHAR :
						value = zpl_cast( gen_u64 ) zpl_cast( gen_u8 ) va_arg( va, int );
						break;
					case GEN_FMT_SHORT :
						value = zpl_cast( gen_u64 ) zpl_cast( gen_u16 ) va_arg( va, int );
						break;
					case GEN_FMT_LONG :
						value = zpl_cast( gen_u64 ) va_arg( va, unsigned long );
						break;
					case GEN_FMT_LLONG :
						value = zpl_cast( gen_u64 ) va_arg( va, unsigned long long );
						break;
					case GEN_FMT_SIZE :
						value = zpl_cast( gen_u64 ) va_arg( va, gen_uw );
						break;
					case GEN_FMT_INTPTR :
						value = zpl_cast( gen_u64 ) va_arg( va, gen_uptr );
						break;
					default :
						value = zpl_cast( gen_u64 ) va_arg( va, unsigned int );
						break;
				}

				len = gen__print_u64( text, remaining, &info, value );
			}
			else
			{
				gen_s64 value = 0;
				switch ( info.flags & GEN_FMT_INTS )
				{
					case GEN_FMT_CHAR :
						value = zpl_cast( gen_s64 ) zpl_cast( gen_s8 ) va_arg( va, int );
						break;
					case GEN_FMT_SHORT :
						value = zpl_cast( gen_s64 ) zpl_cast( gen_s16 ) va_arg( va, int );
						break;
					case GEN_FMT_LONG :
						value = zpl_cast( gen_s64 ) va_arg( va, long );
						break;
					case GEN_FMT_LLONG :
						value = zpl_cast( gen_s64 ) va_arg( va, long long );
						break;
					case GEN_FMT_SIZE :
						value = zpl_cast( gen_s64 ) va_arg( va, gen_uw );
						break;
					case GEN_FMT_INTPTR :
						value = zpl_cast( gen_s64 ) va_arg( va, gen_uptr );
						break;
					default :
						value = zpl_cast( gen_s64 ) va_arg( va, int );
						break;
				}

				len = gen__print_i64( text, remaining, &info, value );
			}
		}

		text += len;
		if ( len >= remaining )
			remaining = gen_min( remaining, 1 );
		else
			remaining -= len;
	}

	*text++ = '\0';
	res     = ( text - text_begin );
	return ( res >= max_len || res < 0 ) ? -1 : res;
}

#pragma endregion Printing

