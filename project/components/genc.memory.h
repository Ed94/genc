#pragma region Memory
//! Checks if value is power of 2.
GEN_DEF_INLINE gen_b32 gen_is_power_of_two( gen_sw x );

//! Aligns address to specified alignment.
GEN_DEF_INLINE void* gen_align_forward( void* ptr, gen_sw alignment );

//! Aligns value to a specified alignment.
GEN_DEF_INLINE gen_s64 gen_align_forward_i64( gen_s64 value, gen_sw alignment );

//! Aligns value to a specified alignment.
GEN_DEF_INLINE gen_u64 gen_align_forward_u64( gen_u64 value, gen_uw alignment );

//! Moves pointer forward by bytes.
GEN_DEF_INLINE void* gen_pointer_add( void* ptr, gen_sw bytes );

//! Moves pointer backward by bytes.
GEN_DEF_INLINE void* gen_pointer_sub( void* ptr, gen_sw bytes );

//! Moves pointer forward by bytes.
GEN_DEF_INLINE void const* gen_pointer_add_const( void const* ptr, gen_sw bytes );

//! Moves pointer backward by bytes.
GEN_DEF_INLINE void const* gen_pointer_sub_const( void const* ptr, gen_sw bytes );

//! Calculates difference between two addresses.
GEN_DEF_INLINE gen_sw gen_pointer_diff( void const* begin, void const* end );

#define gen_ptr_add       gen_pointer_add
#define gen_ptr_sub       gen_pointer_sub
#define gen_ptr_add_const gen_pointer_add_const
#define gen_ptr_sub_const gen_pointer_sub_const
#define gen_ptr_diff      gen_pointer_diff

//! Clears up memory at location by specified size.

//! @param ptr Memory location to clear up.
//! @param size The size to clear up with.
GEN_DEF_INLINE void gen_zero_size( void* ptr, gen_sw size );

#ifndef gen_zero_item
//! Clears up an item.
#	define gen_zero_item( t ) gen_zero_size( ( t ), size_of( *( t ) ) )    // NOTE: Pass pointer of struct

//! Clears up an array.
#	define gen_zero_array( a, count ) gen_zero_size( ( a ), size_of( *( a ) ) * count )
#endif

//! Copy memory from source to destination.
GEN_DEF_INLINE void* gen_mem_move( void* dest, void const* source, gen_sw size );

//! Set constant value at memory location with specified size.
GEN_DEF_INLINE void* gen_mem_set( void* data, gen_u8 byte_value, gen_sw size );

//! Compare two memory locations with specified size.
GEN_DEF_INLINE gen_s32 gen_mem_compare( void const* s1, void const* s2, gen_sw size );

//! Swap memory contents between 2 locations with size.
void gen_mem_swap( void* i, void* j, gen_sw size );

//! Search for a constant value within the size limit at memory location.
void const* gen_mem_find( void const* data, gen_u8 byte_value, gen_sw size );

//! Search for a constant value within the size limit at memory location in backwards.
void const* gen_memrchr( void const* data, gen_u8 byte_value, gen_sw size );

//! Copy non-overlapping memory from source to destination.
void* gen_mem_copy( void* dest, void const* source, gen_sw size );

#ifndef gen_memcopy_array

//! Copy non-overlapping array.
#	define gen_memcopy_array( dst, src, count ) gen_mem_copy( ( dst ), ( src ), size_of( *( dst ) ) * ( count ) )
#endif

//! Copy an array.
#ifndef gen_memmove_array
#	define gen_memmove_array( dst, src, count ) gen_mem_move( ( dst ), ( src ), size_of( *( dst ) ) * ( count ) )
#endif

#ifndef GEN_BIT_CAST
#	define GEN_BIT_CAST( dest, source )                                                                                            \
		do                                                                                                                          \
		{                                                                                                                           \
			GEN_STATIC_ASSERT( size_of( *( dest ) ) <= size_of( source ), "size_of(*(dest)) !<= size_of(source)" ); \
			gen_mem_copy( ( dest ), &( source ), size_of( *dest ) );                                                  \
		} while ( 0 )
