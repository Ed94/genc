#pragma region AST

#define GEN_AST_POD_SIZE 128
#define GEN_AST_ARRSPECS_CAP     \
(                                \
	 GEN_AST_POD_SIZE            \
	 - sizeof(gen_AST*) * 3      \
	 - sizeof(gen_StringCached)  \
	 - sizeof(gen_CodeT)         \
	 - sizeof(gen_s32)           \
)                                \
/ sizeof(gen_SpecifierT)

typedef struct gen_AST;
typedef struct gen_AST* gen_Code; // Just an ease of use typedef since this type is used so often.

struct gen_AST
{
	union {
		struct
		{
			gen_Code      Attributes;     // Enum, Function, Struct, Typedef, Union,Variable
			gen_Code      Specs;          // Function, Type symbol, Variable
			union {
				gen_Code  ReturnType;     // Function
				gen_Code  UnderlyingType; // Enum, Typedef (C++ 20 supports underlying type for enum)
				gen_Code  ValueType;      // Parameter, Variable
			};
			union {
				gen_Code  BitfieldSize;   // Variable (Struct Data Member)
				gen_Code  Params;         // Function, Generic, Template_Macro
			};
			union {
				gen_Code  ArrExpr;        // Type Symbol
				gen_Code  Body;           // Enum, Function, Struct, Union
				gen_Code  Value;          // Parameter, Variable
			};
		};
		gen_StringCached  Content;        // Attributes, Comment, Execution, Include
		gen_SpecifierT    ArrSpecs[ GEN_AST_ARRSPECS_CAP ]; // Specifiers
	};
	union {
		gen_Code Prev;
		gen_Code Front;
		gen_Code Last;
	};
	union {
		gen_Code Next;
		gen_Code Back;
	};
	gen_Code              Parent;
	gen_StringCached      Name;
	gen_CodeT             Type;
	union {
		gen_b32           IsFunction; // Used by typedef to not serialize the name field.
		gen_s32           NumEntries;
	};
};

#pragma region AST Types
/*
	Show only relevant members of the gen_AST for its type.
	gen_AST* fields are replaced with gen_AST* types.
		- Guards assignemnts to gen_AST* fields to ensure the gen_AST is duplicated if assigned to another parent.
*/

typedef struct gen_AST_Body       gen_AST_Body;
typedef struct gen_AST_Attributes gen_AST_Attributes;
typedef struct gen_AST_Comment    gen_AST_Comment;
typedef struct gen_AST_Enum       gen_AST_Enum;
typedef struct gen_AST_Exec       gen_AST_Exec;
typedef struct gen_AST_Include    gen_AST_Include;
typedef struct gen_AST_Fn         gen_AST_Fn;
typedef struct gen_AST_Param      gen_AST_Param;
typedef struct gen_AST_Specifiers gen_CodeSpecifiers;
typedef struct gen_AST_Struct     gen_AST_Struct;
typedef struct gen_AST_Type       gen_AST_Type;
typedef struct gen_AST_Typedef    gen_AST_Typedef;
typedef struct gen_AST_Union      gen_AST_Union;
typedef struct gen_AST_Var        gen_AST_Var;

typedef struct gen_AST_Body*       gen_CodeBody;
typedef struct gen_AST_Attributes* gen_CodeAttributes;
typedef struct gen_AST_Comment*    gen_CodeComment;
typedef struct gen_AST_Enum*       gen_CodeEnum;
typedef struct gen_AST_Exec*       gen_CodeExec;
typedef struct gen_AST_Include*    gen_CodeInclude;
typedef struct gen_AST_Fn*         gen_CodeFn;
typedef struct gen_AST_Param*      gen_CodeParam;
typedef struct gen_AST_Specifiers* gen_CodeSpecifiers;
typedef struct gen_AST_Struct*     gen_CodeStruct;
typedef struct gen_AST_Type*       gen_CodeType;
typedef struct gen_AST_Typedef*    gen_CodeTypedef;
typedef struct gen_AST_Union*      gen_CodeUnion;
typedef struct gen_AST_Var*        gen_CodeVar;

struct gen_AST_Body
{
	char              _PAD_[ sizeof(gen_SpecifierT) * GEN_AST_ARRSPECS_CAP ];
	gen_Code          Front;
	gen_Code          Back;
	gen_Code          Parent;
	gen_StringCached  Name;
	gen_CodeT         Type;
	gen_s32           NumEntries;
};
static_assert( sizeof(gen_AST_Body) == sizeof(gen_AST), "ERROR: AST_Filtered is not the same size as gen_AST");

struct gen_AST_Attributes
{
	union {
		char             _PAD_[ sizeof(gen_SpecifierT) * GEN_AST_ARRSPECS_CAP ];
		gen_StringCached Content;
	};
	gen_Code             Prev;
	gen_Code             Next;
	gen_Code             Parent;
	gen_StringCached     Name;
	gen_CodeT            Type;
	char                 _PAD_UNUSED_[ sizeof(gen_s32) ];
};
static_assert( sizeof(gen_AST_Attributes) == sizeof(gen_AST), "ERROR: AST_Attributes is not the same size as gen_AST");

