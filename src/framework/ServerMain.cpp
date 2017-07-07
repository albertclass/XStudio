#include "ServerDefines.h"
#include "ServerService.h"
#include "UnitTest.h"

#if defined(_WINDOWS)
///
/// InvalidParameterHandler
/// 异常参数调用时的处理函数，由此函数则不调用dr.waston！
/// \author xufeng04
/// \date [6/6/2014]
///
void InvalidParameterHandler( const wchar_t* expr
							  , const wchar_t* func
							  , const wchar_t* file
							  , unsigned int   line
							  , uintptr_t      resv )
{
	if( func && file && expr )
	{
		SYS_ERROR(
			"Invalid parameter detected in: Function:%S File:%S Line:%d Expression:%S"
			, func
			, file
			, line
			, expr );
	}

	DumpStackFrame();
	XGC_DEBUG_MESSAGE( "非法的参数调用，请检查调用项并排除BUG。" );
	// _invoke_watson( expr, func, file, line, resv );
	throw std::runtime_error( "invalid parameter detected" );
}

///
/// 虚调用处理函数
/// \author xufeng04
/// \date [6/6/2014]
///
void __cdecl PureCallHandler()
{
	DumpStackFrame();
	XGC_DEBUG_MESSAGE( "PURECALL" );
	SYS_ERROR( "PURECALL" );
	throw std::exception( "pure call" );
}
#endif

///
/// 主函数入口
/// [11/28/2014] create by albert.xu
///
int main( int argc, char *argv[] )
{
	#ifdef _WINDOWS

	// open memleak report
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	// invalid parameter callback set.
	_set_invalid_parameter_handler( InvalidParameterHandler );
	// purecall set
	_set_purecall_handler( PureCallHandler );
	#endif

	// set default locale
	setlocale( LC_ALL, "chs" );

	if( argc > 1 )
	{
		if( strcasecmp( argv[1], "-install" ) == 0 )
		{
			xgc_lpcstr lpServiceName = xgc_nullptr;
			xgc_lpcstr lpServiceDisp = xgc_nullptr;
			xgc_lpcstr lpServiceDesc = xgc_nullptr;

			xgc_lpcstr lpConfigFile = "Config.ini";
			if( argc > 2 )
				lpConfigFile = argv[2];

			char szConfPath[XGC_MAX_PATH];
			get_absolute_path( szConfPath, sizeof( szConfPath ), "%s", lpConfigFile );
			ini_reader ini;
			if( false == ini.load( szConfPath ) )
			{
				printf( "read config file %s failed. please check and try again.", szConfPath );
				return -1;
			}

			lpServiceName = ini.get_item_value( "ServiceCfg", "ServiceName", xgc_nullptr );
			if( xgc_nullptr == lpServiceName )
			{
				printf( "section 'ServiceCfg' key 'ServiceName' is empty. it must configuration" );
				return -1;
			}


			lpServiceDisp = ini.get_item_value( "ServiceCfg", "DisplayName", lpServiceName );
			lpServiceDesc = ini.get_item_value( "ServiceCfg", "Description", "这个人很懒注册服务的时候都不写服务描述。" );

			return InstallService( lpConfigFile, lpServiceName, lpServiceDisp, lpServiceDesc );
		}
		else if( strcasecmp( argv[1], "-uninstall" ) == 0 )
		{
			xgc_lpcstr lpServiceName = xgc_nullptr;

			if( argc > 2 )
				lpServiceName = argv[2];
			else
				lpServiceName = get_module_name();

			return UnInstallService( lpServiceName );
		}
		else if( strcasecmp( argv[1], "-service" ) == 0 )
		{
			ServiceRun( argc - 1, argv + 1 );
			return 0;
		}
		else if( strcasecmp( argv[1], "-debug" ) == 0 )
		{
			char path[XGC_MAX_PATH] = { 0 };
			get_absolute_path( path, sizeof( path ), "config*.ini" );

			char choice = 0;
			int n = 0;

			std::vector< std::string > choice_list;

			do
			{
				puts( "debug mode" );
				puts( "choice your config file:" );

				n = 0;

				list_directory( path, [&choice_list, &n]( xgc_lpcstr root, xgc_lpcstr rel, xgc_lpcstr fname )->bool{
					if( fname )
					{
						choice_list.push_back( fname );
						printf( "%d. %s\n", n, fname );

						++n;
					}

					return true;
				});

				printf( "q. exit\n" );
				printf( "press menu number to continue ..." );

				if( ( choice = _getch() ) == 'q' )
					return 1;

			} while( false == isdigit( choice ) || ( choice - '0' >= n ) );

			char config_file[256] = {};
			strcpy_s( config_file, choice_list[choice - '0'].c_str() );
			argv[1] = config_file;
			return ServerMain( argc, argv );
		}
		else if( strcasecmp( argv[1], "-test" ) == 0 )
		{
			return UTestMain( argc - 1, argv + 1 );
		}
	}

	return ServerMain( argc, argv );
}
