///
/// CopyRight ? 2016 X Studio
/// \file serialization.h
/// \date 八月 2016
///
/// \author albert.xu windxu@126.com
/// \brief 序列化类， 用于序列化数据到缓冲
/// 

#pragma once
#ifndef _SERIALIZATION_H_
#define _SERIALIZATION_H_

#include "defines.h"
#include "exports.h"

#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>

#include <exception>
namespace xgc
{
	namespace common
	{
		#define serialization_types class _Buf, template< class > class _Rec, template< class, template< class > class > class _Tx
		#define serialization _Tx< _Buf, _Rec >

		template< serialization_types, class type >
		serialization& operator <<( serialization& stream, type v );
		// 不实现，因为匹配到这里代表没有对应的特化

		template< serialization_types, class type >
		serialization& operator >>( serialization& stream, type &v );
		// 不实现，因为匹配到这里代表没有对应的特化

		//////////////////////////////////////////////////////////////////////////
		// 函数类型特化

		//////////////////////////////////////////////////////////////////////////
		// 数值类型特化
		template< serialization_types >
		serialization& operator << ( serialization& stream, serialization& ( __cdecl *_Pfn )( serialization& ) )
		{
			return _Pfn( stream );
		}

		template< serialization_types >
		serialization& __cdecl endl( serialization& stream )
		{
			stream << '\n';
			return stream;
		}

		
		#define serialization_basic_type_w( type ) \
		template< serialization_types >\
		serialization& operator <<( serialization& stream, type v )\
		{ \
			stream.write_some( (xgc_byte*)&v, sizeof(type) ); \
			return stream; \
		}\

		#define serialization_basic_type_r( type ) \
		template< serialization_types >\
		serialization& operator >>( serialization& stream, type &v )\
		{ \
			stream.read_some( (xgc_byte*)&v, sizeof(type) ); \
			return stream; \
		}\

		// operator <<
		serialization_basic_type_w( xgc_bool );
		serialization_basic_type_w( xgc_wchar );

		serialization_basic_type_w( xgc_long );
		serialization_basic_type_w( xgc_int8 );
		serialization_basic_type_w( xgc_int16 );
		serialization_basic_type_w( xgc_int32 );
		serialization_basic_type_w( xgc_int64 );

		serialization_basic_type_w( xgc_ulong );
		serialization_basic_type_w( xgc_uint8 );
		serialization_basic_type_w( xgc_uint16 );
		serialization_basic_type_w( xgc_uint32 );
		serialization_basic_type_w( xgc_uint64 );

		serialization_basic_type_w( xgc_real32 );
		serialization_basic_type_w( xgc_real64 );

		// operator >>
		serialization_basic_type_r( xgc_bool );
		serialization_basic_type_r( xgc_wchar );

		serialization_basic_type_r( xgc_long );
		serialization_basic_type_r( xgc_int8 );
		serialization_basic_type_r( xgc_int16 );
		serialization_basic_type_r( xgc_int32 );
		serialization_basic_type_r( xgc_int64 );

		serialization_basic_type_r( xgc_ulong );
		serialization_basic_type_r( xgc_uint8 );
		serialization_basic_type_r( xgc_uint16 );
		serialization_basic_type_r( xgc_uint32 );
		serialization_basic_type_r( xgc_uint64 );

		serialization_basic_type_r( xgc_real32 );
		serialization_basic_type_r( xgc_real64 );

		//////////////////////////////////////////////////////////////////////////
		// 缓冲类型特化

		///
		/// \brief 静态缓冲特化函数
		///
		/// \author albert.xu
		/// \date 2016/03/01 14:10
		///
		template< serialization_types, xgc_size S >
		serialization& operator << ( serialization& stream, static_buffer< S > &buf )
		{
			xgc_uint32 size = (xgc_uint32) XGC_MIN( stream.space(), buf.capacity() );
			stream.write_some( &size, sizeof( xgc_uint32 ) );
			stream.write_some( buf.base(), buf.capacity() );
			return stream;
		}

