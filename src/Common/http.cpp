#include "defines.h"
#include "exports.h"
#include "logger.h"

#if defined(_WINDOWS)
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#endif

#include "http.h"

namespace xgc
{
	namespace http
	{
		static char dec2hexChar( short int n )
		{
			if( 0 <= n && n <= 9 )
			{
				return char( short( '0' ) + n );
			}
			else if( 10 <= n && n <= 15 )
			{
				return char( short( 'A' ) + n - 10 );
			}
			else
			{
				return char( 0 );
			}
		}

		static short int hexChar2dec( char c )
		{
			if( '0' <= c && c <= '9' )
			{
				return short( c - '0' );
			}
			else if( 'a' <= c && c <= 'f' )
			{
				return ( short( c - 'a' ) + 10 );
			}
			else if( 'A' <= c && c <= 'F' )
			{
				return ( short( c - 'A' ) + 10 );
			}
			else
			{
				return -1;
			}
		}

		xgc_string EnCodeEscapeURL( xgc_lpcstr URL )
		{
			xgc_string result = "";
			while( *URL )
			{
				char c = *URL;
				if( ( '0' <= c && c <= '9' ) ||
					( 'a' <= c && c <= 'z' ) ||
					( 'A' <= c && c <= 'Z' ) ||
					c == '/' || c == '.' )
				{
					result += c;
				}
				else
				{
					int j = (short int) c;
					if( j < 0 )
					{
						j += 256;
					}
					int i1, i0;
					i1 = j / 16;
					i0 = j - i1 * 16;
					result += '%';
					result += dec2hexChar( i1 );
					result += dec2hexChar( i0 );
				}

				++URL;
			}
			return result;
		}

		xgc_string DeCodeEscapeURL( xgc_lpcstr URL )
		{
			xgc_string result = "";
			while( *URL )
			{
				char c = *URL;
				if( c != '%' )
				{
					result += c;
				}
				else
				{
					char c1 = *( ++URL ); if( c1 == 0 ) break;
					char c0 = *( ++URL ); if( c0 == 0 ) break;
					int num = 0;
					num += hexChar2dec( c1 ) * 16 + hexChar2dec( c0 );
					result += char( num );
				}
			}
			return result;
		}

		xgc_bool SetWininetMaxConnection( xgc_uint32 nMaxConnection )
		{
			#if defined(_WINDOWS)
			ULONG nMaxConnect = nMaxConnection;
			BOOL bRet = ::InternetSetOption( xgc_nullptr, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &nMaxConnect, sizeof( nMaxConnect ) );
			bRet &= ::InternetSetOption( xgc_nullptr, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, &nMaxConnect, sizeof( nMaxConnect ) );
			if( !bRet )
			{
				USR_ERROR( "设置Wininet最大连接数失败[%u]", nMaxConnection );
				return false;
			}

			return true;
			#endif

			return true;
		}

		xgc_bool SyncHttpRequest( xgc_lpcstr url, xgc_string &result )
		{
			#if defined(_WINDOWS)
			HINTERNET hInternetSession = InternetOpen( "Microsoft Internet Explorer", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );

			if( hInternetSession == NULL )
			{
				USR_WARNING( "InternetOpen error. url : %s", url );
				return false;
			}

			xgc_uint32 dwTimeout = 3 * 1000;
			if( !InternetSetOption( hInternetSession, INTERNET_OPTION_RECEIVE_TIMEOUT, (LPVOID) &dwTimeout, sizeof( dwTimeout ) ) )
			{
				USR_WARNING( "InternetSetOption return false. url : %s", url );
				InternetCloseHandle( hInternetSession );
				return false;
			}

			HINTERNET hURL = InternetOpenUrlA( hInternetSession, url, NULL, 0, INTERNET_FLAG_RELOAD, 0 );

			if( hURL == NULL )
			{
				USR_WARNING( "InternetOpenUrl error. code = %d. url : %s ", GetLastError(), url );
				InternetCloseHandle( hInternetSession );
				return false;
			}

			char buffer[50000] = { 0 };
			DWORD readsize = 1;
			while( readsize != 0 )
			{
				BOOL bRet = InternetReadFile( hURL, buffer, sizeof( buffer ), &readsize );
				if( bRet == FALSE )
					break;

				result.append( buffer, readsize );
			}

			DBG_INFO( "InternetReadFile total read %u ", result.size() );
			InternetCloseHandle( hURL );
			InternetCloseHandle( hInternetSession );
			#endif // _WINDOWS

			return true;
		}
	}
}