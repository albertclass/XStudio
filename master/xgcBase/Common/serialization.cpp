#include "serialization.h"
namespace XGC
{
	namespace common
	{
		template< class type >
		serialization& serialization_in( serialization& stream, type v )	{ stream.putn( (xgc_byte*)&v, sizeof(v) ); return stream; }

		template< class type >
		serialization& serialization_out( serialization& stream, type &v )	{ stream.getn( (xgc_byte*)&v, sizeof(type) ); return stream; }

		serialization& operator << ( serialization& stream, bool c )			{ return serialization_in< bool >( stream, c ); }
		serialization& operator << ( serialization& stream, char c )			{ return serialization_in< char >( stream, c ); }
		serialization& operator << ( serialization& stream, short c )			{ return serialization_in< short >( stream, c ); }
		serialization& operator << ( serialization& stream, int c )				{ return serialization_in< int >( stream, c ); }
		serialization& operator << ( serialization& stream, long c )			{ return serialization_in< long >( stream, c ); }
		serialization& operator << ( serialization& stream, long long c )		{ return serialization_in< long long >( stream, c ); }
		serialization& operator << ( serialization& stream, unsigned char c )	{ return serialization_in< unsigned char >( stream, c ); }
		serialization& operator << ( serialization& stream, unsigned short c )	{ return serialization_in< unsigned short >( stream, c ); }
		serialization& operator << ( serialization& stream, unsigned int c )	{ return serialization_in< unsigned int >( stream, c ); }
		serialization& operator << ( serialization& stream, unsigned long c )	{ return serialization_in< unsigned long >( stream, c ); }
		serialization& operator << ( serialization& stream, unsigned long long c ){ return serialization_in< unsigned long long >( stream, c ); }
		serialization& operator << ( serialization& stream, float c )			{ return serialization_in< float >( stream, c ); }
		serialization& operator << ( serialization& stream, double c )			{ return serialization_in< double >( stream, c ); }
		serialization& operator << ( serialization& stream, long double c )		{ return serialization_in< long double >( stream, c ); }
		serialization& operator << ( serialization& stream, const std::string &c )
		{
			stream.putn( (xgc_byte*)c.c_str(), c.length()+1 );
			return stream;
		}

		serialization& operator << ( serialization& stream, const std::wstring &c )
		{
			stream.putn( (xgc_byte*)c.c_str(), (c.length()<<1)+sizeof( wchar_t/*std::wstring::traits_type::char_type*/ ) );
			return stream;
		}

		serialization& operator << ( serialization& stream, const char* c )
		{
			if( c )
				stream.putn( (xgc_byte*) c, strlen( c ) + 1 );
			else
				stream.putn( (xgc_byte*) "null", 5 );
			return stream;
		}

		serialization& operator << ( serialization& stream, const wchar_t* c )
		{
			if( c )
				stream.putn( (xgc_byte*) c, ( wcslen( c ) + 1 )*sizeof( wchar_t ) );
			else
				stream.putn( (xgc_byte*) L"null", 5 * sizeof( wchar_t ) );
			return stream;
		}

		//serialization& operator << ( serialization& stream, const buffer &c )
		//{
		//	size_t size = c.capacity();
		//	stream.putn( (char*)&size, sizeof(size_t) );
		//	stream.putn( (char*)c.base(), size );
		//	return stream;
		//}

		serialization& operator >> ( serialization& stream, bool &c )			{ return serialization_out< bool >( stream, c ); }
		serialization& operator >> ( serialization& stream, char &c )			{ return serialization_out< char >( stream, c ); }
		serialization& operator >> ( serialization& stream, short &c )			{ return serialization_out< short >( stream, c ); }
		serialization& operator >> ( serialization& stream, int &c )			{ return serialization_out< int >( stream, c ); }
		serialization& operator >> ( serialization& stream, long &c )			{ return serialization_out< long >( stream, c ); }
		serialization& operator >> ( serialization& stream, long long &c )		{ return serialization_out< long long >( stream, c ); }
		serialization& operator >> ( serialization& stream, unsigned char &c )	{ return serialization_out< unsigned char >( stream, c ); }
		serialization& operator >> ( serialization& stream, unsigned short &c )	{ return serialization_out< unsigned short >( stream, c ); }
		serialization& operator >> ( serialization& stream, unsigned int &c )	{ return serialization_out< unsigned int >( stream, c ); }
		serialization& operator >> ( serialization& stream, unsigned long &c )	{ return serialization_out< unsigned long >( stream, c ); }
		serialization& operator >> ( serialization& stream, unsigned long long &c){ return serialization_out< unsigned long long >( stream, c ); }
		serialization& operator >> ( serialization& stream, float &c )			{ return serialization_out< float >( stream, c ); }
		serialization& operator >> ( serialization& stream, double &c )			{ return serialization_out< double >( stream, c ); }
		serialization& operator >> ( serialization& stream, long double &c )	{ return serialization_out< long double >( stream, c ); }
		serialization& operator >> ( serialization& stream, std::string &c )
		{
			c = (const char*)stream.rd_ptr();
			stream.seek( c.length()+1, serialization::eWithCurrent, serialization::eRead );
			return stream;
		}

