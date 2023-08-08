#pragma once

#define GEN_DEFINE_LIBRARY_CODE_CONSTANTS
#define GEN_ENFORCE_STRONG_CODE_TYPES
#define GEN_EXPOSE_BACKEND
#include "gen/gen.hpp"

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
	String array_type = String::fmt_buf( GlobalAllocator, "gen_%.*s", array_name.Len, array_name.Ptr );
	String fn         = String::fmt_buf( GlobalAllocator, "gen_%.*s", array_name.Len, array_name.Ptr );
	str_to_lower(fn.Data);

#pragma push_macro( "GEN_ASSERT" )
#undef GEN_ASSERT
	CodeBody result = parse_global_body( token_fmt( "array_type", (StrC)array_type, "fn", (StrC)fn, "type", (StrC)type
	, stringize(
		typedef <type>* <array_type>;

		<array_type> <fn>_make        ( gen_AllocatorInfo allocator );
		<array_type> <fn>_make_reserve( gen_AllocatorInfo allocator, gen_uw capacity );
		bool         <fn>_append      ( <array_type>*  self, <type> value );
		bool         <fn>_append_items( <array_type>*  self, <type>* items, gen_uw item_num );
		bool         <fn>_append_at   ( <array_type>*  self, <type> item, gen_uw idx );
		bool         <fn>_append_at   ( <array_type>*  self, <type>* items, gen_uw item_num, gen_uw idx );
		<type>*      <fn>_back        ( <array_type>  self );
		void         <fn>_clear       ( <array_type>  self );
		bool         <fn>_fill		  ( <array_type>  self, gen_uw begin, gen_uw end, <type> value );
		void         <fn>_free        ( <array_type>  self );
		bool         <fn>_grow        ( <array_type>* self, gen_uw min_capacity );
		<type>       <fn>_pop 	      ( <array_type>  self );
		bool         <fn>_reserve     ( <array_type>* self, gen_uw new_capacity );
		bool         <fn>_resize      ( <array_type>* self, gen_uw num );
		bool         <fn>_set_capacity( <array_type>* self, gen_uw new_capacity );

		<array_type> <fn>_make( gen_AllocatorInfo allocator )
		{
			return <fn>_make_reserve( allocator, gen_array_grow_formula( 0 ) );
		}

		<array_type> <fn>_make_reserve( gen_AllocatorInfo allocator, gen_uw capacity )
		{
			ArrayHeader* header = cast(ArrayHeader*, gen_alloc( allocator, sizeof(ArrayHeader) + sizeof(<type>) * capacity ) );

			if ( header == NULL )
				return NULL;

			header->Allocator = allocator;
			header->Capacity  = capacity;
			header->Num       = 0;

			return cast( <type>*, header + 1 );
		}

		bool <fn>_append( <array_type>* self, <type> value )
		{
			ArrayHeader* header = gen_array_header( * self );

			if ( header->Num == header->Capacity )
			{
				if ( ! <fn>_grow( self, header->Capacity))
					return false;

				header = gen_array_header( * self );
			}

			(* self)[ header->Num ] = value;
			header->Num++;

			return true;
		}

		bool <fn>_append_items( <array_type>* self, <type>* items, gen_uw item_num )
		{
			ArrayHeader* header = gen_array_header( * self );

			if ( header->Num + item_num > header->Capacity )
			{
				if ( ! <fn>_grow( self, header->Capacity + item_num ))
					return false;

				header = gen_array_header( * self );
			}

			gen_mem_copy( (* self) + header->Num, items, sizeof(<type>) * item_num );
			header->Num += item_num;

			return true;
		}

		bool <fn>_append_at( <array_type>* self, <type>* item, gen_uw idx )
		{
			ArrayHeader* header = gen_array_header( * self );

			if ( idx >= header->Num )
				idx = header->Num - 1;

			if ( idx < 0 )
				idx = 0;

			if ( header->Capacity < header->Num + 1 )
			{
				if ( ! <fn>_grow( self, header->Capacity + 1 ) )
					return false;

				header = gen_array_header( * self );
			}

			<array_type> target = (* self) + idx;

			gen_mem_move( target + 1, target, (header->Num - idx) * sizeof(<array_type>) );
			header->Num++;

			return true;
		}

		bool <fn>_append_items_at( <array_type>* self, <type>* items, gen_uw item_num, gen_uw idx )
		{
			ArrayHeader* header = gen_array_header( * self );

			if ( idx >= header->Num )
			{
				return <fn>_append_items( self, items, item_num );
			}

			if ( item_num > header->Capacity )
			{
				if ( ! <fn>_grow( self, item_num + header->Capacity ) )
					return false;

				header = gen_array_header( * self );
			}

			<type>* target = (* self) + idx + item_num;
			<type>* src    = (* self) + idx;

			gen_mem_move( target, src, (header->Num - idx) * sizeof(<type>) );
			gen_mem_copy( src, items, item_num * sizeof(<type>) );
			header->Num += item_num;

			return true;
		}

		<type>* <fn>_back( <array_type> self )
		{
			ArrayHeader* header = gen_array_header( self );

			if ( header->Num == 0 )
				return NULL;

			return self + header->Num - 1;
		}

		void <fn>_clear( <array_type> self )
		{
			ArrayHeader* header = gen_array_header( self );
			header->Num = 0;
		}

		bool <fn>_fill( <array_type> self, gen_uw begin, gen_uw end, <type> value )
		{
			ArrayHeader* header = gen_array_header( self );

			if ( begin < 0 || end >= header->Num )
				return false;

			for ( gen_sw idx = begin; idx < end; idx ++ )
				self[ idx ] = value;

			return true;
		}

		void <fn>_free( <array_type> self )
		{
			ArrayHeader* header = gen_array_header( self );
			gen_free( header->Allocator, header );
			self = NULL;
		}

		bool <fn>_grow( <array_type>* self, gen_uw min_capacity )
		{
			ArrayHeader* header       = gen_array_header( *self );
			gen_uw       new_capacity = gen_array_grow_formula( header->Capacity );

			if ( new_capacity < min_capacity )
				new_capacity = min_capacity;

			return <fn>_set_capacity( self, new_capacity );
		}

		<type> <fn>_pop( <array_type> self )
		{
			ArrayHeader* header = gen_array_header( self );
			GEN_ASSERT( header->Num > 0 );

			<type> result = self[ header->Num - 1 ];
			header->Num--;
			return result;
		}

		void <fn>_remove_at( <array_type> self, gen_uw idx )
		{
			ArrayHeader* header = gen_array_header( self );
			GEN_ASSERT( idx < header->Num );

			gen_mem_move( self + idx, self + idx + 1, sizeof( <type> ) * ( header->Num - idx - 1 ) );
			header->Num--;
		}

		bool <fn>_reserve( <array_type>* self, gen_uw new_capacity )
		{
			ArrayHeader* header = gen_array_header( * self );

			if ( header->Capacity < new_capacity )
				return <fn>_set_capacity( self, new_capacity );

			return true;
		}

		bool <fn>_resize( <array_type>* self, gen_uw num )
		{
			ArrayHeader* header = gen_array_header( * self );

			if ( header->Capacity < num )
			{
				if ( ! <fn>_grow( self, num ) )
					return false;

				header = gen_array_header( * self );
			}

			header->Num = num;
			return true;
		}

		bool <fn>_set_capacity( <array_type>* self, gen_uw new_capacity )
		{
			ArrayHeader* header = gen_array_header( * self );

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
#pragma pop_macro( "GEN_ASSERT" )

	return result;
};
