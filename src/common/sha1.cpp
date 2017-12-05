/*
	100% free public domain implementation of the SHA-1 algorithm
	by Dominik Reichl <dominik.reichl@t-online.de>
	Web: http://www.dominik-reichl.de/

	Version 1.6 - 2005-02-07 (thanks to Howard Kapustein for patches)
	- You can set the endianness in your files, no need to modify the
	  header file of the CSHA1 class any more
	- Aligned data support
	- Made support/compilation of the utility functions (ReportHash
	  and HashFile) optional (useful, if bytes count, for example in
	  embedded environments)

	Version 1.5 - 2005-01-01
	- 64-bit compiler compatibility added
	- Made variable wiping optional (define SHA1_WIPE_VARIABLES)
	- Removed unnecessary variable initializations
	- ROL32 improvement for the Microsoft compiler (using _rotl)

	======== Test Vectors (from FIPS PUB 180-1) ========

	sha1("abc") =
		A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D

	sha1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") =
		84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1

	sha1(A million repetitions of "a") =
		34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/
#pragma warning( disable:4996 )
#include "defines.h"
#include "exports.h"

#define STRING_TYPE_LEN (-1)

#include "sha1.h"
#include "allocator.h"

#include <memory.h> // Needed for memset and memcpy

#ifdef _MSC_VER
#include <stdlib.h>
#endif

// You can define the endian mode in your files, without modifying the sha1
// source files. Just #define SHA1_LITTLE_ENDIAN or #define SHA1_BIG_ENDIAN
// in your files, before including the sha1.h header file. If you don't
// define anything, the class defaults to little endian.

# if !defined(SHA1_LITTLE_ENDIAN) && !defined(SHA1_BIG_ENDIAN)
#	define SHA1_LITTLE_ENDIAN
# endif

// Same here. If you want variable wiping, #define SHA1_WIPE_VARIABLES, if
// not, #define SHA1_NO_WIPE_VARIABLES. If you don't define anything, it
// defaults to wiping.

# if !defined(SHA1_WIPE_VARIABLES) && !defined(SHA1_NO_WIPE_VARIABLES)
#	define SHA1_WIPE_VARIABLES
# endif

namespace xgc
{
	namespace encrypt
	{
		/////////////////////////////////////////////////////////////////////////////
		// Declare sha1 workspace
		enum
		{
			SHA1_DIGEST_LENGTH = 20,
			SHA1_BLOCK_SIZE = 64,
		};

		typedef union
		{
			xgc_uint8  c[64];
			xgc_uint32 l[16];
		} SHA1_WORKSPACE_BLOCK;

		class CSHA1
		{
		public:
			// Constructor and Destructor
			CSHA1();
			~CSHA1();

			xgc_uint32 m_state[5];
			xgc_uint32 m_count[2];
			xgc_uint32 __reserved1[1];
			xgc_uint8  m_buffer[SHA1_BLOCK_SIZE];
			xgc_uint8  m_digest[SHA1_DIGEST_LENGTH];
			xgc_uint32 __reserved2[3];

			void Reset();

			// Update the hash value
			void Update( xgc_uint8 *data, xgc_uint32 len );

			// Finalize hash and report
			void Final();

			void GetHash( xgc_uint8 *puDest, bool bHex = false );

		private:
			// Private SHA-1 transformation
			void Transform( xgc_uint32 *state, xgc_uint8 *buffer );

			// Member variables
			xgc_uint8 m_workspace[64];
			SHA1_WORKSPACE_BLOCK *m_block; // sha1 pointer to the byte array above
		};

#ifdef SHA1_UTILITY_FUNCTIONS
#define SHA1_MAX_FILE_BUFFER 8000
#endif

		// Rotate x bits to the left
#ifndef ROL32
#ifdef _MSC_VER
#define ROL32(_val32, _nBits) _rotl(_val32, _nBits)
#else
#define ROL32(_val32, _nBits) (((_val32)<<(_nBits))|((_val32)>>(32-(_nBits))))
#endif
#endif

#ifdef SHA1_LITTLE_ENDIAN
#define SHABLK0(i) (m_block->l[i] = \
	( ROL32( m_block->l[i], 24 ) & 0xFF00FF00 ) | ( ROL32( m_block->l[i], 8 ) & 0x00FF00FF ) )
#else
#define SHABLK0(i) (m_block->l[i])
#endif

#define SHABLK(i) (m_block->l[i&15] = ROL32(m_block->l[(i+13)&15] ^ m_block->l[(i+8)&15] \
	^ m_block->l[( i + 2 ) & 15] ^ m_block->l[i & 15], 1 ) )

		// SHA-1 rounds
#define _R0(v,w,x,y,z,i) { z+=((w&(x^y))^y)+SHABLK0(i)+0x5A827999+ROL32(v,5); w=ROL32(w,30); }
#define _R1(v,w,x,y,z,i) { z+=((w&(x^y))^y)+SHABLK(i)+0x5A827999+ROL32(v,5); w=ROL32(w,30); }
#define _R2(v,w,x,y,z,i) { z+=(w^x^y)+SHABLK(i)+0x6ED9EBA1+ROL32(v,5); w=ROL32(w,30); }
#define _R3(v,w,x,y,z,i) { z+=(((w|x)&y)|(w&x))+SHABLK(i)+0x8F1BBCDC+ROL32(v,5); w=ROL32(w,30); }
#define _R4(v,w,x,y,z,i) { z+=(w^x^y)+SHABLK(i)+0xCA62C1D6+ROL32(v,5); w=ROL32(w,30); }

		CSHA1::CSHA1()
		{
			m_block = (SHA1_WORKSPACE_BLOCK *)m_workspace;

			Reset();
		}

		CSHA1::~CSHA1()
		{
			Reset();
		}

		void CSHA1::Reset()
		{
			// sha1 initialization constants
			m_state[0] = 0x67452301;
			m_state[1] = 0xEFCDAB89;
			m_state[2] = 0x98BADCFE;
			m_state[3] = 0x10325476;
			m_state[4] = 0xC3D2E1F0;

			m_count[0] = 0;
			m_count[1] = 0;
		}

		void CSHA1::Transform( xgc_uint32 *state, xgc_uint8 *buffer )
		{
			// Copy state[] to working vars
			xgc_uint32 a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];

			memcpy( m_block, buffer, 64 );

			// 4 rounds of 20 operations each. Loop unrolled.
			_R0( a, b, c, d, e, 0 ); _R0( e, a, b, c, d, 1 ); _R0( d, e, a, b, c, 2 ); _R0( c, d, e, a, b, 3 );
			_R0( b, c, d, e, a, 4 ); _R0( a, b, c, d, e, 5 ); _R0( e, a, b, c, d, 6 ); _R0( d, e, a, b, c, 7 );
			_R0( c, d, e, a, b, 8 ); _R0( b, c, d, e, a, 9 ); _R0( a, b, c, d, e, 10 ); _R0( e, a, b, c, d, 11 );
			_R0( d, e, a, b, c, 12 ); _R0( c, d, e, a, b, 13 ); _R0( b, c, d, e, a, 14 ); _R0( a, b, c, d, e, 15 );
			_R1( e, a, b, c, d, 16 ); _R1( d, e, a, b, c, 17 ); _R1( c, d, e, a, b, 18 ); _R1( b, c, d, e, a, 19 );
			_R2( a, b, c, d, e, 20 ); _R2( e, a, b, c, d, 21 ); _R2( d, e, a, b, c, 22 ); _R2( c, d, e, a, b, 23 );
			_R2( b, c, d, e, a, 24 ); _R2( a, b, c, d, e, 25 ); _R2( e, a, b, c, d, 26 ); _R2( d, e, a, b, c, 27 );
			_R2( c, d, e, a, b, 28 ); _R2( b, c, d, e, a, 29 ); _R2( a, b, c, d, e, 30 ); _R2( e, a, b, c, d, 31 );
			_R2( d, e, a, b, c, 32 ); _R2( c, d, e, a, b, 33 ); _R2( b, c, d, e, a, 34 ); _R2( a, b, c, d, e, 35 );
			_R2( e, a, b, c, d, 36 ); _R2( d, e, a, b, c, 37 ); _R2( c, d, e, a, b, 38 ); _R2( b, c, d, e, a, 39 );
			_R3( a, b, c, d, e, 40 ); _R3( e, a, b, c, d, 41 ); _R3( d, e, a, b, c, 42 ); _R3( c, d, e, a, b, 43 );
			_R3( b, c, d, e, a, 44 ); _R3( a, b, c, d, e, 45 ); _R3( e, a, b, c, d, 46 ); _R3( d, e, a, b, c, 47 );
			_R3( c, d, e, a, b, 48 ); _R3( b, c, d, e, a, 49 ); _R3( a, b, c, d, e, 50 ); _R3( e, a, b, c, d, 51 );
			_R3( d, e, a, b, c, 52 ); _R3( c, d, e, a, b, 53 ); _R3( b, c, d, e, a, 54 ); _R3( a, b, c, d, e, 55 );
			_R3( e, a, b, c, d, 56 ); _R3( d, e, a, b, c, 57 ); _R3( c, d, e, a, b, 58 ); _R3( b, c, d, e, a, 59 );
			_R4( a, b, c, d, e, 60 ); _R4( e, a, b, c, d, 61 ); _R4( d, e, a, b, c, 62 ); _R4( c, d, e, a, b, 63 );
			_R4( b, c, d, e, a, 64 ); _R4( a, b, c, d, e, 65 ); _R4( e, a, b, c, d, 66 ); _R4( d, e, a, b, c, 67 );
			_R4( c, d, e, a, b, 68 ); _R4( b, c, d, e, a, 69 ); _R4( a, b, c, d, e, 70 ); _R4( e, a, b, c, d, 71 );
			_R4( d, e, a, b, c, 72 ); _R4( c, d, e, a, b, 73 ); _R4( b, c, d, e, a, 74 ); _R4( a, b, c, d, e, 75 );
			_R4( e, a, b, c, d, 76 ); _R4( d, e, a, b, c, 77 ); _R4( c, d, e, a, b, 78 ); _R4( b, c, d, e, a, 79 );

			// Add the working vars back into state
			state[0] += a;
			state[1] += b;
			state[2] += c;
			state[3] += d;
			state[4] += e;

			// Wipe variables
#ifdef SHA1_WIPE_VARIABLES
			a = b = c = d = e = 0;
#endif
		}

		// Use this function to hash in binary data and strings
		void CSHA1::Update( xgc_uint8 *data, xgc_uint32 len )
		{
			xgc_uint32 i, j;

			j = ( m_count[0] >> 3 ) & 63;

			if( ( m_count[0] += len << 3 ) < ( len << 3 ) ) m_count[1]++;

			m_count[1] += ( len >> 29 );

			if( ( j + len ) > 63 )
			{
				i = 64 - j;
				memcpy( &m_buffer[j], data, i );
				Transform( m_state, m_buffer );

				for( ; i + 63 < len; i += 64 ) Transform( m_state, &data[i] );

				j = 0;
			}
			else i = 0;

			memcpy( &m_buffer[j], &data[i], len - i );
		}

		void CSHA1::Final()
		{
			xgc_uint32 i;
			xgc_uint8 finalcount[8];

			for( i = 0; i < 8; i++ )
				finalcount[i] = (xgc_uint8)( ( m_count[( ( i >= 4 ) ? 0 : 1 )]
				>> ( ( 3 - ( i & 3 ) ) * 8 ) ) & 255 ); // Endian independent

			Update( ( xgc_uint8 * )"\200", 1 );

			while( ( m_count[0] & 504 ) != 448 )
				Update( ( xgc_uint8 * )"\0", 1 );

			Update( finalcount, 8 ); // Cause a SHA1Transform()

			for( i = 0; i < 20; i++ )
			{
				m_digest[i] = (xgc_uint8)( ( m_state[i >> 2] >> ( ( 3 - ( i & 3 ) ) * 8 ) ) & 255 );
			}

			// Wipe variables for security reasons
#ifdef SHA1_WIPE_VARIABLES
			i = 0;
			memset( m_buffer, 0, sizeof( m_buffer ) );
			memset( m_state, 0, sizeof( m_state ) );
			memset( m_count, 0, sizeof( m_count ) );
			memset( finalcount, 0, sizeof(finalcount) );
			Transform( m_state, m_buffer );
#endif
		}

		// Get the raw message digest
		void CSHA1::GetHash( xgc_uint8 *puDest, bool bHex /*= false*/ )
		{
			const xgc_char hex[] = "0123456789abcdef";

			if( bHex )
			{
				for( int i = 0; i < XGC_COUNTOF( m_digest ); ++i )
				{
					*puDest++ = hex[( m_digest[i] & 0xf0 ) >> 4];
					*puDest++ = hex[( m_digest[i] & 0x0f )];
				}
			}
			else
			{
				memcpy( puDest, m_digest, SHA1_DIGEST_LENGTH );
			}
		}


		void sha1( const char * _data, int _dataLen, const char * _key, int _keyLen, char * digest, bool hex /* = true */ )
		{
			CSHA1 csha1;

			char ipad[SHA1_BLOCK_SIZE] = { 0 };
			char opad[SHA1_BLOCK_SIZE] = { 0 };
			char key[SHA1_BLOCK_SIZE] = { 0 };

			if( _keyLen > SHA1_BLOCK_SIZE )
			{
				csha1.Reset();
				csha1.Update( (xgc_uint8*)_key, _keyLen );
				csha1.Final();

				csha1.GetHash( (xgc_uint8 *)key );
			}
			else
			{
				memcpy( key, _key, _keyLen ); // leave 0 at the end of the key buffer
			}

			/* repeated SHA1_BLOCK_SIZE times for values in ipad and opad */
			memset( ipad, 0x36, SHA1_BLOCK_SIZE );
			memset( opad, 0x5c, SHA1_BLOCK_SIZE );

			for( int i = 0; i != SHA1_BLOCK_SIZE; ++i )
			{
				ipad[i] ^= key[i];
			}

			for( int j = 0; j != SHA1_BLOCK_SIZE; ++j )
			{
				opad[j] ^= key[j];
			}

			xgc_string buffer;
			buffer.append( (const char*)ipad, SHA1_BLOCK_SIZE );
			buffer.append( (const char*)_data, _dataLen );

			csha1.Reset();
			csha1.Update( (xgc_uint8 *)buffer.data(), SHA1_BLOCK_SIZE + _dataLen );
			csha1.Final();

			csha1.GetHash( (xgc_uint8 *)digest );

			buffer.assign( "" );
			buffer.append( (const char*)opad, SHA1_BLOCK_SIZE );
			buffer.append( (const char*)digest, SHA1_DIGEST_LENGTH );
			csha1.Reset();
			csha1.Update( (xgc_uint8 *)buffer.data(), SHA1_BLOCK_SIZE + SHA1_DIGEST_LENGTH );
			csha1.Final();

			csha1.GetHash( (xgc_uint8 *)digest, hex );

			return;
		}
	}
}