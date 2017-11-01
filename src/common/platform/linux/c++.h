#ifndef __XGC_CPLUSPLUS_H__
#define __XGC_CPLUSPLUS_H__

#define __STDC_FORMAT_MACROS
#include <malloc.h>
#include <unistd.h>

#define XGC_NEW new
#define XGC_INLINE __inline
#define XGC_DECLSPEC_THREAD __thread
#define XGC_CDECL_CALL 

#define __cdecl

typedef int errno_t;

#if (__GNUC__ == 4 && __GNUC_MINOR__ < 6)
#	define noexcept		//no support of noexcept until GCC 4.6
#endif

#define _TRUNCATE (-1)

XGC_INLINE int vsprintf_s( char *const buffer, size_t size, const char * format, va_list args )
{
	return vsnprintf( buffer, size, format, args );
}

template< size_t size >
XGC_INLINE int vsprintf_s( char (&buffer)[size], const char * format, va_list args )
{
	return vsnprintf( buffer, size, format, args );
}

XGC_INLINE int vfprintf_s( FILE *fp, const char * format, va_list args )
{
	return vfprintf( fp, format, args );
}

XGC_INLINE int vsnprintf_s( char * buffer, size_t size, size_t count, const char * format, va_list args )
{
	if( count == -1 || count > size )
		count = size;

	return vsnprintf( buffer, count, format, args );
}

template< size_t size >
XGC_INLINE int vsnprintf_s( char (&buffer)[size], size_t count, const char * format, va_list args )
{
	if( count == -1 || count > size )
		count = size;

	return vsnprintf( buffer, count, format, args );
}

XGC_INLINE int sprintf_s( char * buffer, size_t size, const char * format, ... )
{
	va_list args;
	va_start( args, format );
	int write = vsnprintf( buffer, size, format, args );
	va_end( args );
	return write;
}

template< size_t size >
XGC_INLINE int sprintf_s( char (&buffer)[size], const char * format, ... )
{
	va_list args;
	va_start( args, format );
	int write = vsnprintf( buffer, size, format, args );
	va_end( args );
	return write;
}

XGC_INLINE int snprintf_s( char * buffer, size_t size, size_t count, const char * format, ... )
{
	if( count == -1 || count > size )
		count = size;

	va_list args;
	va_start( args, format );
	int write = vsnprintf( buffer, count, format, args );
	va_end( args );
	return write;
}

template< size_t size >
XGC_INLINE int snprintf_s( char (&buffer)[size], size_t count, const char * format, ... )
{
	if( count == -1 || count > size )
		count = size;

	va_list args;
	va_start( args, format );
	int write = vsnprintf( buffer, count, format, args );
	va_end( args );
	return write;
}

// strcpy safe version
errno_t strcpy_s( char *destination, size_t count, const char *source );

// strcpy template safe version
template< size_t count >
XGC_INLINE errno_t strcpy_s( char (&destnation)[count], const char *source )
{
	return strcpy_s( destnation, count, source );
}

// strncpy safe version
errno_t strncpy_s( char *destination, size_t count, const char *source, size_t copied );

// strncpy template safe version
template< size_t count >
XGC_INLINE errno_t strncpy_s( char (&destnation)[count], const char *source, size_t copied )
{
	return strncpy_s( destnation, count, source, copied );
}

// string connection safe version
errno_t strcat_s( char *destination, size_t count, const char *source );

// string connection template safe version
template< size_t count >
errno_t strcat_s( char (&destnation)[count], const char *source )
{
	return strcat_s( destnation, count, source );
}

///////////////////////////////////////////////////////////////////////////
/// file operator
///////////////////////////////////////////////////////////////////////////
XGC_INLINE errno_t fopen_s( FILE **fp, const char *filename, const char *mode )
{
	FILE *file = fopen( filename, mode );
	if( file == nullptr )
		return -1;

	*fp = file;
	return 0;
}

// get a char no echo and block
int _getch();

// get a char be echo and block
int _getche();

// string token safe version
char* strtok_s( char* string, char const* control, char** context );

// low level file function
#define _eof 	eof
#define _open 	open
#define _read 	read
#define _write 	write
#define _lseek	lseek
#define _close 	close
#define _access access
#define _tell(fd) _lseek(fd, 0, SEEK_CUR)

#define _fstat fstat
#define _stat stat

#define memsize	malloc_usable_size

#define XGC_ASSERT_MSG(expr, FMT, ...) \
	if( !(expr) ) { fprintf( stderr, "%s:%d:" FMT "\n", __FILE__, __LINE__, ##__VA_ARGS__ ); }

#endif  // __XGC_CPLUSPLUS_H__