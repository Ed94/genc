#pragma region ADT

typedef enum gen_ADT_Type
{
	gen_EADT_TYPE_UNINITIALISED, /* node was not initialised, this is a programming error! */
	gen_EADT_TYPE_ARRAY,
	gen_EADT_TYPE_OBJECT,
	gen_EADT_TYPE_STRING,
	gen_EADT_TYPE_MULTISTRING,
	gen_EADT_TYPE_INTEGER,
	gen_EADT_TYPE_REAL,
} gen_ADT_Type;

typedef enum gen_ADT_Props
{
	gen_EADT_PROPS_NONE,
	gen_EADT_PROPS_NAN,
	gen_EADT_PROPS_NAN_NEG,
	gen_EADT_PROPS_INFINITY,
	gen_EADT_PROPS_INFINITY_NEG,
	gen_EADT_PROPS_FALSE,
	gen_EADT_PROPS_TRUE,
	gen_EADT_PROPS_NULL,
	gen_EADT_PROPS_IS_EXP,
	gen_EADT_PROPS_IS_HEX,

	// Used internally so that people can fill in real numbers they plan to write.
	gen_EADT_PROPS_IS_PARSED_REAL,
} gen_ADT_Props;

typedef enum gen_ADT_NamingStyle
{
	gen_EADT_NAME_STYLE_DOUBLE_QUOTE,
	gen_EADT_NAME_STYLE_SINGLE_QUOTE,
	gen_EADT_NAME_STYLE_NO_QUOTES,
} gen_ADT_NamingStyle;

typedef enum gen_ADT_AssignStyle
{
	gen_EADT_ASSIGN_STYLE_COLON,
	gen_EADT_ASSIGN_STYLE_EQUALS,
	gen_EADT_ASSIGN_STYLE_LINE,
} gen_ADT_AssignStyle;

typedef enum gen_ADT_DelimStyle
{
	gen_EADT_DELIM_STYLE_COMMA,
	gen_EADT_DELIM_STYLE_LINE,
	gen_EADT_DELIM_STYLE_NEWLINE,
} gen_ADT_DelimStyle;

typedef enum gen_ADT_Error
{
	gen_EADT_ERROR_NONE,
	gen_EADT_ERROR_INTERNAL,
	gen_EADT_ERROR_ALREADY_CONVERTED,
	gen_EADT_ERROR_INVALID_TYPE,
	gen_EADT_ERROR_OUT_OF_MEMORY,
} gen_ADT_Error;

typedef struct gen_ADT_Node
{
	char const*          name;
	struct gen_ADT_Node* parent;

	/* properties */
	gen_u8 type  : 4;
	gen_u8 props : 4;
#ifndef GEN_PARSER_DISABLE_ANALYSIS
	gen_u8 cfg_mode          : 1;
	gen_u8 name_style        : 2;
	gen_u8 assign_style      : 2;
	gen_u8 delim_style       : 2;
	gen_u8 delim_line_width  : 4;
	gen_u8 assign_line_width : 4;
#endif

	/* adt data */
	union
	{
		char const*          string;
		struct gen_ADT_Node* nodes;    ///< zpl_array

		struct
		{
			union
			{
				gen_f64 real;
				gen_s64 integer;
			};

#ifndef GEN_PARSER_DISABLE_ANALYSIS
			/* number analysis */
			gen_s32 base;
			gen_s32 base2;
			gen_u8  base2_offset : 4;
			gen_s8  exp          : 4;
			gen_u8  neg_zero     : 1;
			gen_u8  lead_digit   : 1;
#endif
		};
	};
} gen_ADT_Node;

/* ADT NODE LIMITS
 * delimiter and assignment segment width is limited to 128 whitespace symbols each.
 * real number limits decimal position to 128 places.
 * real number exponent is limited to 64 digits.
 */

/**
 * @brief Initialise an ADT object or array
 *
 * @param node
 * @param backing Memory allocator used for descendants
 * @param name Node's name
 * @param is_array
 * @return error code
 */
gen_u8 gen_adt_make_branch( gen_ADT_Node* node, gen_AllocatorInfo backing, char const* name, gen_b32 is_array );

/**
 * @brief Destroy an ADT branch and its descendants
 *
 * @param node
 * @return error code
 */
gen_u8 gen_adt_destroy_branch( gen_ADT_Node* node );

/**
 * @brief Initialise an ADT leaf
 *
 * @param node
 * @param name Node's name
 * @param type Node's type (use zpl_adt_make_branch for container nodes)
 * @return error code
 */
gen_u8 gen_adt_make_leaf( gen_ADT_Node* node, char const* name, gen_u8 type );


/**
 * @brief Fetch a node using provided URI string.
 *
 * This method uses a basic syntax to fetch a node from the ADT. The following features are available
 * to retrieve the data:
 *
 * - "a/b/c" navigates through objects "a" and "b" to get to "c"
 * - "arr/[foo=123]/bar" iterates over "arr" to find any object with param "foo" that matches the value "123", then gets its field called "bar"
 * - "arr/3" retrieves the 4th element in "arr"
 * - "arr/[apple]" retrieves the first element of value "apple" in "arr"
 *
 * @param node ADT node
 * @param uri Locator string as described above
 * @return zpl_adt_node*
 *
 * @see code/apps/examples/json_get.c
 */
gen_ADT_Node* gen_adt_query( gen_ADT_Node* node, char const* uri );

/**
 * @brief Find a field node within an object by the given name.
 *
 * @param node
 * @param name
 * @param deep_search Perform search recursively
 * @return zpl_adt_node * node
 */