		///
		/// \brief 静态缓冲特化函数
		///
		/// \author albert.xu
		/// \date 2016/03/01 14:10
		///
		template< serialization_types, xgc_size S >
		serialization& operator >> ( serialization& stream, static_buffer< S > &buf )
		{
			xgc_uint32 size;
			stream.read_some( &size, sizeof( xgc_uint32 ) );
			size = XGC_MIN( size, (xgc_uint32) stream.leave() );

			stream.read_some( buf.base(), buf.capacity() );
			return stream;
		}

		///
		/// \brief 引用缓冲特化函数
		///
		/// \author albert.xu
		/// \date 2016/03/01 14:10
		///
		template< serialization_types >
		serialization& operator << ( serialization& stream, const reference_buffer &buf )
		{
			xgc_uint32 size = (xgc_uint32) XGC_MIN( stream.space(), buf.capacity() );
			stream.write_some( &size, sizeof( xgc_uint32 ) );
			stream.write_some( buf.base(), buf.capacity() );
			return stream;
		}

		///
		/// \brief 引用缓冲特化函数
		///
		/// \author albert.xu
		/// \date 2016/03/01 14:10
		///
		template< serialization_types, xgc_size S >
		serialization& operator >> ( serialization& stream, reference_buffer &buf )
		{
			xgc_uint32 size;
			stream.read_some( &size, sizeof( xgc_uint32 ) );
			size = XGC_MIN( size, (xgc_uint32) stream.leave() );

			stream.read_some( buf.base(), buf.capacity() );
			return stream;
		}

		///
		/// \brief 线性缓冲特化函数
		///
		/// \author albert.xu
		/// \date 2016/09/18 16:48
		///
		template< serialization_types, class buffer, template< class > class recorder >
		serialization& operator << ( serialization& stream, linear_buffer< buffer, recorder > &buf )
		{
			xgc_uint32 size = (xgc_uint32) XGC_MIN( stream.space(), buf.leave() );
			stream.write_some( &size, sizeof( xgc_uint32 ) );
			buf.read_some( (xgc_lpvoid)stream.wd_ptr(), size );

			stream.plus_wd( size );
			return stream;
		}

		///
		/// \brief 线性缓冲特化函数
		///
		/// \author albert.xu
		/// \date 2016/03/01 14:10
		///
		template< serialization_types, class buffer, template< class > class recorder >
		serialization& operator >> ( serialization& stream, linear_buffer< buffer, recorder > &buf )
		{
			xgc_uint32 size;
			stream.read_some( &size, sizeof( xgc_uint32 ) );
			size = XGC_MIN( size, (xgc_uint32) stream.leave() );
			auto writebytes = buf.write_some( stream.rd_ptr(), size );

			rd_plus( writebytes );

			return stream;
		}

		///
		/// \brief 环缓冲特化函数
		///
		/// \author albert.xu
		/// \date 2016/03/01 14:10
		///
		template< serialization_types, class buffer, template< class > class recorder >
		serialization& operator << ( serialization& stream, ring_buffer< buffer, recorder > &buf )
		{
			xgc_uint32 size = (xgc_uint32) buf.leave();
			stream.write_some( &size, sizeof( xgc_uint32 ) );
			char buffer[1024*32];
			auto readbytes = buf.read_some( buffer, sizeof( buffer ) );
			while( readbytes )
			{
				stream.write_some( buffer, readbytes );
				readbytes = buf.read_some( buffer, sizeof( buffer ) );
			}

			return stream;
		}

		///
		/// \brief 环缓冲特化函数
		///
		/// \author albert.xu
		/// \date 2016/03/01 14:10
		///
		template< serialization_types, class buffer, template< class > class recorder >
		serialization& operator >> ( serialization& stream, ring_buffer< buffer, recorder > &buf )
		{
			xgc_uint32 size;
			xgc_uint32 readtotal = 0U;
			stream.read_some( &size, sizeof( xgc_uint32 ) );
			size = XGC_MIN( size, (xgc_uint32) leave() );
			char buffer[1024 * 32];
			auto readbytes = stream.read_some( buffer, size );

			while( readtotal < size )
			{
				buf.write_some( buffer, readbytes );
				readbytes = stream.read_some( buffer, size - readtotal );
				readtotal += readbytes;
			}

			stream.rd_plus( readtotal );

			return stream;
		}