#endif

#ifndef gen_kilobytes
#	define gen_kilobytes( x ) ( ( x ) * ( gen_s64 )( 1024 ) )
#	define gen_megabytes( x ) ( gen_kilobytes( x ) * ( gen_s64 )( 1024 ) )
#	define gen_gigabytes( x ) ( gen_megabytes( x ) * ( gen_s64 )( 1024 ) )
#	define gen_terabytes( x ) ( gen_gigabytes( x ) * ( gen_s64 )( 1024 ) )
#endif


/* inlines */

#define GEN__ONES          ( zpl_cast( gen_uw ) - 1 / GEN_U8_MAX )
#define GEN__HIGHS         ( GEN__ONES * ( GEN_U8_MAX / 2 + 1 ) )
#define GEN__HAS_ZERO( x ) ( ( ( x )-GEN__ONES ) & ~( x )&GEN__HIGHS )

GEN_IMPL_INLINE void* gen_align_forward( void* ptr, gen_sw alignment )
{
	gen_uptr p;

	GEN_ASSERT( gen_is_power_of_two( alignment ) );

	p = zpl_cast( gen_uptr ) ptr;
	return zpl_cast( void* )( ( p + ( alignment - 1 ) ) & ~( alignment - 1 ) );
}

GEN_IMPL_INLINE gen_s64 gen_align_forward_i64( gen_s64 value, gen_sw alignment )
{
	return value + ( alignment - value % alignment ) % alignment;
}

GEN_IMPL_INLINE gen_u64 gen_align_forward_u64( gen_u64 value, gen_uw alignment )
{
	return value + ( alignment - value % alignment ) % alignment;
}

GEN_IMPL_INLINE void* gen_pointer_add( void* ptr, gen_sw bytes )
{
	return zpl_cast( void* )( zpl_cast( gen_u8* ) ptr + bytes );
}

GEN_IMPL_INLINE void* gen_pointer_sub( void* ptr, gen_sw bytes )
{
	return zpl_cast( void* )( zpl_cast( gen_u8* ) ptr - bytes );
}

GEN_IMPL_INLINE void const* gen_pointer_add_const( void const* ptr, gen_sw bytes )
{
	return zpl_cast( void const* )( zpl_cast( gen_u8 const* ) ptr + bytes );
}

GEN_IMPL_INLINE void const* gen_pointer_sub_const( void const* ptr, gen_sw bytes )
{
	return zpl_cast( void const* )( zpl_cast( gen_u8 const* ) ptr - bytes );
}

GEN_IMPL_INLINE gen_sw gen_pointer_diff( void const* begin, void const* end )
{
	return zpl_cast( gen_sw )( zpl_cast( gen_u8 const* ) end - zpl_cast( gen_u8 const* ) begin );
}

GEN_IMPL_INLINE void gen_zero_size( void* ptr, gen_sw size )
{
	gen_mem_set( ptr, 0, size );
}

#if defined( _MSC_VER ) && ! defined( __clang__ )
#	pragma intrinsic( __movsb )
#endif

