#pragma region Printing

typedef struct gen_FileInfo gen_FileInfo;

#ifndef GEN_PRINTF_MAXLEN
#	define GEN_PRINTF_MAXLEN 65536
#endif

// NOTE: A locally persisting buffer is used internally
char*   gen_str_fmt_buf       ( char const* fmt, ... );
char*   gen_str_fmt_buf_va    ( char const* fmt, va_list va );
gen_sw  gen_str_fmt_va        ( char* str, gen_sw n, char const* fmt, va_list va );
gen_sw  gen_str_fmt_file      ( gen_FileInfo* f, char const* fmt, ... );
gen_sw  gen_str_fmt_file_va   ( gen_FileInfo* f, char const* fmt, va_list va );
gen_sw  gen_str_fmt_out_va    ( char const* fmt, va_list va );
gen_sw  gen_str_fmt_out_err   ( char const* fmt, ... );
gen_sw  gen_str_fmt_out_err_va( char const* fmt, va_list va );

#define GEN_MSG_INVALID_VALUE "INVALID VALUE PROVIDED";

inline
gen_sw gen_log_fmt(char const* fmt, ...)
{
	gen_sw res;
	va_list va;

	va_start(va, fmt);
	res = gen_str_fmt_out_va(fmt, va);
	va_end(va);

	return res;
}

inline
gen_sw fatal(char const* fmt, ...)
{
	local_persist thread_local
	char buf[GEN_PRINTF_MAXLEN] = { 0 };

	va_list va;

#if Build_Debug
	va_start(va, fmt);
	gen_str_fmt_va(buf, GEN_PRINTF_MAXLEN, fmt, va);
	va_end(va);

	assert_crash(buf);
	return -1;
#else
	va_start(va, fmt);
	gen_str_fmt_out_err_va( fmt, va);
	va_end(va);

	exit(1);
	return -1;
#endif
}

#pragma endregion Printing
