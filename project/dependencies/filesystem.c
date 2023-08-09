#pragma region Filesystem

#if defined( GEN_SYSTEM_WINDOWS ) || defined( GEN_SYSTEM_CYGWIN )

internal wchar_t* gen__alloc_utf8_to_ucs2( gen_AllocatorInfo a, char const* text, gen_sw* w_len_ )
{
	wchar_t* w_text = NULL;
	gen_sw   len = 0, w_len = 0, w_len1 = 0;
	if ( text == NULL )
	{
		if ( w_len_ )
			*w_len_ = w_len;
		return NULL;
	}
	len = gen_str_len( text );
	if ( len == 0 )
	{
		if ( w_len_ )
			*w_len_ = w_len;
		return NULL;
	}
	w_len = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, text, zpl_cast( int ) len, NULL, 0 );
	if ( w_len == 0 )
	{
		if ( w_len_ )
			*w_len_ = w_len;
		return NULL;
	}
	w_text = gen_alloc_array( a, wchar_t, w_len + 1 );
	w_len1 = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, text, zpl_cast( int ) len, w_text, zpl_cast( int ) w_len );
	if ( w_len1 == 0 )
	{
		gen_free( a, w_text );
		if ( w_len_ )
			*w_len_ = 0;
		return NULL;
	}
	w_text[ w_len ] = 0;
	if ( w_len_ )
		*w_len_ = w_len;
	return w_text;
}

internal GEN_FILE_SEEK_PROC( gen__win32_file_seek )
{
	LARGE_INTEGER li_offset;
	li_offset.QuadPart = offset;
	if ( ! SetFilePointerEx( fd.p, li_offset, &li_offset, whence ) )
	{
		return false;
	}

	if ( new_offset )
		*new_offset = li_offset.QuadPart;
	return true;
}

internal GEN_FILE_READ_AT_PROC( gen__win32_file_read )
{
	gen_unused( stop_at_newline );
	gen_b32 result = false;
	gen__win32_file_seek( fd, offset, gen_ESeekWhence_BEGIN, NULL );
	DWORD size_ = zpl_cast( DWORD )( size > GEN_I32_MAX ? GEN_I32_MAX : size );
	DWORD bytes_read_;
	if ( ReadFile( fd.p, buffer, size_, &bytes_read_, NULL ) )
	{
		if ( bytes_read )
			*bytes_read = bytes_read_;
		result = true;
	}

	return result;
}

internal GEN_FILE_WRITE_AT_PROC( gen__win32_file_write )
{
	DWORD size_ = zpl_cast( DWORD )( size > GEN_I32_MAX ? GEN_I32_MAX : size );
	DWORD bytes_written_;
	gen__win32_file_seek( fd, offset, gen_ESeekWhence_BEGIN, NULL );
	if ( WriteFile( fd.p, buffer, size_, &bytes_written_, NULL ) )
	{
		if ( bytes_written )
			*bytes_written = bytes_written_;
		return true;
	}
	return false;
}

internal GEN_FILE_CLOSE_PROC( gen__win32_file_close )
{
	CloseHandle( fd.p );
}

gen_FileOperations const gen_default_file_operations = { gen__win32_file_read, gen__win32_file_write, gen__win32_file_seek, gen__win32_file_close };