		//////////////////////////////////////////////////////////////////////////
		// 字符串类型特化

		///
		/// \brief std::string 序列化特化函数
		///
		/// \author albert.xu
		/// \date 2016/09/18 16:22
		///
		template< serialization_types >
		serialization& operator << ( serialization& stream, const std::string &c )
		{
			stream.write_some( (xgc_byte*)c.c_str(), c.length()+1 );
			return stream;
		}

		template< serialization_types >
		serialization& operator >> ( serialization& stream, std::string &c )
		{
			c = (const char*)stream.rd_ptr();
			stream.rd_plus( c.length()+1 );
			return stream;
		}

		///
		/// \brief std::wstring 序列化特化函数
		///
		/// \author albert.xu
		/// \date 2016/09/18 16:23
		///
		template< serialization_types >
		serialization& operator << ( serialization& stream, const std::wstring &c )
		{
			stream.write_some( (xgc_byte*)c.c_str(), (c.length()<<1)+sizeof( wchar_t/*std::wstring::traits_type::char_type*/ ) );
			return stream;
		}

		template< serialization_types >
		serialization& operator >> ( serialization& stream, std::wstring &c )
		{
			c = (const wchar_t*)stream.rd_ptr();
			stream.rd_plus( (c.length()<<1)+sizeof(wchar_t) );
			return stream;
		}

		///
		/// \brief 字符串指针序列化特化函数
		///
		/// \author albert.xu
		/// \date 2016/09/18 16:24
		///
		template< serialization_types >
		serialization& operator << ( serialization& stream, const char* c )
		{
			if( c )
				stream.write_some( (xgc_byte*) c, strlen( c ) + 1 );
			else
				stream.write_some( (xgc_byte*) "null", 5 );
			return stream;
		}

		template< serialization_types >
		serialization& operator >> ( serialization& stream, const char* &c )
		{
			c = (const char*)stream.rd_ptr();
			stream.plus_rd( (xgc_long)strlen(c)+1 );
			return stream;
		}

		///
		/// \brief 宽字符串指针序列化特化函数
		///
		/// \author albert.xu
		/// \date 2016/09/18 16:25
		///
		template< serialization_types >
		serialization& operator << ( serialization& stream, const wchar_t* c )
		{
			if( c )
				stream.write_some( (xgc_byte*) c, ( wcslen( c ) + 1 )*sizeof( wchar_t ) );
			else
				stream.write_some( (xgc_byte*) L"null", 5 * sizeof( wchar_t ) );
			return stream;
		}

		template< serialization_types >
		serialization& operator >> ( serialization& stream, const wchar_t* &c )
		{
			c = (const wchar_t*)stream.rd_ptr();
			stream.rd_plus( (wcslen(c) + 1) * sizeof(wchar_t) );
			return stream;
		}

		//////////////////////////////////////////////////////////////////////////
		// 容器类型特化
		template< serialization_types, class type, class alloc >
		serialization& operator << ( serialization& stream, const std::vector< type, alloc >& c )
		{
			stream << c.size();
			std::vector< type, alloc >::const_iterator iter = c.begin();
			std::vector< type, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << *iter++;
			return stream;
		}

		template< serialization_types, class type, class alloc >
		serialization& operator << ( serialization& stream, const std::vector< type*, alloc >& c )
		{
			stream << c.size();
			std::vector< type*, alloc >::const_iterator iter = c.begin();
			std::vector< type*, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << **iter++;
			return stream;
		}

		template< serialization_types, class type, class alloc >
		serialization& operator << ( serialization& stream, const std::list< type, alloc >& c )
		{
			stream << c.size();
			std::list< type, alloc >::const_iterator iter = c.begin();
			std::list< type, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << *iter++;
			return stream;
		}

