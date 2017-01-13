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

#include <functional>
#include <datetime.h>

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

#define LOGEND "\n"

#define LOGEXT( FUNCTION, LINE, LEVEL, FMT, ... )	xgc::logger::write( FUNCTION, LINE, LEVEL, FMT, ##__VA_ARGS__ )
#define LOGFMT( LEVEL, FMT, ... )					xgc::logger::write( LEVEL, FMT, ##__VA_ARGS__ )

#define SYS_ERROR( FMT, ... )	xgc::logger::write( __FILE__, __LINE__, LOGLVL_SYS_ERROR,	FMT, ##__VA_ARGS__ )
#define SYS_WARNING( FMT, ... ) xgc::logger::write( __FILE__, __LINE__, LOGLVL_SYS_WARNING,	FMT, ##__VA_ARGS__ )
#define SYS_INFO( FMT, ... )	xgc::logger::write( __FILE__, __LINE__, LOGLVL_SYS_INFO,	FMT, ##__VA_ARGS__ )
#define USR_ERROR( FMT, ... )	xgc::logger::write( __FILE__, __LINE__, LOGLVL_USR_ERROR,	FMT, ##__VA_ARGS__ )
#define USR_WARNING( FMT, ... ) xgc::logger::write( __FILE__, __LINE__, LOGLVL_USR_WARNING,	FMT, ##__VA_ARGS__ )
#define USR_INFO( FMT, ... )	xgc::logger::write( __FILE__, __LINE__, LOGLVL_USR_INFO,	FMT, ##__VA_ARGS__ )
#define DBG_ERROR( FMT, ... )	xgc::logger::write( __FILE__, __LINE__, LOGLVL_DBG_ERROR,	FMT, ##__VA_ARGS__ )
#define DBG_WARNING( FMT, ... ) xgc::logger::write( __FILE__, __LINE__, LOGLVL_DBG_WARNING,	FMT, ##__VA_ARGS__ )
#define DBG_INFO( FMT, ... )	xgc::logger::write( __FILE__, __LINE__, LOGLVL_DBG_INFO,	FMT, ##__VA_ARGS__ )
#define SHM_LOG( FMT, ... )		xgc::logger::write_shared( __FILE__, __LINE__, FMT, ##__VA_ARGS__ )

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


		///
		/// [12/16/2013 albert.xu]
		/// 初始化日志模块
		///
		COMMON_API xgc_bool init_logger_ex( xgc_lpcstr path );

		///
		/// [1/10/2014 albert.xu]
		/// 清理日志系统
		///
		COMMON_API xgc_void fini_logger_ex();

		class COMMON_API logger_impl
		{
		private:
			typedef std::function< xgc_size (xgc_char*, xgc_size) > log_span;

			struct
			{
				xgc_lpcstr file;
				xgc_lpcstr func;
				xgc_lpcstr tags;
				xgc_ulong  line;

				xgc_lpcstr fmt;
				va_list	   args;
			} context;

			xgc_vector<log_span> format;
			xgc_unordered_set<xgc_string> filter_include;
			xgc_unordered_set<xgc_string> filter_exclude;

			xgc_list< std::function< void( xgc_lpcstr, xgc_size ) > > adapters;

		public:
			xgc_void add_include( xgc_lpcstr keyword )
			{
				filter_include.insert( keyword );
			}

			xgc_void del_include( xgc_lpcstr keyword )
			{
				filter_include.erase( keyword );
			}

			xgc_void add_exclude( xgc_lpcstr keyword )
			{
				filter_include.insert( keyword );
			}

			xgc_void del_exclude( xgc_lpcstr keyword )
			{
				filter_include.erase( keyword );
			}

			xgc_void add_adapter( std::function< void( xgc_lpcstr, xgc_size ) > &&adapter )
			{
				adapters.emplace_back( std::forward< std::function< void( xgc_lpcstr, xgc_size) > >(adapter) );
			}

			xgc_void parse_format( xgc_lpcstr fmt )
			{
				auto ptr = fmt;
				auto beg = fmt;

				xgc_bool in_match = false;
				xgc_long barckets = 0; 

				while( ptr && *ptr )
				{
					if( *ptr == '$' && *(ptr+1) == '(' )
					{
						if( ptr > beg )
						{
							format.emplace_back( std::bind( &logger_impl::span, this, std::placeholders::_1, std::placeholders::_2, xgc_string(beg, ptr - beg) ) );
						}

						if( strncmp( ptr, "$(date)", 7 ) == 0 )
						{
							format.emplace_back( std::bind( &logger_impl::date, this, std::placeholders::_1, std::placeholders::_2 ) );
							ptr += 7;
						}
						else if( strncmp( ptr, "$(time)", 7 ) == 0 )
						{
							format.emplace_back( std::bind( &logger_impl::time, this, std::placeholders::_1, std::placeholders::_2 ) );
							ptr += 7;
						}
						else if( strncmp( ptr, "$(datetime)", 13 ) == 0 )
						{
							format.emplace_back( std::bind( &logger_impl::datetime, this, std::placeholders::_1, std::placeholders::_2 ) );
							ptr += 13;
						}
						else if( strncmp( ptr, "$(file)", 7 ) == 0 )
						{
							format.emplace_back( std::bind( &logger_impl::file, this, std::placeholders::_1, std::placeholders::_2 ) );
							ptr += 7;
						}
						else if( strncmp( ptr, "$(func)", 7 ) == 0 )
						{
							format.emplace_back( std::bind( &logger_impl::func, this, std::placeholders::_1, std::placeholders::_2 ) );
							ptr += 7;
						}
						else if( strncmp( ptr, "$(line)", 7 ) == 0 )
						{
							format.emplace_back( std::bind( &logger_impl::line, this, std::placeholders::_1, std::placeholders::_2 ) );
							beg = ptr += 7;
						}
						else if( strncmp( ptr, "$(message)", 10 ) == 0 )
						{
							format.emplace_back( std::bind( &logger_impl::message, this, std::placeholders::_1, std::placeholders::_2 ) );
							beg = ptr += 10;
						}

					}
					
					++ptr;
				}
			}

			xgc_void write( xgc_lpcstr file, xgc_lpcstr func, xgc_ulong line, xgc_lpcstr tags, xgc_lpcstr fmt, ... )
			{
				xgc_char log[2048] = { 0 };
				xgc_size len = 0;

				va_list args;
				va_start( args, fmt );
				context.file = file;
				context.func = func;
				context.tags = tags;
				context.line = line;
				context.fmt	 = fmt;
				context.args = args;

				for( auto &span : format )
				{
					auto cpy = span( log + len, sizeof(log) - len );
					if( cpy < 0 )
						cpy = sprintf_s( log + len, sizeof(log) - len, "<format error>" );

					len += cpy;
				}

				for( auto adapter : adapters )
				{
					adapter( log, len );
				}
				va_end(args);
			}

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

		class COMMON_API logger
		{
		private:
			logger_impl root;
			xgc_unordered_map< xgc_string, logger_impl* > loggers;

		public:
			logger_impl& get( xgc_lpcstr name )
			{
				auto it = loggers.find( name );
				if( it != loggers.end() )
					return *(it->second);

				return root;
			}

			logger_impl& get_or_create( xgc_lpcstr name )
			{
				auto it = loggers.find( name );
				if( it != loggers.end() )
					return *(it->second);

				auto log = XGC_NEW logger_impl();
				auto ib = loggers.insert( std::make_pair(name, log) );
				XGC_ASSERT( ib.second );

				return *(ib.first->second);
			}
		};


		COMMON_API logger_impl& get_logger( xgc_lpcstr name );
	}
}

#endif

