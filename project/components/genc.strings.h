#pragma region Strings
typedef struct gen_StrDef gen_StrDef;
struct gen_StrDef
{
	gen_sw      Len;
	char const* Ptr;
};
typedef char const* gen_Str;

typedef char*       gen_String;
typedef char const* gen_StringCached;
#pragma region Strings
