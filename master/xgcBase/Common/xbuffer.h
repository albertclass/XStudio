#pragma once
#ifndef _XBUFFER_H_
#define _XBUFFER_H_
#include "defines.h"
#include "exports.h"

namespace XGC
{
	namespace common
	{
		struct COMMON_API ring_buffer
		{
			xgc_size head;
			xgc_size tail;
			xgc_size fill;
			xgc_size size;

			ring_buffer( xgc_size max_size )
				: head( 0 )
				, tail( 0 )
				, fill( 0 )
				, size( max_size )
			{
			}

			xgc_size push( xgc_size s = 1 )
			{
				if( fill + s > size )
					return -1;

				xgc_size pos = tail;

				tail += s;
				fill += s;
				tail %= size;

				return pos;
			}

			xgc_size poll( xgc_size s = 1 )
			{
				if( fill < s )
					return -1;

				xgc_size pos = tail;

				head += s;
				fill -= s;
				head %= size;

				return pos;
			}

			xgc_bool empty()
			{
				return fill == 0;
			}

			xgc_bool full()
			{
				return fill == size;
			}
		};
	}
	//////////////////////////////////////////////////////////////////////////
	// 静态缓冲区
	// 因没有虚函数,故在尺寸上该类和声明char数组是一致的.
	//////////////////////////////////////////////////////////////////////////
	template< xgc_size S >
	struct xBufferStaticStorage
	{
	protected:
		xBufferStaticStorage( const xBufferStaticStorage& rhs )
		{
			memcpy( buffer, rhs.base(), XGC_MIN( S, rhs.length() ) );
		}

		xBufferStaticStorage( xgc_lpvoid data, xgc_size size )
		{
			memcpy( buffer, data, XGC_MIN( S, size ) );
		}

		xgc_byte buffer[S]; // 缓冲区

		xgc_lpvoid	base() { return buffer; }
		xgc_lpcvoid	base()const { return buffer; }
		xgc_size	size()const { return S; }

