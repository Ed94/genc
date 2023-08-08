#pragma region Debug

void gen_assert_handler( char const* condition, char const* file, gen_s32 line, char const* msg, ... )
{
	gen__printf_err( "%s:(%d): Assert Failure: ", file, line );

	if ( condition )
		gen__printf_err( "`%s` ", condition );

	if ( msg )
	{
		va_list va;
		va_start( va, msg );
		gen__printf_err_va( msg, va );
		va_end( va );
	}

	gen__printf_err( "%s", "\n" );
}

gen_s32 gen_assert_crash( char const* condition )
{
	GEN_PANIC( condition );
	return 0;
}

#if defined( GEN_SYSTEM_UNIX ) || defined( GEN_SYSTEM_MACOS )
#	include <sched.h>
#endif

#if defined( GEN_SYSTEM_WINDOWS )
void gen_process_exit( gen_u32 code )
{
	ExitProcess( code );
}
#else
#	include <stdlib.h>

void gen_process_exit( gen_u32 code )
{
	exit( code );
}
#endif

#pragma endregion Debug

