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
	char scratch_mem[kilobytes(1)];
	Arena scratch = Arena::init_from_memory( scratch_mem, sizeof(scratch_mem) );

	file_read_contents( scratch, zero_terminate, "./components/ECode.csv" );

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
	char scratch_mem[kilobytes(1)];
	Arena scratch = Arena::init_from_memory( scratch_mem, sizeof(scratch_mem) );

	file_read_contents( scratch, zero_terminate, "./components/ESpecifier.csv" );

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

	Code push_ignores = scan_file( "helpers/gen.push_ignores.inline.hpp" );
	Code pop_ignores  = scan_file( "helpers/gen.pop_ignores.inline.hpp" );

	// genc_dep.h
	{
		Code header_start = scan_file( "./dependencies/genc.header_start.h");
		Code macros       = scan_file( "./dependencies/genc.macros.h" );
		Code basic_types  = scan_file( "./dependencies/genc.basic_types.h");
		Code debug        = scan_file( "./dependencies/genc.debug.h" );
		Code memory       = scan_file( "./dependencies/genc.memory.h" );
		Code string_ops   = scan_file( "./dependencies/genc.string_ops.h" );
		Code printing     = scan_file( "./dependencies/genc.printing.h" );
		Code hashing 	  = scan_file( "./dependencies/genc.hashing.h" );
		Code strings      = scan_file( "./dependencies/genc.strings.h" );
		Code timing       = scan_file( "./dependencies/genc.timing.h" );

		Code filesystem   = scan_file( "./dependencies/genc.filesystem.h" );
		Code adt          = scan_file( "./dependencies/genc.adt.h" );
		Code csv          = scan_file( "./dependencies/genc.csv.h" );

		CodeBody array_base_impl = gen_array_base();
		CodeBody array_cstr      = gen_array( txt_StrC("char*"), txt_StrC("Array_CStr") );
		CodeBody array_sw        = gen_array( txt_StrC("gen_sw"), txt_StrC("Array_sw") );

		CodeBody hashtable_base_impl   = gen_hashtable_base();
		CodeBody hashtable_stringcache = gen_hashtable( txt_StrC("gen_StringCached"), txt_StrC("StringCache") );

		Builder
		deps_header = Builder::open("genc.dep.h");
		deps_header.print_fmt("// This file is intended to be included within genc.h (There is no pragma diagnostic ignores)\n\n");
			deps_header.print( header_start );
			deps_header.print( macros );
			deps_header.print( basic_types );
			deps_header.print( debug );
			deps_header.print( memory );
			deps_header.print( string_ops );
			deps_header.print( printing );
			deps_header.print( strings );
			deps_header.print( timing );
			{
				// Containers
				deps_header.print_fmt("#pragma region Containers\n");
				deps_header.print( array_base_impl );
				deps_header.print( fmt_newline );
				deps_header.print( array_cstr );
				deps_header.print( array_sw );
				deps_header.print( hashtable_base_impl );
				deps_header.print( hashtable_stringcache );
				deps_header.print_fmt("#pragma endregion Containers\n\n");
			}
			deps_header.print( filesystem );
			deps_header.print( adt );
			deps_header.print( csv );
		deps_header.write();
	}

	// genc_dep.c
	{
		Code impl_start = scan_file( "./dependencies/genc.impl_start.c");
		Code debug 	    = scan_file( "./dependencies/genc.debug.c" );
		Code string_ops = scan_file( "./dependencies/genc.string_ops.c" );
		Code hashing    = scan_file( "./dependencies/genc.hashing.c" );
		Code timing     = scan_file( "./dependencies/genc.timing.c" );

		Builder
		deps_impl = Builder::open( "genc.dep.c" );
		deps_impl.print_fmt("// This file is intended to be included within genc.c (There is no pragma diagnostic ignores)\n\n");
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
		Code attributes = scan_file("./components/genc.attributes.h");

		CodeBody ecode      = gen_ecode();
		CodeBody especifier = gen_especifier();

		Code data_structures = scan_file("./components/genc.data_structures.h");
		Code gen_interface   = scan_file("./components/genc.interface.h");

		Code gen_builder     = scan_file("./components/genc.builder.h");

		Builder
		genc_header = Builder::open("genc.h");
		genc_header.print_fmt( gen_time_guard );
		genc_header.print_fmt( gen_dep_wrap );

			genc_header.print_fmt("#pragma region Types\n");
			genc_header.print( ecode );
			genc_header.print( especifier );
			genc_header.print( attributes );
			genc_header.print_fmt("#pragma endregion Types\n");

			genc_header.print( data_structures );
			genc_header.print( gen_interface );
			genc_header.print( gen_builder );

		genc_header.write();
	}

	// genc.c
	{
		Builder
		genc_source = Builder::open("genc.c");
			genc_source.print_fmt( gen_time_guard );
		genc_source.write();
	}

	gen::deinit();
	return 0;
}
