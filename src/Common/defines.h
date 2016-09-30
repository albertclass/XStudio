#ifndef _MACRO_DEFINE_H
#define _MACRO_DEFINE_H

#include "config.h"

//-------------------------------------------------------------------------------------------------------------------------------------------------//
// 安全删除定义
//-------------------------------------------------------------------------------------------------------------------------------------------------//
#ifndef SAFE_DELETE
#	define SAFE_DELETE( p )			do{delete(p);(p) = NULL;}while(false);
#endif

#ifndef SAFE_DELETE_TYPE
#	define SAFE_DELETE_TYPE( p, t )	do{delete((t*)p);(p) = NULL;}while(false);
#endif

#ifndef SAFE_DELETE_ARRAY
#	define SAFE_DELETE_ARRAY( p )	do{delete[] (p); (p) = NULL;}while(false);
#endif

#ifndef SAFE_RELEASE
#	define SAFE_RELEASE( p )		do{if(p){ (p)->Release(); (p) = NULL; }}while(false);
#endif

#define SUB(x,y) ((x)=(x)>(y)? ((x)-(y)): 0)

//-------------------------------------------------------------------------------------------------------------------------------------------------//
#define XGC_UNREFERENCED_PARAMETER(P) (P)

#if defined( _MSC_VER )
#	if _MSC_VER >= 1600
#		define xgc_nullptr nullptr
#	else
#		define xgc_nullptr NULL
#	endif
#elif defined( __GNUC__ )
#	if __GNUC__ >= 4
#		define xgc_nullptr nullptr
#	else
#		define xgc_nullptr NULL
#	endif
#endif

#define XGC_MAKESW(a, b) ( ( (xgc_uint16(a) ) & 0xff)       | ( ( xgc_uint16(b) & 0xff ) << 8 ) )
#define XGC_MAKEDW(a, b) ( ( (xgc_uint32(a) ) & 0xffff)     | ( ( xgc_uint32(b) & 0xffff) << 16 ) )
#define XGC_MAKEQW(a, b) ( ( (xgc_uint64(a) ) & 0xffffffff) | ( ( xgc_uint64(b) & 0xffffffff) << 32 ) )

#define XGC_LOWORD(l) ((xgc_uint16)(((xgc_uintptr)(l)) & 0xffff))
#define XGC_HIWORD(l) ((xgc_uint16)((((xgc_uintptr)(l)) >> 16) & 0xffff))
#define XGC_LOBYTE(w) ((xgc_uint8)(((xgc_uintptr)(w)) & 0xff))
#define XGC_HIBYTE(w) ((xgc_uint8)((((xgc_uintptr)(w)) >> 8) & 0xff))

//-------------------------------------------------------------------------------------------------------------------------------------------------//
// 定义字符串处理宏
//-------------------------------------------------------------------------------------------------------------------------------------------------//
#define INCSZ( pch )			( pch = (xgc_lpstr)_mbsinc( (xgc_byte const*)pch ) )
#define INCNSZ( pch, count )	( pch = _mbsninc( pch, count ) )
#define CMPSZ( pch1, pch2 )		( _mbccmp( pch1, ( xgc_lpcwstr )pch2 ) == 0 )
#define CPYSZ( pch1, pch2 )		( _mbccpy( pch1, pch2 ) )
#define DEFCH( ch )				( ( xgc_bytecptr )&ch[0] )

typedef void				xgc_void;
typedef bool				xgc_bool;
typedef char				xgc_char;
typedef wchar_t				xgc_wchar;
typedef char				xgc_int8;
typedef short				xgc_int16;
typedef int					xgc_int32;
typedef int64_t				xgc_int64;
typedef long				xgc_long;
typedef float				xgc_real32;
typedef double				xgc_real64;

typedef unsigned char		xgc_uint8;
typedef unsigned short		xgc_uint16;
typedef unsigned int		xgc_uint32;
typedef unsigned long		xgc_ulong;
typedef uint64_t			xgc_uint64;

typedef xgc_uint8			xgc_byte;
typedef xgc_uint16			xgc_word;
typedef xgc_uint32			xgc_quat;

typedef const xgc_char*		xgc_lpcstr;
typedef xgc_char*			xgc_lpstr;

typedef const xgc_wchar*	xgc_lpcwstr;
typedef xgc_wchar*			xgc_lpwstr;

typedef const xgc_byte*		xgc_bytecptr;
typedef xgc_byte*			xgc_byteptr;

typedef intptr_t			xgc_intptr;
typedef uintptr_t	 		xgc_uintptr;

typedef const void			*xgc_lpcvoid;
typedef void				*xgc_lpvoid;

typedef size_t				xgc_size;
typedef xgc_uint32			xgc_time32;
typedef xgc_uint64			xgc_time64;
typedef time_t				xgc_time_t;
typedef xgc_lpvoid			xgc_handle;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// XGC_ASSERT 定义
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------//
#define XGC_STATIC_ASSERT(expr) \
	do { xgc_char _static_assert[expr] = {0}; } while( false )

