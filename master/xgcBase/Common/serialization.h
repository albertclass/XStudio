#pragma once
#include "defines.h"
#include "exports.h"

#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>
namespace XGC
{
	namespace common
	{
		template< class buffer_type >
		class COMMON_API serialization_buffer
		{
			typedef buffer_type* buffer_point;
			typedef const buffer_type* buffer_cpoint;
		public:
			serialization_buffer()
				: m_data( xgc_nullptr )
				, m_capacity( 0 )
			{

			}

			serialization_buffer( buffer_cpoint data, xgc_size size )
				: m_data( data )
				, m_capacity( size )
			{
			}

			serialization_buffer( const serialization_buffer &rsh )
				: m_data( rsh.m_data )
				, m_capacity( rsh.m_capacity )
			{
			}

			serialization_buffer& operator=( const serialization_buffer& rsh )
			{
				m_data = rsh.m_data;
				m_capacity = rsh.m_capacity;
				return *this;
			}

			size_t size()const
			{
				return m_capacity;
			}

			const buffer_point base()const
			{
				return m_data;
			}

			buffer_point base()
			{
				return m_data;
			}

		private:
			buffer_point	m_data;
			xgc_size		m_capacity;
		};

		class COMMON_API serialization
		{
		protected:
			void _init_buf( xgc_uint8 mode, xgc_size off );

		public:
			enum eMethod { eWrite = 1, eRead = 2, eRW = 3 };
			enum ePosition { eWithHead = 0, eWithCurrent = 1, eWithForward = 2, eWithTail = 4 };
			serialization( xgc_void );

			//serialization( const char *ptr, int length, eMethod mode = serialization::eRW, xgc_size off = 0 );

			//serialization( const unsigned char *ptr, int length, eMethod mode = serialization::eRW, xgc_size off = 0 );

			///
			/// 序列化构造
			/// [3/27/2015] create by albert.xu
			///
			serialization( xgc_lpvoid data, xgc_size length, eMethod mode = serialization::eRW, xgc_size off = 0 );

			///
			/// 拷贝构造
			/// [3/27/2015] create by albert.xu
			///
			serialization& operator=( const serialization &rsh )
			{
				if( space() < rsh.wd() )
					throw( std::logic_error( "overflow" ) );
				putn( rsh.rd_ptr(), rsh.leave() );
				return *this;
			}

			//bool setbuf( const xgc_byte *ptr, int length, eMethod mode = serialization::eRW, xgc_size off = 0 );

			xgc_size seek( xgc_size pos, ePosition enPos, eMethod enMethod );

			xgc_size capacity()const
			{
				return _length;
			}

			// 返回未写入空间大小
			xgc_size space()const
			{
				return _wsize - _wd;
			}

			// 返回未读取空间大小
			xgc_size leave()const
			{
				return _rsize - _rd;
			}

			const xgc_byte *rd_ptr()const
			{
				return _ptr + _rd;
			}

			const xgc_byte *wd_ptr()const
			{
				return _ptr + _wd;
			}

			const xgc_byte *base()const
			{
				return _ptr;
			}

			const xgc_byte *back()const
			{
				return _ptr + _length;
			}

			xgc_size rd()const
			{
				return _rd;
			}

			xgc_size wd()const
			{
				return _wd;
			}

			xgc_size putn( const xgc_byte *p, xgc_size n );

			xgc_size getn( xgc_byte *p, xgc_size n );

			template< typename element_type >
			serialization& operator << ( const serialization_buffer< element_type > &buf )
			{
				xgc_uint32 size = (xgc_uint32) buf.size();
				putn( (xgc_byte*) &size, sizeof( size_t ) );
				putn( buf.base(), buf.size() );
				return *this;
			}

			template< typename element_type >
			serialization& operator >> ( serialization_buffer< element_type > &buf )
			{
				xgc_uint32 size;
				getn( (xgc_byte*) &size, sizeof( size_t ) );
				// buf = cbuffer( rd_ptr(), size );
				size = __min( size, buf.size() );
				memcpy( (void*) buf.base(), rd_ptr(), size );
				_rd += size;

				return *this;
			}
		private:
			const
			xgc_byte*	_ptr;		// 缓冲区指针
			xgc_size	_length;	// 缓冲区长度
			xgc_size	_rsize;		// 读缓冲长度
			xgc_size	_wsize;		// 写缓冲长度
			xgc_size	_rd;		// 读偏移
			xgc_size	_wd;		// 写偏移
		};

		COMMON_API serialization& operator << ( serialization& stream, bool c );
		COMMON_API serialization& operator << ( serialization& stream, char c );
		COMMON_API serialization& operator << ( serialization& stream, short c );
		COMMON_API serialization& operator << ( serialization& stream, int c );
		COMMON_API serialization& operator << ( serialization& stream, long c );
		COMMON_API serialization& operator << ( serialization& stream, long long c );
		COMMON_API serialization& operator << ( serialization& stream, unsigned char c );
		COMMON_API serialization& operator << ( serialization& stream, unsigned short c );
		COMMON_API serialization& operator << ( serialization& stream, unsigned int c );
		COMMON_API serialization& operator << ( serialization& stream, unsigned long c );
		COMMON_API serialization& operator << ( serialization& stream, unsigned long long c );
		COMMON_API serialization& operator << ( serialization& stream, float c );
		COMMON_API serialization& operator << ( serialization& stream, double c );
		COMMON_API serialization& operator << ( serialization& stream, long double c );
		COMMON_API serialization& operator << ( serialization& stream, const std::string& c );
		COMMON_API serialization& operator << ( serialization& stream, const std::wstring& c );
		COMMON_API serialization& operator << ( serialization& stream, const char* c );
		COMMON_API serialization& operator << ( serialization& stream, const wchar_t* c );
		//serialization& operator << ( serialization& stream, const buffer &c );

