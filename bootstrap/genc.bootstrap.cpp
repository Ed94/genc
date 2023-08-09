#define GEN_DEFINE_LIBRARY_CODE_CONSTANTS
#define GEN_ENFORCE_STRONG_CODE_TYPES
#define GEN_EXPOSE_BACKEND
#define GEN_IMPLEMENTATION
#include "gen/gen.hpp"
#include "genc.array.hpp"
#include "genc.hashtable.hpp"

using namespace gen;

CodeBody gen_ecode()
{
	char scratch_mem[kilobytes(4)];
	Arena scratch = Arena::init_from_memory( scratch_mem, sizeof(scratch_mem) );

	file_read_contents( scratch, zero_terminate, "./enums/ECode.csv" );

	CSV_Object csv_nodes;
	csv_parse( &csv_nodes, scratch_mem, GlobalAllocator, false );

	Array<ADT_Node> enum_strs = csv_nodes.nodes[0].nodes;

	String enum_entries   = String::make_reserve( GlobalAllocator, kilobytes(1) );
	String to_str_entries = String::make_reserve( GlobalAllocator, kilobytes(1) );

	for ( ADT_Node node : enum_strs )
	{
		char const* code = node.string;
		enum_entries.append_fmt( "gen_ECode_%s,\n", code );
		to_str_entries.append_fmt( "{ sizeof(\"%s\"), \"%s\" },", code, code );
	}

	CodeTypedef td_enum = parse_typedef(code(typedef gen_u32 gen_CodeT;));

	CodeEnum enum_code = parse_enum( token_fmt( "entries", (StrC)enum_entries, stringize(
		enum gen_ECode
		{
			<entries>
			gen_ECode_Num
		};
	)));

	CodeFn to_str = parse_function( token_fmt( "entries", (StrC)to_str_entries, stringize(
		gen_Str gen_ecode_to_str( gen_CodeT type )
		{
			local_persist
			gen_Str lookup[] {
				<entries>
			};

			return lookup[ type ];
		}
	)));

	return def_global_body( args( td_enum, enum_code, to_str ) );
}

CodeBody gen_especifier()
{
	char scratch_mem[kilobytes(4)];
	Arena scratch = Arena::init_from_memory( scratch_mem, sizeof(scratch_mem) );

	file_read_contents( scratch, zero_terminate, "./enums/ESpecifier.csv" );

	CSV_Object csv_nodes;
	csv_parse( &csv_nodes, scratch_mem, GlobalAllocator, false );

	Array<ADT_Node> enum_strs = csv_nodes.nodes[0].nodes;
	Array<ADT_Node> str_strs  = csv_nodes.nodes[1].nodes;

	String enum_entries   = String::make_reserve( GlobalAllocator, kilobytes(1) );
	String to_str_entries = String::make_reserve( GlobalAllocator, kilobytes(1) );

	for (uw idx = 0; idx < enum_strs.num(); idx++)
	{
		char const* enum_str     = enum_strs[idx].string;
		char const* entry_to_str = str_strs [idx].string;

		enum_entries.append_fmt( "gen_ESpecifier_%s,\n", enum_str );
		to_str_entries.append_fmt( "{ sizeof(\"%s\"), \"%s\" },", entry_to_str, entry_to_str);
	}

	CodeTypedef td_enum   = parse_typedef(code(typedef gen_u32 gen_SpecifierT; ));
	CodeEnum    enum_code = parse_enum(token_fmt("entries", (StrC)enum_entries, stringize(
		enum gen_ESpecifier
		{
			<entries>
			gen_ESpecifier_Num
		};
	)));
	CodeFn to_str = parse_function(token_fmt("entries", (StrC)to_str_entries, stringize(
		gen_Str gen_especifier_to_str(gen_SpecifierT type)
		{
			local_persist
			gen_Str lookup[] {
				<entries>
			};

			return lookup[ type ];
		}
	)));

	return def_global_body( args( td_enum, enum_code, to_str ) );
}