struct gen_AST_Comment
{
	union {
		char             _PAD_[ sizeof(gen_SpecifierT) * GEN_AST_ARRSPECS_CAP ];
		gen_StringCached Content;
	};
	gen_Code             Prev;
	gen_Code             Next;
	gen_Code             Parent;
	gen_StringCached     Name;
	gen_CodeT            Type;
	char                 _PAD_UNUSED_[ sizeof(gen_s32) ];
};
static_assert( sizeof(gen_AST_Comment) == sizeof(gen_AST), "ERROR: AST_Comment is not the same size as gen_AST");

struct gen_AST_Enum
{
	union {
		char                    _PAD_[ sizeof(gen_SpecifierT) * GEN_AST_ARRSPECS_CAP ];
		struct
		{
			gen_CodeAttributes Attributes;
			char               _PAD_SPEC_  [ sizeof(gen_AST*) ];
			gen_CodeType       UnderlyingType;
			char	           _PAD_PARAMS_[ sizeof(gen_AST*) ];
			gen_CodeBody       Body;
		};
	};
	gen_Code                   Prev;
	gen_Code                   Next;
	gen_Code                   Parent;
	gen_StringCached           Name;
	gen_CodeT                  Type;
	char 			           _PAD_UNUSED_[ sizeof(gen_s32) ];
};
static_assert( sizeof(gen_AST_Enum) == sizeof(gen_AST), "ERROR: AST_Enum is not the same size as gen_AST");

struct gen_AST_Exec
{
	union {
		char             _PAD_[ sizeof(gen_SpecifierT) * GEN_AST_ARRSPECS_CAP ];
		gen_StringCached Content;
	};
	gen_Code          Prev;
	gen_Code          Next;
	gen_Code          Parent;
	gen_StringCached  Name;
	gen_CodeT         Type;
	char              _PAD_UNUSED_[ sizeof(gen_s32) ];
};
static_assert( sizeof(gen_AST_Exec) == sizeof(gen_AST), "ERROR: AST_Exec is not the same size as gen_AST");

struct gen_AST_Include
{
	union {
		char             _PAD_[ sizeof(gen_SpecifierT) * GEN_AST_ARRSPECS_CAP ];
		gen_StringCached Content;
	};
	gen_Code             Prev;
	gen_Code             Next;
	gen_Code             Parent;
	gen_StringCached     Name;
	gen_CodeT            Type;
	char 			     _PAD_UNUSED_[ sizeof(gen_s32) ];
};
static_assert( sizeof(gen_AST_Include) == sizeof(gen_AST), "ERROR: AST_Include is not the same size as gen_AST");

struct gen_AST_Fn
{
	union {
		char                   _PAD_[ sizeof(gen_SpecifierT) * GEN_AST_ARRSPECS_CAP ];
		struct
		{
			gen_CodeAttributes Attributes;
			gen_CodeSpecifiers Specs;
			gen_CodeType       ReturnType;
			gen_CodeParam 	   Params;
			gen_CodeBody       Body;
		};
	};
	gen_Code                   Prev;
	gen_Code                   Parent;
	gen_Code                   Next;
	gen_StringCached           Name;
	gen_CodeT                  Type;
	char 			           _PAD_UNUSED_[ sizeof(gen_s32) ];
};
static_assert( sizeof(gen_AST_Fn) == sizeof(gen_AST), "ERROR: AST_Fn is not the same size as gen_AST");

struct gen_AST_Param
{
	union {
		char 		      _PAD_[ sizeof(gen_SpecifierT) * GEN_AST_ARRSPECS_CAP ];
		struct
		{
			char 	      _PAD_PROPERTIES_2_[ sizeof(gen_AST*) * 2 ];
			gen_CodeType  ValueType;
			char 	      _PAD_PROPERTIES_[ sizeof(gen_AST*) ];
			gen_Code      Value;
		};
	};
	gen_Code              Last;
	gen_Code              Next;
	gen_Code              Parent;
	gen_StringCached      Name;
	gen_CodeT             Type;
	gen_s32               NumEntries;
};
static_assert( sizeof(gen_AST_Param) == sizeof(gen_AST), "ERROR: AST_Param is not the same size as gen_AST");

struct gen_AST_Specifier
{
	gen_SpecifierT        ArrSpecs[ GEN_AST_ARRSPECS_CAP ];
	gen_Code              Prev;
	gen_Code              Next;
	gen_Code              Parent;
	gen_StringCached      Name;
	gen_CodeT             Type;
	gen_s32               NumEntries;
};
	static_assert( sizeof(gen_AST_Specifier) == sizeof(gen_AST), "ERROR: AST_Specifier is not the same size as gen_AST");

struct gen_AST_Struct
{
	union {
		char                    _PAD_[ sizeof(gen_SpecifierT) * GEN_AST_ARRSPECS_CAP ];
		struct
		{
			gen_CodeAttributes  Attributes;
			char 	            _PAD_SPECS_     [ sizeof(gen_Code) * 3 ];
			gen_CodeBody        Body;
		};
	};
	gen_Code                    Prev;
	gen_Code                    Next;
	gen_Code                    Parent;
	gen_StringCached            Name;
	gen_CodeT                   Type;
	char 			            _PAD_UNUSED_[ sizeof(gen_s32) ];
};
static_assert( sizeof(gen_AST_Struct) == sizeof(gen_AST), "ERROR: AST_Struct is not the same size as gen_AST");

