#ifndef _CONFIG_H_
#define _CONFIG_H_

#if defined( WIN32 ) || defined( WIN64 )
#	define WINDOWS
#	define _WINDOWS
#endif

#ifdef _MSC_VER
#	pragma message( __FILE__ " using visual studio" )
#elif defined( __GNUC__ )
#	pragma message( "using gnuc" )
#	define __cdecl
#else
#	pragma message( "using other" )
#endif

#ifdef _WINDOWS
#	define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#	define NOMINMAX
#	include <Windows.h>
#endif

#ifdef _MSC_VER
#	include <crtdefs.h>
#	define _CRTDBG_MAP_ALLOC
#	include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#if defined( _DEBUG ) && defined(_MSC_VER)
#	define XGC_NEW new ( _NORMAL_BLOCK, __FILE__, __LINE__ )
#else
#	define XGC_NEW new
#endif

#ifdef _MSC_VER
#	define XGC_INLINE __inline
#elif __GNUC__
#	define XGC_INLINE inline
#endif

#ifdef _WINDOWS
#	define xgc_invalid_handle INVALID_HANDLE_VALUE
#elif defined( __GNUC__ )
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

///////////////////////////////////////////////////////////
// adpater linux to windows
#ifdef _WINDOWS
#	define strcasecmp _stricmp
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

// low level file function
#define _open open
#define _read read
#define _write write
#define _close close

#define _fstat fstat
#define _stat stat

#endif //__GNUC__

#endif //_CONFIG_H_