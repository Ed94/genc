#pragma region ADT

#define gen__adt_fprintf( s_, fmt_, ... )                      \
	do                                                         \
	{                                                          \
		if ( gen_str_fmt_file( s_, fmt_, ##__VA_ARGS__ ) < 0 ) \
			return gen_EADT_ERROR_OUT_OF_MEMORY;               \
	} while ( 0 )

gen_u8 gen_adt_make_branch( gen_ADT_Node* node, gen_AllocatorInfo backing, char const* name, gen_b32 is_array )
{
	gen_u8 type = gen_EADT_TYPE_OBJECT;
	if ( is_array )
	{
		type = gen_EADT_TYPE_ARRAY;
	}
	gen_ADT_Node* parent = node->parent;
	gen_zero_item( node );
	node->type   = type;
	node->name   = name;
	node->parent = parent;
	if ( ! gen_array_init( node->nodes, backing ) )
		return gen_EADT_ERROR_OUT_OF_MEMORY;
	return 0;
}

gen_u8 gen_adt_destroy_branch( gen_ADT_Node* node )
{
	GEN_ASSERT_NOT_NULL( node );
	if ( ( node->type == gen_EADT_TYPE_OBJECT || node->type == gen_EADT_TYPE_ARRAY ) && node->nodes )
	{
		for ( gen_sw i = 0; i < gen_array_count( node->nodes ); ++i )
		{
			gen_adt_destroy_branch( node->nodes + i );
		}

		gen_array_free( node->nodes );
	}
	return 0;
}

gen_u8 gen_adt_make_leaf( gen_ADT_Node* node, char const* name, gen_u8 type )
{
	GEN_ASSERT( type != gen_EADT_TYPE_OBJECT && type != gen_EADT_TYPE_ARRAY );
	gen_ADT_Node* parent = node->parent;
	gen_zero_item( node );
	node->type   = type;
	node->name   = name;
	node->parent = parent;
	return 0;
}

gen_ADT_Node* gen_adt_find( gen_ADT_Node* node, char const* name, gen_b32 deep_search )
{
	if ( node->type != gen_EADT_TYPE_OBJECT )
	{
		return NULL;
	}

	for ( gen_sw i = 0; i < gen_array_count( node->nodes ); i++ )
	{
		if ( ! gen_str_compare( node->nodes[ i ].name, name ) )
		{
			return ( node->nodes + i );
		}
	}

	if ( deep_search )
	{
		for ( gen_sw i = 0; i < gen_array_count( node->nodes ); i++ )
		{
			gen_ADT_Node* res = gen_adt_find( node->nodes + i, name, deep_search );

			if ( res != NULL )
				return res;
		}
	}

	return NULL;
}

internal gen_ADT_Node* gen__adt_get_value( gen_ADT_Node* node, char const* value )
{
	switch ( node->type )
	{
		case gen_EADT_TYPE_MULTISTRING :
		case gen_EADT_TYPE_STRING :
			{
				if ( node->string && ! gen_str_compare( node->string, value ) )
				{
					return node;
				}
			}
			break;
		case gen_EADT_TYPE_INTEGER :
		case gen_EADT_TYPE_REAL :
			{
				char         back[ 4096 ] = { 0 };
				gen_FileInfo tmp;

				/* allocate a file descriptor for a memory-mapped number to string conversion, input source buffer is not cloned, however. */
				gen_file_stream_open( &tmp, gen_heap(), ( gen_u8* )back, gen_size_of( back ), gen_EFileStream_WRITABLE );
				gen_adt_print_number( &tmp, node );

				gen_sw  fsize = 0;
				gen_u8* buf   = gen_file_stream_buf( &tmp, &fsize );

				if ( ! gen_str_compare( ( char const* )buf, value ) )
				{
					gen_file_close( &tmp );
					return node;
				}

				gen_file_close( &tmp );
			}
			break;
		default :
			break; /* node doesn't support value based lookup */
	}

	return NULL;
}

internal gen_ADT_Node* gen__adt_get_field( gen_ADT_Node* node, char* name, char* value )
{
	for ( gen_sw i = 0; i < gen_array_count( node->nodes ); i++ )
	{
		if ( ! gen_str_compare( node->nodes[ i ].name, name ) )
		{
			gen_ADT_Node* child = &node->nodes[ i ];
			if ( gen__adt_get_value( child, value ) )
			{
				return node; /* this object does contain a field of a specified value! */
			}
		}
	}

	return NULL;
}

gen_ADT_Node* gen_adt_query( gen_ADT_Node* node, char const* uri )
{
	GEN_ASSERT_NOT_NULL( uri );

	if ( *uri == '/' )
	{
		uri++;
	}

	if ( *uri == 0 )
	{
		return node;
	}

	if ( ! node || ( node->type != gen_EADT_TYPE_OBJECT && node->type != gen_EADT_TYPE_ARRAY ) )
	{
		return NULL;
	}

#if defined gen_EADT_URI_DEBUG || 0
	gen_str_fmt_out( "uri: %s\n", uri );
#endif

	char *        p = ( char* )uri, *b = p, *e = p;
	gen_ADT_Node* found_node = NULL;

	b = p;
	p = e     = ( char* )gen_str_skip( p, '/' );
	char* buf = gen_str_fmt_buf( "%.*s", ( int )( e - b ), b );

	/* handle field value lookup */
	if ( *b == '[' )
	{
		char *l_p = buf + 1, *l_b = l_p, *l_e = l_p, *l_b2 = l_p, *l_e2 = l_p;
		l_e  = ( char* )gen_str_skip( l_p, '=' );
		l_e2 = ( char* )gen_str_skip( l_p, ']' );

		if ( ( ! *l_e && node->type != gen_EADT_TYPE_ARRAY ) || ! *l_e2 )
		{
			GEN_ASSERT_MSG( 0, "Invalid field value lookup" );
			return NULL;
		}

		*l_e2 = 0;

		/* [field=value] */
		if ( *l_e )
		{
			*l_e = 0;
			l_b2 = l_e + 1;

			/* run a value comparison against our own fields */
			if ( node->type == gen_EADT_TYPE_OBJECT )
			{
				found_node = gen__adt_get_field( node, l_b, l_b2 );
			}

			/* run a value comparison against any child that is an object node */
			else if ( node->type == gen_EADT_TYPE_ARRAY )
			{
				for ( gen_sw i = 0; i < gen_array_count( node->nodes ); i++ )
				{
					gen_ADT_Node* child = &node->nodes[ i ];
					if ( child->type != gen_EADT_TYPE_OBJECT )
					{
						continue;
					}

					found_node = gen__adt_get_field( child, l_b, l_b2 );

					if ( found_node )
						break;
				}
			}
		}
		/* [value] */
		else
		{
			for ( gen_sw i = 0; i < gen_array_count( node->nodes ); i++ )
			{
				gen_ADT_Node* child = &node->nodes[ i ];
				if ( gen__adt_get_value( child, l_b2 ) )
				{
					found_node = child;
					break; /* we found a matching value in array, ignore the rest of it */
				}
			}
		}

		/* go deeper if uri continues */
		if ( *e )
		{
			return gen_adt_query( found_node, e + 1 );
		}
	}
	/* handle field name lookup */
	else if ( node->type == gen_EADT_TYPE_OBJECT )
	{
		found_node = gen_adt_find( node, buf, false );

		/* go deeper if uri continues */
		if ( *e )
		{
			return gen_adt_query( found_node, e + 1 );
		}
	}
	/* handle array index lookup */
	else
	{
		gen_sw idx = ( gen_sw )gen_str_to_i64( buf, NULL, 10 );
		if ( idx >= 0 && idx < gen_array_count( node->nodes ) )
		{
			found_node = &node->nodes[ idx ];

			/* go deeper if uri continues */
			if ( *e )
			{
				return gen_adt_query( found_node, e + 1 );
			}
		}
	}

	return found_node;
}

gen_ADT_Node* gen_adt_alloc_at( gen_ADT_Node* parent, gen_sw index )
{
	if ( ! parent || ( parent->type != gen_EADT_TYPE_OBJECT && parent->type != gen_EADT_TYPE_ARRAY ) )
	{
		return NULL;
	}

	if ( ! parent->nodes )
		return NULL;

	if ( index < 0 || index > gen_array_count( parent->nodes ) )
		return NULL;

	gen_ADT_Node o = { 0 };
	o.parent       = parent;
	if ( ! gen_array_append_at( parent->nodes, o, index ) )
		return NULL;

	return parent->nodes + index;
}

gen_ADT_Node* gen_adt_alloc( gen_ADT_Node* parent )
{
	if ( ! parent || ( parent->type != gen_EADT_TYPE_OBJECT && parent->type != gen_EADT_TYPE_ARRAY ) )
	{
		return NULL;
	}

	if ( ! parent->nodes )
		return NULL;

	return gen_adt_alloc_at( parent, gen_array_count( parent->nodes ) );
}

gen_b8 gen_adt_set_obj( gen_ADT_Node* obj, char const* name, gen_AllocatorInfo backing )
{
	return gen_adt_make_branch( obj, backing, name, 0 );
}

gen_b8 gen_adt_set_arr( gen_ADT_Node* obj, char const* name, gen_AllocatorInfo backing )
{
	return gen_adt_make_branch( obj, backing, name, 1 );
}

gen_b8 gen_adt_set_str( gen_ADT_Node* obj, char const* name, char const* value )
{
	gen_adt_make_leaf( obj, name, gen_EADT_TYPE_STRING );
	obj->string = value;
	return true;
}

gen_b8 gen_adt_set_flt( gen_ADT_Node* obj, char const* name, gen_f64 value )
{
	gen_adt_make_leaf( obj, name, gen_EADT_TYPE_REAL );
	obj->real = value;
	return true;
}

gen_b8 gen_adt_set_int( gen_ADT_Node* obj, char const* name, gen_s64 value )
{
	gen_adt_make_leaf( obj, name, gen_EADT_TYPE_INTEGER );
	obj->integer = value;
	return true;
}

gen_ADT_Node* gen_adt_move_node_at( gen_ADT_Node* node, gen_ADT_Node* new_parent, gen_sw index )
{
	GEN_ASSERT_NOT_NULL( node );
	GEN_ASSERT_NOT_NULL( new_parent );
	gen_ADT_Node* old_parent = node->parent;
	gen_ADT_Node* new_node   = gen_adt_alloc_at( new_parent, index );
	*new_node                = *node;
	new_node->parent         = new_parent;
	if ( old_parent )
	{
		gen_adt_remove_node( node );
	}
	return new_node;
}

gen_ADT_Node* gen_adt_move_node( gen_ADT_Node* node, gen_ADT_Node* new_parent )
{
	GEN_ASSERT_NOT_NULL( node );
	GEN_ASSERT_NOT_NULL( new_parent );
	GEN_ASSERT( new_parent->type == gen_EADT_TYPE_ARRAY || new_parent->type == gen_EADT_TYPE_OBJECT );
	return gen_adt_move_node_at( node, new_parent, gen_array_count( new_parent->nodes ) );
}

void gen_adt_swap_nodes( gen_ADT_Node* node, gen_ADT_Node* other_node )
{
	GEN_ASSERT_NOT_NULL( node );
	GEN_ASSERT_NOT_NULL( other_node );
	gen_ADT_Node* parent                 = node->parent;
	gen_ADT_Node* other_parent           = other_node->parent;
	gen_sw        index                  = ( gen_pointer_diff( parent->nodes, node ) / gen_size_of( gen_ADT_Node ) );
	gen_sw        index2                 = ( gen_pointer_diff( other_parent->nodes, other_node ) / gen_size_of( gen_ADT_Node ) );
	gen_ADT_Node  temp                   = parent->nodes[ index ];
	temp.parent                          = other_parent;
	other_parent->nodes[ index2 ].parent = parent;
	parent->nodes[ index ]               = other_parent->nodes[ index2 ];
	other_parent->nodes[ index2 ]        = temp;
}

void gen_adt_remove_node( gen_ADT_Node* node )
{
	GEN_ASSERT_NOT_NULL( node );
	GEN_ASSERT_NOT_NULL( node->parent );
	gen_ADT_Node* parent = node->parent;
	gen_sw        index  = ( gen_pointer_diff( parent->nodes, node ) / gen_size_of( gen_ADT_Node ) );
	gen_array_remove_at( parent->nodes, index );
}

gen_ADT_Node* gen_adt_append_obj( gen_ADT_Node* parent, char const* name )
{
	gen_ADT_Node* o = gen_adt_alloc( parent );
	if ( ! o )
		return NULL;
	if ( gen_adt_set_obj( o, name, GEN_ARRAY_HEADER( parent->nodes )->allocator ) )
	{
		gen_adt_remove_node( o );
		return NULL;
	}
	return o;
}

gen_ADT_Node* gen_adt_append_arr( gen_ADT_Node* parent, char const* name )
{
	gen_ADT_Node* o = gen_adt_alloc( parent );
	if ( ! o )
		return NULL;
	if ( gen_adt_set_arr( o, name, GEN_ARRAY_HEADER( parent->nodes )->allocator ) )
	{
		gen_adt_remove_node( o );
		return NULL;
	}
	return o;
}

gen_ADT_Node* gen_adt_append_str( gen_ADT_Node* parent, char const* name, char const* value )
{
	gen_ADT_Node* o = gen_adt_alloc( parent );
	if ( ! o )
		return NULL;
	gen_adt_set_str( o, name, value );
	return o;
}

gen_ADT_Node* gen_adt_append_flt( gen_ADT_Node* parent, char const* name, gen_f64 value )
{
	gen_ADT_Node* o = gen_adt_alloc( parent );
	if ( ! o )
		return NULL;
	gen_adt_set_flt( o, name, value );
	return o;
}

gen_ADT_Node* gen_adt_append_int( gen_ADT_Node* parent, char const* name, gen_s64 value )
{
	gen_ADT_Node* o = gen_adt_alloc( parent );
	if ( ! o )
		return NULL;
	gen_adt_set_int( o, name, value );
	return o;
}

/* parser helpers */
char* gen_adt_parse_number_strict( gen_ADT_Node* node, char* base_str )
{
	GEN_ASSERT_NOT_NULL( node );
	GEN_ASSERT_NOT_NULL( base_str );
	char *p = base_str, *e = p;

	while ( *e )
		++e;

	while ( *p && ( gen_str_find( "eE.+-", *p ) || gen_char_is_hex_digit( *p ) ) )
	{
		++p;
	}

	if ( p >= e )
	{
		return gen_adt_parse_number( node, base_str );
	}

	return base_str;
}

char* gen_adt_parse_number( gen_ADT_Node* node, char* base_str )
{
	GEN_ASSERT_NOT_NULL( node );
	GEN_ASSERT_NOT_NULL( base_str );
	char *p = base_str, *e = p;

	gen_s32 base         = 0;
	gen_s32 base2        = 0;
	gen_u8  base2_offset = 0;
	gen_s8  exp = 0, orig_exp = 0;
	gen_u8  neg_zero   = 0;
	gen_u8  lead_digit = 0;
	gen_u8  node_type  = 0;
	gen_u8  node_props = 0;

	/* skip false positives and special cases */
	if ( ! ! gen_str_find( "eE", *p ) || ( ! ! gen_str_find( ".+-", *p ) && ! gen_char_is_hex_digit( *( p + 1 ) ) && *( p + 1 ) != '.' ) )
	{
		return ++base_str;
	}

	node_type = gen_EADT_TYPE_INTEGER;
	neg_zero  = false;

	gen_sw ib        = 0;
	char   buf[ 48 ] = { 0 };

	if ( *e == '+' )
		++e;
	else if ( *e == '-' )
	{
		buf[ ib++ ] = *e++;
	}

	if ( *e == '.' )
	{
		node_type   = gen_EADT_TYPE_REAL;
		node_props  = gen_EADT_PROPS_IS_PARSED_REAL;
		lead_digit  = false;
		buf[ ib++ ] = '0';
		do
		{
			buf[ ib++ ] = *e;
		} while ( gen_char_is_digit( *++e ) );
	}
	else
	{
		if ( ! gen_str_compare( e, "0x", 2 ) || ! gen_str_compare( e, "0X", 2 ) )
		{
			node_props = gen_EADT_PROPS_IS_HEX;
		}

		/* bail if ZPL_ADT_PROPS_IS_HEX is unset but we get 'x' on input */
		if ( gen_char_to_lower( *e ) == 'x' && ( node_props != gen_EADT_PROPS_IS_HEX ) )
		{
			return ++base_str;
		}

		while ( gen_char_is_hex_digit( *e ) || gen_char_to_lower( *e ) == 'x' )
		{
			buf[ ib++ ] = *e++;
		}

		if ( *e == '.' )
		{
			node_type    = gen_EADT_TYPE_REAL;
			lead_digit   = true;
			gen_u32 step = 0;

			do
			{
				buf[ ib++ ] = *e;
				++step;
			} while ( gen_char_is_digit( *++e ) );

			if ( step < 2 )
			{
				buf[ ib++ ] = '0';
			}
		}
	}

	/* check if we have a dot here, this is a false positive (IP address, ...) */
	if ( *e == '.' )
	{
		return ++base_str;
	}

	gen_f32 eb          = 10;
	char    expbuf[ 6 ] = { 0 };
	gen_sw  expi        = 0;

	if ( *e && ! ! gen_str_find( "eE", *e ) )
	{
		++e;
		if ( *e == '+' || *e == '-' || gen_char_is_digit( *e ) )
		{
			if ( *e == '-' )
			{
				eb = 0.1f;
			}
			if ( ! gen_char_is_digit( *e ) )
			{
				++e;
			}
			while ( gen_char_is_digit( *e ) )
			{
				expbuf[ expi++ ] = *e++;
			}
		}

		orig_exp = exp = ( gen_u8 )gen_str_to_i64( expbuf, NULL, 10 );
	}

	if ( node_type == gen_EADT_TYPE_INTEGER )
	{
		node->integer = gen_str_to_i64( buf, 0, 0 );
#ifndef GEN_PARSER_DISABLE_ANALYSIS
		/* special case: negative zero */
		if ( node->integer == 0 && buf[ 0 ] == '-' )
		{
			neg_zero = true;
		}
#endif
		while ( orig_exp-- > 0 )
		{
			node->integer *= ( gen_s64 )eb;
		}
	}
	else
	{
		node->real = gen_str_to_f64( buf, 0 );

#ifndef GEN_PARSER_DISABLE_ANALYSIS
		char *q = buf, *base_string = q, *base_string2 = q;
		base_string           = zpl_cast( char* ) gen_str_skip( base_string, '.' );
		*base_string          = '\0';
		base_string2          = base_string + 1;
		char* base_string_off = base_string2;
		while ( *base_string_off++ == '0' )
			base2_offset++;

		base  = ( gen_s32 )gen_str_to_i64( q, 0, 0 );
		base2 = ( gen_s32 )gen_str_to_i64( base_string2, 0, 0 );
		if ( exp )
		{
			exp        = exp * ( ! ( eb == 10.0f ) ? -1 : 1 );
			node_props = gen_EADT_PROPS_IS_EXP;
		}

		/* special case: negative zero */
		if ( base == 0 && buf[ 0 ] == '-' )
		{
			neg_zero = true;
		}
#endif
		while ( orig_exp-- > 0 )
		{
			node->real *= eb;
		}
	}

	node->type  = node_type;
	node->props = node_props;

#ifndef GEN_PARSER_DISABLE_ANALYSIS
	node->base         = base;
	node->base2        = base2;
	node->base2_offset = base2_offset;
	node->exp          = exp;
	node->neg_zero     = neg_zero;
	node->lead_digit   = lead_digit;
#else
	gen_unused( base );
	gen_unused( base2 );
	gen_unused( base2_offset );
	gen_unused( exp );
	gen_unused( neg_zero );
	gen_unused( lead_digit );
#endif
	return e;
}

gen_ADT_Error gen_adt_print_number( gen_FileInfo* file, gen_ADT_Node* node )
{
	GEN_ASSERT_NOT_NULL( file );
	GEN_ASSERT_NOT_NULL( node );
	if ( node->type != gen_EADT_TYPE_INTEGER && node->type != gen_EADT_TYPE_REAL )
	{
		return gen_EADT_ERROR_INVALID_TYPE;
	}

#ifndef GEN_PARSER_DISABLE_ANALYSIS
	if ( node->neg_zero )
	{
		gen__adt_fprintf( file, "-" );
	}
#endif

	switch ( node->type )
	{
		case gen_EADT_TYPE_INTEGER :
			{
				if ( node->props == gen_EADT_PROPS_IS_HEX )
				{
					gen__adt_fprintf( file, "0x%llx", ( long long )node->integer );
				}
				else
				{
					gen__adt_fprintf( file, "%lld", ( long long )node->integer );
				}
			}
			break;

		case gen_EADT_TYPE_REAL :
			{
				if ( node->props == gen_EADT_PROPS_NAN )
				{
					gen__adt_fprintf( file, "NaN" );
				}
				else if ( node->props == gen_EADT_PROPS_NAN_NEG )
				{
					gen__adt_fprintf( file, "-NaN" );
				}
				else if ( node->props == gen_EADT_PROPS_INFINITY )
				{
					gen__adt_fprintf( file, "Infinity" );
				}
				else if ( node->props == gen_EADT_PROPS_INFINITY_NEG )
				{
					gen__adt_fprintf( file, "-Infinity" );
				}
				else if ( node->props == gen_EADT_PROPS_TRUE )
				{
					gen__adt_fprintf( file, "true" );
				}
				else if ( node->props == gen_EADT_PROPS_FALSE )
				{
					gen__adt_fprintf( file, "false" );
				}
				else if ( node->props == gen_EADT_PROPS_NULL )
				{
					gen__adt_fprintf( file, "null" );
#ifndef GEN_PARSER_DISABLE_ANALYSIS
				}
				else if ( node->props == gen_EADT_PROPS_IS_EXP )
				{
					gen__adt_fprintf( file, "%lld.%0*d%llde%lld", ( long long )node->base, node->base2_offset, 0, ( long long )node->base2, ( long long )node->exp );
				}
				else if ( node->props == gen_EADT_PROPS_IS_PARSED_REAL )
				{
					if ( ! node->lead_digit )
						gen__adt_fprintf( file, ".%0*d%lld", node->base2_offset, 0, ( long long )node->base2 );
					else
						gen__adt_fprintf( file, "%lld.%0*d%lld", ( long long int )node->base2_offset, 0, ( int )node->base, ( long long )node->base2 );
#endif
				}
				else
				{
					gen__adt_fprintf( file, "%f", node->real );
				}
			}
			break;
	}

	return gen_EADT_ERROR_NONE;
}

gen_ADT_Error gen_adt_print_string( gen_FileInfo* file, gen_ADT_Node* node, char const* escaped_chars, char const* escape_symbol )
{
	GEN_ASSERT_NOT_NULL( file );
	GEN_ASSERT_NOT_NULL( node );
	GEN_ASSERT_NOT_NULL( escaped_chars );
	if ( node->type != gen_EADT_TYPE_STRING && node->type != gen_EADT_TYPE_MULTISTRING )
	{
		return gen_EADT_ERROR_INVALID_TYPE;
	}

	/* escape string */
	char const *p = node->string, *b = p;

	if ( ! p )
		return gen_EADT_ERROR_NONE;

	do
	{
		p = gen_str_skip_any( p, escaped_chars );
		gen__adt_fprintf( file, "%.*s", gen_ptr_diff( b, p ), b );
		if ( *p && ! ! gen_str_find( escaped_chars, *p ) )
		{
			gen__adt_fprintf( file, "%s%c", escape_symbol, *p );
			p++;
		}
		b = p;
	} while ( *p );

	return gen_EADT_ERROR_NONE;
}

gen_ADT_Error gen_adt_str_to_number( gen_ADT_Node* node )
{
	GEN_ASSERT( node );

	if ( node->type == gen_EADT_TYPE_REAL || node->type == gen_EADT_TYPE_INTEGER )
		return gen_EADT_ERROR_ALREADY_CONVERTED; /* this is already converted/parsed */
	if ( node->type != gen_EADT_TYPE_STRING && node->type != gen_EADT_TYPE_MULTISTRING )
	{
		return gen_EADT_ERROR_INVALID_TYPE;
	}

	gen_adt_parse_number( node, ( char* )node->string );

	return gen_EADT_ERROR_NONE;
}

gen_ADT_Error gen_adt_str_to_number_strict( gen_ADT_Node* node )
{
	GEN_ASSERT( node );

	if ( node->type == gen_EADT_TYPE_REAL || node->type == gen_EADT_TYPE_INTEGER )
		return gen_EADT_ERROR_ALREADY_CONVERTED; /* this is already converted/parsed */
	if ( node->type != gen_EADT_TYPE_STRING && node->type != gen_EADT_TYPE_MULTISTRING )
	{
		return gen_EADT_ERROR_INVALID_TYPE;
	}

	gen_adt_parse_number_strict( node, ( char* )node->string );

	return gen_EADT_ERROR_NONE;
}

#undef gen__adt_fprintf

#pragma endregion ADT

