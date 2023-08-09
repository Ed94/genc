#pragma region Builder

typedef struct gen_Builder gen_Builder;
struct gen_Builder
{
	gen_FileInfo File;
	gen_String   Buffer;
};

void gen_builder_print( gen_Code );
void gen_builder_print_fmt( char const* fmt, ... );
bool gen_builder_open( char const* path );
void gen_builder_write();

#pragma endregion Builder

