#include "XHeader.h"
#include "XAttribute.h"

namespace xgc
{
	//////////////////////////////////////////////////////////////////////////
	// 字符串赋值
	xgc_void XAttribute::setString( xgc_lpcstr _Val, xgc_size _Length )
	{
		XGC_ASSERT_RETURN( _Val && isString(), XGC_NONE );

		xgc_size _Len = ( -1 == _Length ) ? strlen( _Val ) : _Length;

		if( *mpString == xgc_nullptr )
		{
			XAttribute::StringPtr pNew = ( XAttribute::StringPtr )malloc( sizeof( XAttribute::String ) + _Len );
			if( pNew == xgc_nullptr )
				return;

			( *mpString ) = pNew;
			( *mpString )->alc = (xgc_uint16) _Len + 1;
		}
		else if( ( *mpString )->alc <= _Len )
		{
			XAttribute::StringPtr pNew = ( XAttribute::StringPtr )realloc( ( *mpString ), sizeof( XAttribute::String ) + _Len );
			if( pNew == xgc_nullptr )
				return;

			( *mpString ) = pNew;
			( *mpString )->alc = (xgc_uint16) _Len + 1;
		}

		strcpy_s( ( *mpString )->str, ( *mpString )->alc, _Val );
		( *mpString )->len = (xgc_uint16) _Len;
	}

	// 缓冲区赋值
	xgc_void XAttribute::setBuffer( xgc_lpvoid _Val, xgc_size _Length )
	{
		XGC_ASSERT_RETURN( isBuffer(), xgc_void( 0 ) );

		if( ( *mpBuffer ) == xgc_nullptr )
		{
			xgc_size alloc_size = sizeof( XAttribute::Buffer ) + _Length;
			( *mpBuffer ) = ( XAttribute::BufferPtr )malloc( alloc_size );
			if( ( *mpBuffer ) )
			{
				( *mpBuffer )->alc = (xgc_uint16)_Length;
				( *mpBuffer )->len = (xgc_uint16)_Length;
			}
		}
		else if( ( *mpBuffer )->alc < _Length )
		{
			xgc_size alloc_size = sizeof( XAttribute::Buffer ) + _Length;

			XAttribute::BufferPtr pNewBuffer = ( XAttribute::BufferPtr ) realloc( ( *mpBuffer ), alloc_size );
			if( pNewBuffer )
			{
				( *mpBuffer ) = pNewBuffer;
				( *mpBuffer )->alc = (xgc_uint16) _Length;
			}
		}

		memcpy( ( *mpBuffer )->buf, _Val, _Length );
	}

	xgc_bool XAttribute::setBufferLength( xgc_size _Len, xgc_byte _Fill /*= 0 */ )const
	{
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( isBuffer(), false );

		if( ( *mpBuffer ) == xgc_nullptr )
		{
			xgc_size alloc_size = sizeof( XAttribute::Buffer ) + _Len;
			( *mpBuffer ) = ( XAttribute::BufferPtr )malloc( alloc_size );
			XGC_ASSERT_RETURN( ( *mpBuffer ), false );
				
			( *mpBuffer )->alc = (xgc_uint16)_Len;
			( *mpBuffer )->len = (xgc_uint16)_Len;

			memset( ( *mpBuffer )->buf, _Fill, _Len );
		}
		else if( ( *mpBuffer )->alc < _Len )
		{
			xgc_size alloc_size = sizeof( XAttribute::Buffer ) + _Len;

			XAttribute::BufferPtr pNewBuffer = ( XAttribute::BufferPtr ) realloc( ( *mpBuffer ), alloc_size );
			XGC_ASSERT_RETURN( pNewBuffer, false );
			( *mpBuffer ) = pNewBuffer;
			( *mpBuffer )->alc = (xgc_uint16)_Len;
			memset( ( *mpBuffer )->buf + ( *mpBuffer )->len, _Fill, _Len - ( *mpBuffer )->len );
		}
		return true;
		FUNCTION_END;
		return false;
	}

	xgc_size XAttribute::getBufferLength()const
	{
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( isBuffer(), false );
		if( ( *mpBuffer ) )
			return ( *mpBuffer )->len;
		FUNCTION_END;
		return 0;
	}