// static warning
#define attention( describe ) message( __FILE__ "(" XGC_TOSTRING(__LINE__) "): 注意：" describe )

// 使用举例
// #pragma attention( "test todo" )

#ifdef _DEBUG
#	define XGC_ASSERT(expr)						XGC_ASSERT_MSG(expr,#expr,0)
#	define XGC_ASSERT_RETURN(expr,ret,...)		if(!(expr)){ XGC_ASSERT_MSG(false,#expr##__VA_ARGS__); return ret; }
#	define XGC_ASSERT_THROW(expr,ret,...)		if(!(expr)){ XGC_ASSERT_MSG(false,#expr##__VA_ARGS__); throw( ret ); }
#	define XGC_ASSERT_BREAK(expr,...)			if(!(expr)){ XGC_ASSERT_MSG(false,#expr##__VA_ARGS__); break; }
#	define XGC_ASSERT_CONTINUE(expr,...)		if(!(expr)){ XGC_ASSERT_MSG(false,#expr##__VA_ARGS__); continue; }
#	define XGC_ASSERT_RELEASE(expr,msg,...)		if(!(expr)){ XGC_ASSERT_MSG(false,#expr##__VA_ARGS__); }else{ (expr)->Release(); }
#	define XGC_ASSERT_MESSAGE(expr,msg,...)		if(!(expr)){ XGC_ASSERT_MSG(false,msg,__VA_ARGS__); }
#	define XGC_DEBUG_MESSAGE(msg,...)			XGC_ASSERT_MSG(false,msg,__VA_ARGS__)
#	define XGC_ASSERT_POINTER(expr)				XGC_ASSERT_MSG(expr,"NULL POINT FOUND, IS'T RIGHT?")
#	define XGC_VERIFY(expr)						XGC_ASSERT_MSG(expr,"")
#	define XGC_DEBUG_CODE( ... )				__VA_ARGS__
#elif defined( _ASSERT_LOG )
#	define XGC_ASSERT(expr)						if(!(expr)){ DBG_WARNING( "ASSERT " "%s", #expr ); }
#	define XGC_ASSERT_RETURN(expr,ret,...)		if(!(expr)){ DBG_WARNING( "ASSERT " #expr##__VA_ARGS__ ); return ret; }
#	define XGC_ASSERT_THROW(expr,ret,...)		if(!(expr)){ DBG_WARNING( "ASSERT " #expr##__VA_ARGS__ ); throw( ret ); }
#	define XGC_ASSERT_BREAK(expr,...)			if(!(expr)){ DBG_WARNING( "ASSERT " #expr##__VA_ARGS__ ); break; }
#	define XGC_ASSERT_CONTINUE(expr,...)		if(!(expr)){ DBG_WARNING( "ASSERT " #expr##__VA_ARGS__ ); continue; }
#	define XGC_ASSERT_RELEASE(expr,msg,...)		if(!(expr)){ DBG_WARNING( "ASSERT " #expr##__VA_ARGS__ ); }else{(expr)->Release();}
#	define XGC_ASSERT_MESSAGE(expr,msg,...)		if(!(expr)){ DBG_WARNING( "ASSERT " msg, __VA_ARGS__ ); }
#	define XGC_DEBUG_MESSAGE(msg,...)			if(!(true)){ DBG_WARNING( "ASSERT " msg, __VA_ARGS__ ); }
#	define XGC_ASSERT_POINTER(expr)				if(!(expr)){ DBG_WARNING( "ASSERT " "%s", #expr ); }
#	define XGC_VERIFY(expr)						XGC_ASSERT(expr)
#	define XGC_DEBUG_CODE( ... )
#else
#	define XGC_ASSERT(expr)						(void)(expr)
#	define XGC_ASSERT_RETURN(expr,ret,...)		if(!(expr)){return ret;}
#	define XGC_ASSERT_THROW(expr,ret,...)		if(!(expr)){throw( ret );}
#	define XGC_ASSERT_BREAK(expr,...)			if(!(expr)){break;}
#	define XGC_ASSERT_CONTINUE(expr,...)		if(!(expr)){continue;}
#	define XGC_ASSERT_RELEASE(expr,msg,...)		if( (expr)){(expr)->Release();}
#	define XGC_ASSERT_MESSAGE(expr,msg,...)		(void)(expr)
#	define XGC_DEBUG_MESSAGE(msg,...)			(void)(0)
#	define XGC_ASSERT_POINTER(expr)				(void)(expr)
#	define XGC_VERIFY(expr)						(void)(expr)
#	define XGC_DEBUG_CODE( ... )
#endif

//-------------------------------------------------------------------------------------------------------------------------------------------------//
// helper macros define
#define XGC_BIT(w)			(1ULL<<(w))