GEN_NEVER_INLINE GEN_FILE_OPEN_PROC( gen__win32_file_open )
{
	DWORD    desired_access;
	DWORD    creation_disposition;
	void*    handle;
	wchar_t* w_text;

	switch ( mode & GEN_FILE_MODES )
	{
		case gen_EFileMode_READ :
			desired_access       = GENERIC_READ;
			creation_disposition = OPEN_EXISTING;
			break;
		case gen_EFileMode_WRITE :
			desired_access       = GENERIC_WRITE;
			creation_disposition = CREATE_ALWAYS;
			break;
		case gen_EFileMode_APPEND :
			desired_access       = GENERIC_WRITE;
			creation_disposition = OPEN_ALWAYS;
			break;
		case gen_EFileMode_READ | gen_EFileMode_RW :
			desired_access       = GENERIC_READ | GENERIC_WRITE;
			creation_disposition = OPEN_EXISTING;
			break;
		case gen_EFileMode_WRITE | gen_EFileMode_RW :
			desired_access       = GENERIC_READ | GENERIC_WRITE;
			creation_disposition = CREATE_ALWAYS;
			break;
		case gen_EFileMode_APPEND | gen_EFileMode_RW :
			desired_access       = GENERIC_READ | GENERIC_WRITE;
			creation_disposition = OPEN_ALWAYS;
			break;
		default :
			GEN_PANIC( "Invalid file mode" );
			return gen_EFileError_INVALID;
	}

	w_text = gen__alloc_utf8_to_ucs2( gen_heap_allocator(), filename, NULL );
	handle = CreateFileW( w_text, desired_access, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL );

	gen_free( gen_heap_allocator(), w_text );

	if ( handle == INVALID_HANDLE_VALUE )
	{
		DWORD err = GetLastError();
		switch ( err )
		{
			case ERROR_FILE_NOT_FOUND :
				return gen_EFileError_NOT_EXISTS;
			case ERROR_FILE_EXISTS :
				return gen_EFileError_EXISTS;
			case ERROR_ALREADY_EXISTS :
				return gen_EFileError_EXISTS;
			case ERROR_ACCESS_DENIED :
				return gen_EFileError_PERMISSION;
		}
		return gen_EFileError_INVALID;
	}

	if ( mode & gen_EFileMode_APPEND )
	{
		LARGE_INTEGER offset = { 0 };
		if ( ! SetFilePointerEx( handle, offset, NULL, gen_ESeekWhence_END ) )
		{
			CloseHandle( handle );
			return gen_EFileError_INVALID;
		}
	}

	fd->p = handle;
	*ops  = gen_default_file_operations;
	return gen_EFileError_NONE;
}

#else    // POSIX
#	include <fcntl.h>

internal GEN_FILE_SEEK_PROC( gen__posix_file_seek )
{
#	if defined( GEN_SYSTEM_OSX )
	gen_s64 res = lseek( fd.i, offset, whence );
#	else    // TODO(ZaKlaus): @fixme lseek64
	gen_s64 res = lseek( fd.i, offset, whence );
#	endif
	if ( res < 0 )
		return false;
	if ( new_offset )
		*new_offset = res;
	return true;
}

internal GEN_FILE_READ_AT_PROC( gen__posix_file_read )
{
	gen_unused( stop_at_newline );
	gen_sw res = pread( fd.i, buffer, size, offset );
	if ( res < 0 )
		return false;
	if ( bytes_read )
		*bytes_read = res;
	return true;
}

internal GEN_FILE_WRITE_AT_PROC( gen__posix_file_write )
{
	gen_sw  res;
	gen_s64 curr_offset = 0;
	gen__posix_file_seek( fd, 0, gen_ESeekWhence_CURRENT, &curr_offset );
	if ( curr_offset == offset )
	{
		// NOTE: Writing to stdout et al. doesn't like pwrite for numerous reasons
		res = write( zpl_cast( int ) fd.i, buffer, size );
	}
	else
	{
		res = pwrite( zpl_cast( int ) fd.i, buffer, size, offset );
	}
	if ( res < 0 )
		return false;
	if ( bytes_written )
		*bytes_written = res;
	return true;
}

internal GEN_FILE_CLOSE_PROC( gen__posix_file_close )
{
	close( fd.i );
}

gen_FileOperations const gen_default_file_operations = { gen__posix_file_read, gen__posix_file_write, gen__posix_file_seek, gen__posix_file_close };

