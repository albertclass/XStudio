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
#include "datetime.h"

#include <functional>

#define LOGEND "\n"

#define LOGEXT( FILE, FUNC, LINE, TAGS, FMT, ... ) xgc::common::get_logger( "sys.info" ).write( FILE, FUNC, LINE, TAGS, FMT, ##__VA_ARGS__ )
#define LOGFMT( TAGS, FMT, ... ) xgc::common::get_logger( "sys.info" ).write( __FILE__, __func__, __LINE__, TAGS, FMT, ##__VA_ARGS__ )

#define SYS_ERROR( FMT, ... )	xgc::common::get_logger( "sys"   ).write( __FILE__, __func__, __LINE__, "error",	FMT, ##__VA_ARGS__ )
#define SYS_WARNING( FMT, ... ) xgc::common::get_logger( "sys"   ).write( __FILE__, __func__, __LINE__, "warning",	FMT, ##__VA_ARGS__ )
#define SYS_INFO( FMT, ... )	xgc::common::get_logger( "sys"   ).write( __FILE__, __func__, __LINE__, "info",		FMT, ##__VA_ARGS__ )
#define USR_ERROR( FMT, ... )	xgc::common::get_logger( "user"  ).write( __FILE__, __func__, __LINE__, "error",	FMT, ##__VA_ARGS__ )
#define USR_WARNING( FMT, ... ) xgc::common::get_logger( "user"  ).write( __FILE__, __func__, __LINE__, "warning",	FMT, ##__VA_ARGS__ )
#define USR_INFO( FMT, ... )	xgc::common::get_logger( "user"  ).write( __FILE__, __func__, __LINE__, "info",		FMT, ##__VA_ARGS__ )
#define DBG_ERROR( FMT, ... )	xgc::common::get_logger( "debug" ).write( __FILE__, __func__, __LINE__, "error",	FMT, ##__VA_ARGS__ )
#define DBG_WARNING( FMT, ... ) xgc::common::get_logger( "debug" ).write( __FILE__, __func__, __LINE__, "warning",	FMT, ##__VA_ARGS__ )
#define DBG_INFO( FMT, ... )	xgc::common::get_logger( "debug" ).write( __FILE__, __func__, __LINE__, "info",		FMT, ##__VA_ARGS__ )
#define SHM_ERROR( FMT, ... )	xgc::common::get_logger( "share" ).write( __FILE__, __func__, __LINE__, "error",	FMT, ##__VA_ARGS__ )
#define SHM_WARNING( FMT, ... )	xgc::common::get_logger( "share" ).write( __FILE__, __func__, __LINE__, "warning",	FMT, ##__VA_ARGS__ )
#define SHM_INFO( FMT, ... )	xgc::common::get_logger( "share" ).write( __FILE__, __func__, __LINE__, "info",		FMT, ##__VA_ARGS__ )

namespace xgc
{
	namespace common
	{
		///
		/// [12/16/2013 albert.xu]
		/// 初始化日志模块
		///
		COMMON_API xgc_bool init_logger( xgc_lpcstr path );

		///
		/// [1/10/2014 albert.xu]
		/// 清理日志系统
		///
		COMMON_API xgc_void fini_logger();

		///
		/// [1/10/2014 albert.xu]
		/// 写文件日志
		///
		COMMON_API xgc_void write_file( xgc_lpcstr file, xgc_lpcstr format, ... );

		class COMMON_API logger;
		///
		/// [1/10/2014 albert.xu]
		/// 日志适配器
		///
		class COMMON_API logger_adapter
		{
		public:
			virtual xgc_void write( xgc_lpvoid data, xgc_size size ) = 0;
			virtual ~logger_adapter(){}
		};

		///
		/// \brief 日志实现类
		///
		/// [01/16/2017 albert.xu]
		///
		class COMMON_API logger_impl
		{
		private:
			///
			/// \brief 字符串片段
			///
			/// [01/16/2017 albert.xu]
			///
			typedef std::function< xgc_size (xgc_char*, xgc_size) > string_span;

