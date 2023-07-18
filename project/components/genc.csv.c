#pragma region CSV
gen_u8 gen_csv_parse_delimiter( gen_CSV_Object* root, char* text, gen_AllocatorInfo allocator, gen_b32 has_header, char delim )
{
	gen_CSV_Error err = gen_ECSV_Error__NONE;
	GEN__ASSERT_NOT_NULL( root );
	GEN__ASSERT_NOT_NULL( text );
	gen_zero_item( root );
	gen_adt_make_branch( root, allocator, NULL, has_header ? false : true );
	char * p = text, *b = p, *e = p;
	gen_sw colc = 0, total_colc = 0;

	do
	{
		char d = 0;
		p      = zpl_cast( char* ) gen_str_trim( p, false );
		if ( *p == 0 )
			break;
		gen_ADT_Node row_item = { 0 };
		row_item.type         = gen_EADTTYPE_STRING;
#ifndef GEN__PARSER_DISABLE_ANALYSIS
		row_item.name_style = gen_EADTNAME_STYLE_NO_QUOTES;
#endif

		/* handle string literals */
		if ( *p == '"' )
		{
			p = b = e       = p + 1;
			row_item.string = b;
#ifndef GEN__PARSER_DISABLE_ANALYSIS
			row_item.name_style = gen_EADTNAME_STYLE_DOUBLE_QUOTE;
#endif
			do
			{
				e = zpl_cast( char* ) gen_str_skip( e, '"' );
				if ( *e && *( e + 1 ) == '"' )
				{
					e += 2;
				}
				else
					break;
			} while ( *e );
			if ( *e == 0 )
			{
				GEN__CSV_ASSERT( "unmatched quoted string" );
				err = gen_ECSV_Error__UNEXPECTED_END_OF_INPUT;
				return err;
			}
			*e = 0;
			p  = zpl_cast( char* ) gen_str_trim( e + 1, true );
			d  = *p;

			/* unescape escaped quotes (so that unescaped text escapes :) */
			{
				char* ep = b;
				do
				{
					if ( *ep == '"' && *( ep + 1 ) == '"' )
					{
						gen_mem_move( ep, ep + 1, gen_str_len( ep ) );
					}
					ep++;
				} while ( *ep );
			}
		}
		else if ( *p == delim )
		{
			d               = *p;
			row_item.string = "";
		}
		else if ( *p )
		{
			/* regular data */
			b = e           = p;
			row_item.string = b;
			do
			{
				e++;
			} while ( *e && *e != delim && *e != '\n' );
			if ( *e )
			{
				p = zpl_cast( char* ) gen_str_trim( e, true );
				while ( gen_char_is_space( *( e - 1 ) ) )
				{
					e--;
				}
				d  = *p;
				*e = 0;
			}
			else
			{
				d = 0;
				p = e;
			}

			/* check if number and process if so */
			gen_b32 skip_number = false;
			char*   num_p       = b;
			do
			{
				if ( ! gen_char_is_hex_digit( *num_p ) && ( ! gen_str_find( "+-.eExX", *num_p ) ) )
				{
					skip_number = true;
					break;
				}
			} while ( *num_p++ );

			if ( ! skip_number )
			{
				gen_adt_str_to_number( &row_item );
			}
		}

		if ( colc >= gen_array_count( root->nodes ) )
		{
			gen_adt_append_arr( root, NULL );
		}

		gen_array_append( root->nodes[ colc ].nodes, row_item );

		if ( d == delim )
		{
			colc++;
			p++;
		}
		else if ( d == '\n' || d == 0 )
		{
			/* check if number of rows is not mismatched */
			if ( total_colc < colc )
				total_colc = colc;
			else if ( total_colc != colc )
			{
				GEN__CSV_ASSERT( "mismatched rows" );
				err = gen_ECSV_Error__MISMATCHED_ROWS;
				return err;
			}
			colc = 0;
			if ( d != 0 )
				p++;
		}
	} while ( *p );

	if ( gen_array_count( root->nodes ) == 0 )
	{
		GEN__CSV_ASSERT( "unexpected end of input. stream is empty." );
		err = gen_ECSV_Error__UNEXPECTED_END_OF_INPUT;
		return err;
	}

	/* consider first row as a header. */
	if ( has_header )
	{
		for ( gen_sw i = 0; i < gen_array_count( root->nodes ); i++ )
		{
			gen_CSV_Object* col = root->nodes + i;
			gen_CSV_Object* hdr = col->nodes;
			col->name           = hdr->string;
			gen_array_remove_at( col->nodes, 0 );
		}
	}

	return err;
}