gen_ADT_Node* gen_adt_find( gen_ADT_Node* node, char const* name, gen_b32 deep_search );

/**
 * @brief Allocate an unitialised node within a container at a specified index.
 *
 * @param parent
 * @param index
 * @return zpl_adt_node * node
 */
gen_ADT_Node* gen_adt_alloc_at( gen_ADT_Node* parent, gen_sw index );

/**
 * @brief Allocate an unitialised node within a container.
 *
 * @param parent
 * @return zpl_adt_node * node
 */
gen_ADT_Node* gen_adt_alloc( gen_ADT_Node* parent );

/**
 * @brief Move an existing node to a new container at a specified index.
 *
 * @param node
 * @param new_parent
 * @param index
 * @return zpl_adt_node * node
 */
gen_ADT_Node* gen_adt_move_node_at( gen_ADT_Node* node, gen_ADT_Node* new_parent, gen_sw index );

/**
 * @brief Move an existing node to a new container.
 *
 * @param node
 * @param new_parent
 * @return zpl_adt_node * node
 */
gen_ADT_Node* gen_adt_move_node( gen_ADT_Node* node, gen_ADT_Node* new_parent );

/**
 * @brief Swap two nodes.
 *
 * @param node
 * @param other_node
 * @return
 */
void gen_adt_swap_nodes( gen_ADT_Node* node, gen_ADT_Node* other_node );

/**
 * @brief Remove node from container.
 *
 * @param node
 * @return
 */
void gen_adt_remove_node( gen_ADT_Node* node );

/**
 * @brief Initialise a node as an object
 *
 * @param obj
 * @param name
 * @param backing
 * @return
 */
gen_b8 gen_adt_set_obj( gen_ADT_Node* obj, char const* name, gen_AllocatorInfo backing );

/**
 * @brief Initialise a node as an array
 *
 * @param obj
 * @param name
 * @param backing
 * @return
 */
gen_b8 gen_adt_set_arr( gen_ADT_Node* obj, char const* name, gen_AllocatorInfo backing );

/**
 * @brief Initialise a node as a string
 *
 * @param obj
 * @param name
 * @param value
 * @return
 */
gen_b8 gen_adt_set_str( gen_ADT_Node* obj, char const* name, char const* value );

/**
 * @brief Initialise a node as a float
 *
 * @param obj
 * @param name
 * @param value
 * @return
 */
gen_b8 gen_adt_set_flt( gen_ADT_Node* obj, char const* name, gen_f64 value );

/**
 * @brief Initialise a node as a signed integer
 *
 * @param obj
 * @param name
 * @param value
 * @return
 */
gen_b8 gen_adt_set_int( gen_ADT_Node* obj, char const* name, gen_s64 value );

/**
 * @brief Append a new node to a container as an object
 *
 * @param parent
 * @param name
 * @return*
 */
gen_ADT_Node* gen_adt_append_obj( gen_ADT_Node* parent, char const* name );

/**
 * @brief Append a new node to a container as an array
 *
 * @param parent
 * @param name
 * @return*
 */
gen_ADT_Node* gen_adt_append_arr( gen_ADT_Node* parent, char const* name );

/**
 * @brief Append a new node to a container as a string
 *
 * @param parent
 * @param name
 * @param value
 * @return*
 */
gen_ADT_Node* gen_adt_append_str( gen_ADT_Node* parent, char const* name, char const* value );

/**
 * @brief Append a new node to a container as a float
 *
 * @param parent
 * @param name
 * @param value
 * @return*
 */
gen_ADT_Node* gen_adt_append_flt( gen_ADT_Node* parent, char const* name, gen_f64 value );

/**
 * @brief Append a new node to a container as a signed integer
 *
 * @param parent
 * @param name
 * @param value
 * @return*
 */
gen_ADT_Node* gen_adt_append_int( gen_ADT_Node* parent, char const* name, gen_s64 value );

/* parser helpers */

/**
 * @brief Parses a text and stores the result into an unitialised node.
 *
 * @param node
 * @param base
 * @return*
 */
char* gen_adt_parse_number( gen_ADT_Node* node, char* base );

/**
 * @brief Parses a text and stores the result into an unitialised node.
 * This function expects the entire input to be a number.
 *
 * @param node
 * @param base
 * @return*
 */
char* gen_adt_parse_number_strict( gen_ADT_Node* node, char* base_str );

/**
 * @brief Parses and converts an existing string node into a number.
 *
 * @param node
 * @return
 */
gen_ADT_Error gen_adt_str_to_number( gen_ADT_Node* node );

/**
 * @brief Parses and converts an existing string node into a number.
 * This function expects the entire input to be a number.
 *
 * @param node
 * @return
 */
gen_ADT_Error gen_adt_str_to_number_strict( gen_ADT_Node* node );

/**
 * @brief Prints a number into a file stream.
 *
 * The provided file handle can also be a memory mapped stream.
 *
 * @see zpl_file_stream_new
 * @param file
 * @param node
 * @return
 */
gen_ADT_Error gen_adt_print_number( gen_FileInfo* file, gen_ADT_Node* node );

/**
 * @brief Prints a string into a file stream.
 *
 * The provided file handle can also be a memory mapped stream.
 *
 * @see zpl_file_stream_new
 * @param file
 * @param node
 * @param escaped_chars
 * @param escape_symbol
 * @return
 */
gen_ADT_Error gen_adt_print_string( gen_FileInfo* file, gen_ADT_Node* node, char const* escaped_chars, char const* escape_symbol );

#pragma endregion ADT