		template< class type, class alloc >
		serialization& operator << ( serialization& stream, const std::vector< type, alloc >& c )
		{
			stream << c.size();
			std::vector< type, alloc >::const_iterator iter = c.begin();
			std::vector< type, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << *iter++;
			return stream;
		}

		template< class type, class alloc >
		serialization& operator << ( serialization& stream, const std::vector< type*, alloc >& c )
		{
			stream << c.size();
			std::vector< type*, alloc >::const_iterator iter = c.begin();
			std::vector< type*, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << **iter++;
			return stream;
		}

		template< class type, class alloc >
		serialization& operator << ( serialization& stream, const std::list< type, alloc >& c )
		{
			stream << c.size();
			std::list< type, alloc >::const_iterator iter = c.begin();
			std::list< type, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << *iter++;
			return stream;
		}

		template< class type, class alloc >
		serialization& operator << ( serialization& stream, const std::list< type*, alloc >& c )
		{
			stream << c.size();
			std::list< type*, alloc >::const_iterator iter = c.begin();
			std::list< type*, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << **iter++;
			return stream;
		}

		template< class type, class alloc >
		serialization& operator << ( serialization& stream, const std::deque< type, alloc >& c )
		{
			stream << c.size();
			std::deque< type, alloc >::const_iterator iter = c.begin();
			std::deque< type, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << *iter++;
			return stream;
		}

		template< class type, class alloc >
		serialization& operator << ( serialization& stream, const std::deque< type*, alloc >& c )
		{
			stream << c.size();
			std::deque< type*, alloc >::const_iterator iter = c.begin();
			std::deque< type*, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << **iter++;
			return stream;
		}

		template< class type, class compair, class alloc >
		serialization& operator << ( serialization& stream, const std::set< type, compair, alloc >& c )
		{
			stream << c.size();
			std::set< type, compair, alloc >::const_iterator iter = c.begin();
			std::set< type, compair, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << *iter++;
			return stream;
		}

		template< class type, class compair, class alloc >
		serialization& operator << ( serialization& stream, const std::set< type*, compair, alloc >& c )
		{
			stream << c.size();
			std::set< type*, compair, alloc >::const_iterator iter = c.begin();
			std::set< type*, compair, alloc >::const_iterator iend = c.end();
			while( iter != iend )
				stream << **iter++;
			return stream;
		}

		template< class key, class value, class compair, class alloc >
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

		template< class key, class value >
		serialization& operator << ( serialization& stream, const std::pair< key, value >& c )
		{
			return stream << c.first << c.second;
		}

		template< class key, class value >
		serialization& operator << ( serialization& stream, const std::pair< key, value* >& c )
		{
			return stream << c.first << *c.second;
		}

		COMMON_API serialization& operator >> ( serialization& stream, bool &c );
		COMMON_API serialization& operator >> ( serialization& stream, char &c );
		COMMON_API serialization& operator >> ( serialization& stream, short &c );
		COMMON_API serialization& operator >> ( serialization& stream, int &c );
		COMMON_API serialization& operator >> ( serialization& stream, long &c );
		COMMON_API serialization& operator >> ( serialization& stream, long long &c );
		COMMON_API serialization& operator >> ( serialization& stream, unsigned char &c );
		COMMON_API serialization& operator >> ( serialization& stream, unsigned short &c );
		COMMON_API serialization& operator >> ( serialization& stream, unsigned int &c );
		COMMON_API serialization& operator >> ( serialization& stream, unsigned long &c );
		COMMON_API serialization& operator >> ( serialization& stream, unsigned long long &c );
		COMMON_API serialization& operator >> ( serialization& stream, float &c );
		COMMON_API serialization& operator >> ( serialization& stream, double &c );
		COMMON_API serialization& operator >> ( serialization& stream, long double &c );
		COMMON_API serialization& operator >> ( serialization& stream, std::string &c );
		COMMON_API serialization& operator >> ( serialization& stream, std::wstring &c );
		COMMON_API serialization& operator >> ( serialization& stream, const char* &c );
		COMMON_API serialization& operator >> ( serialization& stream, const wchar_t* &c );
		//serialization& operator >> ( serialization& stream, const buffer &c );

		template< class type, class alloc >
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

		template< class type, class alloc >
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

		template< class type, class alloc >
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

		template< class type, class alloc >
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

		template< class type, class alloc >
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

		template< class type, class alloc >
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

		template< class type, class compair, class alloc >
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

		template< class type, class compair, class alloc >
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

		template< class key, class value, class compair, class alloc >
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

		template< class key, class value >
		serialization& operator >> ( serialization& stream, std::pair< key, value >& c )
		{
			stream >> c.first >> c.second;
			return stream;
		}

		template< class key, class value >
		serialization& operator >> ( serialization& stream, std::pair< const key, value >& c )
		{
			stream >> const_cast<key&>( c.first ) >> c.second;
			return stream;
		}

		template< class key, class value >
		serialization& operator >> ( serialization& stream, std::pair< key, value* >& c )
		{
			c.second = XGC_NEW value();
			stream >> c.first >> *c.second;
			return stream;
		}

		template< class key, class value >
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
