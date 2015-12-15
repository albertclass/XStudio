#ifndef _XUTILITY_H_
#define _XUTILITY_H_

#include "defines.h"
#include "exports.h"

namespace XGC
{
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
	
	XGC_INLINE xgc_size nearest_power( xgc_size base, xgc_size num )
	{
		static const xgc_uint32 _max_size = std::numeric_limits< xgc_uint32 >::max();

		if( num > _max_size / 2 )
		{
			return _max_size;
		}
		else
		{
			xgc_size n = base;

			while( n < num )
				n <<= 1;

			return n;
		}
	}

	struct string_case_less
		: public std::binary_function < xgc_string, xgc_string, bool >
	{	// functor for operator<
		bool operator()( const xgc_string& _Left, const xgc_string& _Right ) const
		{	// apply operator< to operands
			return ( _stricmp( _Left.c_str(), _Right.c_str() ) < 0 );
		}
	};

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
	/// [3/8/2014 albert.xu]
	/// 根据索引获取字符串
	///
	COMMON_API XGC_INLINE xgc_lpcstr val2str( struct string_value* tbl, xgc_uint32 idx )
	{
		while( tbl && tbl->str )
		{
			if( tbl->idx == idx )
				return tbl->str;

			++tbl;
		}

		return xgc_nullptr;
	}

	//-----------------------------------//
	// [3/8/2014 albert.xu]
	// 根据字符串获取索引
	//-----------------------------------//
	COMMON_API XGC_INLINE xgc_size str2val( struct string_value* tbl, xgc_lpcstr str )
	{
		while( tbl && tbl->str )
		{
			if( strcmp( tbl->str, str ) == 0 )
				return tbl->idx;

			++tbl;
		}

		return -1;
	}

	//////////////////////////////////////////////////////////////////////////
	// string convert to numeric
	//////////////////////////////////////////////////////////////////////////
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

	//////////////////////////////////////////////////////////////////////////
	// numeric convert to string
	//////////////////////////////////////////////////////////////////////////
	template< class T, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value == false, xgc_bool >::type = true >
	xgc_lpcstr numeric2str( T _value, xgc_lpstr _buffer, xgc_size _size )
	{
		XGC_ASSERT_RETURN( _size && _buffer, xgc_nullptr );
		errno_t err = _i64toa_s( _value, _buffer, _size, 10 );
		XGC_ASSERT_RETURN( err == 0, xgc_nullptr );
		return _buffer;
	}

	template< class T, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value == true, xgc_bool >::type = true >
	xgc_lpcstr numeric2str( T _value, xgc_lpstr _buffer, xgc_size _size )
	{
		XGC_ASSERT_RETURN( _size && _buffer, xgc_nullptr );
		errno_t err = _ui64toa_s( _value, _buffer, _size, 10 );
		XGC_ASSERT_RETURN( err == 0, xgc_nullptr );
		return _buffer;
	}

	template< class T, typename std::enable_if< std::is_floating_point< T >::value, xgc_bool >::type = true >
	xgc_lpcstr numeric2str( T _value, xgc_lpstr _buffer, xgc_size _size )
	{
		XGC_ASSERT_RETURN( _size && _buffer, xgc_nullptr );
		errno_t err = _fcvt_s( _buffer, _size, _value, 5, xgc_nullptr, xgc_nullptr );
		XGC_ASSERT_RETURN( err == 0, xgc_nullptr );
		return _buffer;
	}

	// auto size
	template< class T, size_t _size, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value == false, xgc_bool >::type = true >
	xgc_lpcstr numeric2str( T _value, xgc_char( &_buffer )[_size] )
	{
		errno_t err = _i64toa_s( _value, _buffer, _size, 10 );
		XGC_ASSERT_RETURN( err == 0, xgc_nullptr );
		return _buffer;
	}

	template< class T, size_t _size, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value == true, xgc_bool >::type = true >
	xgc_lpcstr numeric2str( T _value, xgc_char( &_buffer )[_size] )
	{
		errno_t err = _ui64toa_s( _value, _buffer, _size, 10 );
		XGC_ASSERT_RETURN( err == 0, xgc_nullptr );
		return _buffer;
	}

	template< class T, size_t _size, typename std::enable_if< std::is_floating_point< T >::value, xgc_bool >::type = true >
	xgc_lpcstr numeric2str( T _value, xgc_char( &_buffer )[_size], xgc_int32 _num_of_dec = 4 )
	{
		errno_t err = _fcvt_s( _buffer, _size, _value, _num_of_dec, xgc_nullptr, xgc_nullptr );
		XGC_ASSERT_RETURN( err == 0, xgc_nullptr );
		return _buffer;
	}

	XGC_INLINE xgc_uint32 string_hash( xgc_lpcstr v )
	{
		xgc_lpcstr p;
		xgc_uint32 h = 5381;

		for( p = v; *p != '\0'; p++ )
			h = ( h << 5 ) + h + *p;

		return h;
	}

	/*!
	 * @brief 字符分割
	 * @param src 要分割的字符串
	 * @param tokens 分隔符
	 * @return 以数组的形式返回分割好的字符串
	 * @date [5/30/2014]
	 */
	COMMON_API xgc_vector<xgc_string> string_split( xgc_lpcstr src, xgc_lpcstr delim );

	///
	/// [12/18/2013 albert.xu]
	/// 将Char转为16进制字符
	///
	COMMON_API xgc_void Char2Hex( xgc_byte ch, xgc_lpstr szHex );

	///
	/// [12/18/2013 albert.xu]
	/// 将char数组转为16进制字符串
	///
	COMMON_API xgc_size Binary2Hex( xgc_byte* data, xgc_size size, xgc_lpstr out, xgc_size osize, xgc_size flags );

	///
	/// [12/18/2013 albert.xu]
	/// 将16进制字符转为Char
	///
	COMMON_API xgc_bool Hex2Char( xgc_lpcstr szHex, xgc_byte& rch );

	///
	/// [12/18/2013 albert.xu]
	/// 将16进制字符串转为char数组
	///
	COMMON_API xgc_bool Hex2Binary( xgc_lpcstr pszHexStr, xgc_byteptr pucBinStr, xgc_size iBinSize = 0 );

}
#endif // _XUTILITY_H_