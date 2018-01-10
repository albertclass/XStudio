#include "defines.h"
#include "xvariant.h"
#include "debuger.h"

namespace xgc
{
	///
	/// \brief 默认构造
	/// \author albert.xu
	/// \date 2017/09/08
	///
	xvariant::xvariant()
		: t( vt_void )
		, mVoid( 0 )
	{

	}

	///
	/// \brief 拷贝构造
	/// \author albert.xu
	/// \date 2017/09/08
	///
	xvariant::xvariant( const xvariant & _Val )
		: t( _Val.t )
		, mString( _Val.mString )
	{

	}

	///
	/// \brief 右值构造
	/// \author albert.xu
	/// \date 2017/09/08
	///
	xvariant::xvariant( xvariant && _Val )
		: t( _Val.t )
		, mVoid( xgc_nullptr )
	{
		std::swap( mVoid, _Val.mVoid );
	}

	///
	/// \brief 析构
	/// \author create by albert.xu
	/// \data 9/9/2014
	///
	xvariant::~xvariant()
	{
		if( is_value() )
			this->set_value( 0ULL );
		else if( is_bool() )
			this->set_value( false );
		else if( is_string() )
			free( mString );
		else if( is_buffer() )
			free( mBuffer );
		else
			XGC_DEBUG_MESSAGE( "未知的数据类型被释放" );

		mVoid = xgc_nullptr;
	}
	
	xgc_lpcstr xvariant::type2string( value_type t )
	{
		xgc_lpcstr str_type = "vt_unknowe";
		switch( t )
		{
			case vt_void: str_type = "vt_void"; break;
			case vt_bool: str_type = "vt_bool"; break;
			case vt_char: str_type = "vt_char"; break;
			case vt_byte: str_type = "vt_byte"; break;
			case vt_i16: str_type = "vt_i16"; break;
			case vt_i32: str_type = "vt_i32"; break;
			case vt_i64: str_type = "vt_i64"; break;
			case vt_u16: str_type = "vt_u16"; break;
			case vt_u32: str_type = "vt_u32"; break;
			case vt_u64: str_type = "vt_u64"; break;
			case vt_real: str_type = "vt_real"; break;
			case vt_real64: str_type = "vt_real64"; break;
			case vt_string: str_type = "vt_string"; break;
			case vt_buffer: str_type = "vt_buffer"; break;
		}

		return str_type;
	}

	///
	/// \brief 获取类型所需尺寸
	/// \author albert.xu
	/// \date 2017/09/08
	///

	xgc_size xvariant::type2size( value_type t )
	{
		xgc_size size = 0;
		switch( t )
		{
			case vt_bool:	size = sizeof( Bool );      break;
			case vt_char:	size = sizeof( Char );      break;
			case vt_byte:	size = sizeof( Byte );      break;
			case vt_i16:	size = sizeof( Short );     break;
			case vt_i32:	size = sizeof( Integer );   break;
			case vt_i64:	size = sizeof( Long );      break;
			case vt_u16:	size = sizeof( UShort );    break;
			case vt_u32:	size = sizeof( Unsigned );  break;
			case vt_u64:	size = sizeof( ULong );     break;
			case vt_real:	size = sizeof( Real32 );    break;
			case vt_real64: size = sizeof( Real64 );    break;
			case vt_string: size = sizeof( StringPtr ); break;
			case vt_buffer: size = sizeof( BufferPtr ); break;
		}

		return size;
	}

	///
	/// \brief 获取类型所需尺寸
	/// \author albert.xu
	/// \date 2017/09/08
	///

	xgc_size xvariant::type2size() const
	{
		return type2size( type() );
	}

	///
	/// \brief 是否有符号数
	/// \author albert.xu
	/// \date 2017/09/08
	///

	xgc_bool xvariant::is_integer() const
	{
		switch( type() )
		{
			case vt_char:
			case vt_i16:
			case vt_i32:
			case vt_i64:
			return true;
		}

		return false;
	}

