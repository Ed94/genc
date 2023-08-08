#pragma region Filesystem

typedef gen_u32 gen_FileMode;

typedef enum gen_FileModeFlag
{
	gen_EFileMode_READ   = bit( 0 ),
	gen_EFileMode_WRITE  = bit( 1 ),
	gen_EFileMode_APPEND = bit( 2 ),
	gen_EFileMode_RW     = bit( 3 ),
	GEN_FILE_MODES       = gen_EFileMode_READ | gen_EFileMode_WRITE | gen_EFileMode_APPEND | gen_EFileMode_RW,
} gen_FileModeFlag;

// NOTE: Only used internally and for the file operations
typedef enum gen_SeekWhenceType
{
	gen_ESeekWhence_BEGIN   = 0,
	gen_ESeekWhence_CURRENT = 1,
	gen_ESeekWhence_END     = 2,
} gen_SeekWhenceType;

typedef enum gen_FileError
{
	gen_EFileError_NONE,
	gen_EFileError_INVALID,
	gen_EFileError_INVALID_FILENAME,
	gen_EFileError_EXISTS,
	gen_EFileError_NOT_EXISTS,
	gen_EFileError_PERMISSION,
	gen_EFileError_TRUNCATION_FAILURE,
	gen_EFileError_NOT_EMPTY,
	gen_EFileError_NAME_TOO_LONG,
	gen_EFileError_UNKNOWN,
} gen_FileError;

typedef union gen_FileDescriptor
{
	void*    p;
	gen_sptr i;
	gen_uptr u;
} gen_FileDescriptor;

typedef struct gen_FileOperations gen_FileOperations;

#define GEN_FILE_OPEN_PROC( name )     gen_FileError name( gen_FileDescriptor* fd, gen_FileOperations* ops, gen_FileMode mode, char const* filename )
#define GEN_FILE_READ_AT_PROC( name )  gen_b32 name( gen_FileDescriptor fd, void* buffer, gen_sw size, gen_s64 offset, gen_sw* bytes_read, gen_b32 stop_at_newline )
#define GEN_FILE_WRITE_AT_PROC( name ) gen_b32 name( gen_FileDescriptor fd, void const* buffer, gen_sw size, gen_s64 offset, gen_sw* bytes_written )
#define GEN_FILE_SEEK_PROC( name )     gen_b32 name( gen_FileDescriptor fd, gen_s64 offset, gen_SeekWhenceType whence, gen_s64* new_offset )
#define GEN_FILE_CLOSE_PROC( name )    void name( gen_FileDescriptor fd )

typedef GEN_FILE_OPEN_PROC( gen_file_open_proc );
typedef GEN_FILE_READ_AT_PROC( gen_FileReadProc );
typedef GEN_FILE_WRITE_AT_PROC( gen_FileWriteProc );
typedef GEN_FILE_SEEK_PROC( gen_FileSeekProc );
typedef GEN_FILE_CLOSE_PROC( gen_FileCloseProc );

struct gen_FileOperations
{
	gen_FileReadProc*  read_at;
	gen_FileWriteProc* write_at;
	gen_FileSeekProc*  seek;
	gen_FileCloseProc* close;
};

extern gen_FileOperations const gen_default_file_operations;

typedef gen_u64 gen_FileTime;

typedef enum gen_DirType
{
	GEN_DIR_TYPE_FILE,
	GEN_DIR_TYPE_FOLDER,
	GEN_DIR_TYPE_UNKNOWN,
} gen_DirType;

struct gen_DirInfo;

typedef struct gen_DirEntry
{
	char const*         filename;
	struct gen_DirInfo* dir_info;
	gen_u8              type;
} gen_DirEntry;

typedef struct gen_DirInfo
{
	char const*   fullpath;
	gen_DirEntry* entries;    // zpl_array

	// Internals
	char**     filenames;    // zpl_array
	gen_String buf;
} gen_DirInfo;

typedef struct gen_FileInfo
{
	gen_FileOperations ops;
	gen_FileDescriptor fd;
	gen_b32            is_temp;

	char const*   filename;
	gen_FileTime  last_write_time;
	gen_DirEntry* dir;
} gen_FileInfo;

typedef enum gen_FileStandardType
{
	gen_EFileStandard_INPUT,
	gen_EFileStandard_OUTPUT,
	gen_EFileStandard_ERROR,

	gen_EFileStandard_COUNT,
} gen_FileStandardType;

#define GEN_STDIO_IN  gen_file_get_standard( gen_EFileStandard_INPUT )
#define GEN_STDIO_OUT gen_file_get_standard( gen_EFileStandard_OUTPUT )
#define GEN_STDIO_ERR gen_file_get_standard( gen_EFileStandard_ERROR )

/**
 * Get standard file I/O.
 * @param  std Check zpl_file_standard_type
 * @return     File handle to standard I/O
 */
