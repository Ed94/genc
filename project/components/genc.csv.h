typedef enum gen_csv_error {
    ZPL_CSV_ERROR_NONE,
    ZPL_CSV_ERROR_INTERNAL,
    ZPL_CSV_ERROR_UNEXPECTED_END_OF_INPUT,
    ZPL_CSV_ERROR_MISMATCHED_ROWS,
} gen_csv_error;

typedef gen_adt_node gen_csv_object;

ZPL_DEF_INLINE gen_u8 gen_csv_parse(gen_csv_object *root, char *text, gen_allocator allocator, gen_b32 has_header);
gen_u8 gen_csv_parse_delimiter(gen_csv_object *root, char *text, gen_allocator allocator, gen_b32 has_header, char delim);
void gen_csv_free(gen_csv_object *obj);

ZPL_DEF_INLINE void gen_csv_write(gen_file *file, gen_csv_object *obj);
ZPL_DEF_INLINE gen_string gen_csv_write_string(gen_allocator a, gen_csv_object *obj);
void gen_csv_write_delimiter(gen_file *file, gen_csv_object *obj, char delim);
gen_string gen_csv_write_string_delimiter(gen_allocator a, gen_csv_object *obj, char delim);

/* inline */

ZPL_IMPL_INLINE gen_u8 gen_csv_parse(gen_csv_object *root, char *text, gen_allocator allocator, gen_b32 has_header) {
    return gen_csv_parse_delimiter(root, text, allocator, has_header, ',');
}

ZPL_IMPL_INLINE void gen_csv_write(gen_file *file, gen_csv_object *obj) {
    gen_csv_write_delimiter(file, obj, ',');
}

ZPL_IMPL_INLINE gen_string gen_csv_write_string(gen_allocator a, gen_csv_object *obj) {
    return gen_csv_write_string_delimiter(a, obj, ',');
}
