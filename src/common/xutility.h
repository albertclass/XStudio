///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file xutility.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// һЩ���õĸ�������
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
	/// \brief �������ƶ���Ļ���
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
	struct int2type
	{
		enum { _Val = V };
	};

	template< typename T >
	struct type2type
	{
		typedef T type;
	};

	template< typename T, xgc_size V >
	struct index_type
	{
		enum { _Val = V };
		typedef T type;
	};

	///
	/// \brief ���ݸ�����ֵ���ϻ�ȡ�����2��n�η�ֵ
	///
	/// \author albert.xu
	/// \date 2015/12/31 12:21
	///
	COMMON_API xgc_size nearest_power( xgc_size val, xgc_size pow );

	///
	/// �������ַ���ӳ���
	/// [3/10/2014 albert.xu]
	///
	struct COMMON_API string_value
	{
		xgc_uint32	idx;
		xgc_lpcstr	str;
	};

	///
	/// \brief ����������ȡ�ַ���
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
	/// \brief �����ַ�����ȡ����
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
	int numeric2str( T _value, xgc_lpstr _buffer, xgc_size _size )
	{
		XGC_ASSERT_RETURN( _size && _buffer, -1 );
		int convert = snprintf_s( _buffer, _size, _TRUNCATE, "%lld", (xgc_int64)_value );
		XGC_ASSERT_RETURN( convert < _size, -1 );
		return convert;
	}

	template< class T, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value == true, xgc_bool >::type = true >
	int numeric2str( T _value, xgc_lpstr _buffer, xgc_size _size )
	{
		XGC_ASSERT_RETURN( _size && _buffer, -1 );
		int convert = snprintf_s( _buffer, _size, _TRUNCATE, "%llu", (xgc_uint64)_value );
		XGC_ASSERT_RETURN( convert < _size, -1 );
		return convert;
	}

	template< class T, typename std::enable_if< std::is_floating_point< T >::value, xgc_bool >::type = true >
	int numeric2str( T _value, xgc_lpstr _buffer, xgc_size _size, xgc_int32 _num_of_dec = 4 )
	{
		XGC_ASSERT_RETURN( _size && _buffer, -1 );
		int convert = snprintf_s( _buffer, _size, _TRUNCATE, "%.*lf", _num_of_dec, (xgc_real64)_value );
		XGC_ASSERT_RETURN( convert < _size, -1 );
		return convert;
	}

	// auto size
	template< class T, size_t _size, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value == false, xgc_bool >::type = true >
	int numeric2str( T _value, xgc_char( &_buffer )[_size] )
	{
		int convert = snprintf_s( _buffer, _size, _TRUNCATE, "%lld", (xgc_int64)_value );
		XGC_ASSERT_RETURN( convert < _size, -1 );
		return convert;
	}

	template< class T, size_t _size, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value == true, xgc_bool >::type = true >
	int numeric2str( T _value, xgc_char( &_buffer )[_size] )
	{
		int convert = snprintf_s( _buffer, _size, _TRUNCATE, "%llu", (xgc_uint64)_value );
		XGC_ASSERT_RETURN( convert < _size, -1 );
		return convert;
	}

	template< class T, size_t _size, typename std::enable_if< std::is_floating_point< T >::value, xgc_bool >::type = true >
	int numeric2str( T _value, xgc_char( &_buffer )[_size], xgc_int32 _num_of_dec = 4 )
	{
		int convert = snprintf_s( _buffer, _size, _TRUNCATE, "%.*lf", _num_of_dec, (xgc_real64)_value );
		XGC_ASSERT_RETURN( convert < _size, -1 );
		return convert;
	}

	///
	/// \brief �ַ�����ϣ
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
	/// \brief �ַ��ָ�
	/// \param src Ҫ�ָ���ַ���
	/// \param tokens �ָ���
	/// \return ���������ʽ���طָ�õ��ַ���
	/// \date [5/30/2014]
	///
	template< template< class, class > class _Container = xgc_vector, template< class > class _Ax = xgc_allocator >
	_Container< std::string, _Ax< std::string > > string_split( xgc_lpcstr src, xgc_lpcstr delim )
	{
		_Container< std::string, _Ax< std::string > > r;
		XGC_ASSERT_RETURN( src, r );

		xgc_lpcstr str = src + strspn( src, delim );;
		while( *str )
		{
			xgc_lpcstr brk = strpbrk( str, delim );
			if( xgc_nullptr == brk )
			{
				r.push_back( str );
				break;
			}

			r.push_back( xgc_string( str, brk ) );
			str = brk + strspn( brk, delim );
		}

		return r;
	}

	///
	/// \brief �ַ����ϲ�
	///
	/// \author albert.xu
	/// \date 2017/04/01 12:26
	///
	template< class _Iterator >
	xgc_string string_join( _Iterator &_Begin, _Iterator &_End, xgc_lpcstr join )
	{
		xgc_string r;
		auto it = _Begin;
		r += *it;

		++it;

		while( it != _End )
		{
			r += join;
			r += *it;
			++it;
		}

		return r;
	}

	///
	/// \brief ������������תΪ16�����ַ���
	///
	/// \param data	������ַ
	/// \param size	���ݳ��ȣ����ַ��ƣ�
	/// \param out ���������ַ
	/// \param out_size ���������ȣ����ַ��ƣ�
	/// \param flags ��־ ��δ���壩
	/// \return ����ת���˶����ַ�
	///
	/// \author albert.xu
	/// \date 2015/12/16 17:00
	///
	COMMON_API xgc_size bin2hex( xgc_lpstr data, xgc_size size, xgc_lpstr out, xgc_size out_size, xgc_size flags );

	///
	/// \brief ��ʮ�������ַ���ת��Ϊ����������
	///
	/// \param data	������ַ
	/// \param size	���ݳ��ȣ����ַ��ƣ�
	/// \param out ���������ַ
	/// \param out_size ���������ȣ����ַ��ƣ�
	///
	/// \author albert.xu
	/// \date 2015/12/16 17:00
	///
	COMMON_API xgc_bool hex2bin( xgc_lpcstr data, xgc_size size, xgc_lpstr out, xgc_size out_size/* = 0*/ );

	///
	/// \brief ��ȡ�������������
	/// \author create by albert.xu
	/// \date 2015/12/16 17:00
	///
	COMMON_API std::mt19937& get_random_driver();

	///
	/// ��ȡ�����
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
			return std::uniform_int_distribution<NewT>( Max, Min )( get_random_driver() );
		else
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

	///
	/// \brief ͨ���ƥ���㷨
	///
	/// \author albert.xu
	/// \date 2017/03/30 18:31
	///
	COMMON_API xgc_bool string_match( const char *pattern, xgc_size patternLen, const char *string, xgc_size stringLen, int nocase );

	///
	/// \brief ͨ���ƥ���㷨
	///
	/// \author albert.xu
	/// \date 2017/03/30 18:31
	///
	COMMON_API xgc_bool string_match( const char *pattern, const char *string, int nocase );

	///
	/// \brief �޼��ַ������ü������ַ�
	///
	/// \author albert.xu
	/// \date 2017/03/31 14:23
	///
	COMMON_API xgc_size string_trim_left( xgc_lpstr str, xgc_lpcstr controls );

	///
	/// \brief �޼��ַ������ü��Ҳ���ַ�
	///
	/// \author albert.xu
	/// \date 2017/03/31 14:23
	///
	COMMON_API xgc_size string_trim_right( xgc_lpstr str, xgc_lpcstr controls );

	///
	/// \brief �޼��ַ������ü����е��ַ�
	///
	/// \author albert.xu
	/// \date 2017/03/31 14:23
	///
	COMMON_API xgc_size string_trim_all( xgc_lpstr str, xgc_lpcstr controls );
}
#endif // _XUTILITY_H_