GEN_IMPL_INLINE void* gen_mem_move( void* dest, void const* source, gen_sw n )
{
	if ( dest == NULL )
	{
		return NULL;
	}

	gen_u8*       d = zpl_cast( gen_u8* ) dest;
	gen_u8 const* s = zpl_cast( gen_u8 const* ) source;

	if ( d == s )
		return d;
	if ( s + n <= d || d + n <= s )    // NOTE: Non-overlapping
		return gen_mem_copy( d, s, n );

	if ( d < s )
	{
		if ( zpl_cast( gen_uptr ) s % size_of( gen_sw ) == zpl_cast( gen_uptr ) d % size_of( gen_sw ) )
		{
			while ( zpl_cast( gen_uptr ) d % size_of( gen_sw ) )
			{
				if ( ! n-- )
					return dest;
				*d++ = *s++;
			}
			while ( n >= size_of( gen_sw ) )
			{
				*zpl_cast( gen_sw* ) d  = *zpl_cast( gen_sw* ) s;
				n                      -= size_of( gen_sw );
				d                      += size_of( gen_sw );
				s                      += size_of( gen_sw );
			}
		}
		for ( ; n; n-- )
			*d++ = *s++;
	}
	else
	{
		if ( ( zpl_cast( gen_uptr ) s % size_of( gen_sw ) ) == ( zpl_cast( gen_uptr ) d % size_of( gen_sw ) ) )
		{
			while ( zpl_cast( gen_uptr )( d + n ) % size_of( gen_sw ) )
			{
				if ( ! n-- )
					return dest;
				d[ n ] = s[ n ];
			}
			while ( n >= size_of( gen_sw ) )
			{
				n                             -= size_of( gen_sw );
				*zpl_cast( gen_sw* )( d + n )  = *zpl_cast( gen_sw* )( s + n );
			}
		}
		while ( n )
			n--, d[ n ] = s[ n ];
	}

	return dest;
}

GEN_IMPL_INLINE void* gen_mem_set( void* dest, gen_u8 c, gen_sw n )
{
	if ( dest == NULL )
	{
		return NULL;
	}

	gen_u8* s = zpl_cast( gen_u8* ) dest;
	gen_sw  k;
	gen_u32 c32 = ( ( gen_u32 )-1 ) / 255 * c;

	if ( n == 0 )
		return dest;
	s[ 0 ] = s[ n - 1 ] = c;
	if ( n < 3 )
		return dest;
	s[ 1 ] = s[ n - 2 ] = c;
	s[ 2 ] = s[ n - 3 ] = c;
	if ( n < 7 )
		return dest;
	s[ 3 ] = s[ n - 4 ] = c;
	if ( n < 9 )
		return dest;

	k  = -zpl_cast( gen_sptr ) s & 3;
	s += k;
	n -= k;
	n &= -4;

	*zpl_cast( gen_u32* )( s + 0 )     = c32;
	*zpl_cast( gen_u32* )( s + n - 4 ) = c32;
	if ( n < 9 )
		return dest;
	*zpl_cast( gen_u32* )( s + 4 )      = c32;
	*zpl_cast( gen_u32* )( s + 8 )      = c32;
	*zpl_cast( gen_u32* )( s + n - 12 ) = c32;
	*zpl_cast( gen_u32* )( s + n - 8 )  = c32;
	if ( n < 25 )
		return dest;
	*zpl_cast( gen_u32* )( s + 12 )     = c32;
	*zpl_cast( gen_u32* )( s + 16 )     = c32;
	*zpl_cast( gen_u32* )( s + 20 )     = c32;
	*zpl_cast( gen_u32* )( s + 24 )     = c32;
	*zpl_cast( gen_u32* )( s + n - 28 ) = c32;
	*zpl_cast( gen_u32* )( s + n - 24 ) = c32;
	*zpl_cast( gen_u32* )( s + n - 20 ) = c32;
	*zpl_cast( gen_u32* )( s + n - 16 ) = c32;

	k  = 24 + ( zpl_cast( gen_uptr ) s & 4 );
	s += k;
	n -= k;

	{
		gen_u64 c64 = ( zpl_cast( gen_u64 ) c32 << 32 ) | c32;
		while ( n > 31 )
		{
			*zpl_cast( gen_u64* )( s + 0 )  = c64;
			*zpl_cast( gen_u64* )( s + 8 )  = c64;
			*zpl_cast( gen_u64* )( s + 16 ) = c64;
			*zpl_cast( gen_u64* )( s + 24 ) = c64;

			n -= 32;
			s += 32;
		}
	}

	return dest;
}

