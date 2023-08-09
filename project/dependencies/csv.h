#pragma region CSV

typedef enum gen_CSV_Error
{
	gen_ECSV_Error__NONE,
	gen_ECSV_Error__INTERNAL,
	gen_ECSV_Error__UNEXPECTED_END_OF_INPUT,
	gen_ECSV_Error__MISMATCHED_ROWS,
} gen_CSV_Error;

typedef gen_ADT_Node gen_CSV_Object;

GEN_DEF_INLINE gen_u8 gen_csv_parse( gen_CSV_Object* root, char* text, gen_AllocatorInfo allocator, gen_b32 has_header );
               gen_u8 gen_csv_parse_delimiter( gen_CSV_Object* root, char* text, gen_AllocatorInfo allocator, gen_b32 has_header, char delim );
               void   gen_csv_free( gen_CSV_Object* obj );

GEN_DEF_INLINE void       gen_csv_write( gen_FileInfo* file, gen_CSV_Object* obj );
GEN_DEF_INLINE gen_String gen_csv_write_string( gen_AllocatorInfo a, gen_CSV_Object* obj );
               void       gen_csv_write_delimiter( gen_FileInfo* file, gen_CSV_Object* obj, char delim );
               gen_String gen_csv_write_string_delimiter( gen_AllocatorInfo a, gen_CSV_Object* obj, char delim );

/* inline */

GEN_IMPL_INLINE gen_u8 gen_csv_parse( gen_CSV_Object* root, char* text, gen_AllocatorInfo allocator, gen_b32 has_header )
{
	return gen_csv_parse_delimiter( root, text, allocator, has_header, ',' );
}

GEN_IMPL_INLINE void gen_csv_write( gen_FileInfo* file, gen_CSV_Object* obj )
{
	gen_csv_write_delimiter( file, obj, ',' );
}

GEN_IMPL_INLINE gen_String gen_csv_write_string( gen_AllocatorInfo a, gen_CSV_Object* obj )
{
	return gen_csv_write_string_delimiter( a, obj, ',' );
}

#pragma endregion CSV

