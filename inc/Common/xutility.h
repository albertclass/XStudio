///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file xutility.h
/// \brief 文件简介
/// \author xufeng04
/// \date 二月 2016
///
/// 一些有用的辅助函数
///
///////////////////////////////////////////////////////////////
#ifndef _XUTILITY_H_
#define _XUTILITY_H_

#include "defines.h"
#include "exports.h"

#include <random>

namespace xgc
{
	///
	/// \brief 不允许复制对象的基类
	///
	/// \author albert.xu
	/// \date 2016/01/05 10:17
	///
	struct COMMON_API noncopyable
	{
		noncopyable() {}
	private:
		noncopyable( const noncopyable& rhs ) = delete;

		noncopyable& operator=( const noncopyable& rhs ) = delete;
#if _MSC_VER >= 1600
		noncopyable( const noncopyable&& rhs ) = delete;
		noncopyable& operator=( const noncopyable&& rhs ) = delete;
#endif
	};

	template< class T >
	struct is_numeric : std::conditional <
		std::is_integral< T >::value || std::is_floating_point< T >::value,
		std::true_type, std::false_type
	> ::type
	{
	};

	template< xgc_size V >
	struct Int2Type
	{
		enum { _Val = V };
	};

	template< typename T >
	struct Type2Type
	{
		typedef T Type;
	};

	template< typename T, xgc_size V >
	struct IndexType
	{
		enum { _Val = V };
		typedef T Type;
	};

	///
	/// \brief 根据给定的值向上获取最近的2的n次方值
	///
	/// \author albert.xu
	/// \date 2015/12/31 12:21
	///
	COMMON_API xgc_size nearest_power( xgc_size val, xgc_size pow );

	///
	/// 索引，字符串映射表
	/// [3/10/2014 albert.xu]
	///
	struct COMMON_API string_value
	{
		xgc_uint32	idx;
		xgc_lpcstr	str;
	};

	///
	/// \brief 根据索引获取字符串
	///
	/// \author albert.xu
	/// \date 2015/12/16 16:58
	///
	XGC_INLINE xgc_lpcstr val2str( struct string_value* tbl, xgc_uint32 idx )
	{
		while( tbl && tbl->str )
		{
			if( tbl->idx == idx )
				return tbl->str;

			++tbl;
		}

		return xgc_nullptr;
	}

	///
	/// \brief 根据字符串获取索引
	///
	/// \author albert.xu
	/// \date 2015/12/16 16:58
	///
	XGC_INLINE xgc_size str2val( struct string_value* tbl, xgc_lpcstr str )
	{
		while( tbl && tbl->str )
		{
			if( strcmp( tbl->str, str ) == 0 )
				return tbl->idx;

			++tbl;
		}

		return -1;
	}

	///
	/// \brief string convert to numeric
	///
	template< class R, typename std::enable_if< std::is_integral< R >::value && std::is_unsigned< R >::value == false, xgc_bool >::type = true >
	R str2numeric( xgc_lpcstr _value, xgc_lpstr *_next = xgc_nullptr, int _Radix = 10 )
	{
		return (R) strtoll( _value, _next, _Radix );
	}

	template< class R, typename std::enable_if< std::is_integral< R >::value && std::is_unsigned< R >::value != false, xgc_bool >::type = true >
	R str2numeric( xgc_lpcstr _value, xgc_lpstr *_next = xgc_nullptr, int _Radix = 10 )
	{
		return (R) strtoull( _value, _next, _Radix );
	}

	template< class R, typename std::enable_if< std::is_floating_point< R >::value, xgc_bool >::type = true >
	R str2numeric( xgc_lpcstr _value, xgc_lpstr *_next = xgc_nullptr, int _Radix = 10 )
	{
		return (R) strtod( _value, _next );
	}

	///
	/// \brief numeric convert to string
	///
	template< class T, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value == false, xgc_bool >::type = true >
	xgc_lpcstr numeric2str( T _value, xgc_lpstr _buffer, xgc_size _size )
	{
		XGC_ASSERT_RETURN( _size && _buffer, xgc_nullptr );
		int convert = snprintf_s( _buffer, _size, _TRUNCATE, "%" PRIi64, (xgc_int64)_value );
		XGC_ASSERT_RETURN( convert < _size, xgc_nullptr );
		return _buffer;
	}

	template< class T, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value == true, xgc_bool >::type = true >
	xgc_lpcstr numeric2str( T _value, xgc_lpstr _buffer, xgc_size _size )
	{
		XGC_ASSERT_RETURN( _size && _buffer, xgc_nullptr );
		int convert = snprintf_s( _buffer, _size, _TRUNCATE, "%" PRIu64, (xgc_uint64)_value );
		XGC_ASSERT_RETURN( convert < _size, xgc_nullptr );
		return _buffer;
	}