void gen_csv_free( gen_CSV_Object* obj )
{
	gen_adt_destroy_branch( obj );
}

void gen__csv_write_record( gen_FileInfo* file, gen_CSV_Object* node )
{
	switch ( node->type )
	{
		case gen_EADTTYPE_STRING :
			{
#ifndef GEN__PARSER_DISABLE_ANALYSIS
				switch ( node->name_style )
				{
					case gen_EADTNAME_STYLE_DOUBLE_QUOTE :
						{
							gen_str_fmt_file( file, "\"" );
							gen_adt_print_string( file, node, "\"", "\"" );
							gen_str_fmt_file( file, "\"" );
						}
						break;

					case gen_EADTNAME_STYLE_NO_QUOTES :
						{
#endif
							gen_str_fmt_file( file, "%s", node->string );
#ifndef GEN__PARSER_DISABLE_ANALYSIS
						}
						break;
				}
#endif
			}
			break;

		case gen_EADTTYPE_REAL :
		case gen_EADTTYPE_INTEGER :
			{
				gen_adt_print_number( file, node );
			}
			break;
	}
}

void gen__csv_write_header( gen_FileInfo* file, gen_CSV_Object* header )
{
	gen_CSV_Object temp = *header;
	temp.string         = temp.name;
	temp.type           = gen_EADTTYPE_STRING;
	gen__csv_write_record( file, &temp );
}

void gen_csv_write_delimiter( gen_FileInfo* file, gen_CSV_Object* obj, char delimiter )
{
	GEN__ASSERT_NOT_NULL( file );
	GEN__ASSERT_NOT_NULL( obj );
	GEN__ASSERT( obj->nodes );
	gen_sw cols = gen_array_count( obj->nodes );
	if ( cols == 0 )
		return;

	gen_sw rows = gen_array_count( obj->nodes[ 0 ].nodes );
	if ( rows == 0 )
		return;

	gen_b32 has_headers = obj->nodes[ 0 ].name != NULL;

	if ( has_headers )
	{
		for ( gen_sw i = 0; i < cols; i++ )
		{
			gen__csv_write_header( file, &obj->nodes[ i ] );
			if ( i + 1 != cols )
			{
				gen_str_fmt_file( file, "%c", delimiter );
			}
		}
		gen_str_fmt_file( file, "\n" );
	}

	for ( gen_sw r = 0; r < rows; r++ )
	{
		for ( gen_sw i = 0; i < cols; i++ )
		{
			gen__csv_write_record( file, &obj->nodes[ i ].nodes[ r ] );
			if ( i + 1 != cols )
			{
				gen_str_fmt_file( file, "%c", delimiter );
			}
		}
		gen_str_fmt_file( file, "\n" );
	}
}

gen_String gen_csv_write_string_delimiter( gen_AllocatorInfo a, gen_CSV_Object* obj, char delimiter )
{
	gen_FileInfo tmp;
	gen_file_stream_new( &tmp, a );
	gen_csv_write_delimiter( &tmp, obj, delimiter );
	gen_sw     fsize;
	gen_u8*    buf    = gen_file_stream_buf( &tmp, &fsize );
	gen_String output = gen_string_make_length( a, ( char* )buf, fsize );
	gen_file_close( &tmp );
	return output;
}
#pragma endregion CSV