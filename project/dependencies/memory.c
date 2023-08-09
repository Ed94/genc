#pragma region Memory

void gen_mem_swap( void* i, void* j, gen_sw size )
{
	if ( i == j )
		return;

	if ( size == 4 )
	{
		gen_swap( gen_u32, *zpl_cast( gen_u32* ) i, *zpl_cast( gen_u32* ) j );
	}
	else if ( size == 8 )
	{
		gen_swap( gen_u64, *zpl_cast( gen_u64* ) i, *zpl_cast( gen_u64* ) j );
	}
	else if ( size < 8 )
	{
		gen_u8* a = zpl_cast( gen_u8* ) i;
		gen_u8* b = zpl_cast( gen_u8* ) j;
		if ( a != b )
		{
			while ( size-- )
			{
				gen_swap( gen_u8, *a++, *b++ );
			}
		}
	}
	else
	{
		char buffer[ 256 ];

		while ( size > gen_size_of( buffer ) )
		{
			gen_mem_swap( i, j, gen_size_of( buffer ) );
			i     = gen_pointer_add( i, gen_size_of( buffer ) );
			j     = gen_pointer_add( j, gen_size_of( buffer ) );
			size -= gen_size_of( buffer );
		}

		gen_mem_copy( buffer, i, size );
		gen_mem_copy( i, j, size );
		gen_mem_copy( j, buffer, size );
	}
}

void const* gen_mem_find( void const* data, gen_u8 c, gen_sw n )
{
	gen_u8 const* s = zpl_cast( gen_u8 const* ) data;
	while ( ( zpl_cast( gen_uptr ) s & ( sizeof( gen_uw ) - 1 ) ) && n && *s != c )
	{
		s++;
		n--;
	}
	if ( n && *s != c )
	{
		gen_sw const* w;
		gen_sw        k = GEN__ONES * c;
		w               = zpl_cast( gen_sw const* ) s;
		while ( n >= gen_size_of( gen_sw ) && ! GEN__HAS_ZERO( *w ^ k ) )
		{
			w++;
			n -= gen_size_of( gen_sw );
		}
		s = zpl_cast( gen_u8 const* ) w;
		while ( n && *s != c )
		{
			s++;
			n--;
		}
	}

	return n ? zpl_cast( void const* ) s : NULL;
}

void* gen_mem_copy( void* dest, void const* source, gen_sw n )
{
	if ( dest == NULL )
	{
		return NULL;
	}

	return memcpy( dest, source, n );

	// TODO: Re-work the whole method
	return dest;
}

#define GEN_HEAP_STATS_MAGIC 0xDEADC0DE

typedef struct gen__heap_stats
{
	gen_u32 magic;
	gen_sw  used_memory;
	gen_sw  alloc_count;
} gen__heap_stats;

global gen__heap_stats gen__heap_stats_info;

void gen_heap_stats_init( void )
{
	gen_zero_item( &gen__heap_stats_info );
	gen__heap_stats_info.magic = GEN_HEAP_STATS_MAGIC;
}

gen_sw gen_heap_stats_used_memory( void )
{
	GEN_ASSERT_MSG( gen__heap_stats_info.magic == GEN_HEAP_STATS_MAGIC, "gen_heap_stats is not initialised yet, call gen_heap_stats_init first!" );
	return gen__heap_stats_info.used_memory;
}

gen_sw gen_heap_stats_alloc_count( void )
{
	GEN_ASSERT_MSG( gen__heap_stats_info.magic == GEN_HEAP_STATS_MAGIC, "gen_heap_stats is not initialised yet, call gen_heap_stats_init first!" );
	return gen__heap_stats_info.alloc_count;
}

void gen_heap_stats_check( void )
{
	GEN_ASSERT_MSG( gen__heap_stats_info.magic == GEN_HEAP_STATS_MAGIC, "gen_heap_stats is not initialised yet, call gen_heap_stats_init first!" );
	GEN_ASSERT( gen__heap_stats_info.used_memory == 0 );
	GEN_ASSERT( gen__heap_stats_info.alloc_count == 0 );
}

typedef struct gen__heap_alloc_info
{
	gen_sw size;
	void*  physical_start;
} gen__heap_alloc_info;

