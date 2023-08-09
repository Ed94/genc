#pragma region Strings

typedef struct gen_Str gen_Str;
struct gen_Str
{
	gen_sw      Len;
	char const* Ptr;
};
typedef char const* gen_StrC;

#define gen_strc_len( strc ) ( (sw)( strc - sizeof(gen_sw) ) )

typedef char*       gen_String;
typedef char const* gen_StringCached;

#pragma endregion Strings

