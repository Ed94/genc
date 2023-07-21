#pragma region Gen Interface

// Initialize the library.
// This currently just initializes the gen_CodePool.
void gen_init();

// Currently manually free's the arenas, gen_Code for checking for leaks.
// However on Windows at least, it doesn't need to occur as the OS will clean up after the process.
void gen_deinit();

// Clears the allocations, but doesn't return to the heap, the calls init() again.
// Ease of use.
void gen_reset();

// Used internally to retrive or make string allocations.
// Strings are stored in a series of string arenas of fixed size (GEN_SIZE_PER_STRING_ARENA)
gen_StringCached gen_get_cached_string( gen_Str str );

/*
	This provides a fresh gen_Code AST.
	The gen interface use this as their method from getting a new AST object from the gen_CodePool.
	Use this if you want to make your own API for formatting the supported gen_Code Types.
*/
gen_Code gen_make_code();

// Set these before calling gen's init() procedure.
// Data

void gen_set_allocator_data_arrays ( gen_AllocatorInfo data_array_allocator );
void gen_set_allocator_code_pool   ( gen_AllocatorInfo pool_allocator );
void gen_set_allocator_lexer       ( gen_AllocatorInfo lex_allocator );
void gen_set_allocator_string_arena( gen_AllocatorInfo string_allocator );
void gen_set_allocator_string_table( gen_AllocatorInfo string_allocator );
void gen_set_allocator_type_table  ( gen_AllocatorInfo type_reg_allocator );

#pragma region Upfront
gen_CodeAttributes gen_def_attributes( gen_Str content );
gen_CodeComment    gen_def_comment   ( gen_Str content );

gen_CodeEnum gen_def_enum( gen_Str name , gen_Code body = NoCode, gen_CodeType type = NoCode, gen_CodeAttributes attributes = NoCode );

gen_CodeExec gen_def_execution( gen_Str content );

gen_CodeFn gen_def_function( gen_Str name, gen_CodeParam params = NoCode, gen_CodeType ret_type = NoCode, gen_Code body = NoCode
	, gen_CodeSpecifiers specifiers = NoCode, gen_CodeAttributes attributes = NoCode );

gen_CodeInclude gen_def_include( gen_Str content );

gen_CodeParam      gen_def_param    ( gen_CodeType type, gen_Str name, gen_Code value = NoCode );
gen_CodeSpecifiers gen_def_specifier( gen_SpecifierT specifier );

gen_CodeStruct gen_def_struct( gen_Str name, gen_Code body = NoCode, gen_CodeAttributes attributes = NoCode );

gen_CodeType    gen_def_type   ( gen_Str name, gen_Code arrayexpr = NoCode, gen_CodeSpecifiers specifiers = NoCode, gen_CodeAttributes attributes = NoCode );
gen_CodeTypedef gen_def_typedef( gen_Str name, gen_Code type, gen_CodeAttributes attributes = NoCode );

gen_CodeUnion gen_def_union( gen_Str name, gen_Code body, gen_CodeAttributes attributes = NoCode );

gen_CodeVar gen_def_variable( gen_CodeType type, gen_Str name, gen_Code value = NoCode
	, gen_CodeSpecifiers specifiers = NoCode, gen_CodeAttributes attributes = NoCode );

// Constructs an empty body. Use AST::validate_body() to check if the body is was has valid entries.
gen_CodeBody gen_def_body( gen_CodeT type );

// There are two options for defining a struct body, either varadically provided with the args macro to auto-deduce the arg num,
/// or provide as an array of gen_Code objects.

gen_CodeBody       gen_def_enum_body    ( gen_s32 num, ... );
gen_CodeBody       gen_def_enum_body    ( gen_s32 num, gen_Code* codes );
gen_CodeBody       gen_def_function_body( gen_s32 num, ... );
gen_CodeBody       gen_def_function_body( gen_s32 num, gen_Code* codes );
gen_CodeBody       gen_def_global_body  ( gen_s32 num, ... );
gen_CodeBody       gen_def_global_body  ( gen_s32 num, gen_Code* codes );
gen_CodeParam      gen_def_params       ( gen_s32 num, ... );
gen_CodeParam      gen_def_params       ( gen_s32 num, gen_CodeParam* params );
gen_CodeSpecifiers gen_def_specifiers   ( gen_s32 num, ... );
gen_CodeSpecifiers gen_def_specifiers   ( gen_s32 num, gen_SpecifierT* specs );
gen_CodeBody       gen_def_struct_body  ( gen_s32 num, ... );
gen_CodeBody       gen_def_struct_body  ( gen_s32 num, gen_Code* codes );
gen_CodeBody       gen_def_union_body   ( gen_s32 num, ... );
gen_CodeBody       gen_def_union_body   ( gen_s32 num, gen_Code* codes );
#pragma endregion Upfront

#pragma region Parsing
gen_CodeEnum    gen_parse_enum       ( gen_Str enum_def      );
gen_CodeFn      gen_parse_function   ( gen_Str fn_def        );
gen_CodeBody    gen_parse_global_body( gen_Str body_def      );
gen_CodeStruct  gen_parse_struct     ( gen_Str struct_def    );
gen_CodeType    gen_parse_type       ( gen_Str type_def      );
gen_CodeTypedef gen_parse_typedef    ( gen_Str typedef_def   );
gen_CodeUnion   gen_parse_union      ( gen_Str union_def     );
gen_CodeVar     gen_parse_variable   ( gen_Str var_def       );
#pragma endregion Parsing

#pragma region Untyped

gen_sw  gen_token_fmt_va( char* buf, gen_uw buf_size, gen_s32 num_tokens, va_list va );
gen_Str gen_token_fmt_impl( gen_sw, ... );

gen_Code gen_untyped_str( gen_Str content);
gen_Code gen_untyped_fmt( char const* fmt, ... );
gen_Code gen_untyped_token_fmt( char const* fmt, gen_s32 num_tokens, ... );

#pragma endregion Untyped

#pragma endregion Gen Interaface