GEN_ALLOCATOR_PROC( gen_heap_allocator_proc )
{
	void* ptr = NULL;
	gen_unused( allocator_data );
	gen_unused( old_size );
	if ( ! alignment )
		alignment = GEN_DEFAULT_MEMORY_ALIGNMENT;

#ifdef GEN_HEAP_ANALYSIS
	gen_sw alloc_info_size      = gen_size_of( gen__heap_alloc_info );
	gen_sw alloc_info_remainder = ( alloc_info_size % alignment );
	gen_sw track_size           = gen_max( alloc_info_size, alignment ) + alloc_info_remainder;
	switch ( type )
	{
		case gen_EAllocation_FREE :
			{
				if ( ! old_memory )
					break;
				gen__heap_alloc_info* alloc_info  = zpl_cast( gen__heap_alloc_info* ) old_memory - 1;
				gen__heap_stats_info.used_memory -= alloc_info->size;
				gen__heap_stats_info.alloc_count--;
				old_memory = alloc_info->physical_start;
			}
			break;
		case gen_EAllocation_ALLOC :
			{
				size += track_size;
			}
			break;
		default :
			break;
	}
#endif

	switch ( type )
	{
#if defined( GEN_COMPILER_MSVC ) || ( defined( GEN_COMPILER_GCC ) && defined( GEN_SYSTEM_WINDOWS ) ) || ( defined( GEN_COMPILER_TINYC ) && defined( GEN_SYSTEM_WINDOWS ) )
		case gen_EAllocation_ALLOC :
			ptr = _aligned_malloc( size, alignment );
			if ( flags & GEN_ALLOCATOR_FLAG_CLEAR_TO_ZERO )
				gen_zero_size( ptr, size );
			break;
		case gen_EAllocation_FREE :
			_aligned_free( old_memory );
			break;
		case gen_EAllocation_RESIZE :
			{
				gen_AllocatorInfo a = gen_heap_allocator();
				ptr                 = gen_default_resize_align( a, old_memory, old_size, size, alignment );
			}
			break;

#elif defined( GEN_SYSTEM_LINUX ) && ! defined( GEN_CPU_ARM ) && ! defined( GEN_COMPILER_TINYC )
		case gen_EAllocation_ALLOC :
			{
				ptr = aligned_alloc( alignment, ( size + alignment - 1 ) & ~( alignment - 1 ) );

				if ( flags & GEN_ALLOCATOR_FLAG_CLEAR_TO_ZERO )
				{
					gen_zero_size( ptr, size );
				}
			}
			break;

		case gen_EAllocation_FREE :
			{
				free( old_memory );
			}
			break;

		case gen_EAllocation_RESIZE :
			{
				gen_AllocatorInfo a = gen_heap_allocator();
				ptr                 = gen_default_resize_align( a, old_memory, old_size, size, alignment );
			}
			break;
#else
		case gen_EAllocation_ALLOC :
			{
				posix_memalign( &ptr, alignment, size );

				if ( flags & GEN_ALLOCATOR_FLAG_CLEAR_TO_ZERO )
				{
					gen_zero_size( ptr, size );
				}
			}
			break;

		case gen_EAllocation_FREE :
			{
				free( old_memory );
			}
			break;

		case gen_EAllocation_RESIZE :
			{
				gen_AllocatorInfo a = gen_heap_allocator();
				ptr                 = gen_default_resize_align( a, old_memory, old_size, size, alignment );
			}
			break;
#endif

		case gen_EAllocation_FREE_ALL :
			break;
	}

#ifdef GEN_HEAP_ANALYSIS
	if ( type == gen_EAllocation_ALLOC )
	{
		gen__heap_alloc_info* alloc_info = zpl_cast( gen__heap_alloc_info* )( zpl_cast( char* ) ptr + alloc_info_remainder );
		gen_zero_item( alloc_info );
		alloc_info->size                  = size - track_size;
		alloc_info->physical_start        = ptr;
		ptr                               = zpl_cast( void* )( alloc_info + 1 );
		gen__heap_stats_info.used_memory += alloc_info->size;
		gen__heap_stats_info.alloc_count++;
	}
#endif

	return ptr;
}