GEN_IMPL_INLINE gen_s32 gen_mem_compare( void const* s1, void const* s2, gen_sw size )
{
	gen_u8 const* s1p8 = zpl_cast( gen_u8 const* ) s1;
	gen_u8 const* s2p8 = zpl_cast( gen_u8 const* ) s2;

	if ( s1 == NULL || s2 == NULL )
	{
		return 0;
	}

	while ( size-- )
	{
		gen_sw d;
		if ( ( d = ( *s1p8++ - *s2p8++ ) ) != 0 )
			return zpl_cast( gen_s32 ) d;
	}
	return 0;
}

GEN_IMPL_INLINE gen_b32 gen_is_power_of_two( gen_sw x )
{
	if ( x <= 0 )
		return false;
	return ! ( x & ( x - 1 ) );
}

typedef enum gen_alloc_type {
    GEN_ALLOCATION_ALLOC,
    GEN_ALLOCATION_FREE,
    GEN_ALLOCATION_FREE_ALL,
    GEN_ALLOCATION_RESIZE,
} gen_alloc_type;

// NOTE: This is useful so you can define an allocator of the same type and parameters
#define GEN_ALLOCATOR_PROC(name)                                                                                                     \
void *name(void *allocator_data, gen_alloc_type type, gen_sw size, gen_sw alignment, void *old_memory, \
gen_sw old_size, gen_u64 flags)
typedef GEN_ALLOCATOR_PROC(gen_allocator_proc);


typedef struct gen_allocator {
    gen_allocator_proc *proc;
    void *data;
} gen_allocator;

typedef enum gen_alloc_flag {
    GEN_ALLOCATOR_FLAG_CLEAR_TO_ZERO = bit(0),
} gen_alloc_flag;

#ifndef GEN_DEFAULT_MEMORY_ALIGNMENT
#define GEN_DEFAULT_MEMORY_ALIGNMENT (2 * size_of(void *))
#endif

#ifndef GEN_DEFAULT_ALLOCATOR_FLAGS
#define GEN_DEFAULT_ALLOCATOR_FLAGS (GEN_ALLOCATOR_FLAG_CLEAR_TO_ZERO)
#endif

//! Allocate memory with specified alignment.
GEN_DEF_INLINE void *gen_alloc_align(gen_allocator a, gen_sw size, gen_sw alignment);

//! Allocate memory with default alignment.
GEN_DEF_INLINE void *gen_alloc(gen_allocator a, gen_sw size);

//! Free allocated memory.
GEN_DEF_INLINE void gen_free(gen_allocator a, void *ptr);

//! Free all memory allocated by an allocator.
GEN_DEF_INLINE void gen_free_all(gen_allocator a);

//! Resize an allocated memory.
GEN_DEF_INLINE void *gen_resize(gen_allocator a, void *ptr, gen_sw old_size, gen_sw new_size);

//! Resize an allocated memory with specified alignment.
GEN_DEF_INLINE void *gen_resize_align(gen_allocator a, void *ptr, gen_sw old_size, gen_sw new_size, gen_sw alignment);

//! Allocate memory and copy data into it.
GEN_DEF_INLINE void *gen_alloc_copy(gen_allocator a, void const *src, gen_sw size);

//! Allocate memory with specified alignment and copy data into it.
GEN_DEF_INLINE void *gen_alloc_copy_align(gen_allocator a, void const *src, gen_sw size, gen_sw alignment);

//! Allocate memory for null-terminated C-String.
char *gen_alloc_str(gen_allocator a, char const *str);

//! Allocate memory for C-String with specified size.
GEN_DEF_INLINE char *gen_alloc_str_len(gen_allocator a, char const *str, gen_sw len);

#ifndef gen_alloc_item

//! Allocate memory for an item.
#define gen_alloc_item(allocator_, Type) (Type *)gen_alloc(allocator_, gen_size_of(Type))

