#ifndef __CONFIG_H__
#define __CONFIG_H__

#if defined( WIN32 ) || defined( WIN64 )
#	define _WINDOWS
#elif defined( LINUX32 ) || defined( LINUX64 )
#	define _LINUX
#endif

#ifdef _MESSAGE_TIPS
#	ifdef _LINUX
#		pragma message( "system using linux" )
#	endif
#
#	ifdef _WINDOWS
#		pragma message( "system using windows" )
#	endif
#	ifdef _MSC_VER
#		pragma message( "compiler using visual studio" )
#	elif __GNUC__
#		pragma message( "compiler using gnuc" )
#	else
#		pragma message( "compiler using other" )
#	endif
#endif

#if __WORDSIZE == 64
#	define _AI_X32
#elif defined  __x86_64__
#	define _AI_X64
#endif

#ifdef _WINDOWS
#	define _CRTDBG_MAP_ALLOC
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif
#	include "platform/windows/sys.h"
#endif

#ifdef _LINUX
#	include "platform/linux/sys.h"
#endif

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <set>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <deque>
#include <vector>
#include <string>
#include <array>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#include <chrono>
#include <memory>
#include <functional>

#if defined(XGC_ALLOCATOR)
#	include "allocator.h"
#	define xgc_allocator xgc::common::allocator
#	define xgc_allocator_fast xgc::common::allocator
#else
#	define xgc_allocator std::allocator
#	define xgc_allocator_fast std::allocator
#endif

#pragma warning( disable:4251 )

namespace xgc
{
	using string = std::basic_string< char, std::char_traits<char>, xgc_allocator< char > >;

	template<class _Kty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast<_Kty> >
	using set = std::set< _Kty, _Pr, _Alloc >;

	template<class _Ty1, class _Ty2 >
	using pair = std::pair< _Ty1, _Ty2 >;

	template<class _Kty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast<_Kty> >
	using multiset = std::multiset< _Kty, _Pr, _Alloc >;

	template<class _Kty, class _Ty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
	using map = std::map< _Kty, _Ty, _Pr, _Alloc >;

	template<class _Kty, class _Ty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
	using multimap = std::multimap< _Kty, _Ty, _Pr, _Alloc >;

	template<class _Kty, class _Ty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
	using unordered_map = std::unordered_map< _Kty, _Ty, _Hasher, _Keyeq, _Alloc >;

	template<class _Kty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = xgc_allocator_fast< _Kty > >
	using unordered_set = std::unordered_set< _Kty, _Hasher, _Keyeq, _Alloc >;

	template<class _Kty, class _Ty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
	using xgc_unordered_multimap = std::unordered_multimap < _Kty, _Ty, _Hasher, _Keyeq, _Alloc >;

	template<class _Kty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = xgc_allocator_fast< _Kty > >
	using unordered_multiset = std::unordered_multiset < _Kty, _Hasher, _Keyeq, _Alloc >;

	template<class _Ty, class _Ax = xgc_allocator< _Ty > >
	using list = std::list< _Ty, _Ax >;

	template<class _Ty, class _Ax = xgc_allocator< _Ty > >
	using deque = std::deque< _Ty, _Ax >;

	template< class _Ty, class _Ax = xgc_allocator_fast<_Ty>, class _Container = std::deque<_Ty, _Ax> >
	using queue = std::queue< _Ty, _Container >;

	template< class _Ty, class _Ax = xgc_allocator_fast<_Ty>, class _Container = std::deque<_Ty, _Ax> >
	using stack = std::stack< _Ty, _Container >;

	template< class _Ty, class _Ax = xgc_allocator_fast<_Ty> >
	using vector = std::vector< _Ty, _Ax >;

	template < class _Ty, size_t _Size >
	using array = std::array< _Ty, _Size >;

	template < class ... _Types >
	using tuple = std::tuple < _Types... >;
}

#include "exports.h"
///////////////////////////////////////////////////////////
// adpater linux to windows
#define SLASH_ALL "\\/"

#if defined( __GNUC__ )
#	include "platform/linux/gcc.h"
#endif //__GNUC__

#if defined( _MSC_VER )
#	include "platform/windows/msvc.h"
#endif
#endif //__CONFIG_H__