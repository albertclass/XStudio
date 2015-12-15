#ifndef _MACRO_DEFINE_H
#define _MACRO_DEFINE_H

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define NOMINMAX
#include <Windows.h>
#include <crtdefs.h>
#endif

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
#	define XGC_NEW new ( _NORMAL_BLOCK, __FILE__, __LINE__ )
#else
#	define XGC_NEW new
#endif


//-------------------------------------------------------------------------------------------------------------------------------------------------//
// 安全删除定义
//-------------------------------------------------------------------------------------------------------------------------------------------------//
#ifndef SAFE_DELETE
#	define SAFE_DELETE( p )			do{delete(p);(p) = NULL;}while(false);
#endif

#ifndef SAFE_DELETE_ARRAY
#	define SAFE_DELETE_ARRAY( p )	do{delete[] (p); (p) = NULL;}while(false);
#endif 

#ifndef SAFE_RELEASE
#	define SAFE_RELEASE( p )		do{if(p){ (p)->Release(); (p) = NULL; }}while(false);
#endif

#define SUB(x,y) ((x)=(x)>(y)? ((x)-(y)): 0)

//-------------------------------------------------------------------------------------------------------------------------------------------------//

#ifndef XGC_INLINE
#	define XGC_INLINE	__inline
#endif

#define XGC_UNREFERENCED_PARAMETER(P)          (P)

#if _MSC_VER >= 1600
#	define xgc_nullptr nullptr
#else
#	define xgc_nullptr NULL
#endif

#ifndef MAKEWORD
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#endif

#ifndef MAKELONG
#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#endif

#define XGC_MAKESW(a, b) ( ( (xgc_uint16(a) ) & 0xff)       | ( ( xgc_uint16(b) & 0xff ) << 8 ) )
#define XGC_MAKEDW(a, b) ( ( (xgc_uint32(a) ) & 0xffff)     | ( ( xgc_uint32(b) & 0xffff) << 16 ) )
#define XGC_MAKEQW(a, b) ( ( (xgc_uint64(a) ) & 0xffffffff) | ( ( xgc_uint64(b) & 0xffffffff) << 32 ) )

#define XGC_LOWORD(l) ((xgc_uint16)(((xgc_uintptr)(l)) & 0xffff))
#define XGC_HIWORD(l) ((xgc_uint16)((((xgc_uintptr)(l)) >> 16) & 0xffff))
#define XGC_LOBYTE(w) ((xgc_uint8)(((xgc_uintptr)(w)) & 0xff))
#define XGC_HIBYTE(w) ((xgc_uint8)((((xgc_uintptr)(w)) >> 8) & 0xff))
//-------------------------------------------------------------------------------------------------------------------------------------------------//
// 定义基本类型
//-------------------------------------------------------------------------------------------------------------------------------------------------//
#if defined( _UNICODE )

typedef wchar_t					xgc_tchar;

#define INCSZ( pch )			( ++pch )
#define INCNSZ( pch, count )	( pch+=count )
#define CMPSZ( pch1, pch2 )		( pch1 == pch2 )
#define EQUSZ( pch1, pch2 )		( *pch1 = *pch2 )
#define DEFCH( ch )				( ch )

#	ifndef _T
#		define _T(x)			L( x )
#	endif
#else

typedef char					xgc_tchar;

#define INCSZ( pch )			( pch = _mbsinc( pch ) )
#define INCNSZ( pch, count )	( pch = _mbsninc( pch, count ) )
#define CMPSZ( pch1, pch2 )		( _mbccpy( pch1, ( xgc_lpcwstr )pch2 ) )
#define EQUSZ( pch1, pch2 )		( _mbccmp( pch1, pch2 ) == 0 )
#define DEFCH( ch )				( ( xgc_bytecptr )&ch[0] )

#	ifndef _T
#		define _T(x)			( x )
#	endif
#endif

#if !defined(xgc_ia64)
#	if !defined(__midl) && (defined(_X86_) || defined(_MIX86)) && _MSC_VER >= 1300
#		define xgc_ia64 __w64
#	else
#		define	xgc_ia64
#	endif
#endif

#ifndef _NO_DEFINES
#define _NO_DEFINES
typedef void				xgc_void;
typedef bool				xgc_bool;
typedef char				xgc_char;
typedef wchar_t				xgc_wchar;
typedef char				xgc_int8;
typedef short				xgc_int16;
typedef int					xgc_int32;
typedef __int64				xgc_int64;
typedef long				xgc_long;
typedef float				xgc_real32;
typedef double				xgc_real64;

typedef unsigned char		xgc_byte;
typedef unsigned short		xgc_word;
typedef unsigned int		xgc_quat;

typedef unsigned char		xgc_uint8;
typedef unsigned short		xgc_uint16;
typedef unsigned int		xgc_uint32;
typedef unsigned long		xgc_ulong;
typedef unsigned __int64	xgc_uint64;

typedef const xgc_tchar*	xgc_lpctstr;
typedef xgc_tchar*			xgc_lptstr;

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
typedef xgc_lpvoid			xgc_handle;
#endif // _DEFINES

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// XGC_ASSERT 定义
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------//
#define XGC_STATIC_ASSERT(expr) \
	do { xgc_char _static_assert[expr] = {0}; } while( false )