		xgc_void	assign( xgc_lpvoid data, xgc_size length )
		{
			XGC_ASSERT( length <= capacity );
			memcpy( buffer, data, XGC_MIN( capacity, length ) );
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// 托管缓冲区
	//////////////////////////////////////////////////////////////////////////
	struct COMMON_API xBufferWarpStorage
	{
	protected:
		xBufferWarpStorage( const xBufferWarpStorage& rhs )
			: pbuffer( const_cast< xgc_lpvoid >( rhs.base() ) )
			, capacity( rhs.size() )
		{

		}

		xBufferWarpStorage( xgc_lpvoid data, xgc_size size )
			: pbuffer( data )
			, capacity( size )
		{
		}

		xgc_size	capacity;	// 容量
		xgc_lpvoid	pbuffer;	// 缓冲区指针

		xgc_lpvoid	base() { return pbuffer; }
		xgc_lpcvoid	base()const { return pbuffer; }
		xgc_size	size()const { return capacity; }
		xgc_void	assign( xgc_lpvoid data, xgc_size length )
		{
			pbuffer = data;
			capacity = length;
		}
	};

	template< class Storage >
	class xBufferImpl : protected Storage
	{
	public:
		xBufferImpl()
			: Storage( xgc_nullptr, 0 )
		{
		}

		xBufferImpl( xgc_lpvoid date, xgc_size size )
			: Storage( date, size )
		{
		}

		template< class S >
		xBufferImpl( const xBufferImpl< S >& rhs )
			: Storage( (xgc_lpvoid)rhs.data(), rhs.length() )
		{
		}

		//-----------------------------------//
		// [1/6/2014 albert.xu]
		// 带填充的初始化
		//-----------------------------------//
		xBufferImpl( xgc_lpvoid date, xgc_size size, xgc_int32 val )
			: Storage( date, size )
		{
			fill( val );
		}

		//-----------------------------------//
		// [1/6/2014 albert.xu]
		// 带填充的初始化模板版本
		//-----------------------------------//
		template< class Storage >
		xBufferImpl( const xBufferImpl< Storage >& rhs, xgc_int32 val )
			: Storage( rhs )
		{
			fill( val );
		}

		xgc_lpvoid data()
		{
			return base();
		}

		xgc_lpcvoid data()const
		{
			return base();
		}

		xgc_size length()const
		{
			return size();
		}

		xBufferImpl& assign( xgc_lpvoid data, xgc_size size )
		{
			Storage::assign( data, size );
			return *this;
		}

		xBufferImpl& assign( xBufferImpl& rhs )
		{
			return assign( rhs.base(), rhs.size() );
		}

		xBufferImpl& operator=( xBufferImpl& rhs )
		{
			return assign( rhs );
		}

		//-----------------------------------//
		// [1/7/2014 albert.xu]
		// 写数据到缓冲区
		//-----------------------------------//
		xgc_size write( xgc_size pos, xgc_lpcvoid data, xgc_size len )
		{
			XGC_ASSERT_RETURN( pos < size(), 0 );

			xgc_size wlen = XGC_MIN( len, size() - pos );
			memcpy( (xgc_lpstr)base() + pos, data, wlen );

			return wlen;
		}

		xgc_size write( xgc_lpcvoid data, xgc_size size )
		{
			return write( 0, data, size );
		}

		template< typename BufferStorage >
		xgc_size write( const xBufferImpl< BufferStorage >& buf, xgc_size len = -1 )
		{
			if( len == -1 )
				len = buf.length( );

			return write( 0, buf.base(), XGC_MIN( len, buf.length() ) );
		}

		template< class BufferStorage >
		xgc_size write( xgc_size pos, const xBufferImpl< BufferStorage >& buf, xgc_size len = -1 )
		{
			if( len == -1 )
				len = buf.length( );

			return write( pos, buf.base(), len );
		}

		template< class ReturnBuffer >
		ReturnBuffer part( xgc_size pos, xgc_size len = -1 )
		{
			if( pos > length() )
				pos = length();

			if( len == -1 )
				len = length() - pos;

			XGC_ASSERT_MESSAGE( len <= length() - pos, "给予的长度参数超出预期。%u > %u - %u", len, length(), pos );

			return ReturnBuffer( (xgc_lpstr)base( ) + pos, len );
		}

		template< class ReturnBuffer >
		const ReturnBuffer part( xgc_size pos, xgc_size len = -1 )const
		{
			if( pos > length() )
				pos = length();

			if( len == -1 )
				len = length() - pos;

			XGC_ASSERT_MESSAGE( len <= length() - pos, "给予的长度参数超出预期。%u > %u - %u", len, length(), pos );

			return ReturnBuffer( (xgc_lpvoid)((xgc_lpcstr)base() + pos), len );
		}

		//-----------------------------------//
		// [1/6/2014 albert.xu]
		// 填充整个缓冲区
		//-----------------------------------//
		xgc_void fill( xgc_int32 v )
		{
			memset( base(), v, size() );
		}

		//-----------------------------------//
		// [1/6/2014 albert.xu]
		// 操作符重载
		//-----------------------------------//
		xgc_char operator[]( xgc_size index )const
		{
			// XGC_ASSERT_MESSAGE( index < capacity(), "buffer read over bound!" );
			return ( (xgc_lpcstr) base() )[index];
		}
	};

	typedef xBufferImpl< xBufferWarpStorage > xBuffer;

	template< xgc_size S >	using xStaticBuffer = xBufferImpl< xBufferStaticStorage< S > >;
}


template<>
struct std::hash<XGC::xBuffer>
	: public unary_function<XGC::xBuffer, size_t>
{	// hash functor for plain old data
	typedef XGC::xBuffer _Kty;
	size_t operator()( const _Kty& _Keyval ) const
	{	// hash _Keyval to size_t value by pseudorandomizing transform
		return ( _Hash_seq( (const unsigned char *)_Keyval.data(), _Keyval.length() ) );
	}
};

template<>
struct std::equal_to<XGC::xBuffer>
	: public binary_function<XGC::xBuffer, XGC::xBuffer, bool>
{	// functor for operator==
	typedef XGC::xBuffer _Ty;
	bool operator()( const _Ty& _Left, const _Ty& _Right ) const
	{	// apply operator== to operands
		if( _Left.length() != _Right.length() )
			return false;
		const char* _LeftData = (const char*) _Left.data();
		const char* _RightData = (const char*) _Right.data();
		xgc_size n = 0;
		while( n++ < _Left.length() )
		{
			if( *_LeftData++ != *_RightData++ )
				return false;
// 			if( *( (const char*) _LeftData ) != *( (const char*) _RightData ) )
// 				return false;
		}
		return true;
	}
};
#endif