//! Allocate memory for an array of items.
#define gen_alloc_array(allocator_, Type, count) (Type *)gen_alloc(allocator_, gen_size_of(Type) * (count))
#endif

/* heap memory analysis tools */
/* define GEN_HEAP_ANALYSIS to enable this feature */
/* call gen_heap_stats_init at the beginning of the entry point */
/* you can call gen_heap_stats_check near the end of the execution to validate any possible leaks */
void gen_heap_stats_init(void);
gen_sw gen_heap_stats_used_memory(void);
gen_sw gen_heap_stats_alloc_count(void);
void gen_heap_stats_check(void);

//! Allocate/Resize memory using default options.

//! Use this if you don't need a "fancy" resize allocation
GEN_DEF_INLINE void *gen_default_resize_align(gen_allocator a, void *ptr, gen_sw old_size, gen_sw new_size, gen_sw alignment);

//! The heap allocator backed by operating system's memory manager.
GEN_DEF_INLINE gen_allocator gen_heap_allocator(void);
GEN_ALLOCATOR_PROC(gen_heap_allocator_proc);

#ifndef gen_malloc

//! Helper to allocate memory using heap allocator.
#define gen_malloc(sz) GEN_NS(gen_alloc)(GEN_NS(gen_heap_allocator)( ), sz)

//! Helper to free memory allocated by heap allocator.
#define gen_mfree(ptr) GEN_NS(gen_free)(GEN_NS(gen_heap_allocator)( ), ptr)

//! Alias to heap allocator.
#define gen_heap GEN_NS(gen_heap_allocator)
#endif

//
// Arena Allocator
//

typedef struct gen_arena {
    gen_allocator backing;
    void *physical_start;
    gen_sw total_size;
    gen_sw total_allocated;
    gen_sw temp_count;
} gen_arena;

//! Initialize memory arena from existing memory region.
GEN_DEF_INLINE void gen_arena_init_from_memory(gen_arena *arena, void *start, gen_sw size);

//! Initialize memory arena using existing memory allocator.
GEN_DEF_INLINE void gen_arena_init_from_allocator(gen_arena *arena, gen_allocator backing, gen_sw size);

//! Initialize memory arena within an existing parent memory arena.
GEN_DEF_INLINE void gen_arena_init_sub(gen_arena *arena, gen_arena *parent_arena, gen_sw size);

//! Release the memory used by memory arena.
GEN_DEF_INLINE void gen_arena_free(gen_arena *arena);


//! Retrieve memory arena's aligned allocation address.
GEN_DEF_INLINE gen_sw gen_arena_alignment_of(gen_arena *arena, gen_sw alignment);

//! Retrieve memory arena's remaining size.
GEN_DEF_INLINE gen_sw gen_arena_size_remaining(gen_arena *arena, gen_sw alignment);

//! Check whether memory arena has any temporary snapshots.
GEN_DEF_INLINE void gen_arena_check(gen_arena *arena);

//! Allocation Types: alloc, free_all, resize
GEN_DEF_INLINE gen_allocator gen_arena_allocator(gen_arena *arena);
GEN_ALLOCATOR_PROC(gen_arena_allocator_proc);


typedef struct gen_arena_snapshot {
    gen_arena *arena;
    gen_sw original_count;
} gen_arena_snapshot;

//! Capture a snapshot of used memory in a memory arena.
GEN_DEF_INLINE gen_arena_snapshot gen_arena_snapshot_begin(gen_arena *arena);

//! Reset memory arena's usage by a captured snapshot.
GEN_DEF_INLINE void gen_arena_snapshot_end(gen_arena_snapshot tmp_mem);

//
// Pool Allocator
//


typedef struct gen_pool {
    gen_allocator backing;
    void *physical_start;
    void *free_list;
    gen_sw block_size;
    gen_sw block_align;
    gen_sw total_size;
    gen_sw num_blocks;
} gen_pool;


