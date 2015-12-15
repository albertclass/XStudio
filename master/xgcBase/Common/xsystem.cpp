#include "xsystem.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <time.h>

#include <wininet.h>
#pragma comment(lib, "wininet.lib")

#include <psapi.h>
#pragma comment(lib, "psapi.lib")

namespace XGC
{
	static xgc_char gModuleName[_MAX_FNAME] = { 0 };
	static xgc_char gModulePath[_MAX_PATH] = { 0 };

	xgc_lpcstr GetModuleName( HMODULE hModule/* = xgc_nullptr*/ )
	{
		if( gModuleName[0] == 0 )
		{
			xgc_size cpyLen = GetModuleFileNameA( hModule, gModuleName, sizeof( gModuleName ) );
			if( cpyLen == 0 )
				return gModuleName;

			xgc_lpstr p = strrchr( gModuleName, '\\' );
			if( p )
				strcpy_s( gModuleName, p + 1 );
		}
		return gModuleName;
	}

	xgc_lpcstr GetModulePath( HMODULE hModule/* = xgc_nullptr*/ )
	{
		if( gModulePath[0] == 0 )
		{
			DWORD dwRet = GetModuleFileNameA( xgc_nullptr, gModulePath, sizeof( gModulePath ) );
			XGC_ASSERT_RETURN( dwRet, "" );
			xgc_lpstr last = xgc_nullptr;
			xgc_lpstr found = strchr( gModulePath, '\\' );
			while( found )
			{
				last = found++;
				found = strchr( found, '\\' );
			}

			if( last ) last[1] = 0;
		}
		return gModulePath;
	}

	xgc_lpcstr GetNormalPath( xgc_lpstr szOut, xgc_size nSize, xgc_lpcstr lpRelativePath, ... )
	{
		int cpy1 = 0;
		xgc_lpcstr driver = strchr( lpRelativePath, ':' );
		if( xgc_nullptr == driver )
		{
			cpy1 = sprintf_s( szOut, nSize, "%s", GetModulePath() );
			XGC_ASSERT_RETURN( cpy1 >= 0, xgc_nullptr );
		}
		else if( driver - lpRelativePath != 1 )
		{
			return xgc_nullptr;
		}
		else if( driver[1] != '\\' && driver[1] != '/' )
		{
			return xgc_nullptr;
		}

		va_list args;
		va_start( args, lpRelativePath );
		int cpy2 = vsprintf_s( szOut + cpy1, nSize - cpy1, lpRelativePath, args );
		va_end( args );

		if( cpy2 < 0 )
			return xgc_nullptr;

		if( cpy2 == sizeof(szOut) -cpy1 )
			return xgc_nullptr;

		return _fullpath( szOut, szOut, nSize );
	}

	xgc_void GetMemoryStatus( xgc_uint64 *pnPMem, xgc_uint64 *pnVMem )
	{
		XGC_ASSERT_RETURN( pnPMem != pnVMem, xgc_void( 0 ) );
		if( pnPMem == xgc_nullptr && pnVMem == xgc_nullptr )
			return;

		PROCESS_MEMORY_COUNTERS MEM;
		GetProcessMemoryInfo( GetCurrentProcess(), &MEM, sizeof( MEM ) );
		if( pnPMem )
			*pnPMem = MEM.WorkingSetSize;

		if( pnVMem )
			*pnVMem = MEM.PagefileUsage;
	}

	std::mt19937& GetRandomDriver()
	{
		static std::default_random_engine mt( (xgc_int32) time( xgc_nullptr ) );

		return mt;
	}

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
		ULONG nMaxConnect = nMaxConnection;
		BOOL bRet = ::InternetSetOption( xgc_nullptr, INTERNET_OPTION_MAX_CONNS_PER_SERVER, &nMaxConnect, sizeof( nMaxConnect ) );
		bRet &= ::InternetSetOption( xgc_nullptr, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, &nMaxConnect, sizeof( nMaxConnect ) );
		if( !bRet )
		{
			USR_ERROR( "设置Wininet最大连接数失败[%u]", nMaxConnection );
		}
		return bRet != 0;
	}

	xgc_bool SyncHttpRequest( xgc_lpcstr url, xgc_string &result )
	{
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

		return true;
	}

}