	template< class T, typename std::enable_if< std::is_floating_point< T >::value, xgc_bool >::type = true >
	xgc_lpcstr numeric2str( T _value, xgc_lpstr _buffer, xgc_size _size, xgc_int32 _num_of_dec = 4 )
	{
		XGC_ASSERT_RETURN( _size && _buffer, xgc_nullptr );
		int convert = snprintf_s( _buffer, _size, _TRUNCATE, "%.*lf", _num_of_dec, (xgc_real64)_value );
		XGC_ASSERT_RETURN( convert < _size, xgc_nullptr );
		return _buffer;
	}

	// auto size
	template< class T, size_t _size, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value == false, xgc_bool >::type = true >
	xgc_lpcstr numeric2str( T _value, xgc_char( &_buffer )[_size] )
	{
		int convert = snprintf_s( _buffer, _size, _TRUNCATE, "%" PRIi64, (xgc_int64)_value );
		XGC_ASSERT_RETURN( convert < _size, xgc_nullptr );
		return _buffer;
	}

	template< class T, size_t _size, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value == true, xgc_bool >::type = true >
	xgc_lpcstr numeric2str( T _value, xgc_char( &_buffer )[_size] )
	{
		int convert = snprintf_s( _buffer, _size, _TRUNCATE, "%" PRIu64, (xgc_uint64)_value );
		XGC_ASSERT_RETURN( convert < _size, xgc_nullptr );
		return _buffer;
	}

	template< class T, size_t _size, typename std::enable_if< std::is_floating_point< T >::value, xgc_bool >::type = true >
	xgc_lpcstr numeric2str( T _value, xgc_char( &_buffer )[_size], xgc_int32 _num_of_dec = 4 )
	{
		int convert = snprintf_s( _buffer, _size, _TRUNCATE, "%.*lf", _num_of_dec, (xgc_real64)_value );
		XGC_ASSERT_RETURN( convert < _size, xgc_nullptr );
		return _buffer;
	}

	///
	/// \brief 字符串哈希
	///
	/// \author albert.xu
	/// \date 2015/12/31 12:23
	///
	XGC_INLINE xgc_uint32 string_hash( xgc_lpcstr v )
	{
		xgc_lpcstr p;
		xgc_uint32 h = 5381;

		for( p = v; *p != '\0'; p++ )
			h = ( h << 5 ) + h + *p;

		return h;
	}

	///
	/// \brief 字符分割
	/// \param src 要分割的字符串
	/// \param tokens 分隔符
	/// \return 以数组的形式返回分割好的字符串
	/// \date [5/30/2014]
	///
	COMMON_API xgc_vector<xgc_string> string_split( xgc_lpcstr src, xgc_lpcstr delim );

	///
	/// \brief 将二进制数据转为16进制字符串
	///
	/// \param data	数据首址
	/// \param size	数据长度（按字符计）
	/// \param out 输出缓冲首址
	/// \param out_size 缓冲区长度（按字符计）
	/// \param flags 标志 （未定义）
	/// \return 返回转换了多少字符
	///
	/// \author albert.xu
	/// \date 2015/12/16 17:00
	///
	COMMON_API xgc_size bin2hex( xgc_lpstr data, xgc_size size, xgc_lpstr out, xgc_size out_size, xgc_size flags );

	///
	/// \brief 将十六进制字符串转换为二进制数据
	///
	/// \param data	数据首址
	/// \param size	数据长度（按字符计）
	/// \param out 输出缓冲首址
	/// \param out_size 缓冲区长度（按字符计）
	///
	/// \author albert.xu
	/// \date 2015/12/16 17:00
	///
	COMMON_API xgc_bool hex2bin( xgc_lpstr data, xgc_size size, xgc_lpstr out, xgc_size out_size );

	///
	/// \brief 获取随机数发生引擎
	/// \author create by albert.xu
	/// \date 2015/12/16 17:00
	///
	COMMON_API std::mt19937& get_random_driver();

	///
	/// 获取随机数
	/// \author create by albert.xu
	/// \date 2015/12/16 17:00
	///
	template < class T1, class T2, typename std::enable_if< !std::is_floating_point< T1 >::value && !std::is_floating_point<T2>::value, xgc_bool >::type = true >
	auto random_range( T1 Min, T2 Max )->decltype( Min + Max )
	{
		if( Min == Max )
			return Min;

		using NewT = decltype( Min + Max );

		if( Min > Max )
			std::swap( Min, Max );

		return std::uniform_int_distribution<NewT>( Min, Max )( get_random_driver() );
	}

	template< class T1, class T2, typename std::enable_if< std::is_floating_point< T1 >::value && std::is_floating_point<T2>::value, xgc_bool >::type = true>
	auto random_range( T1 Min, T2 Max )->decltype( Min + Max )
	{
		if( Min == Max ) 
			return Min;

		using NewT = decltype( Min + Max );

		if( Min > Max ) 
			std::swap( Min, Max );

		return std::uniform_real_distribution<NewT>( Min, std::nextafter( Max, std::numeric_limits<NewT>::max() ) )(get_random_driver());
	}
}
#endif // _XUTILITY_H_