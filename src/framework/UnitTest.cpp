#include "ServerDefines.h"
#include "UnitTest.h"
UnitTestManager& getUnitTestMgr()
{
	static UnitTestManager inst;
	return inst;
}

xgc_bool UTestInit( ini_reader &ini, xgc_lpvoid params )
{
	UnitTest *pTest = reinterpret_cast<UnitTest*>( params );
	XGC_ASSERT_RETURN( pTest, false );
	return pTest->Init( ini );
}

xgc_bool UTestMain( int argc, char* argv[] )
{
	xgc_bool bRet = true;
	xgc_size nPage = 0;

	auto &pool = getUnitTestMgr();
	while( true )
	{
		puts( "---------------------------------" );
		puts( "-- UnitTest Framework ver 0.10 --" );
		puts( "---------------------------------" );
		for( auto i = 1; i < 10; ++i )
		{
			auto n = i - 1;
			if( nPage * 9 + n >= pool.size() )
				break;

			printf( "%3d. %s\n", i, std::get< 0 >( pool[nPage * 9 + n] ).c_str() );
		}
		puts( "--" );
		puts( "  n. next page" );
		puts( "  p. prev page" );
		puts( "--" );
		puts( "  q. quit" );
		puts( "---------------------------------" );
		puts( "choice : (press a key ...)" );

		xgc_char ch = _getch();
		if( ch == 'q' )
		{
			break;
		}
		else if( ch >= '1' && ch <= '9' )
		{
			auto s = ch - '1';
			if( nPage * 9 + s >= pool.size() )
				break;

			auto t = std::get< 1 >( pool[nPage * 9 + s] );
			t->Main( argc, argv );
		}
		else if( ch == 'n' || ch == '.' )
		{
			nPage = ( nPage < pool.size() / 9 ? nPage + 1 : pool.size() / 9 );
		}
		else if( ch == 'p' || ch == ',' )
		{
			nPage = ( nPage ? nPage - 1 : 0 );
		}
	}

	return bRet;
}