#define XGC_CHKBIT(v,bit_idx)	(((v)&XGC_BIT(bit_idx))!=0)
#define XGC_GETBIT(v,bit_idx)	(((v)&XGC_BIT(bit_idx))>>(bit_idx))
#define XGC_SETBIT(v,bit_idx)	(((v)|XGC_BIT(bit_idx)))
#define XGC_CLRBIT(v,bit_idx)	(((v)&(~XGC_BIT(bit_idxw))))

#define XGC_CHK_FLAGS(v,flags)	(((v)&(flags))==flags)
#define XGC_ADD_FLAGS(v,flags)	(((v)|(flags)))
#define XGC_CLR_FLAGS(v,flags)	(((v)&(~(flags))))

#define XGC_ALIGNOF(_Size, _Alignment) (((_Size) + (_Alignment) - 1) & ~((_Alignment)-1))
#define XGC_COUNTOF(_Array) (sizeof(_Array) / sizeof(_Array[0]))

#define XGC_CHECK_ARRAY_INDEX( _Array, _Index ) (_Index < XGC_COUNTOF(_Array) && _Index >= 0)

#define XGC_MIN( a, b )	((a)>(b)?(b):(a))
#define XGC_MAX( a, b )	((a)<(b)?(b):(a))
#define XGC_RNG( value, lower, upper ) ((value)<(lower)?(lower):(value)>(upper)?(upper):(value))
//-------------------------------------------------------------------------------------------------------------------------------------------------//
#define XGC_NONE xgc_void( 0 )

#define XGC_HEADER_FROM_MEMBER( TYPE, MEMBER_POINT, MEMBER_NAME ) ((TYPE *)( MEMBER_POINT - (((TYPE *)0)->MEMBER_NAME) ))

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

#if defined(XGC_ALLOCATOR)
#	include "allocator.h"
#	define xgc_allocator xgc::common::allocator
#	define xgc_allocator_fast xgc::common::allocator
#elif defined(BOOST_ALLOCATOR)
#	include "boost/pool/pool_alloc.hpp"
#	include "boost/pool/singleton_pool.hpp"
#	include "boost/pool/detail/mutex.hpp"
#	define xgc_allocator boost::pool_allocator
#	define xgc_allocator_fast boost::fast_pool_allocator
#else
#	define xgc_allocator std::allocator
#	define xgc_allocator_fast std::allocator
#endif

#pragma warning( disable:4251 )
using xgc_string = std::basic_string< char, std::char_traits<char>, xgc_allocator< char > >;

template<class _Kty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast<_Kty> >
using xgc_set = std::set< _Kty, _Pr, _Alloc >;

template<class _Ty1, class _Ty2 >
using xgc_pair = std::pair< _Ty1, _Ty2 >;

template<class _Kty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast<_Kty> >
using xgc_multiset = std::multiset< _Kty, _Pr, _Alloc >;

template<class _Kty, class _Ty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
using xgc_map = std::map< _Kty, _Ty, _Pr, _Alloc >;

template<class _Kty, class _Ty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
using xgc_multimap = std::multimap< _Kty, _Ty, _Pr, _Alloc >;

template<class _Kty, class _Ty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
using xgc_unordered_map = std::unordered_map< _Kty, _Ty, _Hasher, _Keyeq, _Alloc >;

template<class _Kty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = xgc_allocator_fast< _Kty > >
using xgc_unordered_set = std::unordered_set< _Kty, _Hasher, _Keyeq, _Alloc > ;

template<class _Kty, class _Ty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
using xgc_unordered_multimap = std::unordered_multimap < _Kty, _Ty, _Hasher, _Keyeq, _Alloc > ;

template<class _Kty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = xgc_allocator_fast< _Kty > >
using xgc_unordered_multiset = std::unordered_multiset < _Kty, _Hasher, _Keyeq, _Alloc > ;

template<class _Ty, class _Ax = xgc_allocator< _Ty > >
using xgc_list = std::list< _Ty, _Ax >;

template<class _Ty, class _Ax = xgc_allocator< _Ty > >
using xgc_deque = std::deque< _Ty, _Ax >;

template< class _Ty, class _Ax = xgc_allocator_fast<_Ty>, class _Container = std::deque<_Ty, _Ax> >
using xgc_queue = std::queue< _Ty, _Container >;

template< class _Ty, class _Ax = xgc_allocator_fast<_Ty>, class _Container = std::deque<_Ty, _Ax> >
using xgc_stack = std::stack< _Ty, _Container >;

template< class _Ty, class _Ax = xgc_allocator_fast<_Ty> >
using xgc_vector = std::vector< _Ty, _Ax >;

template < class _Ty, size_t _Size >
using xgc_array = std::array< _Ty, _Size >;

template < class ... _Types >
using xgc_tuple = std::tuple < _Types... >;

#endif //_MACRO_DEFINE_H