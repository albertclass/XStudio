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

#define LOGEND "\n"

#define LOGEXT( FILE, FUNC, LINE, TAGS, FMT, ... ) xgc::common::get_logger( "USR" ).write( FILE, FUNC, LINE, TAGS, FMT, ##__VA_ARGS__ )
#define LOGFMT( TAGS, FMT, ... ) xgc::common::get_logger( "USR" ).write( __FILE__, __func__, __LINE__, TAGS, FMT, ##__VA_ARGS__ )

#define SYS_ERROR( FMT, ... )	xgc::common::get_logger( "SYS" ).write( __FILE__, __func__, __LINE__, "error",		FMT, ##__VA_ARGS__ )
#define SYS_WARNING( FMT, ... ) xgc::common::get_logger( "SYS" ).write( __FILE__, __func__, __LINE__, "warning",	FMT, ##__VA_ARGS__ )
#define SYS_INFO( FMT, ... )	xgc::common::get_logger( "SYS" ).write( __FILE__, __func__, __LINE__, "info",		FMT, ##__VA_ARGS__ )
#define USR_ERROR( FMT, ... )	xgc::common::get_logger( "USR" ).write( __FILE__, __func__, __LINE__, "error",		FMT, ##__VA_ARGS__ )
#define USR_WARNING( FMT, ... ) xgc::common::get_logger( "USR" ).write( __FILE__, __func__, __LINE__, "warning",	FMT, ##__VA_ARGS__ )
#define USR_INFO( FMT, ... )	xgc::common::get_logger( "USR" ).write( __FILE__, __func__, __LINE__, "info",		FMT, ##__VA_ARGS__ )
#define DBG_ERROR( FMT, ... )	xgc::common::get_logger( "DBG" ).write( __FILE__, __func__, __LINE__, "error",		FMT, ##__VA_ARGS__ )
#define DBG_WARNING( FMT, ... ) xgc::common::get_logger( "DBG" ).write( __FILE__, __func__, __LINE__, "warning",	FMT, ##__VA_ARGS__ )
#define DBG_INFO( FMT, ... )	xgc::common::get_logger( "DBG" ).write( __FILE__, __func__, __LINE__, "info",		FMT, ##__VA_ARGS__ )
#define SHM_ERROR( FMT, ... )	xgc::common::get_logger( "SHM" ).write( __FILE__, __func__, __LINE__, "error",		FMT, ##__VA_ARGS__ )
#define SHM_WARNING( FMT, ... )	xgc::common::get_logger( "SHM" ).write( __FILE__, __func__, __LINE__, "warning",	FMT, ##__VA_ARGS__ )
#define SHM_INFO( FMT, ... )	xgc::common::get_logger( "SHM" ).write( __FILE__, __func__, __LINE__, "info",		FMT, ##__VA_ARGS__ )

namespace xgc
{
	namespace common
	{
		class ini_reader;
		///
		/// [12/16/2013 albert.xu]
		/// 初始化日志模块
		///
		COMMON_API xgc_bool init_logger( xgc_lpcstr path );

		///
		/// [12/16/2013 albert.xu]
		/// 初始化日志模块
		///
		COMMON_API xgc_bool init_logger( ini_reader &ini );

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
		/// 日志格式化器
		///
		class COMMON_API logger_formater
		{
		public:
			// 日志输出上下文
			struct context
			{
				/// 文件名
				xgc_lpcstr file;
				/// 函数名
				xgc_lpcstr func;
				/// 标签
				xgc_lpcstr tags;
				/// 行号
				xgc_ulong line;

				/// 消息格式串
				xgc_lpcstr fmt;
				/// 消息参数
				va_list args;
			};

			///
			/// \brief 字符串片段
			///
			/// [01/16/2017 albert.xu]
			///
			typedef std::function< xgc_size( logger_formater*, const context &, xgc_char*, xgc_size ) > format_span;

		protected:
			// 日志缓冲
			xgc_lpstr message_;

			// 消息格式化序列
			xgc_vector< format_span > format;

