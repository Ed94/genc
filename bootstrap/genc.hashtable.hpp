#pragma once

#define GEN_DEFINE_LIBRARY_CODE_CONSTANTS
#define GEN_ENFORCE_STRONG_CODE_TYPES
#define GEN_EXPOSE_BACKEND
#include "gen/gen.hpp"
#include "genc.array.hpp"

using namespace gen;

CodeBody gen_hashtable_base()
{
	return parse_global_body( code(
		typedef struct gen_HT_FindResult gen_HT_FindResult;
		struct gen_HT_FindResult
		{
			gen_sw HashIndex;
			gen_sw PrevIndex;
			gen_sw EntryIndex;
		};
	));
}

CodeBody gen_hashtable( StrC type, StrC hashtable_name )
{
	String
	fn = String::make_reserve( GlobalAllocator, hashtable_name.Len + sizeof("gen") );
	fn.append_fmt( "gen_%.*s", hashtable_name.Len, hashtable_name.Ptr );
	str_to_lower(fn.Data);

	String
	tbl_type = String::make_reserve( GlobalAllocator, hashtable_name.Len + sizeof("gen") );
	tbl_type.append_fmt( "gen_%.*s", hashtable_name.Len, hashtable_name.Ptr );

	String name_lower = String::make( GlobalAllocator, hashtable_name );
	str_to_lower( name_lower.Data );

	String hashtable_entry   = String::fmt_buf( GlobalAllocator, "gen_HTE_%.*s",           hashtable_name.Len, hashtable_name.Ptr );
	String entry_array_name  = String::fmt_buf( GlobalAllocator, "Arr_HTE_%.*s",     hashtable_name.Len, hashtable_name.Ptr );
	String entry_array_fn_ns = String::fmt_buf( GlobalAllocator, "gen_arr_hte_%.*s", name_lower.length(), name_lower.Data );

	CodeBody hashtable_types = parse_global_body( token_fmt(
		"type",        (StrC) type,
		"tbl_name",    (StrC) hashtable_name,
		"tbl_type",    (StrC) tbl_type,
	stringize(
		typedef struct gen_HTE_<tbl_name> gen_HTE_<tbl_name>;
		struct gen_HTE_<tbl_name>
		{
			gen_u64    Key;
			gen_sw     Next;
			<type>     Value;
		};

		typedef void (* <tbl_type>_MapProc)    ( <tbl_type> self, gen_u64 key, <type> value );
		typedef void (* <tbl_type>_MapMutProc) ( <tbl_type> self, gen_u64 key, <type>* value );
	)));

	CodeBody entry_array = gen_array( hashtable_entry, entry_array_name );

#pragma push_macro( "GEN_ASSERT" )
	CodeBody hashtable_def = parse_global_body( token_fmt(
		"type",           (StrC) type,
		"tbl_name",       (StrC) hashtable_name,
		"tbl_type",       (StrC) tbl_type,
		"fn",             (StrC) fn,
		"array_entry",    (StrC) entry_array_name,
		"fn_array_entry", (StrC) entry_array_fn_ns,
	stringize(
		typedef struct <tbl_type> <tbl_type>;
		struct <tbl_type>
		{
			gen_Array_sw      Hashes;
			gen_<array_entry> Entries;
		};

		<tbl_type> <fn>_make        ( gen_AllocatorInfo allocator );
		<tbl_type> <fn>_make_reserve( gen_AllocatorInfo allocator, gen_sw num );
		void       <fn>_clear       ( <tbl_type> self );
		void       <fn>_destroy     ( <tbl_type> self );
		<type>*    <fn>_get         ( gen_u64 key );
		void       <fn>_map         ( <tbl_type>_MapProc map_proc );
		void       <fn>_map_mut     ( <tbl_type>_MapMutProc map_proc );
		void       <fn>_grow        ( <tbl_type> self );
		void       <fn>_rehash      ( <tbl_type> self, gen_sw new_num );
		void       <fn>_rehash_fast ( <tbl_type> self );
		void       <fn>_remove      ( <tbl_type> self, gen_u64 key );
		void       <fn>_remove_entry( <tbl_type> self, gen_sw idx );
		void       <fn>_set         ( <tbl_type> self, gen_u64 key, <type> value );
		gen_sw     <fn>_slot        ( <tbl_type> self, gen_u64 key );

		gen_sw             <fn>_add_entry( <tbl_type> self, gen_u64 key );
		gen_HT_FindResult  <fn>_find     ( <tbl_type> self );
		gen_b32            <fn>_full     ( <tbl_type> self );

		<tbl_type> <fn>_make( gen_AllocatorInfo allocator )
		{
			<tbl_type>
			result        = { nullptr, nullptr };
			result.Hashes  = gen_array_sw_make( allocator );
			result.Entries = <fn_array_entry>_make( allocator );

			return result;
		}

		<tbl_type> <fn>_make_reserve( gen_AllocatorInfo allocator, gen_sw num )
		{
			<tbl_type>
			result         = { nullptr, nullptr };
			result.Hashes  = gen_array_sw_make_reserve( allocator, num );
			result.Entries = <fn_array_entry>_make_reserve( allocator, num );

			return result;
		}

		void <fn>_clear( <tbl_type> self )
		{
			for ( gen_sw idx = 0; idx < gen_array_header( self.Hashes )->Num; idx++ )
				self.Hashes[idx] = -1;

			gen_array_sw_clear( self.Hashes );
			<fn_array_entry>_clear( self.Entries );
		}
	)));
#pragma pop_macro( "GEN_ASSERT" )

	return def_global_body(args(
		hashtable_types,
		fmt_newline,
		entry_array,
		fmt_newline,
		hashtable_def
	));
}
