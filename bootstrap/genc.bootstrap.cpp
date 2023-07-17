#define GEN_FEATURE_PARSING
#define GEN_DEFINE_LIBRARY_CODE_CONSTANTS
#define GEN_ENFORCE_STRONG_CODE_TYPES
#define GEN_EXPOSE_BACKEND
#include "gencpp/gen.cpp"

using namespace gen;

// TODO : Can easily make all enums + funcs with this...
Code gen_enum_from_csv( char const* path )
{
	return CodeInvalid;
}

Code scan_file( char const* path )
{
	FileInfo file;

	FileError error = file_open_mode( & file, EFileMode_READ, path );
	if ( error != EFileError_NONE )
	{
		fatal( "scan_file: Could not open genc.macro.h: %s", path );
	}

	sw fsize = file_size( & file );
	if ( fsize <= 0 )
	{
		fatal("scan_file: %s is empty", path );
	}

	String str = String::make_reserve( Memory::GlobalAllocator, fsize );
		file_read( & file, str, fsize );
		str.get_header().Length = fsize;

	file_close( & file );

	return untyped_str( str );
}

CodeBody gen_ecode()
{
	local_persist
	char const* codes[] {
		"Untyped",
		"Comment",
		"PlatformAttributes",
		"Enum",
		"Enum_Body",
		"Execution",
		"Function",
		"Function_Fwd",
		"Function_Body",
		"Global_Body",
		"Generic",
		"Parameters",
		"Include",
		"Specifiers",
		"Struct",
		"Struct_Fwd",
		"Struct_Body",
		"Template_Macro",
		"Typedef",
		"Union",
		"Variable",
	};
	constexpr s32 num_codes = sizeof(codes) / sizeof(char const*);
	constexpr s32 str_size = sizeof(codes) * 2;

	char  str_entries_mem[ kilobytes(64) ];
	Arena str_entries_arena = Arena::init_from_memory( str_entries_mem, sizeof(str_entries_mem) );

	String enum_entries   = String::make_reserve( str_entries_arena, str_size );
	String to_str_entries = String::make_reserve( str_entries_arena, str_size * 10 );

	CodeBody body = def_body( ECode::Enum_Body );

	for ( s32 idx = 0; idx < num_codes; idx++ )
	{
		enum_entries.append_fmt( "%s,\n", codes[idx] );
		to_str_entries.append_fmt( "{ sizeof(\"%s\"), \"%s\" },\n", codes[idx], codes[idx] );
	}

	CodeTypedef td_enum = parse_typedef(code(typedef u32 gen_CodeT;));

	CodeEnum enum_code = parse_enum( token_fmt( "entries", (StrC)enum_entries, stringize(
		enum gen_ECode
		{
			Invalid,
			<entries>
			NumTypes = Variable
		};
	)));

	CodeFn to_str = parse_function( token_fmt( "entries", (StrC)to_str_entries, stringize(
		gen_StrC gen_ecode_to_str( gen_CodeT type )
		{
			local_persist
			gen_StrC lookup[ NumTypes ] {
				{ sizeof("Invalid"), "Invalid" },
				<entries>
				{ sizeof( "NumTypes", "NumTypes" ) }
			};

			return lookup[ type ];
		}
	)));

	return def_global_body( args( td_enum, enum_code, to_str ) );
}

CodeBody gen_especifier()
{


	return CodeInvalid;
}

int main()
{
	using namespace gen;
	gen::init();

	// Dependencies
	Code header_start = scan_file( "./components/genc.header_start.h");
	Code macros       = scan_file( "./components/genc.macros.h" );
	Code basic_types  = scan_file( "./components/genc.basic_types.h");
	Code debug        = scan_file( "./components/genc.debug.h" );
	Code memory       = scan_file( "./components/genc.memory.h" );
	Code strings      = scan_file( "./components/genc.strings.h" );

	// Library
	Code attributes = scan_file("./components/genc.attributes.h");

	CodeBody ecode      = gen_ecode();
	// CodeBody especifier = gen_especifier();

	Code data_structures = scan_file("./components/genc.data_structures.h");

	Builder
	genc_header;
	genc_header.open("genc.h");
	genc_header.print_fmt("#if gen_time\n");

		genc_header.print(header_start);
		genc_header.print(macros);
		genc_header.print(basic_types);
		genc_header.print(debug);
		genc_header.print(memory);
		genc_header.print(strings);

		genc_header.print(ecode);
		genc_header.print(attributes);
		genc_header.print(data_structures);

	genc_header.print_fmt("// gen_time\n#endif\n");
	genc_header.write();
	gen::deinit();
	return 0;
}
