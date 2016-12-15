#ifndef _CONFIG_H_
#define _CONFIG_H_

#if defined( WIN32 ) || defined( WIN64 )
#	define _WINDOWS
#elif defined( LINUX32 ) || defined( LINUX64 )
#	define _LINUX
#endif

#ifndef _MESSAGE_TIPS
#	ifdef _LINUX
#		pragma message( "system using linux" )
#	endif
#
#	ifdef _WINDOWS
#		pragma message( "system using windows" )
#	endif
#	ifdef _MSC_VER
#		pragma message( __FILE__ " using visual studio" )
#	elif defined( __GNUC__ )
#		pragma message( "using gnuc" )
#	else
#		pragma message( "using other" )
#	endif
#	define _MESSAGE_TIPS
#endif

#ifdef _WINDOWS
#	define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#	define NOMINMAX
#	include <Windows.h>
#endif

#ifdef _LINUX
#	include <sys/time.h>
#	include <sys/types.h>
#endif // _LINUX


#if defined(_MSC_VER)
#	define _CRTDBG_MAP_ALLOC
#	include <crtdefs.h>
#	include <crtdbg.h>
#	include <io.h>
#endif

#if defined( __GNUC__ )
#	define __STDC_FORMAT_MACROS
#	include <linux/limits.h>
#	include <malloc.h>
#	include <unistd.h>
#endif

#if defined(__GNUC__)
#	define __cdecl
#endif

#if defined(__GNUC__)
#	define _MAX_PATH 	PATH_MAX
#	define _MAX_FNAME 	NAME_MAX
#endif


#if defined(_MSC_VER)
#	if (_MSC_VER < 1900)
#		//About std.experimental.filesystem.
#		//Through VC2013 has provided <filesystem>, but all the names are given in namespace std. It's hard to alias these names into std::experimental,
#		//So Nana use nana.filesystem implement instead for VC2013
#
#		//Nana defines some macros for lack of support of keywords
#		define _ALLOW_KEYWORD_MACROS
#
#		define CXX_NO_INLINE_NAMESPACE //no support of C++11 inline namespace until Visual C++ 2015
#		define noexcept		//no support of noexcept until Visual C++ 2015

#		define constexpr const	//no support of constexpr until Visual C++ 2015 ? const ??
#	else
#		undef STD_FILESYSTEM_NOT_SUPPORTED
#	endif
#elif defined(__GNUC__)
#	if (__GNUC__ == 4 && __GNUC_MINOR__ < 6)
#		define noexcept		//no support of noexcept until GCC 4.6
#	endif
#endif

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#if defined( _DEBUG ) && defined(_MSC_VER)
#	define XGC_NEW new ( _NORMAL_BLOCK, __FILE__, __LINE__ )
#else
#	define XGC_NEW new
#endif

#ifdef _MSC_VER
#	define XGC_INLINE inline
#	define XGC_DECLSPEC_THREAD __declspec( thread )
#elif __GNUC__
#	define XGC_INLINE __inline
#	define XGC_DECLSPEC_THREAD __thread
#endif

#if defined(_WINDOWS)
#	define xgc_invalid_handle INVALID_HANDLE_VALUE
#elif defined(_LINUX)
#	define xgc_invalid_handle (-1)
#endif

#ifdef _MSC_VER
#	define XGC_ASSERT_MSG(expr, msg, ...) \
		(void)( ( !!( expr ) ) || \
		( 1 != _CrtDbgReport( _CRT_ASSERT, __FILE__, __LINE__, NULL, msg, ##__VA_ARGS__ ) ) || \
		( _CrtDbgBreak(), 0 ) )
#elif defined( __GNUC__ )
#	define XGC_ASSERT_MSG(expr, msg, ...) \
		if( !expr ) \
		{\
			fprintf( stderr, "%s:%d:" msg "\n", __FILE__, __LINE__, ##__VA_ARGS__ );\
		}
#endif

#ifdef _LINUX
typedef int errno_t;
#endif

///////////////////////////////////////////////////////////
// adpater linux to windows
#ifdef _WINDOWS
#	define strcasecmp _stricmp
#	define strncasecmp _strnicmp
#	define memsize	_msize
#endif

#if defined( __GNUC__ )
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

XGC_INLINE errno_t strcpy_s( char *destination, size_t count, const char *source )
{
	if( nullptr == source )
		return -1;

	size_t i = 0;
	while( i < count - 1 && source[i] )
	{
		destination[i] = source[i];
		++i;
	}

	destination[i] = 0;

	return 0;
}

template< size_t count >
XGC_INLINE errno_t strcpy_s( char (&destnation)[count], const char *source )
{
	return strcpy_s( destnation, count, source );
}

XGC_INLINE errno_t strcat_s( char *destination, size_t count, const char *source )
{
	if( nullptr == source )
		return -1;

	size_t i = strlen(destination);
	size_t p = 0;
	while( i < count - 1 && source[p] )
	{
		destination[i] = source[p];
		++i;
		++p;
	}

	destination[i] = 0;

	return 0;
}

template< size_t count >
XGC_INLINE errno_t strcat_s( char (&destnation)[count], const char *source )
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

// low level file function
#define _open open
#define _read read
#define _write write
#define _close close

#define _fstat fstat
#define _stat stat

#define memsize	malloc_usable_size
#endif //__GNUC__

#endif //_CONFIG_H_