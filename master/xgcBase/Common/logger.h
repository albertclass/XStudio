#ifndef FRM_LOGGER_H
#define FRM_LOGGER_H
#include "defines.h"
#include "exports.h"
#include "allocator.h"
#include <sstream>

enum COMMON_API EExeLogLvl
{
	LOGLVL_SYS_ERROR = 0,
	LOGLVL_SYS_WARNING,
	LOGLVL_SYS_INFO,
	LOGLVL_USR_ERROR,
	LOGLVL_USR_WARNING,
	LOGLVL_USR_INFO,
	LOGLVL_DBG_ERROR,
	LOGLVL_DBG_WARNING,
	LOGLVL_DBG_INFO,
	LOGLVL_SCRIPT_ERROR,
	LOGLVL_SCRIPT_WARNING,
	LOGLVL_SCRIPT_INFO,
	LOGLVL_MAX,
};

#ifndef MSG_MARK 
#define MSG_MARK "MARK", 0
#endif

#define LOGEXT( FUNCTION, LINE, LEVEL, FMT, ... )	XGC::Logger::WriteLog( FUNCTION, LINE, LEVEL, FMT, __VA_ARGS__ )
#define LOGFMT( LEVEL, FMT, ... )	XGC::Logger::WriteLog( LEVEL, FMT, __VA_ARGS__ )

#define SYS_ERROR( FMT, ... )	XGC::Logger::WriteLog( __FILE__, __LINE__, LOGLVL_SYS_ERROR,	FMT, __VA_ARGS__ )
#define SYS_WARNING( FMT, ... ) XGC::Logger::WriteLog( __FILE__, __LINE__, LOGLVL_SYS_WARNING,	FMT, __VA_ARGS__ )
#define SYS_INFO( FMT, ... )	XGC::Logger::WriteLog( __FILE__, __LINE__, LOGLVL_SYS_INFO,		FMT, __VA_ARGS__ )
#define USR_ERROR( FMT, ... )	XGC::Logger::WriteLog( __FILE__, __LINE__, LOGLVL_USR_ERROR,	FMT, __VA_ARGS__ )
#define USR_WARNING( FMT, ... ) XGC::Logger::WriteLog( __FILE__, __LINE__, LOGLVL_USR_WARNING,	FMT, __VA_ARGS__ )
#define USR_INFO( FMT, ... )	XGC::Logger::WriteLog( __FILE__, __LINE__, LOGLVL_USR_INFO,		FMT, __VA_ARGS__ )
#define DBG_ERROR( FMT, ... )	XGC::Logger::WriteLog( __FILE__, __LINE__, LOGLVL_DBG_ERROR,	FMT, __VA_ARGS__ )
#define DBG_WARNING( FMT, ... ) XGC::Logger::WriteLog( __FILE__, __LINE__, LOGLVL_DBG_WARNING,	FMT, __VA_ARGS__ )
#define DBG_INFO( FMT, ... )	XGC::Logger::WriteLog( __FILE__, __LINE__, LOGLVL_DBG_INFO,		FMT, __VA_ARGS__ )

#define SHM_LOG( FMT, ... ) XGC::Logger::WriteShm( __FILE__, __LINE__, FMT, __VA_ARGS__ )

namespace XGC
{
	namespace Logger
	{
		typedef void( *LoggerCallback )( xgc_lpvoid context, xgc_lpcstr text, xgc_size size );

		///
		/// [12/16/2013 albert.xu]
		/// 初始化日志模块
		///
		COMMON_API xgc_bool InitExeLogger( xgc_lpcstr pszConfig, LoggerCallback pfnCallback, xgc_lpvoid pContext );

		///
		/// [1/10/2014 albert.xu]
		/// 清理日志系统
		///
		COMMON_API xgc_void FiniExeLogger();

		///
		/// [12/26/2013 albert.xu]
		/// 初始化共享内存日志
		///
		COMMON_API xgc_bool InitShmLogger( xgc_lpcstr lpPath, xgc_lpcstr pszShmName );

		///
		/// [12/16/2013 albert.xu]
		/// 重载日志配置
		///
		COMMON_API xgc_bool Reload();

		///
		/// [12/16/2013 albert.xu]
		/// 写日志
		///
		COMMON_API xgc_void WriteLog( EExeLogLvl nLevel, xgc_lpcstr pszFmt, ... );

		///
		/// [12/16/2013 albert.xu]
		/// 写日志
		///
		COMMON_API xgc_void WriteLog( xgc_lpcstr pszFunction, xgc_int32 nLine, EExeLogLvl nLevel, xgc_lpcstr pszFmt, ... );
		///
		/// [3/4/2014 albert.xu]
		/// 共享内存日志
		///
		COMMON_API xgc_void WriteShm( xgc_lpcstr pszFunction, xgc_int32 nLine, xgc_lpcstr format, ... );

		///
		/// [3/4/2014 albert.xu]
		/// 文件日志
		///
		COMMON_API xgc_void FileLog( xgc_lpcstr file, xgc_lpcstr format, ... );
	}
}

#endif