		protected:
			xgc_size date( xgc_char* buf, xgc_size len );

			xgc_size time( xgc_char* buf, xgc_size len );

			xgc_size datetime( xgc_char* buf, xgc_size len );

			xgc_size file( const context &ctx, xgc_char* buf, xgc_size len );

			xgc_size func( const context &ctx, xgc_char* buf, xgc_size len );

			xgc_size tags( const context &ctx, xgc_char* buf, xgc_size len );

			xgc_size line( const context &ctx, xgc_char* buf, xgc_size len );

			xgc_size endl( xgc_char* buf, xgc_size len );

			xgc_size span( xgc_char* buf, xgc_size len, const xgc_string &span );

			xgc_size message( const context &ctx, xgc_char* buf, xgc_size len );

		public:
			///
			/// \brief 构造日志格式化对象
			/// \author Albert.xu
			/// \date 2017/07/25
			///
			logger_formater();

			///
			/// \brief 析构日志格式化对象
			/// \author Albert.xu
			/// \date 2017/07/25
			///
			~logger_formater();

			///
			/// \brief 解析格式化字符串
			/// \author Albert.xu
			/// \date 2017/07/25
			///
			xgc_void parse_format( xgc_lpcstr fmt );

			///
			/// \brief 解析格式化字符串
			/// \author Albert.xu
			/// \date 2017/07/25
			///
			xgc_long parse_message( const context &ctx );

			///
			/// \brief 是否为空
			/// \author Albert.xu
			/// \date 2017/07/25
			///
			xgc_bool empty()const
			{
				return format.empty();
			}

			///
			/// \brief 解析格式化字符串
			/// \author Albert.xu
			/// \date 2017/07/25
			///
			xgc_lpvoid log()const
			{
				return message_;
			}
		};

		///
		/// [1/10/2014 albert.xu]
		/// 日志适配器
		///
		class COMMON_API logger_adapter
		{
		protected:
			logger_formater formater;

		public:
			logger_adapter()
			{

			}

			virtual ~logger_adapter() 
			{
			}

			xgc_void parse_format( xgc_lpcstr format )
			{
				if( xgc_nullptr == format )
					return;

				formater.parse_format( format );
			}

			virtual xgc_long write( const logger_formater::context& ctx )
			{
				if( formater.empty() )
					return -1;

				xgc_long len = formater.parse_message( ctx );
				if( len > 0 )
					write( formater.log(), len );

				return len;
			}

			virtual xgc_void join() = 0;
			virtual xgc_void write( xgc_lpvoid data, xgc_size size ) = 0;
		};

		///
		/// \brief 日志实现类
		///
		/// [01/16/2017 albert.xu]
		///
		class COMMON_API logger_impl
		{
		private:
			/// 日志名
			xgc_char logger_name[32];

			/// 过滤包含关键字的消息
			xgc_unordered_set<xgc_string> filter_include;
			/// 过滤排除关键字的消息
			xgc_unordered_set<xgc_string> filter_exclude;
			/// 输出适配器
			xgc_list< logger_adapter* > adapters;
			/// 格式化器
			logger_formater formater;

			friend class logger;
		private:
			logger_impl( xgc_lpcstr name );

			~logger_impl();

			logger_impl( const logger_impl & ) = delete;
		public:
			// 设置日志格式化对象
			xgc_void parse_format( xgc_lpcstr format )
			{
				formater.parse_format( format );
			}

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
			xgc_void add_adapter( logger_adapter* adapter )
			{
				adapters.push_back( adapter );
			}

			///
			/// \brief 写日志
			/// \author albert.xu
			/// \date 2017/01/16
			///
			xgc_void write( xgc_lpcstr file, xgc_lpcstr func, xgc_ulong line, xgc_lpcstr tags, xgc_lpcstr fmt, ... );
		};

		/// get logger by logger name
		COMMON_API logger_impl& get_logger( xgc_lpcstr name );

		/// get logger by logger name
		COMMON_API logger_impl& new_logger( xgc_lpcstr name );
	}
}

#endif