	// 拷贝
	xgc_void XAttribute::assign( const XAttribute& _Val )
	{
		FUNCTION_BEGIN;
		switch( _Val.getType() )
		{
		case VT_BOOL:
			setValue( *_Val.mpBool );
			break;
		case VT_CHAR:
			setValue( *_Val.mpChar );
			break;
		case VT_BYTE:
			setValue( *_Val.mpByte );
			break;
		case VT_I16:
			setValue( *_Val.mpShort );
			break;
		case VT_U16:
			setValue( *_Val.mpUShort );
			break;
		case VT_I32:
			setValue( *_Val.mpInteger );
			break;
		case VT_U32:
			setValue( *_Val.mpUnsigned );
			break;
		case VT_REAL32:
			setValue( *_Val.mpReal32 );
			break;
		case VT_STRING:
			setString( ( *_Val.mpString )->str );
			break;
		case VT_BUFFER:
			setBuffer( ( *_Val.mpBuffer )->buf, ( *_Val.mpBuffer )->len );
			break;
		default:
			XGC_DEBUG_MESSAGE( "非法的类型!" );
			break;
		}
		FUNCTION_END;
	}

	xgc_lpcstr XAttribute::toRawString( xgc_lpcstr lpDefault /*= ""*/ ) const
	{
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( isString(), lpDefault );

		return ( *mpString ) ? (xgc_lpcstr) ( *mpString )->str : lpDefault;
		FUNCTION_END;

		return lpDefault;
	}

	xgc_lpvoid XAttribute::toRawBuffer() const
	{
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( isBuffer(), xgc_nullptr );
		return ( *mpBuffer ) ? ( *mpBuffer )->buf : xgc_nullptr;
		FUNCTION_END;

		return xgc_nullptr;
	}

	xgc::string XAttribute::toString()const
	{
		FUNCTION_BEGIN;
		xgc_char szValue[64] = { 0 };
		switch( getType() )
		{
		case VT_CHAR:
			sprintf_s( szValue, "%d", *mpChar );
			break;
		case VT_I16:
			sprintf_s( szValue, "%d", *mpShort );
			break;
		case VT_I32:
			sprintf_s( szValue, "%d", *mpInteger );
			break;
		case VT_I64:
			sprintf_s( szValue, "%I64d", *mpLong );
			break;
		case VT_BYTE:
			sprintf_s( szValue, "%u", *mpByte );
			break;
		case VT_U16:
			sprintf_s( szValue, "%u", *mpUShort );
			break;
		case VT_U32:
			sprintf_s( szValue, "%u", *mpUnsigned );
			break;
		case VT_U64:
			sprintf_s( szValue, "%I64u", *mpULong );
			break;
		case VT_REAL32:
			sprintf_s( szValue, "%f", *mpReal32 );
			break;
		case VT_STRING:
			if( *mpString )
				xgc::string( (xgc_lpcstr) ( *mpString )->str, ( *mpString )->len );
			break;
		case VT_BUFFER:
			if( *mpBuffer )
				xgc::string( (xgc_lpcstr) ( *mpBuffer )->buf, ( *mpBuffer )->len );
			break;
		default:
			break;
		}
		FUNCTION_END;
		return "";
	}

	//-----------------------------------//
	// [1/17/2014 albert.xu]
	// 设置缓冲区位
	//-----------------------------------//
	xgc_void XAttribute::SetBit( xgc_size _Bit )const
	{
		FUNCTION_BEGIN;
		xgc_size idx = ( _Bit >> 3 );
		xgc_char bit = ( 1 << ( 0x07 - ( _Bit & 0x07 ) ) );
		
		xgc_byteptr pb = xgc_nullptr;
		switch( getType() )
		{
		case VT_CHAR:
		case VT_BYTE:
		case VT_I16:
		case VT_U16:
		case VT_I32:
		case VT_U32:
		case VT_I64:
		case VT_U64:
			XGC_ASSERT_RETURN( idx < Type2Size( getType() ), XGC_NONE, "%s", "设置的位超出类型长度。" );
			pb = mpByte;
			break;
		case VT_BUFFER:
			XGC_ASSERT_RETURN( *mpBuffer && idx < ( *mpBuffer )->len, XGC_NONE, "%s", "设置的位长度超出缓冲区" );
			pb = ( *mpBuffer )->buf;
			break;
		default:
			XGC_ASSERT_RETURN( false, xgc_void( 0 ), "对不正确的类型使用SetBit" );
			break;
		}

		pb[idx] |= bit;
		FUNCTION_END;
	}