struct gen_AST_Type
{
	union {
		char 		           _PAD_[ sizeof(gen_SpecifierT) * GEN_AST_ARRSPECS_CAP ];
		struct
		{
			gen_CodeAttributes Attributes;
			gen_CodeSpecifiers Specs;
			char 	           _PAD_PROPERTIES_[ sizeof(gen_AST*) * 2 ];
			gen_AST*           ArrExpr;
		};
	};
	gen_Code                   Prev;
	gen_Code                   Next;
	gen_Code                   Parent;
	gen_StringCached           Name;
	gen_CodeT                  Type;
	char 			           _PAD_UNUSED_[ sizeof(gen_s32) ];
};
static_assert( sizeof(gen_AST_Type) == sizeof(gen_AST), "ERROR: AST_Type is not the same size as gen_AST");

struct gen_AST_Typedef
{
	union {
		char 		           _PAD_[ sizeof(gen_SpecifierT) * GEN_AST_ARRSPECS_CAP ];
		struct
		{
			gen_CodeAttributes Attributes;
			char 	           _PAD_SPECS_     [ sizeof(gen_AST*) ];
			gen_Code           UnderlyingType;
			char 	           _PAD_PROPERTIES_[ sizeof(gen_AST*) * 2 ];
		};
	};
	gen_Code                   Prev;
	gen_Code                   Next;
	gen_Code                   Parent;
	gen_StringCached           Name;
	gen_CodeT                  Type;
	char 			           _PAD_UNUSED_[ sizeof(gen_s32) ];
};
static_assert( sizeof(gen_AST_Typedef) == sizeof(gen_AST), "ERROR: AST_Typedef is not the same size as gen_AST");

struct gen_AST_Union
{
	union {
		char 		            _PAD_[ sizeof(gen_SpecifierT) * GEN_AST_ARRSPECS_CAP ];
		struct
		{
			gen_CodeAttributes  Attributes;
			char 	            _PAD_PROPERTIES_[ sizeof(gen_AST*) * 3 ];
			gen_CodeBody        Body;
		};
	};
	gen_Code                    Prev;
	gen_Code                    Next;
	gen_Code                    Parent;
	gen_StringCached            Name;
	gen_CodeT                   Type;
	char 			            _PAD_UNUSED_[ sizeof(gen_s32) ];
};
static_assert( sizeof(gen_AST_Union) == sizeof(gen_AST), "ERROR: AST_Union is not the same size as gen_AST");

struct gen_AST_Var
{
	union {
		char                    _PAD_[ sizeof(gen_SpecifierT) * GEN_AST_ARRSPECS_CAP ];
		struct
		{
			gen_CodeAttributes Attributes;
			gen_CodeSpecifiers Specs;
			gen_CodeType       ValueType;
			char 	           _PAD_PROPERTIES_[ sizeof(gen_AST*) ];
			gen_Code           Value;
		};
	};
	gen_Code                   Prev;
	gen_Code                   Next;
	gen_Code                   Parent;
	gen_StringCached           Name;
	gen_CodeT                  Type;
	char 			           _PAD_UNUSED_[ sizeof(gen_s32) ];
};
static_assert( sizeof(gen_AST_Var) == sizeof(gen_AST), "ERROR: AST_Var is not the same size as gen_AST");

#pragma endregion AST Types

#define NoCode NULL
extern gen_Code gen_CodeInvalid;

#pragma region Code Interface

void       gen_code_debug_string   ( gen_Code     self );
gen_Code   gen_code_duplicate      ( gen_Code     self );
gen_Code   gen_code_entry          ( gen_Code     self, gen_s32 idx );
bool       gen_code_is_equal       ( gen_Code     self, gen_Code other );
void       gen_code_set_global     ( gen_Code     self );
gen_String gen_code_to_string      ( gen_Code     self );
void       gen_codebody_append     ( gen_CodeBody self, gen_Code entry );
bool       gen_codebody_has_entries( gen_CodeBody self );
bool       gen_codebody_validate   ( gen_CodeBody self );

#define gen_code_assign( self, field, other )                                                                                                                                      \
	do                                                                                                                                                                             \
	{                                                                                                                                                                              \
		if ( other && other->Parent )                                                                                                                                              \
		{                                                                                                                                                                          \
			other = gen_code_duplicate( other );                                                                                                                                   \
		}                                                                                                                                                                          \
                                                                                                                                                                                   \
		other->Parent = self;                                                                                                                                                      \
		self->field   = other;                                                                                                                                                     \
	} while ( 0 )

#define gen_codeparams_begin( params ) gen_CodeParams entry = params->ArrSpecs[0]
#define gen_codeparams_end( params )   params->ArrSpecs + params->NumEntries

#pragma endregion Code Interface

#pragma endregion AST

