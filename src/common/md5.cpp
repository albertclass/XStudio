#include "md5.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#pragma warning( disable:4996 )
namespace xgc
{
	namespace encrypt
	{
		static xgc_string intToString( uint64_t i )
		{
			char str[33] = { 0 };
			char* buf = str + 31;
			do
			{
				*buf-- = (unsigned char) ( i % 10 ) + '0';
				i = i / 10;
			} while( i != 0 );
			return ( buf + 1 );
		}

		void md5_text( char md5[16], char text[33], bool caps )
		{
			char* buf = text;
			const char* hex = caps ? "0123456789ABCDEF" : "0123456789abcdef";

			for( int i = 0; i < 16; ++i )
			{
				*buf++ = hex[( (unsigned char)md5[i] & 0xf0 ) >> 4];
				*buf++ = hex[( (unsigned char)md5[i] & 0x0f )];
			}
			*buf = 0;
		}

		static inline void Md5_Put64( uint64_t data, char buf[8] )
		{
			buf[0] = (uint8_t) ( data );
			buf[1] = (uint8_t) ( data >> 8 );
			buf[2] = (uint8_t) ( data >> 16 );
			buf[3] = (uint8_t) ( data >> 24 );
			buf[4] = (uint8_t) ( data >> 32 );
			buf[5] = (uint8_t) ( data >> 40 );
			buf[6] = (uint8_t) ( data >> 48 );
			buf[7] = (uint8_t) ( data >> 56 );
		}

		static inline void Md5_Put32( uint32_t data, char buf[4] )
		{
			buf[0] = (uint8_t) ( data );
			buf[1] = (uint8_t) ( data >> 8 );
			buf[2] = (uint8_t) ( data >> 16 );
			buf[3] = (uint8_t) ( data >> 24 );
		}

		static inline uint32_t Md5_Get32( const char buf[4] )
		{
			return ( (uint32_t) (uint8_t) buf[0] ) | ( ( (uint32_t) (uint8_t) buf[1] ) << 8 ) | ( ( (uint32_t) (uint8_t) buf[2] ) << 16 ) | ( ( (uint32_t) (uint8_t) buf[3] ) << 24 );
		}

		static inline uint32_t Md5_F( uint32_t x, uint32_t y, uint32_t z )
		{
			return ( x & y ) | ( ( ~x ) & z );
		}

		static inline uint32_t Md5_G( uint32_t x, uint32_t y, uint32_t z )
		{
			return ( x & z ) | ( y & ( ~z ) );
		}

		static inline uint32_t Md5_H( uint32_t x, uint32_t y, uint32_t z )
		{
			return x ^ y ^ z;
		}

		static inline uint32_t Md5_I( uint32_t x, uint32_t y, uint32_t z )
		{
			return y ^ ( x | ( ~z ) );
		}

		static inline uint32_t Md5_S( uint32_t d, int n )		// ��?��?位移
		{
			return ( d << n ) | ( ( ( d & 0xFFFFFFFF ) >> ( 32 - n ) ) );
		}

		static inline void Md5_FF( uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t M, uint32_t s, uint32_t ti )
		{
			a = b + Md5_S( a + Md5_F( b, c, d ) + M + ti, s );
		}

		static inline void Md5_GG( uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t M, uint32_t s, uint32_t ti )
		{
			a = b + Md5_S( a + Md5_G( b, c, d ) + M + ti, s );
		}

		static inline void Md5_HH( uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t M, uint32_t s, uint32_t ti )
		{
			a = b + Md5_S( a + Md5_H( b, c, d ) + M + ti, s );
		}

		static inline void Md5_II( uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t M, uint32_t s, uint32_t ti )
		{
			a = b + Md5_S( a + Md5_I( b, c, d ) + M + ti, s );
		}