#define XGC_ASSERT_MSGW(expr, msg, ...) \
	(void)( ( !!( expr ) ) || \
	( 1 != _CrtDbgReportW( _CRT_ASSERT, _CRT_WIDE( __FILE__ ), __LINE__, NULL, L##msg, __VA_ARGS__ ) ) || \
	( _CrtDbgBreak(), 0 ) )

#define XGC_ASSERT_MSGA(expr, msg, ...) \
	(void)( ( !!( expr ) ) || \
	( 1 != _CrtDbgReport( _CRT_ASSERT, __FILE__, __LINE__, NULL, msg, __VA_ARGS__ ) ) || \
	( _CrtDbgBreak(), 0 ) )

#if defined(_UNICODE) || defined(UNICODE)
#	define XGC_ASSERT_TMSG	XGC_ASSERT_MSGW
#else
#	define XGC_ASSERT_TMSG	XGC_ASSERT_MSGA
#endif

// static warning
#define attention( describe ) message( __FILE__ "(" XGC_TOSTRING(__LINE__) "): 注意：" describe )

// 使用举例
// #pragma attention( "test todo" )

#include "logger.h"

#ifdef _DEBUG
#	define XGC_ASSERT(expr)						XGC_ASSERT_TMSG(expr,#expr,)
#	define XGC_ASSERT_RETURN(expr,ret,...)		if(!(expr)){ XGC_ASSERT_TMSG(false,#expr##__VA_ARGS__); return ret; }
#	define XGC_ASSERT_THROW(expr,ret,...)		if(!(expr)){ XGC_ASSERT_TMSG(false,#expr##__VA_ARGS__); throw( ret ); }
#	define XGC_ASSERT_BREAK(expr,...)			if(!(expr)){ XGC_ASSERT_TMSG(false,#expr##__VA_ARGS__); break; }
#	define XGC_ASSERT_CONTINUE(expr,...)		if(!(expr)){ XGC_ASSERT_TMSG(false,#expr##__VA_ARGS__); continue; }
#	define XGC_ASSERT_RELEASE(expr,msg,...)		if(!(expr)){ XGC_ASSERT_TMSG(false,#expr##__VA_ARGS__); }else{ (expr)->Release(); }
#	define XGC_ASSERT_MESSAGE(expr,msg,...)		if(!(expr)){ XGC_ASSERT_TMSG(false,msg,__VA_ARGS__); }
#	define XGC_DEBUG_MESSAGE(msg,...)			XGC_ASSERT_TMSG(false,msg,__VA_ARGS__)
#	define XGC_ASSERT_POINTER(expr)				XGC_ASSERT_TMSG(expr,"NULL POINT FOUND, IS'T RIGHT?")
#	define XGC_VERIFY(expr)						XGC_ASSERT_TMSG(expr,"")
#	define XGC_DEBUG_CODE( ... )				__VA_ARGS__
#elif defined( _ASSERT_LOG )
#	define XGC_ASSERT(expr)						if(!(expr)){ DBG_WARNING( "ASSERT ""%s", #expr ); }
#	define XGC_ASSERT_RETURN(expr,ret,...)		if(!(expr)){ DBG_WARNING( "ASSERT "#expr##__VA_ARGS__ ); return ret; }
#	define XGC_ASSERT_THROW(expr,ret,...)		if(!(expr)){ DBG_WARNING( "ASSERT "#expr##__VA_ARGS__ ); throw( ret ); }
#	define XGC_ASSERT_BREAK(expr,...)			if(!(expr)){ DBG_WARNING( "ASSERT "#expr##__VA_ARGS__ ); break; }
#	define XGC_ASSERT_CONTINUE(expr,...)		if(!(expr)){ DBG_WARNING( "ASSERT "#expr##__VA_ARGS__ ); continue; }
#	define XGC_ASSERT_RELEASE(expr,msg,...)		if(!(expr)){ DBG_WARNING( "ASSERT "#expr##__VA_ARGS__ ); }else{(expr)->Release();}
#	define XGC_ASSERT_MESSAGE(expr,msg,...)		if(!(expr)){ DBG_WARNING( "ASSERT "msg, __VA_ARGS__ ); }
#	define XGC_DEBUG_MESSAGE(msg,...)			if(!(true)){ DBG_WARNING( "ASSERT "msg, __VA_ARGS__ ); }
#	define XGC_ASSERT_POINTER(expr)				if(!(expr)){ DBG_WARNING( "ASSERT ""%s", #expr ); }
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
#define XGC_BIT(w)			(1<<(w))

#define XGC_CHKBIT(v,w)		(XGC_BIT(w)&(v))
#define XGC_GETBIT(v,w)		((XGC_BIT(w)&(v))>>(w))
#define XGC_SETBIT(v,w)		(XGC_BIT(w)|(v))
#define XGC_CLRBIT(v,w)		((~XGC_BIT(w))&(v))

#define XGC_CHKMASK(v,mask)	((v)&(mask))
#define XGC_GETMASK(v,mask)	(((v)&(mask))>>(w))
#define XGC_SETMASK(v,mask)	((v)|(mask))
#define XGC_CLRMASK(v,mask)	((v)&(~(mask)))

#define XGC_ALIGNMENT(size, alignment) (((size) + (alignment) - 1) & ~((alignment)-1))

#define XGC_COUNTOF _countof
#define xgc_countof _countof

#define XGC_CHECK_ARRAY_INDEX( ARRAY_NAME, ARRAY_INDEX ) (ARRAY_INDEX < XGC_COUNTOF(ARRAY_NAME) && ARRAY_INDEX >= 0)

#define XGC_MIN( a, b )	((a)>(b)?(b):(a))
#define XGC_MAX( a, b )	((a)<(b)?(b):(a))
#define XGC_RNG( a, b, c ) ((a)<(b)?(b):(a)>(c)?(c):(a))
//-------------------------------------------------------------------------------------------------------------------------------------------------//
#define XGC_NONE xgc_void( 0 )

#define HeaderFromMember( TYPE, MEMBER_POINT, MEMBER_NAME ) ((TYPE *)( MEMBER_POINT - (((TYPE *)0)->MEMBER_NAME) ))

const static xgc_uint8 XCB_MAX_HASH_ID = 10;

#endif //_MACRO_DEFINE_H