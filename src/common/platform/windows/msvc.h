#ifndef __XGC_CPLUSPLUS_H__
#define __XGC_CPLUSPLUS_H__

#define _CRTDBG_MAP_ALLOC
#include <crtdefs.h>
#include <crtdbg.h>
#include <io.h>

#if defined(_DEBUG)
#	define XGC_NEW new ( _NORMAL_BLOCK, __FILE__, __LINE__ )
#else
#	define XGC_NEW new
#endif

#define XGC_INLINE inline
#define XGC_DECLSPEC_THREAD __declspec( thread )
#define XGC_CDECL_CALL __cdecl

#if (_MSC_VER < 1900)
#	//About std.experimental.filesystem.
#	//Through VC2013 has provided <filesystem>, but all the names are given in namespace std. It's hard to alias these names into std::experimental,
#	//So Nana use nana.filesystem implement instead for VC2013
#
#	//Nana defines some macros for lack of support of keywords
#	define _ALLOW_KEYWORD_MACROS
#
#	define CXX_NO_INLINE_NAMESPACE //no support of C++11 inline namespace until Visual C++ 2015
#	define noexcept		//no support of noexcept until Visual C++ 2015

#	define constexpr const	//no support of constexpr until Visual C++ 2015 ? const ??
#endif

#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define memsize	_msize
#define snprintf_s _snprintf_s
#define vsnprintf_s _vsnprintf_s

#define XGC_ASSERT_MSG(expr, msg, ...) \
	(void)( ( !!( expr ) ) || \
	( 1 != _CrtDbgReport( _CRT_ASSERT, __FILE__, __LINE__, NULL, msg, ##__VA_ARGS__ ) ) || \
	( _CrtDbgBreak(), 0 ) )


#endif // __XGC_CPLUSPLUS_H__