		serialization& operator >> ( serialization& stream, std::wstring &c )
		{
			c = (const wchar_t*)stream.rd_ptr();
			stream.seek( (c.length()<<1)+sizeof(wchar_t), serialization::eWithCurrent, serialization::eRead );
			return stream;
		}

		serialization& operator >> ( serialization& stream, const char* &c )
		{
			c = (const char*)stream.rd_ptr();
			stream.seek( strlen(c)+1, serialization::eWithCurrent, serialization::eRead );
			return stream;
		}

		serialization& operator >> ( serialization& stream, const wchar_t* &c )
		{
			c = (const wchar_t*)stream.rd_ptr();
			stream.seek( (wcslen(c) + 1) * sizeof(wchar_t), serialization::eWithCurrent, serialization::eRead );
			return stream;
		}

		//serialization& operator >> ( serialization& stream, buffer &c )
		//{
		//	size_t size;
		//	stream.getn( &size, sizeof(size_t) );
		//	if( size > c.capacity() )
		//		throw( std::exception( "overflow", -1 ) );
		//	stream.getn( c.base(), size );
		//}

		void serialization::_init_buf( xgc_uint8 mode, xgc_size off )
		{
			if( mode & eWrite )
			{
				_wd = off;
				_wsize = _length;
			}

			if( mode & eRead )
			{
				_rd = off;
				_rsize = _length;
			}
		}

		xgc_size serialization::seek( xgc_size pos, ePosition enPos, eMethod enMethod )
		{
			switch( enPos )
			{
				case eWithHead:
				XGC_ASSERT( pos <= (xgc_int32) _length && pos >= 0 );
				if( pos > (xgc_int32) _length )
					pos = _length;

				if( enMethod & eWrite )
					_wd = pos;

				if( enMethod & eRead )
					_rd = pos;

				return pos;
				break;

				case eWithTail:
				XGC_ASSERT( pos < 0 );
				if( enMethod & eWrite )
					_wd = _wsize + pos;

				if( enMethod & eRead )
					_rd = _rsize + pos;

				case eWithCurrent:
				if( enMethod & eWrite )
				{
					_wd += pos;
				}

				if( enMethod & eRead )
				{
					_rd += pos;
				}
				break;
				case eWithForward:
				if( enMethod & eWrite )
				{
					_wd -= pos;
				}

				if( enMethod & eRead )
				{
					_rd -= pos;
				}
			}
			return pos;
		}

		serialization::serialization( xgc_void ) : _ptr( xgc_nullptr )
			, _length( 0 )
			, _rsize( 0 )
			, _wsize( 0 )
			, _rd( 0 )
			, _wd( 0 )
		{
		}

		//serialization::serialization( const char *ptr, int length, eMethod mode /*= serialization::eRW*/, xgc_size off /*= 0 */ ) : _ptr( (xgc_byte*) ptr )
		//	, _length( length )
		//	, _rsize( 0 )
		//	, _wsize( 0 )
		//	, _rd( 0 )
		//	, _wd( 0 )
		//{
		//	_init_buf( mode, off );
		//}

		//serialization::serialization( const unsigned char *ptr, int length, eMethod mode /*= serialization::eRW*/, xgc_size off /*= 0 */ ) : _ptr( ptr )
		//	, _length( length )
		//	, _rsize( 0 )
		//	, _wsize( 0 )
		//	, _rd( 0 )
		//	, _wd( 0 )
		//{
		//	_init_buf( mode, off );
		//}

		serialization::serialization( xgc_lpvoid data, xgc_size length, eMethod mode /*= serialization::eRW*/, xgc_size off /*= 0 */ ) 
			: _ptr( (xgc_byte*) data )
			, _length( length )
			, _rsize( 0 )
			, _wsize( 0 )
			, _rd( 0 )
			, _wd( 0 )
		{
			_init_buf( mode, off );
		}

		//bool serialization::setbuf( const xgc_byte *ptr, int length, eMethod mode /*= serialization::eRW*/, xgc_size off /*= 0 */ )
		//{
		//	_ptr = ptr;
		//	_length = length;
		//	_init_buf( mode, off );
		//	return true;
		//}

		xgc_size serialization::putn( const xgc_byte *p, xgc_size n )
		{
			xgc_size cpy = __min( space(), n );
			if( cpy != n )
				throw( std::logic_error( "overflow" ) );
			memcpy( (void*) ( _ptr + _wd ), p, cpy );
			_wd += (xgc_uint32) cpy;
			return cpy;
		}

		xgc_size serialization::getn( xgc_byte *p, xgc_size n )
		{
			xgc_size cpy = __min( _rsize - rd(), n );
			if( cpy != n )
				throw( std::logic_error( "overflow" ) );
			memcpy( p, _ptr + rd(), n );
			_rd += (xgc_uint32) cpy;
			return cpy;
		}

	}
}