	//-----------------------------------//
	// [1/17/2014 albert.xu]
	// 设置缓冲区位
	//-----------------------------------//
	xgc_void XAttribute::ClrBit( xgc_size _Bit )const
	{
		FUNCTION_BEGIN;
		xgc_size idx = ( _Bit >> 3 );
		xgc_char bit = ~( 1 << ( 0x07 - ( _Bit & 0x07 ) ) );

		xgc_byteptr pb = xgc_nullptr;
		switch( getType() )
		{
		case VT_CHAR:
		case VT_BYTE:
		case VT_I16:
		case VT_U16:
		case VT_I32:
		case VT_U32:
		case VT_I64:
		case VT_U64:
			XGC_ASSERT_RETURN( idx < Type2Size( getType() ), xgc_void( 0 ), "设置的位超出类型长度。" );
			pb = mpByte;
			break;
		case VT_BUFFER:
			XGC_ASSERT_RETURN( *mpBuffer && idx < ( *mpBuffer )->len, xgc_void( 0 ), "设置的位长度超出缓冲区" );
			pb = ( *mpBuffer )->buf;
			break;
		default:
			XGC_ASSERT_RETURN( false, xgc_void( 0 ), "对不正确的类型使用SetBit" );
			break;
		}

		pb[idx] &= bit;
		FUNCTION_END;
	}

	//-----------------------------------//
	// [1/17/2014 albert.xu]
	// 获取缓冲区位
	//-----------------------------------//
	xgc_bool XAttribute::GetBit( xgc_size _Bit, xgc_bool _Default )const
	{
		FUNCTION_BEGIN;
		xgc_size idx = _Bit >> 3;
		xgc_byte bit = ( 1 << ( 0x07 - ( _Bit & 0x07 ) ) );

		xgc_byteptr pb = xgc_nullptr;
		switch( getType() )
		{
		case VT_CHAR:
		case VT_BYTE:
		case VT_I16:
		case VT_U16:
		case VT_I32:
		case VT_U32:
		case VT_I64:
		case VT_U64:
			XGC_ASSERT_RETURN( idx < Type2Size( getType() ), _Default, "设置的位超出类型长度。" );
			pb = mpByte;
			break;
		case VT_BUFFER:
			XGC_ASSERT_RETURN( *mpBuffer && idx < ( *mpBuffer )->len, _Default, "设置的位长度超出缓冲区" );
			pb = ( *mpBuffer )->buf;
			break;
		default:
			XGC_ASSERT_RETURN( false, _Default, "对不正确的类型使用SetBit" );
			break;
		}

		return ( pb[idx] & bit ) == bit;
		FUNCTION_END;
		return _Default;
	}

	///
	/// [1/17/2014 albert.xu]
	/// 获取缓冲区位
	///
	XGC_INLINE xgc_bool XAttribute::CmpBit( xgc_size _Bit, xgc_bool _Test )const
	{
		return GetBit( _Bit, true ) == _Test;
	}

	XGC_INLINE xgc_void XAttribute::multiplies( const XAttribute& _Val )
	{
		FUNCTION_BEGIN;
		switch( _Val.getType() )
		{
		case VT_CHAR:
			multiplies( *_Val.mpChar );
			break;
		case VT_I16:
			multiplies( *_Val.mpShort );
			break;
		case VT_I32:
			multiplies( *_Val.mpInteger );
			break;
		case VT_I64:
			multiplies( *_Val.mpLong );
			break;
		case VT_BYTE:
			multiplies( *_Val.mpByte );
			break;
		case VT_U16:
			multiplies( *_Val.mpUShort );
			break;
		case VT_U32:
			multiplies( *_Val.mpUnsigned );
			break;
		case VT_U64:
			multiplies( *_Val.mpULong );
			break;
		case VT_REAL32:
			multiplies( *_Val.mpReal32 );
			break;
		default:
			XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
			break;
		}
		FUNCTION_END;
	}

	XGC_INLINE xgc_void XAttribute::division( const XAttribute& _Val )
	{
		FUNCTION_BEGIN;
		switch( _Val.getType() )
		{
		case VT_CHAR:
			division( *_Val.mpChar );
			break;
		case VT_I16:
			division( *_Val.mpShort );
			break;
		case VT_I32:
			division( *_Val.mpInteger );
			break;
		case VT_I64:
			division( *_Val.mpLong );
			break;
		case VT_BYTE:
			division( *_Val.mpByte );
			break;
		case VT_U16:
			division( *_Val.mpUShort );
			break;
		case VT_U32:
			division( *_Val.mpUnsigned );
			break;
		case VT_U64:
			division( *_Val.mpULong );
			break;
		case VT_REAL32:
			division( *_Val.mpReal32 );
			break;
		default:
			XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
			break;
		}
		FUNCTION_END;
	}