GEN_NEVER_INLINE GEN_FILE_OPEN_PROC( gen__posix_file_open )
{
	gen_s32 os_mode;
	switch ( mode & GEN_FILE_MODES )
	{
		case gen_EFileMode_READ :
			os_mode = O_RDONLY;
			break;
		case gen_EFileMode_WRITE :
			os_mode = O_WRONLY | O_CREAT | O_TRUNC;
			break;
		case gen_EFileMode_APPEND :
			os_mode = O_WRONLY | O_APPEND | O_CREAT;
			break;
		case gen_EFileMode_READ | gen_EFileMode_RW :
			os_mode = O_RDWR;
			break;
		case gen_EFileMode_WRITE | gen_EFileMode_RW :
			os_mode = O_RDWR | O_CREAT | O_TRUNC;
			break;
		case gen_EFileMode_APPEND | gen_EFileMode_RW :
			os_mode = O_RDWR | O_APPEND | O_CREAT;
			break;
		default :
			GEN_PANIC( "Invalid file mode" );
			return gen_EFileError_INVALID;
	}

	fd->i = open( filename, os_mode, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
	if ( fd->i < 0 )
	{
		// TODO: More file errors
		return gen_EFileError_INVALID;
	}

	*ops = gen_default_file_operations;
	return gen_EFileError_NONE;
}

#endif

gen_FileError gen_file_new( gen_FileInfo* f, gen_FileDescriptor fd, gen_FileOperations ops, char const* filename )
{
	gen_FileError err = gen_EFileError_NONE;
	gen_sw        len = gen_str_len( filename );

	f->ops             = ops;
	f->fd              = fd;
	f->dir             = NULL;
	f->last_write_time = 0;
	f->filename        = gen_alloc_array( gen_heap_allocator(), char, len + 1 );
	gen_mem_copy( zpl_cast( char* ) f->filename, zpl_cast( char* ) filename, len + 1 );

	return err;
}

gen_FileError gen_file_open_mode( gen_FileInfo* f, gen_FileMode mode, char const* filename )
{
	gen_FileInfo file_ = { 0 };
	*f                 = file_;
	gen_FileError err;
#if defined( GEN_SYSTEM_WINDOWS ) || defined( GEN_SYSTEM_CYGWIN )
	err = gen__win32_file_open( &f->fd, &f->ops, mode, filename );
#else
	err = gen__posix_file_open( &f->fd, &f->ops, mode, filename );
#endif
	if ( err == gen_EFileError_NONE )
		return gen_file_new( f, f->fd, f->ops, filename );
	return err;
}

internal void gen__dirinfo_free_entry( gen_DirEntry* entry );

gen_FileError gen_file_close( gen_FileInfo* f )
{
	if ( ! f )
		return gen_EFileError_INVALID;

	if ( f->filename )
		gen_free( gen_heap_allocator(), zpl_cast( char* ) f->filename );

#if defined( GEN_SYSTEM_WINDOWS )
	if ( f->fd.p == INVALID_HANDLE_VALUE )
		return gen_EFileError_INVALID;
#else
	if ( f->fd.i < 0 )
		return gen_EFileError_INVALID;
#endif

	if ( f->is_temp )
	{
		f->ops.close( f->fd );
		return gen_EFileError_NONE;
	}

	if ( ! f->ops.read_at )
		f->ops = gen_default_file_operations;
	f->ops.close( f->fd );

	if ( f->dir )
	{
		gen__dirinfo_free_entry( f->dir );
		gen_mfree( f->dir );
		f->dir = NULL;
	}

	return gen_EFileError_NONE;
}

gen_FileError gen_file_create( gen_FileInfo* f, char const* filename )
{
	return gen_file_open_mode( f, gen_EFileMode_WRITE | gen_EFileMode_RW, filename );
}

gen_FileError gen_file_open( gen_FileInfo* f, char const* filename )
{
	return gen_file_open_mode( f, gen_EFileMode_READ, filename );
}

char const* gen_file_name( gen_FileInfo* f )
{
	return f->filename ? f->filename : "";
}

gen_b32 gen_file_has_changed( gen_FileInfo* f )
{
	if ( f->is_temp )
		return false;
	gen_b32      result          = false;
	gen_FileTime last_write_time = gen_fs_last_write_time( f->filename );
	if ( f->last_write_time != last_write_time )
	{
		result             = true;
		f->last_write_time = last_write_time;
	}
	return result;
}

// TODO: Is this a bad idea?
global gen_b32      gen__std_file_set                         = false;
global gen_FileInfo gen__std_files[ gen_EFileStandard_COUNT ] = { { 0 } };

#if defined( GEN_SYSTEM_WINDOWS ) || defined( GEN_SYSTEM_CYGWIN )

gen_FileInfo* gen_file_get_standard( gen_FileStandardType std )
{
	if ( ! gen__std_file_set )
	{
#	define GEN__SET_STD_FILE( type, v ) \
		gen__std_files[ type ].fd.p = v; \
		gen__std_files[ type ].ops  = gen_default_file_operations
		GEN__SET_STD_FILE( gen_EFileStandard_INPUT, GetStdHandle( STD_INPUT_HANDLE ) );
		GEN__SET_STD_FILE( gen_EFileStandard_OUTPUT, GetStdHandle( STD_OUTPUT_HANDLE ) );
		GEN__SET_STD_FILE( gen_EFileStandard_ERROR, GetStdHandle( STD_ERROR_HANDLE ) );
#	undef GEN__SET_STD_FILE
		gen__std_file_set = true;
	}
	return &gen__std_files[ std ];
}

void gen_file_connect_handle( gen_FileInfo* file, void* handle )
{
	GEN_ASSERT_NOT_NULL( file );
	GEN_ASSERT_NOT_NULL( handle );

	if ( file->is_temp )
		return;

	gen_zero_item( file );

	file->fd.p = handle;
	file->ops  = gen_default_file_operations;
}

gen_FileError gen_file_truncate( gen_FileInfo* f, gen_s64 size )
{
	gen_FileError err         = gen_EFileError_NONE;
	gen_s64       prev_offset = gen_file_tell( f );
	gen_file_seek( f, size );
	if ( ! SetEndOfFile( f ) )
		err = gen_EFileError_TRUNCATION_FAILURE;
	gen_file_seek( f, prev_offset );
	return err;
}

gen_b32 gen_fs_exists( char const* name )
{
	WIN32_FIND_DATAW  data;
	wchar_t*          w_text;
	void*             handle;
	gen_b32           found = false;
	gen_AllocatorInfo a     = gen_heap_allocator();

	w_text = gen__alloc_utf8_to_ucs2( a, name, NULL );
	if ( w_text == NULL )
	{
		return false;
	}
	handle = FindFirstFileW( w_text, &data );
	gen_free( a, w_text );
	found = handle != INVALID_HANDLE_VALUE;
	if ( found )
		FindClose( handle );
	return found;
}

#else    // POSIX

gen_FileInfo* gen_file_get_standard( gen_FileStandardType std )
{
	if ( ! gen__std_file_set )
	{
#	define GEN__SET_STD_FILE( type, v ) \
		gen__std_files[ type ].fd.i = v; \
		gen__std_files[ type ].ops  = gen_default_file_operations
		GEN__SET_STD_FILE( gen_EFileStandard_INPUT, 0 );
		GEN__SET_STD_FILE( gen_EFileStandard_OUTPUT, 1 );
		GEN__SET_STD_FILE( gen_EFileStandard_ERROR, 2 );
#	undef GEN__SET_STD_FILE
		gen__std_file_set = true;
	}
	return &gen__std_files[ std ];
}

gen_FileError gen_file_truncate( gen_FileInfo* f, gen_s64 size )
{
	gen_FileError err = gen_EFileError_NONE;
	int           i   = ftruncate( f->fd.i, size );
	if ( i != 0 )
		err = gen_EFileError_TRUNCATION_FAILURE;
	return err;
}

gen_b32 gen_fs_exists( char const* name )
{
	return access( name, F_OK ) != -1;
}

#endif

gen_s64 gen_file_size( gen_FileInfo* f )
{
	gen_s64 size        = 0;
	gen_s64 prev_offset = gen_file_tell( f );
	gen_file_seek_to_end( f );
	size = gen_file_tell( f );
	gen_file_seek( f, prev_offset );
	return size;
}

gen_FileError gen_file_temp( gen_FileInfo* file )
{
	gen_zero_item( file );
	FILE* fd = NULL;

#if ( defined( GEN_SYSTEM_WINDOWS ) && ! defined( GEN_SYSTEM_TINYC ) ) && ! defined( GEN_COMPILER_GCC )
	errno_t errcode = tmpfile_s( &fd );

	if ( errcode != 0 )
	{
		fd = NULL;
	}
#else
	fd         = tmpfile();
#endif

	if ( fd == NULL )
	{
		return gen_EFileError_INVALID;
	}

#if defined( GEN_SYSTEM_WINDOWS ) && ! defined( GEN_COMPILER_GCC )
	file->fd.i = _get_osfhandle( _fileno( fd ) );
#else
	file->fd.i = fileno( fd );
#endif
	file->ops     = gen_default_file_operations;
	file->is_temp = true;
	return gen_EFileError_NONE;
}

gen_FileContents gen_file_read_contents( gen_AllocatorInfo a, gen_b32 zero_terminate, char const* filepath )
{
	gen_FileContents result = { 0 };
	gen_FileInfo     file   = { 0 };

	result.allocator = a;

	if ( gen_file_open( &file, filepath ) == gen_EFileError_NONE )
	{
		gen_sw fsize = zpl_cast( gen_sw ) gen_file_size( &file );
		if ( fsize > 0 )
		{
			result.data = gen_alloc( a, zero_terminate ? fsize + 1 : fsize );
			result.size = fsize;
			gen_file_read_at( &file, result.data, result.size, 0 );
			if ( zero_terminate )
			{
				gen_u8* str  = zpl_cast( gen_u8* ) result.data;
				str[ fsize ] = '\0';
			}
		}
		gen_file_close( &file );
	}

	return result;
}

void gen_file_free_contents( gen_FileContents* fc )
{
	GEN_ASSERT_NOT_NULL( fc->data );
	gen_free( fc->allocator, fc->data );
	fc->data = NULL;
	fc->size = 0;
}

gen_b32 gen_file_write_contents( char const* filepath, void const* buffer, gen_sw size, gen_FileError* err )
{
	gen_FileInfo  f = { 0 };
	gen_FileError open_err;
	gen_b32       write_ok;
	open_err = gen_file_open_mode( &f, gen_EFileMode_WRITE, filepath );

	if ( open_err != gen_EFileError_NONE )
	{
		if ( err )
			*err = open_err;

		return false;
	}

	write_ok = gen_file_write( &f, buffer, size );
	gen_file_close( &f );
	return write_ok;
}

char* gen_file_read_lines( gen_AllocatorInfo allocator, gen_Array( char* ) * lines, char const* filename, gen_b32 strip_whitespace )
{
	gen_FileInfo f = { 0 };
	gen_file_open( &f, filename );
	gen_sw fsize = ( gen_sw )gen_file_size( &f );

	char* contents = ( char* )gen_alloc( allocator, fsize + 1 );
	gen_file_read( &f, contents, fsize );
	contents[ fsize ] = 0;
	*lines            = gen_str_split_lines( allocator, contents, strip_whitespace );
	gen_file_close( &f );

	return contents;
}

#if ! defined( _WINDOWS_ ) && defined( GEN_SYSTEM_WINDOWS )
GEN_IMPORT DWORD WINAPI GetFullPathNameA( char const* lpFileName, DWORD nBufferLength, char* lpBuffer, char** lpFilePart );
GEN_IMPORT DWORD WINAPI GetFullPathNameW( wchar_t const* lpFileName, DWORD nBufferLength, wchar_t* lpBuffer, wchar_t** lpFilePart );
#endif

#pragma endregion Filesystem