gen_FileInfo* gen_file_get_standard( gen_FileStandardType std );

/**
 * Closes the file
 * @param  file
 */
gen_FileError gen_file_close( gen_FileInfo* file );

/**
 * Returns the currently opened file's name
 * @param  file
 */
char const* gen_file_name( gen_FileInfo* file );

/**
 * Connects a system handle to a zpl file.
 * @param  file   Pointer to zpl file
 * @param  handle Low-level OS handle to connect
 */
void gen_file_connect_handle( gen_FileInfo* file, void* handle );

/**
 * Opens a file
 * @param  file
 * @param  filename
 */
gen_FileError gen_file_open( gen_FileInfo* file, char const* filename );

/**
 * Opens a file using a specified mode
 * @param  file
 * @param  mode     Access mode to use
 * @param  filename
 */
gen_FileError gen_file_open_mode( gen_FileInfo* file, gen_FileMode mode, char const* filename );

/**
 * Creates a new file
 * @param  file
 * @param  filename
 */
gen_FileError gen_file_create( gen_FileInfo* file, char const* filename );

/**
 * Constructs a new file from data
 * @param  file
 * @param  fd       Low-level file descriptor to use
 * @param  ops      File operations to rely upon
 * @param  filename
 */
gen_FileError gen_file_new( gen_FileInfo* file, gen_FileDescriptor fd, gen_FileOperations ops, char const* filename );

/**
 * Returns a size of the file
 * @param  file
 * @return      File size
 */
gen_s64 gen_file_size( gen_FileInfo* file );

/**
 * Truncates the file by a specified size
 * @param  file
 * @param  size Size to truncate
 */
gen_FileError gen_file_truncate( gen_FileInfo* file, gen_s64 size );

/**
 * Checks whether a file's been changed since the last check
 * @param  file
 */
gen_b32 gen_file_has_changed( gen_FileInfo* file );

/**
 * Retrieves a directory listing relative to the file
 * @param file
 */
void gen_file_dirinfo_refresh( gen_FileInfo* file );

/**
 * Creates a temporary file
 * @param  file
 */
gen_FileError gen_file_temp( gen_FileInfo* file );

/**
 * Reads file safely
 * @param  file
 * @param  buffer     Buffer to read to
 * @param  size       Size to read
 * @param  offset     Offset to read from
 * @param  bytes_read How much data we've actually read
 */
GEN_DEF_INLINE gen_b32 gen_file_read_at_check( gen_FileInfo* file, void* buffer, gen_sw size, gen_s64 offset, gen_sw* bytes_read );

/**
 * Writes to file safely
 * @param  file
 * @param  buffer        Buffer to read from
 * @param  size          Size to write
 * @param  offset        Offset to write to
 * @param  bytes_written How much data we've actually written
 */
GEN_DEF_INLINE gen_b32 gen_file_write_at_check( gen_FileInfo* file, void const* buffer, gen_sw size, gen_s64 offset, gen_sw* bytes_written );


/**
 * Reads file at a specific offset
 * @param  file
 * @param  buffer     Buffer to read to
 * @param  size       Size to read
 * @param  offset     Offset to read from
 * @param  bytes_read How much data we've actually read
 */
GEN_DEF_INLINE gen_b32 gen_file_read_at( gen_FileInfo* file, void* buffer, gen_sw size, gen_s64 offset );

/**
 * Writes to file at a specific offset
 * @param  file
 * @param  buffer        Buffer to read from
 * @param  size          Size to write
 * @param  offset        Offset to write to
 * @param  bytes_written How much data we've actually written
 */
GEN_DEF_INLINE gen_b32 gen_file_write_at( gen_FileInfo* file, void const* buffer, gen_sw size, gen_s64 offset );

/**
 * Seeks the file cursor from the beginning of file to a specific position
 * @param  file
 * @param  offset Offset to seek to
 */
GEN_DEF_INLINE gen_s64 gen_file_seek( gen_FileInfo* file, gen_s64 offset );

/**
 * Seeks the file cursor to the end of the file
 * @param  file
 */
GEN_DEF_INLINE gen_s64 gen_file_seek_to_end( gen_FileInfo* file );

/**
 * Skips N bytes at the current position
 * @param  file
 * @param  bytes Bytes to skip
 */
GEN_DEF_INLINE gen_s64 gen_file_skip( gen_FileInfo* file, gen_s64 bytes );    // NOTE: Skips a certain amount of bytes

/**
 * Returns the length from the beginning of the file we've read so far
 * @param  file
 * @return      Our current position in file
 */
GEN_DEF_INLINE gen_s64 gen_file_tell( gen_FileInfo* file );

/**
 * Reads from a file
 * @param  file
 * @param  buffer Buffer to read to
 * @param  size   Size to read
 */
GEN_DEF_INLINE gen_b32 gen_file_read( gen_FileInfo* file, void* buffer, gen_sw size );

