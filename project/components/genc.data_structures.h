#define GEN_AST_POD_SIZE 128
#define GEN_AST_ARRSPECS_CAP \
( \
	 GEN_AST_POD_SIZE \
	 - sizeof(gen_AST*) * 2 \
	 - sizeof(gen_StringCached) \
	 - sizeof(gen_CodeT) \
	 - sizeof(gen_s32) \
) \
/ sizeof(gen_SpecifierT)


typedef struct gen_AST;
struct gen_AST
{
	union {
		struct
		{
			gen_AST*      Attributes;     // Enum, Function, Struct, Typedef, Union,Variable
			gen_AST*      Specs;          // Function, Type symbol, Variable
			union {
				gen_AST*  ReturnType;     // Function
				gen_AST*  UnderlyingType; // Enum, Typedef
				gen_AST*  ValueType;      // Parameter, Variable
			};
			gen_AST*      Params;         // Function, Generic, Template_Macro
			union {
				gen_AST*  ArrExpr;        // Type Symbol
				gen_AST*  Body;           // Enum, Function, Struct, Union
				gen_AST*  Value;          // Parameter, Variable
			};
		};
		gen_StringCached  Content;        // Attributes, Comment, Execution, Include
		gen_SpecifierT    ArrSpecs[GEN_AST_ARRSPECS_CAP]; // Specifiers
	};
	union {
		gen_AST* Prev;
		gen_AST* Front;
		gen_AST* Last;
	};
	union {
		gen_AST* Next;
		gen_AST* Back;
	};
	gen_StringCached      Name;
	gen_CodeT             Type;
	gen_s32               NumEntries;
};