			// 日志输出上下文
			struct
			{
				xgc_lpcstr file;
				xgc_lpcstr func;
				xgc_lpcstr tags;
				xgc_ulong  line;

				xgc_lpcstr fmt;
				va_list	   args;
			} context;

			// 消息格式化序列
			xgc_vector<string_span> format;

			// 过滤包含关键字的消息
			xgc_unordered_set<xgc_string> filter_include;
			// 过滤排除关键字的消息
			xgc_unordered_set<xgc_string> filter_exclude;
			// 输出适配器
			xgc_list< logger_adapter* > adapters;

			friend class logger;
		private:
			logger_impl();

			~logger_impl();

			logger_impl( const logger_impl & ) = delete;
		public:
			// 添加包含关键字
			xgc_void add_include( xgc_lpcstr keyword )
			{
				filter_include.insert( keyword );
			}

			// 删除包含关键字
			xgc_void del_include( xgc_lpcstr keyword )
			{
				filter_include.erase( keyword );
			}

			// 添加排除关键字
			xgc_void add_exclude( xgc_lpcstr keyword )
			{
				filter_exclude.insert( keyword );
			}

			// 删除排除关键字
			xgc_void del_exclude( xgc_lpcstr keyword )
			{
				filter_exclude.erase( keyword );
			}

			// 添加适配器
			xgc_void add_adapter( logger_adapter *adapter )
			{
				XGC_ASSERT( adapter );
				adapters.push_back( adapter );
			}

			///
			/// \brief 解析日志格式化配置
			///
			/// [01/16/2017 albert.xu]
			///
			xgc_void parse_format( xgc_lpcstr fmt );

			///
			/// \brief 写日志
			///
			/// [01/16/2017 albert.xu]
			///
			xgc_void write( xgc_lpcstr file, xgc_lpcstr func, xgc_ulong line, xgc_lpcstr tags, xgc_lpcstr fmt, ... );

		private:
			xgc_size date( xgc_char* buf, xgc_size len )
			{
				tm stm;
				time_t tt = (time_t)::time(xgc_nullptr);
				localtime_s( &stm, &tt );
				return strftime( buf, len, "%Y-%m-%d", &stm );
			}

			xgc_size time( xgc_char* buf, xgc_size len )
			{
				tm stm;
				time_t tt = (time_t)::time(xgc_nullptr);
				localtime_s( &stm, &tt );
				return strftime( buf, len, "%H:%M:%S", &stm );
			}

			xgc_size datetime( xgc_char* buf, xgc_size len )
			{
				tm stm;
				time_t tt = (time_t)::time(xgc_nullptr);
				localtime_s( &stm, &tt );
				return strftime( buf, len, "%Y-%m-%d %H:%M:%S", &stm );
			}

			xgc_size file( xgc_char* buf, xgc_size len )
			{
				return sprintf_s( buf, len, "%s", context.file );
			}

			xgc_size func( xgc_char* buf, xgc_size len )
			{
				return sprintf_s( buf, len, "%s", context.func );
			}

			xgc_size tags( xgc_char* buf, xgc_size len )
			{
				return sprintf_s( buf, len, "%s", context.tags );
			}

			xgc_size line( xgc_char* buf, xgc_size len )
			{
				return sprintf_s( buf, len, "%ul", context.line );
			}

			xgc_size span( xgc_char* buf, xgc_size len, const xgc_string &span )
			{
				return sprintf_s( buf, len, "%s", span.c_str() );
			}

			xgc_size message( xgc_char* buf, xgc_size len )
			{
				return vsprintf_s( buf, len, context.fmt, context.args );
			}
		};

		/// get logger by logger name
		COMMON_API logger_impl& get_logger( xgc_lpcstr name );

		/// get logger by logger name
		COMMON_API logger_impl& new_logger( xgc_lpcstr name );
	}
}

#endif