//! Initialize pool allocator.
GEN_DEF_INLINE void gen_pool_init(gen_pool *pool, gen_allocator backing, gen_sw num_blocks, gen_sw block_size);

//! Initialize pool allocator with specific block alignment.
void gen_pool_init_align(gen_pool *pool, gen_allocator backing, gen_sw num_blocks, gen_sw block_size,
                                 gen_sw block_align);

//! Release the resources used by pool allocator.
GEN_DEF_INLINE void gen_pool_free(gen_pool *pool);

//! Allocation Types: alloc, free
GEN_DEF_INLINE gen_allocator gen_pool_allocator(gen_pool *pool);
GEN_ALLOCATOR_PROC(gen_pool_allocator_proc);


/* inlines */

GEN_IMPL_INLINE void *gen_alloc_align(gen_allocator a, gen_sw size, gen_sw alignment) {
    return a.proc(a.data, GEN_ALLOCATION_ALLOC, size, alignment, NULL, 0, GEN_DEFAULT_ALLOCATOR_FLAGS);
}
GEN_IMPL_INLINE void *gen_alloc(gen_allocator a, gen_sw size) {
    return gen_alloc_align(a, size, GEN_DEFAULT_MEMORY_ALIGNMENT);
}
GEN_IMPL_INLINE void gen_free(gen_allocator a, void *ptr) {
    if (ptr != NULL) a.proc(a.data, GEN_ALLOCATION_FREE, 0, 0, ptr, 0, GEN_DEFAULT_ALLOCATOR_FLAGS);
}
GEN_IMPL_INLINE void gen_free_all(gen_allocator a) {
    a.proc(a.data, GEN_ALLOCATION_FREE_ALL, 0, 0, NULL, 0, GEN_DEFAULT_ALLOCATOR_FLAGS);
}
GEN_IMPL_INLINE void *gen_resize(gen_allocator a, void *ptr, gen_sw old_size, gen_sw new_size) {
    return gen_resize_align(a, ptr, old_size, new_size, GEN_DEFAULT_MEMORY_ALIGNMENT);
}
GEN_IMPL_INLINE void *gen_resize_align(gen_allocator a, void *ptr, gen_sw old_size, gen_sw new_size, gen_sw alignment) {
    return a.proc(a.data, GEN_ALLOCATION_RESIZE, new_size, alignment, ptr, old_size, GEN_DEFAULT_ALLOCATOR_FLAGS);
}

GEN_IMPL_INLINE void *gen_alloc_copy(gen_allocator a, void const *src, gen_sw size) {
    return gen_mem_copy(gen_alloc(a, size), src, size);
}
GEN_IMPL_INLINE void *gen_alloc_copy_align(gen_allocator a, void const *src, gen_sw size, gen_sw alignment) {
    return gen_mem_copy(gen_alloc_align(a, size, alignment), src, size);
}

GEN_IMPL_INLINE char *gen_alloc_str_len(gen_allocator a, char const *str, gen_sw len) {
    char *result;
    result = zpl_cast(char *) gen_alloc(a, len + 1);
    gen_mem_move(result, str, len);
    result[len] = '\0';
    return result;
}

GEN_IMPL_INLINE void *gen_default_resize_align(gen_allocator a, void *old_memory, gen_sw old_size, gen_sw new_size,
                                               gen_sw alignment) {
    if (!old_memory) return gen_alloc_align(a, new_size, alignment);

    if (new_size == 0) {
        gen_free(a, old_memory);
        return NULL;
    }

    if (new_size < old_size) new_size = old_size;

    if (old_size == new_size) {
        return old_memory;
    } else {
        void *new_memory = gen_alloc_align(a, new_size, alignment);
        if (!new_memory) return NULL;
        gen_mem_move(new_memory, old_memory, min(new_size, old_size));
        gen_free(a, old_memory);
        return new_memory;
    }
}

//
// Heap Allocator
//

