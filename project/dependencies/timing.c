#ifdef GEN_BENCHMARK
#if defined( GEN_COMPILER_MSVC ) && ! defined( __clang__ )
gen_u64 gen_read_cpu_time_stamp_counter( void )
{
	return __rdtsc();
}
#elif defined( __i386__ )
gen_u64 gen_read_cpu_time_stamp_counter( void )
{
	gen_u64 x;
	__asm__ volatile( ".byte 0x0f, 0x31" : "=A"( x ) );
	return x;
}
#elif defined( __x86_64__ )
gen_u64 gen_read_cpu_time_stamp_counter( void )
{
	gen_u32 hi, lo;
	__asm__ __volatile__( "rdtsc" : "=a"( lo ), "=d"( hi ) );
	return ( zpl_cast( gen_u64 ) lo ) | ( ( zpl_cast( gen_u64 ) hi ) << 32 );
}
#elif defined( __powerpc__ )
gen_u64 gen_read_cpu_time_stamp_counter( void )
{
	gen_u64 result = 0;
	gen_u32 upper, lower, tmp;
	__asm__ volatile(
	    "0:                   \n"
	    "\tmftbu   %0         \n"
	    "\tmftb    %1         \n"
	    "\tmftbu   %2         \n"
	    "\tcmpw    %2,%0      \n"
	    "\tbne     0b         \n"
	    : "=r"( upper ), "=r"( lower ), "=r"( tmp )
	);
	result = upper;
	result = result << 32;
	result = result | lower;

	return result;
}
#elif defined( GEN_SYSTEM_EMSCRIPTEN )
gen_u64 gen_read_cpu_time_stamp_counter( void )
{
	return ( gen_u64 )( emscripten_get_now() * 1e+6 );
}
#elif defined( GEN_CPU_ARM ) && ! defined( GEN_COMPILER_TINYC )
gen_u64 gen_read_cpu_time_stamp_counter( void )
{
#	if defined( __aarch64__ )
	int64_t r = 0;
	asm volatile( "mrs %0, cntvct_el0" : "=r"( r ) );
#	elif ( __ARM_ARCH >= 6 )
	uint32_t r = 0;
	uint32_t pmccntr;
	uint32_t pmuseren;
	uint32_t pmcntenset;

	// Read the user mode perf monitor counter access permissions.
	asm volatile( "mrc p15, 0, %0, c9, c14, 0" : "=r"( pmuseren ) );
	if ( pmuseren & 1 )
	{    // Allows reading perfmon counters for user mode code.
		asm volatile( "mrc p15, 0, %0, c9, c12, 1" : "=r"( pmcntenset ) );
		if ( pmcntenset & 0x80000000ul )
		{    // Is it counting?
			asm volatile( "mrc p15, 0, %0, c9, c13, 0" : "=r"( pmccntr ) );
			// The counter is set up to count every 64th cycle
			return ( ( int64_t )pmccntr ) * 64;    // Should optimize to << 6
		}
	}
#	else
#		error "No suitable method for gen_read_cpu_time_stamp_counter for this cpu type"
#	endif

	return r;
}
#else
gen_u64 gen_read_cpu_time_stamp_counter( void )
{
	GEN_PANIC( "gen_read_cpu_time_stamp_counter is not supported on this particular setup" );
	return -0;
}
#endif

// GEN_BENCHMARK
#endif