		static inline void Md5_Process( uint32_t &A, uint32_t &B, uint32_t &C, uint32_t &D, const char data[64] )
		{
			//
			uint32_t M[16];
			M[0] = Md5_Get32( data );
			M[1] = Md5_Get32( data + 4 );
			M[2] = Md5_Get32( data + 8 );
			M[3] = Md5_Get32( data + 12 );
			M[4] = Md5_Get32( data + 16 );
			M[5] = Md5_Get32( data + 20 );
			M[6] = Md5_Get32( data + 24 );
			M[7] = Md5_Get32( data + 28 );
			M[8] = Md5_Get32( data + 32 );
			M[9] = Md5_Get32( data + 36 );
			M[10] = Md5_Get32( data + 40 );
			M[11] = Md5_Get32( data + 44 );
			M[12] = Md5_Get32( data + 48 );
			M[13] = Md5_Get32( data + 52 );
			M[14] = Md5_Get32( data + 56 );
			M[15] = Md5_Get32( data + 60 );

			//
			uint32_t a = A;
			uint32_t b = B;
			uint32_t c = C;
			uint32_t d = D;

			// round 1
			Md5_FF( a, b, c, d, M[0], 7, 0xD76AA478 );
			Md5_FF( d, a, b, c, M[1], 12, 0xE8C7B756 );
			Md5_FF( c, d, a, b, M[2], 17, 0x242070DB );
			Md5_FF( b, c, d, a, M[3], 22, 0xC1BDCEEE );
			Md5_FF( a, b, c, d, M[4], 7, 0xF57C0FAF );
			Md5_FF( d, a, b, c, M[5], 12, 0x4787C62A );
			Md5_FF( c, d, a, b, M[6], 17, 0xA8304613 );
			Md5_FF( b, c, d, a, M[7], 22, 0xFD469501 );
			Md5_FF( a, b, c, d, M[8], 7, 0x698098D8 );
			Md5_FF( d, a, b, c, M[9], 12, 0x8B44F7AF );
			Md5_FF( c, d, a, b, M[10], 17, 0xFFFF5BB1 );
			Md5_FF( b, c, d, a, M[11], 22, 0x895CD7BE );
			Md5_FF( a, b, c, d, M[12], 7, 0x6B901122 );
			Md5_FF( d, a, b, c, M[13], 12, 0xFD987193 );
			Md5_FF( c, d, a, b, M[14], 17, 0xA679438E );
			Md5_FF( b, c, d, a, M[15], 22, 0x49B40821 );

			// round 2
			Md5_GG( a, b, c, d, M[1], 5, 0xF61E2562 );
			Md5_GG( d, a, b, c, M[6], 9, 0xC040B340 );
			Md5_GG( c, d, a, b, M[11], 14, 0x265E5A51 );
			Md5_GG( b, c, d, a, M[0], 20, 0xE9B6C7AA );
			Md5_GG( a, b, c, d, M[5], 5, 0xD62F105D );
			Md5_GG( d, a, b, c, M[10], 9, 0x02441453 );
			Md5_GG( c, d, a, b, M[15], 14, 0xD8A1E681 );
			Md5_GG( b, c, d, a, M[4], 20, 0xE7D3FBC8 );
			Md5_GG( a, b, c, d, M[9], 5, 0x21E1CDE6 );
			Md5_GG( d, a, b, c, M[14], 9, 0xC33707D6 );
			Md5_GG( c, d, a, b, M[3], 14, 0xF4D50D87 );
			Md5_GG( b, c, d, a, M[8], 20, 0x455A14ED );
			Md5_GG( a, b, c, d, M[13], 5, 0xA9E3E905 );
			Md5_GG( d, a, b, c, M[2], 9, 0xFCEFA3F8 );
			Md5_GG( c, d, a, b, M[7], 14, 0x676F02D9 );
			Md5_GG( b, c, d, a, M[12], 20, 0x8D2A4C8A );

			// round 3
			Md5_HH( a, b, c, d, M[5], 4, 0xFFFA3942 );
			Md5_HH( d, a, b, c, M[8], 11, 0x8771F681 );
			Md5_HH( c, d, a, b, M[11], 16, 0x6D9D6122 );
			Md5_HH( b, c, d, a, M[14], 23, 0xFDE5380C );
			Md5_HH( a, b, c, d, M[1], 4, 0xA4BEEA44 );
			Md5_HH( d, a, b, c, M[4], 11, 0x4BDECFA9 );
			Md5_HH( c, d, a, b, M[7], 16, 0xF6BB4B60 );
			Md5_HH( b, c, d, a, M[10], 23, 0xBEBFBC70 );
			Md5_HH( a, b, c, d, M[13], 4, 0x289B7EC6 );
			Md5_HH( d, a, b, c, M[0], 11, 0xEAA127FA );
			Md5_HH( c, d, a, b, M[3], 16, 0xD4EF3085 );
			Md5_HH( b, c, d, a, M[6], 23, 0x04881D05 );
			Md5_HH( a, b, c, d, M[9], 4, 0xD9D4D039 );
			Md5_HH( d, a, b, c, M[12], 11, 0xE6DB99E5 );
			Md5_HH( c, d, a, b, M[15], 16, 0x1FA27CF8 );
			Md5_HH( b, c, d, a, M[2], 23, 0xC4AC5665 );

			// round 4
			Md5_II( a, b, c, d, M[0], 6, 0xF4292244 );
			Md5_II( d, a, b, c, M[7], 10, 0x432AFF97 );
			Md5_II( c, d, a, b, M[14], 15, 0xAB9423A7 );
			Md5_II( b, c, d, a, M[5], 21, 0xFC93A039 );
			Md5_II( a, b, c, d, M[12], 6, 0x655B59C3 );
			Md5_II( d, a, b, c, M[3], 10, 0x8F0CCC92 );
			Md5_II( c, d, a, b, M[10], 15, 0xFFEFF47D );
			Md5_II( b, c, d, a, M[1], 21, 0x85845DD1 );
			Md5_II( a, b, c, d, M[8], 6, 0x6FA87E4F );
			Md5_II( d, a, b, c, M[15], 10, 0xFE2CE6E0 );
			Md5_II( c, d, a, b, M[6], 15, 0xA3014314 );
			Md5_II( b, c, d, a, M[13], 21, 0x4E0811A1 );
			Md5_II( a, b, c, d, M[4], 6, 0xF7537E82 );
			Md5_II( d, a, b, c, M[11], 10, 0xBD3AF235 );
			Md5_II( c, d, a, b, M[2], 15, 0x2AD7D2BB );
			Md5_II( b, c, d, a, M[9], 21, 0xEB86D391 );

			//
			A += a;
			B += b;
			C += c;
			D += d;
		}