/**
 * Writes to a file
 * @param  file
 * @param  buffer Buffer to read from
 * @param  size   Size to read
 */
GEN_DEF_INLINE gen_b32 gen_file_write( gen_FileInfo* file, void const* buffer, gen_sw size );

typedef struct gen_FileContents
{
	gen_AllocatorInfo allocator;
	void*             data;
	gen_sw            size;
} gen_FileContents;

/**
 * Reads the whole file contents
 * @param  a              Allocator to use
 * @param  zero_terminate End the read data with null terminator
 * @param  filepath       Path to the file
 * @return                File contents data
 */
gen_FileContents gen_file_read_contents( gen_AllocatorInfo a, gen_b32 zero_terminate, char const* filepath );

/**
 * Frees the file content data previously read
 * @param  fc
 */
void gen_file_free_contents( gen_FileContents* fc );

/**
 * Writes content to a file
 */
gen_b32 gen_file_write_contents( char const* filepath, void const* buffer, gen_sw size, gen_FileError* err );

/**
 * Reads the file as array of lines
 *
 * Make sure you free both the returned buffer and the lines (zpl_array)
 * @param  alloc            Allocator to use
 * @param  lines            Reference to zpl_array container we store lines to
 * @param  filename         Path to the file
 * @param  strip_whitespace Strip whitespace when we split to lines?
 * @return                  File content we've read itself
 */
char* gen_file_read_lines( gen_AllocatorInfo allocator, gen_Array_CStr* lines, char const* filename, gen_b32 strip_whitespace );

//! @}

/* inlines */


GEN_IMPL_INLINE gen_b32 gen_file_read_at_check( gen_FileInfo* f, void* buffer, gen_sw size, gen_s64 offset, gen_sw* bytes_read )
{
	if ( ! f->ops.read_at )
		f->ops = gen_default_file_operations;
	return f->ops.read_at( f->fd, buffer, size, offset, bytes_read, false );
}

GEN_IMPL_INLINE gen_b32 gen_file_write_at_check( gen_FileInfo* f, void const* buffer, gen_sw size, gen_s64 offset, gen_sw* bytes_written )
{
	if ( ! f->ops.read_at )
		f->ops = gen_default_file_operations;
	return f->ops.write_at( f->fd, buffer, size, offset, bytes_written );
}

GEN_IMPL_INLINE gen_b32 gen_file_read_at( gen_FileInfo* f, void* buffer, gen_sw size, gen_s64 offset )
{
	return gen_file_read_at_check( f, buffer, size, offset, NULL );
}

GEN_IMPL_INLINE gen_b32 gen_file_write_at( gen_FileInfo* f, void const* buffer, gen_sw size, gen_s64 offset )
{
	return gen_file_write_at_check( f, buffer, size, offset, NULL );
}

GEN_IMPL_INLINE gen_s64 gen_file_seek( gen_FileInfo* f, gen_s64 offset )
{
	gen_s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = gen_default_file_operations;
	f->ops.seek( f->fd, offset, gen_ESeekWhence_BEGIN, &new_offset );
	return new_offset;
}

GEN_IMPL_INLINE gen_s64 gen_file_seek_to_end( gen_FileInfo* f )
{
	gen_s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = gen_default_file_operations;
	f->ops.seek( f->fd, 0, gen_ESeekWhence_END, &new_offset );
	return new_offset;
}

// NOTE: Skips a certain amount of bytes
GEN_IMPL_INLINE gen_s64 gen_file_skip( gen_FileInfo* f, gen_s64 bytes )
{
	gen_s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = gen_default_file_operations;
	f->ops.seek( f->fd, bytes, gen_ESeekWhence_CURRENT, &new_offset );
	return new_offset;
}

GEN_IMPL_INLINE gen_s64 gen_file_tell( gen_FileInfo* f )
{
	gen_s64 new_offset = 0;
	if ( ! f->ops.read_at )
		f->ops = gen_default_file_operations;
	f->ops.seek( f->fd, 0, gen_ESeekWhence_CURRENT, &new_offset );
	return new_offset;
}

GEN_IMPL_INLINE gen_b32 gen_file_read( gen_FileInfo* f, void* buffer, gen_sw size )
{
	gen_s64 cur_offset = gen_file_tell( f );
	gen_b32 result     = gen_file_read_at( f, buffer, size, gen_file_tell( f ) );
	gen_file_seek( f, cur_offset + size );
	return result;
}

GEN_IMPL_INLINE gen_b32 gen_file_write( gen_FileInfo* f, void const* buffer, gen_sw size )
{
	gen_s64 cur_offset = gen_file_tell( f );
	gen_b32 result     = gen_file_write_at( f, buffer, size, gen_file_tell( f ) );
	gen_file_seek( f, cur_offset + size );
	return result;
}

#pragma endregion Filesystem