		template< serialization_types, class type, class alloc >
		serialization& operator << ( serialization& stream, const std::list< type*, alloc >& c )
		{
			stream << c.size();
			std::list< type*, alloc >::const_iterator iter = c.begin();
			std::list< type*, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << **iter++;
			return stream;
		}

		template< serialization_types, class type, class alloc >
		serialization& operator << ( serialization& stream, const std::deque< type, alloc >& c )
		{
			stream << c.size();
			std::deque< type, alloc >::const_iterator iter = c.begin();
			std::deque< type, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << *iter++;
			return stream;
		}

		template< serialization_types, class type, class alloc >
		serialization& operator << ( serialization& stream, const std::deque< type*, alloc >& c )
		{
			stream << c.size();
			std::deque< type*, alloc >::const_iterator iter = c.begin();
			std::deque< type*, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << **iter++;
			return stream;
		}

		template< serialization_types, class type, class compair, class alloc >
		serialization& operator << ( serialization& stream, const std::set< type, compair, alloc >& c )
		{
			stream << c.size();
			std::set< type, compair, alloc >::const_iterator iter = c.begin();
			std::set< type, compair, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << *iter++;
			return stream;
		}

		template< serialization_types, class type, class compair, class alloc >
		serialization& operator << ( serialization& stream, const std::set< type*, compair, alloc >& c )
		{
			stream << c.size();
			std::set< type*, compair, alloc >::const_iterator iter = c.begin();
			std::set< type*, compair, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << **iter++;
			return stream;
		}

		template< serialization_types, class key, class value, class compair, class alloc >
		serialization& operator << ( serialization& stream, const std::map< key, value, compair, alloc >& c )
		{
			stream << c.size();
			std::map< key, value, compair, alloc >::const_iterator iter = c.begin();
			std::map< key, value, compair, alloc >::const_iterator iend = c.end();
			while( iter != iend )
			{
				stream << *iter;
				++iter;
			}
			return stream;
		}

		template< serialization_types, class type, class hasher, class equal_key, class alloc >
		serialization& operator << ( serialization& stream, const std::unordered_set< type, hasher, equal_key, alloc > &c )
		{
			stream << c.size();
			std::unordered_set< type, hasher, equal_key, alloc >::const_iterator iter = c.begin();
			std::unordered_set< type, hasher, equal_key, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << *iter++;
			return stream;
		}

		template< serialization_types, class type, class hasher, class equal_key, class alloc >
		serialization& operator << ( serialization& stream, const std::unordered_set< type*, hasher, equal_key, alloc > &c )
		{
			stream << c.size();
			std::unordered_set< type*, hasher, equal_key, alloc >::const_iterator iter = c.begin();
			std::unordered_set< type*, hasher, equal_key, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << **iter++;
			return stream;
		}

		template< serialization_types, class key, class value, class hasher, class equal_key, class alloc >
		serialization& operator << ( serialization& stream, const std::unordered_map< key, value, hasher, equal_key, alloc >& c )
		{
			stream << c.size();
			std::unordered_map< key, value, hasher, equal_key, alloc >::const_iterator iter = c.begin();
			std::unordered_map< key, value, hasher, equal_key, alloc >::const_iterator iend = c.end();
			while( iter != iend )
			{
				stream << *iter;
				++iter;
			}
			return stream;
		}

		template< serialization_types, class key, class value >
		serialization& operator << ( serialization& stream, const std::pair< key, value >& c )
		{
			return stream << c.first << c.second;
		}

		template< serialization_types, class key, class value >
		serialization& operator << ( serialization& stream, const std::pair< key, value* >& c )
		{
			return stream << c.first << *c.second;
		}

