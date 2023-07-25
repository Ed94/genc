#pragma once

#define GEN_DEFINE_LIBRARY_CODE_CONSTANTS
#define GEN_ENFORCE_STRONG_CODE_TYPES
#define GEN_EXPOSE_BACKEND
#include "gen.hpp"

using namespace gen;

CodeBody gen_array_base()
{
	CodeStruct header = parse_struct( code(
		struct ArrayHeader
		{
			gen_AllocatorInfo Allocator;
			gen_uw            Capacity;
			gen_uw            Num;
		};
	));

	Code grow_formula = untyped_str( txt_StrC( "#define gen_array_grow_formula( value ) ( 2 * value + 8 )" ));
	Code get_header = untyped_str( txt_StrC(   "#define gen_array_header( self ) ( (ArrayHeader*)( self ) - 1)" ));

	return def_global_body( args( header, grow_formula, get_header ) );
};

CodeBody gen_array( StrC type, StrC array_name )
{
	String array_fn = String::make( GlobalAllocator, array_name );
	str_to_lower(array_fn.Data);

	CodeBody result = parse_global_body( token_fmt( "array_type", (StrC)array_name, "array_fn", (StrC)array_fn, "type", (StrC)type
	, stringize(
		typedef <type>* <array_type>;

		<array_type> <array_fn>_make        ( gen_AllocatorInfo allocator );
		<array_type> <array_fn>_make_reserve( gen_AllocatorInfo allocator, gen_uw capacity );
		bool <array_fn>_append      ( <array_type> self, <type> value );
		bool <array_fn>_append_items( <array_type> self, <type>* items, gen_uw item_num );
		bool <array_fn>_append_at   ( <array_type> self, Type item, sw idx );
		bool <array_fn>_append_at   ( <array_type> self, <type>* items, gen_uw item_num )
		bool <array_fn>_grow        ( <array_type>* self, gen_uw min_capacity );
		bool <array_fn>_set_capacity( <array_type>* self, gen_uw new_capacity );

		<array_type> <array_fn>_make( gen_AllocatorInfo allocator )
		{
			return <array_fn>_make_reserve( allocator, gen_array_grow_formula( 0 ) );
		}

		<array_type> <array_fn>_make_reserve( gen_AllocatorInfo allocator, gen_uw capacity )
		{
			ArrayHeader* header = (ArrayHeader*) gen_alloc( allocator, sizeof(ArrayHeader) + sizeof(<type>) * capacity );

			if ( header == NULL )
				return NULL;

			header->Allocator = allocator;
			header->Capacity  = capacity;
			header->Num       = 0;

			return cast( <type>*, header + 1 );
		}

		bool <array_fn>_append( <array_type> self, <type> value )
		{
			ArrayHeader* header = gen_array_header( self );

			if ( header->Num == header->Capacity )
			{
				if ( ! <array_fn>_grow( & self, header->Capacity))
					return false;

				header = gen_array_header( self );
			}

			self[ header->Num ] = value;
			header->Num++;
		}

		bool <array_fn>_append_items( <array_type> self, <type>* items, gen_uw item_num )
		{
			ArrayHeader* header = gen_array_header( self );

			if ( header->Num + item_num > header->Capacity )
			{
				if ( ! <array_fn>_grow( & self, header->Capacity + item_num ))
					return false;

				header = gen_array_header( self );
			}

			gen_mem_copy( self + header->Num, items, sizeof(<type>) * item_num );
			header->Num += item_num;

			return true;
		}

		bool <array_fn>_append_at( <array_type> self, <type>* item, gen_uw idx )
		{
			ArrayHeader* header = gen_array_header( self );

			if ( idx >= header->Num )
				idx = header->Num - 1;

			if ( idx < 0 )
				idx = 0;

			if ( header->Capacity < header->Num + 1 )
			{
				if ( ! <array_fn>_grow( & self, header->Capacity + 1 ) )
					return false;

				header = gen_array_header( self );
			}

			<array_type>* target = self + idx;

			gen_mem_move( target + 1, target, (header->Num - idx) * sizeof(<array_type>) );
			header->Num++;

			return true;
		}

		bool <array_fn>_append_at( <array_type> self, <type>* items, gen_uw item_num )
		{

		}

		bool <array_fn>_grow( <array_type>* self, gen_uw min_capacity )
		{
			ArrayHeader* header       = gen_array_header( *self );
			gen_uw       new_capacity = gen_array_grow_formula( header->Capacity );

			if ( new_capacity < min_capacity )
				new_capacity = min_capacity;

			return <array_fn>_set_capacity( self, new_capacity );
		}

		bool <array_fn>_set_capacity( <array_type>* self, gen_uw new_capacity )
		{
			ArrayHeader* header = gen_array_header( *self );

			if ( new_capacity == header->Capacity )
				return true;

			if ( new_capacity < header->Num )
				header->Num = new_capacity;

			gen_uw       size       = sizeof( ArrayHeader ) + sizeof( <type> ) * new_capacity;
			ArrayHeader* new_header = cast( ArrayHeader*, gen_alloc( header->Allocator, size ));

			if ( new_header == NULL )
				return false;

			gen_mem_move( new_header, & header, sizeof( ArrayHeader ) + sizeof( <type> ) * header->Num );
			gen_free( header->Allocator, & header );

			new_header->Capacity = new_capacity;
			* self = cast( <type>*, new_header + 1 );
			return true;
		}
	)));

	return result;
};