	///
	/// \brief 是否无符号数
	/// \author albert.xu
	/// \date 2017/09/08
	///

	xgc_bool xvariant::is_unsigned() const
	{
		switch( type() )
		{
			case vt_byte:
			case vt_u16:
			case vt_u32:
			case vt_u64:
			return true;
		}

		return false;
	}

	///
	/// \brief 是否实数
	/// \author albert.xu
	/// \date 2017/09/08
	///

	xgc_bool xvariant::is_real() const
	{
		switch( type() )
		{
			case vt_real:
			case vt_real64:
			return true;
		}

		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	// 字符串赋值
	xgc_void xvariant::set_string( xgc_lpcstr _Val, xgc_size _Length )
	{
		XGC_ASSERT_POINTER( _Val );
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( _Val && is_string(), xgc_void( 0 ) );

		xgc_size _Len = ( -1 == _Length ) ? strlen( _Val ) : _Length;

		if( mString == xgc_nullptr )
		{
			StringPtr pNew = ( StringPtr )malloc( sizeof( String ) + _Len );
			if( pNew == xgc_nullptr )
				return;

			( mString ) = pNew;
			( mString )->alc = (xgc_uint16)_Len + 1;
		}
		else if( ( mString )->alc <= _Len )
		{
			StringPtr pNew = ( StringPtr )realloc( ( mString ), sizeof( String ) + _Len );
			if( pNew == xgc_nullptr )
				return;

			( mString ) = pNew;
			( mString )->alc = (xgc_uint16)_Len + 1;
		}

		strcpy_s( ( mString )->str, ( mString )->alc, _Val );
		( mString )->len = (xgc_uint16)_Len;
		FUNCTION_END;
	}

	// 缓冲区赋值
	xgc_void xvariant::set_buffer( xgc_lpvoid _Val, xgc_size _Length )
	{
		XGC_ASSERT_POINTER( _Val );
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( is_buffer(), xgc_void( 0 ) );

		if( mBuffer == xgc_nullptr )
		{
			xgc_size alloc_size = sizeof( Buffer ) + _Length;
			mBuffer = ( BufferPtr )malloc( alloc_size );
			if( mBuffer )
			{
				mBuffer->alc = (xgc_uint16)_Length;
				mBuffer->len = (xgc_uint16)_Length;
			}
		}
		else if( mBuffer->alc < _Length )
		{
			xgc_size alloc_size = sizeof( Buffer ) + _Length;

			BufferPtr pNewBuffer = ( BufferPtr ) realloc( mBuffer, alloc_size );
			if( pNewBuffer )
			{
				mBuffer = pNewBuffer;
				mBuffer->alc = (xgc_uint16)_Length;
			}
		}

		memcpy( mBuffer->buf, _Val, _Length );
		FUNCTION_END;
	}

	xgc_bool xvariant::set_buffer_len( xgc_size _Len, xgc_byte _Fill /*= 0 */ )
	{
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( is_buffer(), false );

		if( mBuffer == xgc_nullptr )
		{
			xgc_size alloc_size = sizeof( Buffer ) + _Len;
			mBuffer = ( BufferPtr )malloc( alloc_size );
			XGC_ASSERT_RETURN( ( mBuffer ), false );

			mBuffer->alc = (xgc_uint16)_Len;
			mBuffer->len = (xgc_uint16)_Len;

			memset( mBuffer->buf, _Fill, _Len );
		}
		else if( mBuffer->alc < _Len )
		{
			xgc_size alloc_size = sizeof( Buffer ) + _Len;

			BufferPtr pNewBuffer = ( BufferPtr ) realloc( mBuffer, alloc_size );
			XGC_ASSERT_RETURN( pNewBuffer, false );
			mBuffer = pNewBuffer;
			mBuffer->alc = (xgc_uint16)_Len;
			memset( mBuffer->buf + mBuffer->len, _Fill, _Len - mBuffer->len );
		}
		return true;
		FUNCTION_END;
		return false;
	}

	xgc_size xvariant::get_buffer_len()const
	{
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( is_buffer(), false );
		if( mBuffer )
			return mBuffer->len;
		FUNCTION_END;
		return 0;
	}

	// 拷贝
	xgc_void xvariant::assign( const xvariant& _Val )
	{
		FUNCTION_BEGIN;
		switch( _Val.type() )
		{
			case vt_bool:
			set_value( _Val.mBool );
			break;
			case vt_char:
			set_value( _Val.mChar );
			break;
			case vt_byte:
			set_value( _Val.mByte );
			break;
			case vt_i16:
			set_value( _Val.mShort );
			break;
			case vt_u16:
			set_value( _Val.mUShort );
			break;
			case vt_i32:
			set_value( _Val.mInteger );
			break;
			case vt_u32:
			set_value( _Val.mUnsigned );
			break;
			case vt_real:
			set_value( _Val.mReal32 );
			break;
			case vt_string:
			set_string( ( _Val.mString )->str );
			break;
			case vt_buffer:
			set_buffer( ( _Val.mBuffer )->buf, ( _Val.mBuffer )->len );
			break;
			default:
			XGC_DEBUG_MESSAGE( "非法的类型!" );
			break;
		}
		FUNCTION_END;
	}

	xgc_lpcstr xvariant::get_string( size_t *len /*= xgc_nullptr*/ ) const
	{
		XGC_ASSERT_RETURN( is_string(), xgc_nullptr );

		if( mString )
		{
			if( len ) *len = mString->len;
			return mString->str;
		}
		else
		{
			if( len ) *len = 0;
			return xgc_nullptr;
		}
	}

	xgc_lpvoid xvariant::get_buffer( size_t *len /*= xgc_nullptr*/ ) const
	{
		XGC_ASSERT_RETURN( is_buffer(), xgc_nullptr );
		if( mBuffer )
		{
			if( len ) *len = mBuffer->len;
			return mBuffer->buf;
		}
		else
		{
			if( len ) *len = 0;
			return xgc_nullptr;
		}
	}

	///
	/// \brief 转为字符串
	/// \author albert.xu
	/// \date 1/9/2014
	///
	xgc::string xvariant::to_string()const
	{
		xgc_char buf[64] = { 0 };
		int copid = 0;
		switch( type() )
		{
			case vt_char:
			copid = sprintf_s( buf, "%d", mChar );
			break;
			case vt_i16:
			copid = sprintf_s( buf, "%d", mShort );
			break;
			case vt_i32:
			copid = sprintf_s( buf, "%d", mInteger );
			break;
			case vt_i64:
			copid = sprintf_s( buf, "%I64d", mLong );
			break;
			case vt_byte:
			copid = sprintf_s( buf, "%u", mByte );
			break;
			case vt_u16:
			copid = sprintf_s( buf, "%u", mUShort );
			break;
			case vt_u32:
			copid = sprintf_s( buf, "%u", mUnsigned );
			break;
			case vt_u64:
			copid = sprintf_s( buf, "%I64u", mULong );
			break;
			case vt_real:
			copid = sprintf_s( buf, "%f", mReal32 );
			break;
			case vt_string:
			if( mString )
				return xgc::string( (xgc_lpcstr)mString->str, mString->len );
			break;
			case vt_buffer:
			if( mBuffer )
				return xgc::string( (xgc_lpcstr)mBuffer->buf, mBuffer->len );
			break;
		}

		return buf;
	}

	///
	/// \brief 转为字符串
	/// \author albert.xu
	/// \date 2017/09/08
	///
	xgc_long xvariant::to_string( xgc_lpvoid buf, xgc_size len )const
	{
		int copid = 0;
		switch( type() )
		{
			case vt_char:
			copid = sprintf_s( (char*)buf, len, "%d", mChar );
			break;
			case vt_i16:
			copid = sprintf_s( (char*)buf, len, "%d", mShort );
			break;
			case vt_i32:
			copid = sprintf_s( (char*)buf, len, "%d", mInteger );
			break;
			case vt_i64:
			copid = sprintf_s( (char*)buf, len, "%I64d", mLong );
			break;
			case vt_byte:
			copid = sprintf_s( (char*)buf, len, "%u", mByte );
			break;
			case vt_u16:
			copid = sprintf_s( (char*)buf, len, "%u", mUShort );
			break;
			case vt_u32:
			copid = sprintf_s( (char*)buf, len, "%u", mUnsigned );
			break;
			case vt_u64:
			copid = sprintf_s( (char*)buf, len, "%I64u", mULong );
			break;
			case vt_real:
			copid = sprintf_s( (char*)buf, len, "%f", mReal32 );
			break;
			case vt_string:
			if( mString )
			{
				copid = XGC_MIN( (int)len, (int)mString->len );
				memcpy( buf, mString->str, copid );
			}
			break;
			case vt_buffer:
			if( mBuffer )
			{
				copid = XGC_MIN( (int)len, (int)mString->len );
				memcpy( buf, mBuffer->buf, copid );
			}
			break;
		}

		return copid;
	}

	//-----------------------------------//
	// [1/17/2014 albert.xu]
	// 设置缓冲区位
	//-----------------------------------//
	xgc_void xvariant::set_bit( xgc_size idx )
	{
		xgc_char bit = ( 1 << ( 0x07 - ( ( idx >> 3 ) & 0x07 ) ) );

		switch( type() )
		{
			case vt_char:
			case vt_byte:
			case vt_i16:
			case vt_u16:
			case vt_i32:
			case vt_u32:
			case vt_i64:
			case vt_u64:
			{
				XGC_ASSERT_RETURN( idx < type2size(), XGC_NONE, "%s", "设置的位超出类型长度。" );
				Byte* bytes = &mByte;
				bytes[idx] |= bit;
			}
			break;
			case vt_buffer:
			{
				XGC_ASSERT_RETURN( mBuffer && idx < mBuffer->len, XGC_NONE, "%s", "设置的位长度超出缓冲区" );
				Byte* bytes = mBuffer->buf;
				bytes[idx] |= bit;
			}
			break;
			default:
			XGC_ASSERT_RETURN( false, xgc_void( 0 ), "对不正确的类型使用SetBit" );
			break;
		}
	}

	//-----------------------------------//
	// [1/17/2014 albert.xu]
	// 设置缓冲区位
	//-----------------------------------//
	xgc_void xvariant::clr_bit( xgc_size idx )
	{
		xgc_char bit = ~( 1 << ( 0x07 - ( ( idx >> 3 ) & 0x07 ) ) );

		xgc_byteptr pb = xgc_nullptr;
		switch( type() )
		{
			case vt_char:
			case vt_byte:
			case vt_i16:
			case vt_u16:
			case vt_i32:
			case vt_u32:
			case vt_i64:
			case vt_u64:
			{
				XGC_ASSERT_RETURN( idx < type2size(), xgc_void( 0 ), "设置的位超出类型长度。" );
				auto bytes = &mByte;
				bytes[idx] &= bit;
			}
			break;
			case vt_buffer:
			{
				XGC_ASSERT_RETURN( mBuffer && idx < ( mBuffer )->len, xgc_void( 0 ), "设置的位长度超出缓冲区" );
				auto bytes = mBuffer->buf;
				bytes[idx] &= bit;
			}
			break;
			default:
			XGC_ASSERT_RETURN( false, xgc_void( 0 ), "对不正确的类型使用SetBit" );
			break;
		}
	}

	//-----------------------------------//
	// [1/17/2014 albert.xu]
	// 获取缓冲区位
	//-----------------------------------//
	xgc_bool xvariant::get_bit( xgc_size idx, xgc_bool v )const
	{
		xgc_byte bit = ( 1 << ( 0x07 - ( ( idx >> 3 ) & 0x07 ) ) );

		xgc_byteptr pb = xgc_nullptr;
		switch( type() )
		{
			case vt_char:
			case vt_byte:
			case vt_i16:
			case vt_u16:
			case vt_i32:
			case vt_u32:
			case vt_i64:
			case vt_u64:
			{
				XGC_ASSERT_RETURN( idx < type2size(), v, "设置的位超出类型长度。" );
				auto bytes = &mByte;
				return ( bytes[idx] & bit ) == bit;
			}
			break;
			case vt_buffer:
			{
				XGC_ASSERT_RETURN( mBuffer && idx < ( mBuffer )->len, v, "设置的位长度超出缓冲区" );
				auto bytes = &mByte;
				return ( bytes[idx] & bit ) == bit;
			}
			break;
			default:
			XGC_ASSERT_RETURN( false, v, "对不正确的类型使用SetBit" );
			break;
		}

		return v;
	}

	///
	/// [1/17/2014 albert.xu]
	/// 获取缓冲区位
	///
	xgc_bool xvariant::cmp_bit( xgc_size idx, xgc_bool v )const
	{
		return get_bit( idx, true ) == v;
	}

	xgc_void xvariant::multiplies( const xvariant& _Val )
	{
		switch( _Val.type() )
		{
			case vt_char:
			multiplies( _Val.mChar );
			break;
			case vt_i16:
			multiplies( _Val.mShort );
			break;
			case vt_i32:
			multiplies( _Val.mInteger );
			break;
			case vt_i64:
			multiplies( _Val.mLong );
			break;
			case vt_byte:
			multiplies( _Val.mByte );
			break;
			case vt_u16:
			multiplies( _Val.mUShort );
			break;
			case vt_u32:
			multiplies( _Val.mUnsigned );
			break;
			case vt_u64:
			multiplies( _Val.mULong );
			break;
			case vt_real:
			multiplies( _Val.mReal32 );
			break;
			default:
			XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
			break;
		}
	}

	xgc_void xvariant::division( const xvariant& _Val )
	{
		switch( _Val.type() )
		{
			case vt_char:
			division( _Val.mChar );
			break;
			case vt_i16:
			division( _Val.mShort );
			break;
			case vt_i32:
			division( _Val.mInteger );
			break;
			case vt_i64:
			division( _Val.mLong );
			break;
			case vt_byte:
			division( _Val.mByte );
			break;
			case vt_u16:
			division( _Val.mUShort );
			break;
			case vt_u32:
			division( _Val.mUnsigned );
			break;
			case vt_u64:
			division( _Val.mULong );
			break;
			case vt_real:
			division( _Val.mReal32 );
			break;
			default:
			XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
			break;
		}
	}

	xgc_void xvariant::plus( const xvariant& _Val )
	{
		switch( _Val.type() )
		{
			case vt_char:
			plus( _Val.mChar );
			break;
			case vt_i16:
			plus( _Val.mShort );
			break;
			case vt_i32:
			plus( _Val.mInteger );
			break;
			case vt_i64:
			plus( _Val.mLong );
			break;
			case vt_byte:
			plus( _Val.mByte );
			break;
			case vt_u16:
			plus( _Val.mUShort );
			break;
			case vt_u32:
			plus( _Val.mUnsigned );
			break;
			case vt_u64:
			plus( _Val.mULong );
			break;
			case vt_real:
			plus( _Val.mReal32 );
			break;
			default:
			XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
			break;
		}
	}

	xgc_void xvariant::minus( const xvariant& _Val )
	{
		switch( _Val.type() )
		{
			case vt_char:
			minus( _Val.mChar );
			break;
			case vt_i16:
			minus( _Val.mShort );
			break;
			case vt_i32:
			minus( _Val.mInteger );
			break;
			case vt_i64:
			minus( _Val.mLong );
			break;
			case vt_byte:
			minus( _Val.mByte );
			break;
			case vt_u16:
			minus( _Val.mUShort );
			break;
			case vt_u32:
			minus( _Val.mUnsigned );
			break;
			case vt_u64:
			minus( _Val.mULong );
			break;
			case vt_real:
			minus( _Val.mReal32 );
			break;
			default:
			XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
			break;
		}
	}
}