GEN_IMPL_INLINE gen_allocator gen_heap_allocator(void) {
    gen_allocator a;
    a.proc = gen_heap_allocator_proc;
    a.data = NULL;
    return a;
}

//
// Arena Allocator
//

GEN_IMPL_INLINE void gen_arena_init_from_memory(gen_arena *arena, void *start, gen_sw size) {
    arena->backing.proc = NULL;
    arena->backing.data = NULL;
    arena->physical_start = start;
    arena->total_size = size;
    arena->total_allocated = 0;
    arena->temp_count = 0;
}

GEN_IMPL_INLINE void gen_arena_init_from_allocator(gen_arena *arena, gen_allocator backing, gen_sw size) {
    arena->backing = backing;
    arena->physical_start = gen_alloc(backing, size); // NOTE: Uses default alignment
    arena->total_size = size;
    arena->total_allocated = 0;
    arena->temp_count = 0;
}

GEN_IMPL_INLINE void gen_arena_init_sub(gen_arena *arena, gen_arena *parent_arena, gen_sw size) {
    gen_arena_init_from_allocator(arena, gen_arena_allocator(parent_arena), size);
}

GEN_IMPL_INLINE void gen_arena_free(gen_arena *arena) {
    if (arena->backing.proc) {
        gen_free(arena->backing, arena->physical_start);
        arena->physical_start = NULL;
    }
}

GEN_IMPL_INLINE gen_sw gen_arena_alignment_of(gen_arena *arena, gen_sw alignment) {
    gen_sw alignment_offset, result_pointer, mask;
    GEN_ASSERT(gen_is_power_of_two(alignment));

    alignment_offset = 0;
    result_pointer = zpl_cast(gen_sw) arena->physical_start + arena->total_allocated;
    mask = alignment - 1;
    if (result_pointer & mask) alignment_offset = alignment - (result_pointer & mask);

    return alignment_offset;
}

GEN_IMPL_INLINE gen_sw gen_arena_size_remaining(gen_arena *arena, gen_sw alignment) {
    gen_sw result = arena->total_size - (arena->total_allocated + gen_arena_alignment_of(arena, alignment));
    return result;
}

GEN_IMPL_INLINE void gen_arena_check(gen_arena *arena) { GEN_ASSERT(arena->temp_count == 0); }

GEN_IMPL_INLINE gen_allocator gen_arena_allocator(gen_arena *arena) {
    gen_allocator allocator;
    allocator.proc = gen_arena_allocator_proc;
    allocator.data = arena;
    return allocator;
}

GEN_IMPL_INLINE gen_arena_snapshot gen_arena_snapshot_begin(gen_arena *arena) {
    gen_arena_snapshot tmp;
    tmp.arena = arena;
    tmp.original_count = arena->total_allocated;
    arena->temp_count++;
    return tmp;
}

GEN_IMPL_INLINE void gen_arena_snapshot_end(gen_arena_snapshot tmp) {
    GEN_ASSERT(tmp.arena->total_allocated >= tmp.original_count);
    GEN_ASSERT(tmp.arena->temp_count > 0);
    tmp.arena->total_allocated = tmp.original_count;
    tmp.arena->temp_count--;
}

//
// Pool Allocator
//

GEN_IMPL_INLINE void gen_pool_init(gen_pool *pool, gen_allocator backing, gen_sw num_blocks, gen_sw block_size) {
    gen_pool_init_align(pool, backing, num_blocks, block_size, GEN_DEFAULT_MEMORY_ALIGNMENT);
}

GEN_IMPL_INLINE void gen_pool_free(gen_pool *pool) {
    if (pool->backing.proc) { gen_free(pool->backing, pool->physical_start); }
}

GEN_IMPL_INLINE gen_allocator gen_pool_allocator(gen_pool *pool) {
    gen_allocator allocator;
    allocator.proc = gen_pool_allocator_proc;
    allocator.data = pool;
    return allocator;
}

#pragma endregion Memory