		template< serialization_types, class type, class alloc >
		serialization& operator >> ( serialization& stream, std::vector< type, alloc >& c )
		{
			std::vector< type, alloc >::size_type size;
			stream >> size;
			c.resize( size );

			for( std::vector< type, alloc >::size_type i = 0; i < size; ++i )
			{
				stream >> c[i];
			}
			return stream;
		}

		template< serialization_types, class type, class alloc >
		serialization& operator >> ( serialization& stream, std::vector< type*, alloc >& c )
		{
			std::vector< type*, alloc >::size_type size;
			stream >> size;
			c.resize( size );

			for( std::vector< type*, alloc >::size_type i = 0; i < size; ++i )
			{
				c[i] = XGC_NEW type();
				stream >> *c[i];
			}
			return stream;
		}

		template< serialization_types, class type, class alloc >
		serialization& operator >> ( serialization& stream, std::list< type, alloc >& c )
		{
			std::list< type, alloc >::size_type		size;
			std::list< type, alloc >::value_type	value;
			stream >> size;

			for( std::list< type, alloc >::size_type i = 0; i < size; ++i )
			{
				stream >> value;
				c.push_back( value );
			}
			return stream;
		}

		template< serialization_types, class type, class alloc >
		serialization& operator >> ( serialization& stream, std::list< type*, alloc >& c )
		{
			std::list< type*, alloc >::size_type	size;
			std::list< type*, alloc >::value_type	_value;
			stream >> size;

			for( std::list< type*, alloc >::size_type i = 0; i < size; ++i )
			{
				_value = XGC_NEW type();
				stream >> *_value;
				c.push_back( _value );
			}
			return stream;
		}

		template< serialization_types, class type, class alloc >
		serialization& operator >> ( serialization& stream, std::deque< type, alloc >& c )
		{
			std::deque< type, alloc >::size_type	size;
			std::deque< type, alloc >::value_type	value;
			stream >> size;

			for( std::queue< type, alloc >::size_type i = 0; i < size; ++i )
			{
				stream >> value;
				c.push_back( value );
			}
			return stream;
		}

		template< serialization_types, class type, class alloc >
		serialization& operator >> ( serialization& stream, std::deque< type*, alloc >& c )
		{
			std::deque< type*, alloc >::size_type	size;
			std::deque< type*, alloc >::value_type	_value;
			stream >> size;

			for( std::queue< type*, alloc >::size_type i = 0; i < size; ++i )
			{
				_value = XGC_NEW type();
				stream >> *_value;
				c.push_back( _value );
			}
			return stream;
		}

		template< serialization_types, class type, class compair, class alloc >
		serialization& operator >> ( serialization& stream, std::set< type, compair, alloc >& c )
		{
			std::set< type, compair, alloc >::size_type	size;
			std::set< type, compair, alloc >::value_type	value;
			stream >> size;

			for( std::set< type, compair, alloc >::size_type i = 0; i < size; ++i )
			{
				stream >> value;
				c.insert( value );
			}

			return stream;
		}

		template< serialization_types, class type, class compair, class alloc >
		serialization& operator >> ( serialization& stream, std::set< type*, compair, alloc >& c )
		{
			std::set< type*, compair, alloc >::size_type	size;
			std::set< type*, compair, alloc >::value_type	_value;
			stream >> size;

			for( std::set< type*, compair, alloc >::size_type i = 0; i < size; ++i )
			{
				_value = XGC_NEW type();
				stream >> *_value;
				c.insert( _value );
			}

			return stream;
		}

		template< serialization_types, class key, class value, class compair, class alloc >
		serialization& operator >> ( serialization& stream, std::map< key, value, compair, alloc >& c )
		{
			size_t	size;
			std::pair< key, value > _value;
			stream >> size;

			for( size_t i = 0; i < size; ++i )
			{
				stream >> _value;
				c.insert( _value );
			}

			return stream;
		}