GEN_ALLOCATOR_PROC( gen_arena_allocator_proc )
{
	gen_Arena* arena = zpl_cast( gen_Arena* ) allocator_data;
	void*      ptr   = NULL;

	gen_unused( old_size );

	switch ( type )
	{
		case gen_EAllocation_ALLOC :
			{
				void*  end        = gen_pointer_add( arena->physical_start, arena->total_allocated );
				gen_sw total_size = gen_align_forward_i64( size, alignment );

				// NOTE: Out of memory
				if ( arena->total_allocated + total_size > zpl_cast( gen_sw ) arena->total_size )
				{
					return NULL;
				}

				ptr                     = gen_align_forward( end, alignment );
				arena->total_allocated += total_size;
				if ( flags & GEN_ALLOCATOR_FLAG_CLEAR_TO_ZERO )
					gen_zero_size( ptr, size );
			}
			break;

		case gen_EAllocation_FREE :
			// NOTE: Free all at once
			// Use Temp_Arena_Memory if you want to free a block
			break;

		case gen_EAllocation_FREE_ALL :
			arena->total_allocated = 0;
			break;

		case gen_EAllocation_RESIZE :
			{
				// TODO: Check if ptr is on top of stack and just extend
				gen_AllocatorInfo a = gen_arena_allocator( arena );
				ptr                 = gen_default_resize_align( a, old_memory, old_size, size, alignment );
			}
			break;
	}
	return ptr;
}

void gen_pool_init_align( gen_Pool* pool, gen_AllocatorInfo backing, gen_sw num_blocks, gen_sw block_size, gen_sw block_align )
{
	gen_sw    actual_block_size, pool_size, block_index;
	void *    data, *curr;
	gen_uptr* end;

	gen_zero_item( pool );

	pool->backing     = backing;
	pool->block_size  = block_size;
	pool->block_align = block_align;
	pool->num_blocks  = num_blocks;

	actual_block_size = block_size + block_align;
	pool_size         = num_blocks * actual_block_size;

	data = gen_alloc_align( backing, pool_size, block_align );

	// NOTE: Init intrusive freelist
	curr = data;
	for ( block_index = 0; block_index < num_blocks - 1; block_index++ )
	{
		gen_uptr* next = zpl_cast( gen_uptr* ) curr;
		*next          = zpl_cast( gen_uptr ) curr + actual_block_size;
		curr           = gen_pointer_add( curr, actual_block_size );
	}

	end  = zpl_cast( gen_uptr* ) curr;
	*end = zpl_cast( gen_uptr ) NULL;

	pool->physical_start = data;
	pool->free_list      = data;
}

GEN_ALLOCATOR_PROC( gen_pool_allocator_proc )
{
	gen_Pool* pool = zpl_cast( gen_Pool* ) allocator_data;
	void*     ptr  = NULL;

	gen_unused( old_size );

	switch ( type )
	{
		case gen_EAllocation_ALLOC :
			{
				gen_uptr next_free;
				GEN_ASSERT( size == pool->block_size );
				GEN_ASSERT( alignment == pool->block_align );
				GEN_ASSERT( pool->free_list != NULL );

				next_free         = *zpl_cast( gen_uptr* ) pool->free_list;
				ptr               = pool->free_list;
				pool->free_list   = zpl_cast( void* ) next_free;
				pool->total_size += pool->block_size;
				if ( flags & GEN_ALLOCATOR_FLAG_CLEAR_TO_ZERO )
					gen_zero_size( ptr, size );
			}
			break;

		case gen_EAllocation_FREE :
			{
				gen_uptr* next;
				if ( old_memory == NULL )
					return NULL;

				next              = zpl_cast( gen_uptr* ) old_memory;
				*next             = zpl_cast( gen_uptr ) pool->free_list;
				pool->free_list   = old_memory;
				pool->total_size -= pool->block_size;
			}
			break;

		case gen_EAllocation_FREE_ALL :
			{
				gen_sw    actual_block_size, block_index;
				void*     curr;
				gen_uptr* end;

				actual_block_size = pool->block_size + pool->block_align;
				pool->total_size  = 0;

				// NOTE: Init intrusive freelist
				curr = pool->physical_start;
				for ( block_index = 0; block_index < pool->num_blocks - 1; block_index++ )
				{
					gen_uptr* next = zpl_cast( gen_uptr* ) curr;
					*next          = zpl_cast( gen_uptr ) curr + actual_block_size;
					curr           = gen_pointer_add( curr, actual_block_size );
				}

				end             = zpl_cast( gen_uptr* ) curr;
				*end            = zpl_cast( gen_uptr ) NULL;
				pool->free_list = pool->physical_start;
			}
			break;

		case gen_EAllocation_RESIZE :
			// NOTE: Cannot resize
			GEN_PANIC( "You cannot resize something allocated by with a pool." );
			break;
	}

	return ptr;
}

#pragma endregion Memory