int main()
{
	using namespace gen;
	gen::init();

	Code push_ignores = scan_file( "helpers/push_ignores.inline.hpp" );
	Code pop_ignores  = scan_file( "helpers/pop_ignores.inline.hpp" );

	// genc_dep.h
	{
		Code header_start = scan_file( "./dependencies/header_start.h");
		Code macros       = scan_file( "./dependencies/macros.h" );
		Code basic_types  = scan_file( "./dependencies/basic_types.h");
		Code debug        = scan_file( "./dependencies/debug.h" );
		Code memory       = scan_file( "./dependencies/memory.h" );
		Code string_ops   = scan_file( "./dependencies/string_ops.h" );
		Code printing     = scan_file( "./dependencies/printing.h" );
		Code hashing 	  = scan_file( "./dependencies/hashing.h" );
		Code strings      = scan_file( "./dependencies/strings.h" );
		Code filesystem   = scan_file( "./dependencies/filesystem.h" );
		Code timing       = scan_file( "./dependencies/timing.h" );

		CodeBody array_base_impl = gen_array_base();
		CodeBody array_cstr      = gen_array( txt("char*"), txt("Array_CStr") );
		CodeBody array_sw        = gen_array( txt("gen_sw"), txt("Array_sw") );

		CodeBody hashtable_base_impl   = gen_hashtable_base();
		CodeBody hashtable_stringcache = gen_hashtable( txt("gen_StringCached"), txt("StringCache") );

		Builder
		header = Builder::open("genc.dep.h");
		header.print_fmt("// This file is intended to be included within genc.h (There is no pragma diagnostic ignores)\n\n");

		header.print( header_start );
		header.print( macros );
		header.print( basic_types );
		header.print( debug );
		header.print( memory );
		header.print( hashing );
		header.print( string_ops );
		header.print( printing );
		header.print( strings );
		{
			// Containers
			header.print_fmt("#pragma region Containers\n\n");
			header.print( array_base_impl );
			header.print( fmt_newline );
			header.print( array_cstr );
			header.print( array_sw );
			header.print( hashtable_base_impl );
			header.print( hashtable_stringcache );
			header.print_fmt("#pragma endregion Containers\n\n");
		}
		header.print( filesystem );
		header.print( timing );

		header.write();
	}

	// genc_dep.c
	{
		Code src_start  = scan_file( "./dependencies/src_start.c");
		Code debug 	    = scan_file( "./dependencies/debug.c" );
		Code string_ops = scan_file( "./dependencies/string_ops.c" );
		Code printing   = scan_file( "./dependencies/printing.c" );
		Code memory     = scan_file( "./dependencies/memory.c" );
		Code hashing    = scan_file( "./dependencies/hashing.c" );
		Code strings    = scan_file( "./dependencies/strings.c" );
		Code filesystem = scan_file( "./dependencies/filesystem.c" );
		Code timing     = scan_file( "./dependencies/timing.c" );

		Builder
		src = Builder::open( "genc.dep.c" );
		src.print_fmt("// This file is intended to be included within genc.c (There is no pragma diagnostic ignores)\n");
		src.print( def_include( txt("genc.dep.h") ) );
		src.print( fmt_newline );

		src.print( src_start );
		src.print( debug );
		src.print( string_ops );
		src.print( printing );
		src.print( memory );
		src.print( hashing );
		src.print( strings );
		src.print( filesystem );
		src.print( timing );

		src.write();
	}

	constexpr
	char const* gen_time_guard =
R"(#pragma once

#if ! defined(GEN_DONT_ENFORCE_GEN_TIME_GUARD) && ! defined(GEN_TIME)
#error Gen.hpp : GEN_TIME not defined
#endif

)";

	constexpr
	char const* gen_dep_wrap =
R"(//! If its desired to roll your own dependencies, define GEN_ROLL_OWN_DEPENDENCIES before including this file.
// Dependencies are derived from the c-zpl library: https://github.com/zpl-c/zpl
#ifndef GEN_ROLL_OWN_DEPENDENCIES
#	include "genc.dep.h"
#endif

)";

	// genc.h
	{
		// Library
		Code attributes = scan_file("./components/attributes.h");

		CodeBody ecode      = gen_ecode();
		CodeBody especifier = gen_especifier();

		Code ast       = scan_file("./components/ast.h");
		Code interface = scan_file("./components/interface.h");

		Builder
		header = Builder::open("genc.h");
		header.print_fmt( gen_time_guard );
		header.print_fmt( gen_dep_wrap );

		header.print_fmt("#pragma region Types\n");
		header.print( ecode );
		header.print( especifier );
		header.print( attributes );
		header.print_fmt("#pragma endregion Types\n");

		header.print( ast );
		header.print( interface );

		header.write();
	}

	// genc.c
	{
		Builder
		src = Builder::open("genc.c");
		src.print( def_include( txt("genc.h") ) );
		// src.print();
		src.write();
	}

	// genc_builder.h
	{
		Code builder_header = scan_file("./file_ops/builder.h");

		Builder
		header = Builder::open("genc_builder.h");
		header.print( pragma_once );
		header.print( def_include( txt("genc.h") ) );
		header.print( builder_header );
		header.write();
	}

	// genc_builder.c
	{
		Code builder_src = scan_file("./file_ops/builder.c");

		Builder
		header = Builder::open("genc_builder.c");
		header.print( def_include( txt("genc_builder.h") ) );
		header.print( builder_src );
		header.write();
	}

	// genc_scanner.h
	{
		Code adt            = scan_file( "./dependencies/adt.h" );
		Code csv            = scan_file( "./dependencies/csv.h" );
		Code builder_header = scan_file("./file_ops/scanner.h");

		Builder
		header = Builder::open("genc_scanner.h");
		header.print( pragma_once );
		header.print( def_include( txt("genc.h") ) );

		header.print_fmt( "#pragma region Parsing\n\n" );
		header.print( adt );
		header.print( csv );
		header.print_fmt( "#pragma endregion Parsing\n" );

		header.print( builder_header );
		header.write();
	}

	// genc_sanncer.c
	{
		Code adt 		 = scan_file( "./dependencies/adt.c" );
		Code csv 		 = scan_file( "./dependencies/csv.c" );
		Code builder_src = scan_file("./file_ops/builder.c");

		Builder
		header = Builder::open("genc_scanner.c");
		header.print( def_include( txt("genc_scanner.h") ) );
		header.print( builder_src );
		header.write();
	}

	gen::deinit();
	return 0;
}
