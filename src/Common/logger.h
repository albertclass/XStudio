///
/// CopyRight ? 2016 X Studio
/// \file logger.h
/// \date 八月 2016
///
/// \author albert.xu windxu@126.com
/// \brief 日志支持
/// 

#ifndef _LOGGER_H_
#define _LOGGER_H_
#include "defines.h"
#include "exports.h"

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

#define LOGEXT( FUNCTION, LINE, LEVEL, FMT, ... )	xgc::logger::write( FUNCTION, LINE, LEVEL, FMT, __VA_ARGS__ )
#define LOGFMT( LEVEL, FMT, ... )					xgc::logger::write( LEVEL, FMT, __VA_ARGS__ )

#define SYS_ERROR( FMT, ... )	xgc::logger::write( __FILE__, __LINE__, LOGLVL_SYS_ERROR,	FMT, __VA_ARGS__ )
#define SYS_WARNING( FMT, ... ) xgc::logger::write( __FILE__, __LINE__, LOGLVL_SYS_WARNING,	FMT, __VA_ARGS__ )
#define SYS_INFO( FMT, ... )	xgc::logger::write( __FILE__, __LINE__, LOGLVL_SYS_INFO,	FMT, __VA_ARGS__ )
#define USR_ERROR( FMT, ... )	xgc::logger::write( __FILE__, __LINE__, LOGLVL_USR_ERROR,	FMT, __VA_ARGS__ )
#define USR_WARNING( FMT, ... ) xgc::logger::write( __FILE__, __LINE__, LOGLVL_USR_WARNING,	FMT, __VA_ARGS__ )
#define USR_INFO( FMT, ... )	xgc::logger::write( __FILE__, __LINE__, LOGLVL_USR_INFO,	FMT, __VA_ARGS__ )
#define DBG_ERROR( FMT, ... )	xgc::logger::write( __FILE__, __LINE__, LOGLVL_DBG_ERROR,	FMT, __VA_ARGS__ )
#define DBG_WARNING( FMT, ... ) xgc::logger::write( __FILE__, __LINE__, LOGLVL_DBG_WARNING,	FMT, __VA_ARGS__ )
#define DBG_INFO( FMT, ... )	xgc::logger::write( __FILE__, __LINE__, LOGLVL_DBG_INFO,	FMT, __VA_ARGS__ )
#define SHM_LOG( FMT, ... )		xgc::logger::write_shared( __FILE__, __LINE__, FMT, __VA_ARGS__ )

namespace xgc
{
	namespace logger
	{
		typedef void( *LoggerCallback )( xgc_lpvoid context, xgc_lpcstr text, xgc_size size );

		///
		/// [12/16/2013 albert.xu]
		/// 初始化日志模块
		///
		COMMON_API xgc_bool init_logger( xgc_lpcstr path, LoggerCallback callback, xgc_lpvoid context );

		///
		/// [1/10/2014 albert.xu]
		/// 清理日志系统
		///
		COMMON_API xgc_void fini_logger();

		///
		/// [12/26/2013 albert.xu]
		/// 初始化共享内存日志
		///
		COMMON_API xgc_bool init_logger_shared( xgc_lpcstr path, xgc_lpcstr name );

		///
		/// [12/16/2013 albert.xu]
		/// 写日志
		///
		COMMON_API xgc_void write( EExeLogLvl level, xgc_lpcstr format, ... );

		///
		/// [12/16/2013 albert.xu]
		/// 写日志
		///
		COMMON_API xgc_void write( xgc_lpcstr function, xgc_int32 line, EExeLogLvl level, xgc_lpcstr format, ... );
		///
		/// [3/4/2014 albert.xu]
		/// 共享内存日志
		///
		COMMON_API xgc_void write_shared( xgc_lpcstr function, xgc_int32 line, xgc_lpcstr format, ... );

		///
		/// [3/4/2014 albert.xu]
		/// 文件日志
		///
		COMMON_API xgc_void write_file( xgc_lpcstr file, xgc_lpcstr format, ... );
	}
}

#endif

