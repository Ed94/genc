#pragma region Debug

#if defined( _MSC_VER )
#	if _MSC_VER < 1300
#		define GEN_DEBUG_TRAP() __asm int 3 /* Trap to debugger! */
#	else
#		define GEN_DEBUG_TRAP() __debugbreak()
#	endif
#elif defined( GEN_COMPILER_TINYC )
#	define GEN_DEBUG_TRAP() process_exit( 1 )
#else
#	define GEN_DEBUG_TRAP() __builtin_trap()
#endif

#define GEN_ASSERT( cond ) GEN_ASSERT_MSG( cond, NULL )

#define GEN_ASSERT_MSG( cond, msg, ... )                                                             \
	do                                                                                               \
	{                                                                                                \
		if ( ! ( cond ) )                                                                            \
		{                                                                                            \
			gen_assert_handler( #cond, __FILE__, zpl_cast( gen_s64 ) __LINE__, msg, ##__VA_ARGS__ ); \
			GEN_DEBUG_TRAP();                                                                        \
		}                                                                                            \
	} while ( 0 )

#define GEN_ASSERT_NOT_NULL( ptr ) GEN_ASSERT_MSG( ( ptr ) != NULL, #ptr " must not be NULL" )

// NOTE: Things that shouldn't happen with a message!
#define GEN_PANIC( msg, ... ) GEN_ASSERT_MSG( 0, msg, ##__VA_ARGS__ )

void     gen_assert_handler( char const* condition, char const* file, gen_s32 line, char const* msg, ... );
gen_s32  gen_assert_crash( char const* condition );
void     gen_process_exit( gen_u32 code );

#if Build_Debug
	#define fatal( fmt, ... )                            \
	do                                                   \
	{                                                    \
		local_persist thread_local                       \
		char buf[GEN_PRINTF_MAXLEN] = { 0 };             \
													     \
		va_list va;                                      \
													     \
		va_start(va, fmt);                               \
		gen_str_fmt_va(buf, GEN_PRINTF_MAXLEN, fmt, va); \
		va_end(va);                                      \
													     \
		GEN_PANIC(buf);                                  \
	}                                                    \
	while (0)
#else

#	define fatal( fmt, ... )						 \
	do                                               \
	{												 \
		local_persist thread_local                   \
		char buf[GEN_PRINTF_MAXLEN] = { 0 };         \
													 \
		va_list va;                                  \
												     \
		va_start(va, fmt);                           \
		gen_str_fmt_out_err_va( fmt, va);            \
		va_end(va);                                  \
													 \
		gen_process_exit(1);                         \
	}             					                 \
	while (0)
#endif

#pragma endregion Debug