		template< serialization_types, class type, class hasher, class equal_key, class alloc >
		serialization& operator >> ( serialization& stream, std::unordered_set< type, hasher, equal_key, alloc >& c )
		{
			std::unordered_set< type, hasher, equal_key, alloc >::size_type	size;
			std::unordered_set< type, hasher, equal_key, alloc >::value_type	value;
			stream >> size;

			for( std::unordered_set< type, hasher, equal_key, alloc >::size_type i = 0; i < size; ++i )
			{
				stream >> value;
				c.insert( value );
			}

			return stream;
		}

		template< serialization_types, class type, class hasher, class equal_key, class alloc >
		serialization& operator >> ( serialization& stream, std::unordered_set< type*, hasher, equal_key, alloc >& c )
		{
			std::unordered_set< type*, hasher, equal_key, alloc >::size_type	size;
			std::unordered_set< type*, hasher, equal_key, alloc >::value_type	_value;
			stream >> size;

			for( std::unordered_set< type*, hasher, equal_key, alloc >::size_type i = 0; i < size; ++i )
			{
				_value = XGC_NEW type();
				stream >> *_value;
				c.insert( _value );
			}

			return stream;
		}

		template< serialization_types, class key, class value, class hasher, class equal_key, class alloc >
		serialization& operator >> ( serialization& stream, std::unordered_map< key, value, hasher, equal_key, alloc >& c )
		{
			size_t	size;
			std::pair< key, value > _value;
			stream >> size;

			for( size_t i = 0; i < size; ++i )
			{
				stream >> _value;
				c.insert( _value );
			}

			return stream;
		}

		template< serialization_types, class key, class value >
		serialization& operator >> ( serialization& stream, std::pair< key, value >& c )
		{
			stream >> c.first >> c.second;
			return stream;
		}

		template< serialization_types, class key, class value >
		serialization& operator >> ( serialization& stream, std::pair< const key, value >& c )
		{
			stream >> const_cast<key&>( c.first ) >> c.second;
			return stream;
		}

		template< serialization_types, class key, class value >
		serialization& operator >> ( serialization& stream, std::pair< key, value* >& c )
		{
			c.second = XGC_NEW value();
			stream >> c.first >> *c.second;
			return stream;
		}

		template< serialization_types, class key, class value >
		serialization& operator >> ( serialization& stream, std::pair< const key, value* >& c )
		{
			c.second = XGC_NEW value();
			stream >> const_cast<key&>( c.first ) >> *c.second;
			return stream;
		}


		//////////////////////////////////////////////////////////////////////////
		// 将缓冲中的数据转为需要的数据类型
		//////////////////////////////////////////////////////////////////////////
		template< typename ret >
		struct buffer_cast_impl
		{
			typedef ret ret_type;
			static ret_type buffer_cast( xgc_lpcvoid cursor, xgc_size size, xgc_lpcvoid *next, xgc_size *release )
			{
				XGC_ASSERT_THROW( size >= sizeof( ret ), std::out_of_range( "read buffer out of range" ) );

				if( next )
					*next = (xgc_lpcstr) cursor + sizeof( ret );

				if( release )
					*release = size - sizeof( ret );

				return *(ret*) cursor;
			}
		};

		template< typename ret >
		struct buffer_cast_impl< ret * >
		{
			typedef const ret *ret_type;
			static ret_type buffer_cast( xgc_lpcvoid cursor, xgc_size size, xgc_lpcvoid *next, xgc_size *release )
			{
				typedef org = std::remove_pointer< ret >::type;
				XGC_ASSERT_THROW( size >= sizeof( org ), std::out_of_range( "read buffer out of range" ) );

				if( next )
					*next = (xgc_lpcstr) cursor + sizeof( org );

				if( release )
					*release = size - sizeof( org );

				return reinterpret_cast<ret_type>( cursor );
			}
		};

		template< typename ret >
		typename buffer_cast_impl< ret >::ret_type buffer_cast( xgc_lpcvoid cursor, xgc_size size, xgc_lpcvoid *next = xgc_nullptr, xgc_size *release = xgc_nullptr )
		{
			return buffer_cast_impl< ret >::buffer_cast( cursor, size, next, release );
		}
	}
};

#endif // _SERIALIZATION_H_