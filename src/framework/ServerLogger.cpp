#include "ServerDefines.h"
#include "ServerBase.h"
#include "ServerLogger.h"

xgc_bool InitializeLogger( ini_reader &ini )
{
	FUNCTION_BEGIN;
	// ��־����
	xgc_lpcstr lpLogPath = ini.get_item_value( "Log", "Path", "Log" );
	xgc_lpcstr lpLogName = ini.get_item_value( "Log", "Name", GetServerName() );

	// ������־Ŀ¼
	xgc_char szLoggerPath[XGC_MAX_PATH];
	get_absolute_path( szLoggerPath, sizeof( szLoggerPath ), "%s\\", lpLogPath );

	if( _access( szLoggerPath, 0 ) == -1 && make_dirs( szLoggerPath ) == -1 )
	{
		SYS_ERROR( "LogĿ¼��Ч�򴴽�ʧ�� Path - %s", szLoggerPath );
		return false;
	}

	xgc_char szExceptionLog[XGC_MAX_PATH];
	get_absolute_path( szExceptionLog, sizeof( szExceptionLog ), "exception\\exception.log" );
	SetExceptionLog( szExceptionLog );

	get_absolute_path( szExceptionLog, sizeof( szExceptionLog ), "exception\\exception.more.log" );
	SetExceptionExt( szExceptionLog );

	get_absolute_path( szExceptionLog, sizeof( szExceptionLog ), "exception\\exception.dmp" );
	SetExceptionDmp( szExceptionLog );

	get_absolute_path( szExceptionLog, sizeof( szExceptionLog ), "debuger.log" );
	SetDebugerLog( szExceptionLog );

	if( init_logger( szLoggerPath ) == false )
	{
		SYS_ERROR( "��ʼ���ļ���־ģ��ʧ�ܡ���־·��%s", szLoggerPath );
		return false;
	}

	SYS_INFO( "��ʼ���ļ���־ģ��ɹ�����־·��%s", szLoggerPath );

	FUNCTION_END;
	return true;
}

xgc_void FinializeLogger()
{
	fini_logger();
}