		static unsigned char Md5_Padding[64] =
		{
			0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};

		void md5_data( const char *data, int data_len, char md5[16] )
		{
			if( data_len < 0 )
			{
				memset( md5, 0, 16 );
				return;
			}

			int padding;
			char length[8];

			int r = data_len % 64;
			padding = ( r < 56 ) ? ( 56 - r ) : ( 56 + 64 - r );

			uint64_t bit_len = data_len * 8;
			Md5_Put64( bit_len, length );

			//
			uint32_t A = 0x67452301;
			uint32_t B = 0xEFCDAB89;
			uint32_t C = 0x98BADCFE;
			uint32_t D = 0x10325476;

			//
			int data_index = 0;
			while( ( data_len - data_index ) >= 64 )
			{
				Md5_Process( A, B, C, D, data + data_index );
				data_index += 64;
			}

			//
			int remain = data_len - data_index;
			assert( remain < 64 );

			if( remain < 56 )	// remain 0 ~ 55 bytes, remain 1 message group
			{
				char msg[64];

				memcpy( msg, data + data_index, remain );
				if( padding > 0 )
				{
					memcpy( msg + remain, Md5_Padding, padding );
				}
				memcpy( msg + remain + padding, length, 8 );

				assert( ( remain + padding + 8 ) == 64 );

				Md5_Process( A, B, C, D, msg );
			}
			else				// remain 56 ~ 63 bytes, remain 2 message groups
			{
				char msg[64];

				//
				memcpy( msg, data + data_index, remain );
				memcpy( msg + remain, Md5_Padding, 64 - remain );

				Md5_Process( A, B, C, D, msg );

				//
				memcpy( msg, Md5_Padding + 64 - remain, padding - ( 64 - remain ) );
				memcpy( msg + padding - ( 64 - remain ), length, 8 );

				assert( ( padding - ( 64 - remain ) + 8 ) == 64 );

				Md5_Process( A, B, C, D, msg );
			}

			//
			Md5_Put32( A, md5 );
			Md5_Put32( B, md5 + 4 );
			Md5_Put32( C, md5 + 8 );
			Md5_Put32( D, md5 + 12 );
		}