	XGC_INLINE xgc_void XAttribute::plus( const XAttribute& _Val )
	{
		FUNCTION_BEGIN;
		switch( _Val.getType() )
		{
		case VT_CHAR:
			plus( *_Val.mpChar );
			break;
		case VT_I16:
			plus( *_Val.mpShort );
			break;
		case VT_I32:
			plus( *_Val.mpInteger );
			break;
		case VT_I64:
			plus( *_Val.mpLong );
			break;
		case VT_BYTE:
			plus( *_Val.mpByte );
			break;
		case VT_U16:
			plus( *_Val.mpUShort );
			break;
		case VT_U32:
			plus( *_Val.mpUnsigned );
			break;
		case VT_U64:
			plus( *_Val.mpULong );
			break;
		case VT_REAL32:
			plus( *_Val.mpReal32 );
			break;
		default:
			XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
			break;
		}
		FUNCTION_END;
	}

	XGC_INLINE xgc_void XAttribute::minus( const XAttribute& _Val )
	{
		FUNCTION_BEGIN;
		switch( _Val.getType() )
		{
		case VT_CHAR:
			minus( *_Val.mpChar );
			break;
		case VT_I16:
			minus( *_Val.mpShort );
			break;
		case VT_I32:
			minus( *_Val.mpInteger );
			break;
		case VT_I64:
			minus( *_Val.mpLong );
			break;
		case VT_BYTE:
			minus( *_Val.mpByte );
			break;
		case VT_U16:
			minus( *_Val.mpUShort );
			break;
		case VT_U32:
			minus( *_Val.mpUnsigned );
			break;
		case VT_U64:
			minus( *_Val.mpULong );
			break;
		case VT_REAL32:
			minus( *_Val.mpReal32 );
			break;
		default:
			XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
			break;
		}
		FUNCTION_END;
	}

	attr_buffer& operator >> ( attr_buffer& stream, XAttribute& c )
	{
		switch( c.getType() )
		{
			case VT_CHAR:
			stream >> *c.mpChar;
			break;
			case VT_BOOL:
			stream >> *c.mpBool;
			break;
			case VT_BYTE:
			stream >> *c.mpByte;
			break;
			case VT_I16:
			stream >> *c.mpShort;
			break;
			case VT_U16:
			stream >> *c.mpUShort;
			break;
			case VT_I32:
			stream >> *c.mpInteger;
			break;
			case VT_U32:
			stream >> *c.mpUnsigned;
			break;
			case VT_I64:
			stream >> *c.mpLong;
			break;
			case VT_U64:
			stream >> *c.mpULong;
			break;
			case VT_REAL32:
			stream >> *c.mpReal32;
			break;
			case VT_REAL64:
			stream >> *c.mpReal64;
			break;
			case VT_STRING:
			{
				xgc_lpcstr _Val;
				stream >> _Val;
				c = _Val;
			}
			break;
			case VT_BUFFER:
			{
				xgc_uint16 uLength = 0;
				stream >> uLength;
				XGC_ASSERT( uLength <= stream.leave() );
				c.setBuffer( (xgc_lpvoid)stream.rd_ptr(), uLength );

				stream.plus_rd( uLength );
			}
			break;
		}
		return stream;
	}

	///
	/// [3/17/2014 albert.xu]
	/// 序列化读
	///
	CORE_API attr_buffer& operator >> ( attr_buffer& stream, XAttribute&& c )
	{
		return stream >> c;
	}

	attr_buffer& operator << ( attr_buffer& stream, const XAttribute& c )
	{
		switch( c.getType() )
		{
			case VT_CHAR:
			stream << *c.mpChar;
			break;
			case VT_BOOL:
			stream << *c.mpBool;
			break;
			case VT_BYTE:
			stream << *c.mpByte;
			break;
			case VT_I16:
			stream << *c.mpShort;
			break;
			case VT_U16:
			stream << *c.mpUShort;
			break;
			case VT_I32:
			stream << *c.mpInteger;
			break;
			case VT_U32:
			stream << *c.mpUnsigned;
			break;
			case VT_I64:
			stream << *c.mpLong;
			break;
			case VT_U64:
			stream << *c.mpULong;
			break;
			case VT_REAL32:
			stream << *c.mpReal32;
			break;
			case VT_REAL64:
			stream << *c.mpReal64;
			break;
			case VT_STRING:
			stream << ( *c.mpString ? ( *c.mpString )->str : "" );
			break;
			case VT_BUFFER:
			if( *c.mpBuffer )
			{
				stream << ( *c.mpBuffer )->len;
				stream.write_some( ( *c.mpBuffer )->buf, ( *c.mpBuffer )->len );
			}
			else
			{
				stream << (xgc_uint16) 0U;
			}
			break;
		}
		return stream;
	}

	CORE_API attr_buffer& operator << ( attr_buffer& stream, XAttribute&& c )
	{
		return stream << c;
	}
}