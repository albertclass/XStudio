#ifndef _MACRO_DEFINE_H
#define _MACRO_DEFINE_H

#include "config.h"
#include "commonspp.h"
//-------------------------------------------------------------------------------------------------------------------------------------------------//
// ��ȫɾ������
//-------------------------------------------------------------------------------------------------------------------------------------------------//
#ifndef SAFE_DELETE
#	define SAFE_DELETE( p )			do{delete(p);(p) = NULL;}while(false)
#endif

#ifndef SAFE_DELETE_TYPE
#	define SAFE_DELETE_TYPE( p, t )	do{delete((t*)p);(p) = NULL;}while(false)
#endif

#ifndef SAFE_DELETE_ARRAY
#	define SAFE_DELETE_ARRAY( p )	do{delete[] (p); (p) = NULL;}while(false)
#endif

#ifndef SAFE_RELEASE
#	define SAFE_RELEASE( p )		do{if(p){ (p)->Release(); (p) = NULL; }}while(false)
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
#		define xgc_nullptr __null
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
// �����ַ��������
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
// XGC_ASSERT ����
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------//
#define XGC_STATIC_ASSERT(expr) \
	do { xgc_char _static_assert[expr] = {0}; } while( false )

// static warning
#define attention( describe ) message( __FILE__ "(" XGC_TOSTRING(__LINE__) "): ע�⣺" describe )

// ʹ�þ���
// #pragma attention( "test todo" )
// #define FIRST_HELPER (F, ...)	F
// #define OTHER_HELPER (F, ...)	__VA_ARGS__
// #define FIRST(...)	FIRST_HELPER(__VA_ARGS__, D)
// #define OTHER(...)	OTHER_HELPER(__VA_ARGS__)

#ifdef _DEBUG
#	define XGC_ASSERT(expr)						XGC_ASSERT_MSG(expr,#expr,0)
#	define XGC_ASSERT_RETURN(expr,ret,...)		if(!(expr)){ XGC_ASSERT_MSG(false,#expr __VA_ARGS__); return ret; }
#	define XGC_ASSERT_THROW(expr,ret,...)		if(!(expr)){ XGC_ASSERT_MSG(false,#expr __VA_ARGS__); throw( ret ); }
#	define XGC_ASSERT_BREAK(expr,...)			if(!(expr)){ XGC_ASSERT_MSG(false,#expr __VA_ARGS__); break; }
#	define XGC_ASSERT_CONTINUE(expr,...)		if(!(expr)){ XGC_ASSERT_MSG(false,#expr __VA_ARGS__); continue; }
#	define XGC_ASSERT_RELEASE(expr,msg,...)		if(!(expr)){ XGC_ASSERT_MSG(false,#expr __VA_ARGS__); }else{ (expr)->Release(); }
#	define XGC_ASSERT_MESSAGE(expr,msg,...)		if(!(expr)){ XGC_ASSERT_MSG(false,msg,##__VA_ARGS__); }
#	define XGC_DEBUG_MESSAGE(msg,...)			XGC_ASSERT_MSG(false,msg,##__VA_ARGS__)
#	define XGC_ASSERT_POINTER(expr)				XGC_ASSERT_MSG((expr),"NULL POINT FOUND, IS'T RIGHT?")
#	define XGC_VERIFY(expr)						XGC_ASSERT_MSG((expr),"")
#	define XGC_DEBUG_CODE( ... )				__VA_ARGS__
#elif defined( _ASSERT_LOG )
#	include "logger.h"
#	define XGC_ASSERT(expr)						if(!(expr)){ DBG_WARNING( "ASSERT " "%s", #expr ); }
#	define XGC_ASSERT_RETURN(expr,ret,...)		if(!(expr)){ DBG_WARNING( "ASSERT " __VA_ARGS__ ); return ret; }
#	define XGC_ASSERT_THROW(expr,ret,...)		if(!(expr)){ DBG_WARNING( "ASSERT " __VA_ARGS__ ); throw( ret ); }
#	define XGC_ASSERT_BREAK(expr,...)			if(!(expr)){ DBG_WARNING( "ASSERT " __VA_ARGS__ ); break; }
#	define XGC_ASSERT_CONTINUE(expr,...)		if(!(expr)){ DBG_WARNING( "ASSERT " __VA_ARGS__ ); continue; }
#	define XGC_ASSERT_RELEASE(expr,msg,...)		if(!(expr)){ DBG_WARNING( "ASSERT " __VA_ARGS__ ); }else{(expr)->Release();}
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

#define XGC_ALIGN_MEM(x, align) (((x) + (align) - 1) & ~((align)-1))

#define XGC_ALIGN_UP(x, align) ((x)&(-align))
#define XGC_ALIGN_DOWN(x, align) (-(-(x)&-(align)))

#define XGC_COUNTOF(_Array) (sizeof(_Array) / sizeof(_Array[0]))

#define XGC_CHECK_ARRAY_INDEX( _Array, _Index ) (_Index < XGC_COUNTOF(_Array) && _Index >= 0)

#define XGC_MIN( a, b )	((a)>(b)?(b):(a))
#define XGC_MAX( a, b )	((a)<(b)?(b):(a))
#define XGC_RNG( value, lower, upper ) ((value)<(lower)?(lower):(value)>(upper)?(upper):(value))
//-------------------------------------------------------------------------------------------------------------------------------------------------//
#define XGC_NONE xgc_void( 0 )

#define XGC_HEADER_FROM_MEMBER( TYPE, MEMBER_POINT, MEMBER_NAME ) ((TYPE *)( MEMBER_POINT - (((TYPE *)0)->MEMBER_NAME) ))
#endif //_MACRO_DEFINE_H