		///
		/// \brief 获取文件md5摘要
		///
		/// \author albert.xu
		/// \date 2016/08/04 11:37
		///
		xgc_bool md5_file( const char *filename, char md5[16] )
		{
			int fd;
			int retry = 5;
			const unsigned long data_size = 8*1024*1024;

			#ifdef _WINDOWS
			if( (fd = _open(filename, O_RDONLY | O_BINARY, S_IREAD)) < 0 )
			#endif

			#ifdef _LINUX
			if( (fd = _open(filename, O_RDONLY, S_IREAD)) < 0 )
			#endif
				return false;

			struct stat s;
			if( fstat( fd, &s ) < 0 )
			{
				_close( fd );
				return false;
			}

			// record file size
			unsigned long file_size = s.st_size;
			if( file_size < 0 )
			{
				memset( md5, 0, 16 );
				return false;
			}

			// alloc data buffer
			char* data_buffer = (char*)malloc( data_size );
			// save read total bytes
			unsigned long read_total = 0;

			int r = file_size % 64;
			int padding = ( r < 56 ) ? ( 56 - r ) : ( 56 + 64 - r );

			char length[8];
			Md5_Put64( file_size * 8, length );

			//
			uint32_t A = 0x67452301;
			uint32_t B = 0xEFCDAB89;
			uint32_t C = 0x98BADCFE;
			uint32_t D = 0x10325476;

			int data_index = 0;
			int remain = 0;

			while( read_total < file_size && retry )
			{
				unsigned long read_bytes = 0;
				unsigned long read_size = XGC_MIN( data_size, file_size - read_total );
				while( read_bytes < read_size && retry )
				{
					int ret = _read( fd, data_buffer + read_bytes, data_size - read_bytes );
					if( ret == -1 )
					{
						--retry;
						continue;
					}

					read_bytes += ret;
				}

				if( retry )
				{
					data_index = 0;

					while( ( read_bytes - data_index ) >= 64 )
					{
						Md5_Process( A, B, C, D, data_buffer + data_index );
						data_index += 64;
					}

					remain = read_bytes - data_index;
					read_total += read_bytes;
				}
			}

			if( read_total == file_size )
			{
				//
				assert( remain < 64 );

				if( remain < 56 )	// remain 0 ~ 55 bytes, remain 1 message group
				{
					char msg[64];

					memcpy( msg, data_buffer + data_index, remain );
					if( padding > 0 )
					{
						memcpy( msg + remain, Md5_Padding, padding );
					}
					memcpy( msg + remain + padding, length, 8 );

					assert( (remain + padding + 8) == 64 );

					Md5_Process( A, B, C, D, msg );
				}
				else				// remain 56 ~ 63 bytes, remain 2 message groups
				{
					char msg[64];

					//
					memcpy( msg, data_buffer + data_index, remain );
					memcpy( msg + remain, Md5_Padding, 64 - remain );

					Md5_Process( A, B, C, D, msg );

					//
					memcpy( msg, Md5_Padding + 64 - remain, padding - (64 - remain) );
					memcpy( msg + padding - (64 - remain), length, 8 );

					assert( (padding - (64 - remain) + 8) == 64 );

					Md5_Process( A, B, C, D, msg );
				}

				//
				Md5_Put32( A, md5 );
				Md5_Put32( B, md5 + 4 );
				Md5_Put32( C, md5 + 8 );
				Md5_Put32( D, md5 + 12 );
			}

			free( data_buffer );
			_close( fd );
			return retry != 0;
		}


	} // namepsace encrypt
}
