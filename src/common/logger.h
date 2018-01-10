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
#include "exports.h"
#include <sstream>

#ifdef _WINDOWS
#	define LOGEND "\r\n"
#endif 

#ifdef _LINUX
#	define LOGEND "\n"
#endif 

#define LOG_EXT_STREAM( LOG, FILE, FUNC, LINE, TAGS ) \
	if( xgc::common::get_logger( #LOG ).filter( TAGS ) ) \
		xgc::common::logger_stream( xgc::common::get_logger( #LOG ), xgc::common::logger_context( FILE, FUNC, LINE, TAGS ) )

#define LOG_TAG_STREAM( LOG, TAGS ) \
	if( xgc::common::get_logger( #LOG ).filter( TAGS ) ) \
		xgc::common::logger_stream( xgc::common::get_logger( #LOG ), xgc::common::logger_context( __FILE__, __func__, __LINE__, TAGS ) )

#define SYS_ERR_STREAM() LOG_TAG_STREAM( SYS, "error"   )
#define SYS_WRN_STREAM() LOG_TAG_STREAM( SYS, "warning" )
#define SYS_TIP_STREAM() LOG_TAG_STREAM( SYS, "info"	)
#define USR_ERR_STREAM() LOG_TAG_STREAM( USR, "error"   )
#define USR_WRN_STREAM() LOG_TAG_STREAM( USR, "warning" )
#define USR_TIP_STREAM() LOG_TAG_STREAM( USR, "info"	)
#define DBG_ERR_STREAM() LOG_TAG_STREAM( DBG, "error"   )
#define DBG_WRN_STREAM() LOG_TAG_STREAM( DBG, "warning" )
#define DBG_TIP_STREAM() LOG_TAG_STREAM( DBG, "info"	)

#define LOG_EXT_FORMAT( LOG, FILE, FUNC, LINE, TAGS, FMT, ... ) \
	if( xgc::common::get_logger( #LOG ).filter( TAGS ) ) \
		xgc::common::get_logger( #LOG ).write( xgc::common::logger_context( FILE, FUNC, LINE, TAGS ), FMT, ##__VA_ARGS__ )
#define LOG_TAG_FORMAT( LOG, TAGS, FMT, ... ) \
	if( xgc::common::get_logger( #LOG ).filter( TAGS ) ) \
		xgc::common::get_logger( #LOG ).write( xgc::common::logger_context( __FILE__, __func__, __LINE__, TAGS ), FMT, ##__VA_ARGS__ )

#define SYS_ERR( FMT, ... )	LOG_TAG_FORMAT( SYS, "error"   , FMT, ##__VA_ARGS__ )
#define SYS_WRN( FMT, ... ) LOG_TAG_FORMAT( SYS, "warning" , FMT, ##__VA_ARGS__ )
#define SYS_TIP( FMT, ... )	LOG_TAG_FORMAT( SYS, "info"	   , FMT, ##__VA_ARGS__ )
#define USR_ERR( FMT, ... )	LOG_TAG_FORMAT( USR, "error"   , FMT, ##__VA_ARGS__ )
#define USR_WRN( FMT, ... ) LOG_TAG_FORMAT( USR, "warning" , FMT, ##__VA_ARGS__ )
#define USR_TIP( FMT, ... )	LOG_TAG_FORMAT( USR, "info"	   , FMT, ##__VA_ARGS__ )
#define DBG_ERR( FMT, ... )	LOG_TAG_FORMAT( DBG, "error"   , FMT, ##__VA_ARGS__ )
#define DBG_WRN( FMT, ... ) LOG_TAG_FORMAT( DBG, "warning" , FMT, ##__VA_ARGS__ )
#define DBG_TIP( FMT, ... )	LOG_TAG_FORMAT( DBG, "info"	   , FMT, ##__VA_ARGS__ )

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

		// 日志输出上下文
		struct COMMON_API logger_context
		{
			/// 行号
			xgc_ulong  line_;
			/// 文件名
			xgc_lpcstr file_;
			/// 函数名
			xgc_lpcstr func_;
			/// 标签
			xgc_lpcstr tags_;
			/// 日志
			xgc_lpcstr logs_;
			/// 日志长度
			xgc_ulong  logs_len_;

			///
			/// \brief 删除默认构造  
			/// \date 1/5/2018
			/// \author albert.xu
			///
			logger_context();

			///
			/// \brief 构造
			/// \date 1/4/2018
			/// \author albert.xu
			///
			logger_context( xgc_lpcstr file, xgc_lpcstr func, xgc_ulong line, xgc_lpcstr tags );

			///
			/// \brief 拷贝构造 
			/// \date 1/4/2018
			/// \author albert.xu
			///
			logger_context( const logger_context& ctx );

			///
			/// \brief 析构函数 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			~logger_context();
		};

		///
		/// [1/10/2014 albert.xu]
		/// 日志格式化器
		///
		class COMMON_API logger_formater
		{
		public:
			///
			/// \brief 字符串片段
			///
			/// [01/16/2017 albert.xu]
			///
			typedef std::function< xgc_size( logger_formater*, const logger_context&, xgc_char*, xgc_size ) > format_span;

		protected:
			// 日志缓冲
			xgc_lpstr message_;

			// 消息格式化序列
			xgc::vector< format_span > format;

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
			/// \brief 格式化日志 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_long parse_log( const logger_context& ctx )
			{
				return parse_message( ctx, &message_ );
			}

			///
			/// \brief 获取格式化日志 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_lpcvoid log()const
			{
				return message_;
			}

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
			/// \brief 格式化日期时间
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size datetime( xgc_char * buf, xgc_size len, xgc_lpcstr fmt );

			///
			/// \brief 格式化路径
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size path( const logger_context& ctx, xgc_char * buf, xgc_size len )
			{
				return sprintf_s( buf, len, "%s", ctx.file_ );
			}

			///
			/// \brief 格式化文件
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size file( const logger_context& ctx, xgc_char * buf, xgc_size len )
			{
				auto last_slash = ctx.file_;
				auto next_slash = ctx.file_;
				while( next_slash = strpbrk( last_slash + 1, SLASH_ALL ) )
					last_slash = next_slash;

				return sprintf_s( buf, len, "%s", last_slash + 1 );
			}

			///
			/// \brief 格式化函数
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size func( const logger_context& ctx, xgc_char * buf, xgc_size len )
			{
				return sprintf_s( buf, len, "%s", ctx.func_ );
			}

			///
			/// \brief 格式化标签
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size tags( const logger_context& ctx, xgc_char * buf, xgc_size len )
			{
				return sprintf_s( buf, len, "%s", ctx.tags_ );
			}

			///
			/// \brief 格式化行号 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size line( const logger_context& ctx, xgc_char * buf, xgc_size len )
			{
				return sprintf_s( buf, len, "(%lu)", ctx.line_ );
			}

			///
			/// \brief 格式化片段
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size span( xgc_char * buf, xgc_size len, const xgc::string & span )
			{
				return sprintf_s( buf, len, "%s", span.c_str() );
			}

			///
			/// \brief 格式化换行
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size endl( xgc_char * buf, xgc_size len )
			{
				return sprintf_s( buf, len, "\n" );
			}

			///
			/// \brief 格式化日志内容
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size message( const logger_context& ctx, xgc_char * buf, xgc_size len )
			{
				return sprintf_s( buf, len, "%s", ctx.logs_ );
			}

		private:
			///
			/// \brief 解析格式化字符串
			/// \author Albert.xu
			/// \date 2017/07/25
			///
			xgc_ulong parse_message( const logger_context &ctx, xgc_char** message );
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

			///
			/// \brief 设置适配器输出格式
			/// \author Albert.xu
			/// \date 2017/07/27
			///
			xgc_void parse_format( xgc_lpcstr format )
			{
				if( xgc_nullptr == format )
					return;

				formater.parse_format( format );
			}

			///
			/// \brief 关闭日志适配器
			/// \author Albert.xu
			/// \date 2017/07/27
			///
			virtual xgc_void join() = 0;

			///
			/// \brief 写日志
			/// \author Albert.xu
			/// \date 2017/07/27
			///
			virtual xgc_long write( xgc_lpcvoid data, xgc_size size ) = 0;

			///
			/// \brief 写格式化日志
			/// \author Albert.xu
			/// \date 2017/07/27
			///
			virtual xgc_long write( logger_context& _ctx )
			{
				if( formater.empty() )
					return -1;

				int len = formater.parse_log( _ctx );

				if( len > 0 )
					return write( formater.log(), len );

				return -1;
			}
		};

		///
		/// \brief 日志实现类
		///
		/// [01/16/2017 albert.xu]
		///
		class COMMON_API logger_impl
		{
			friend class logger;
		private:
			/// 日志名
			xgc_char logger_name[32];

			/// 过滤模式
			xgc_long filter_mode;
			/// 过滤包含关键字的消息
			unordered_set<xgc::string> filter_include;
			/// 过滤排除关键字的消息
			unordered_set<xgc::string> filter_exclude;
			/// 输出适配器
			vector< logger_adapter* > adapters;
			/// 格式化器
			logger_formater formater;
		private:
			///
			/// \brief 不需要默认构造 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			logger_impl() = delete;

			///
			/// \brief 不允许拷贝构造 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			logger_impl( const logger_impl & ) = delete;

			///
			/// \brief 日志实例构造 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			logger_impl( xgc_lpcstr name );

			///
			/// \brief 日志实例析构 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			~logger_impl();

		public:
			///
			/// \brief 设置日志格式化对象
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_void parse_format( xgc_lpcstr format )
			{
				formater.parse_format( format );
			}

			/// 
			/// \brief 添加包含关键字
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_void add_include( string && keyword )
			{
				filter_include.insert( keyword );
			}

			/// 
			/// \brief 添加包含关键字
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_void add_include( const string & keyword )
			{
				filter_include.insert( keyword );
			}

			///
			/// \brief 删除包含关键字 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_void del_include( const string & keyword )
			{
				filter_include.erase( keyword );
			}

			///
			/// \brief  添加排除关键字
			/// \date 1/5/2018
			/// \author albert.xu
			/// 
			xgc_void add_exclude( string && keyword )
			{
				filter_exclude.insert( keyword );
			}

			///
			/// \brief  添加排除关键字
			/// \date 1/5/2018
			/// \author albert.xu
			/// 
			xgc_void add_exclude( const string & keyword )
			{
				filter_exclude.insert( keyword );
			}

			///
			/// \brief 删除排除关键字 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_void del_exclude( const string & keyword )
			{
				filter_exclude.erase( keyword );
			}

			///
			/// \brief 设置过滤模式 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_void set_filter_mode( xgc_long mode )
			{
				filter_mode = mode;
			}

			///
			/// \brief 是否包含过滤字
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_bool filter( xgc_lpcstr tags );

			///
			/// \brief 添加适配器 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_void add_adapter( logger_adapter* adapter )
			{
				adapters.push_back( adapter );
			}

			///
			/// \brief 写日志
			/// \author albert.xu
			/// \date 2017/01/16
			///
			xgc_void write( logger_context& ctx, xgc_lpcstr fmt, ... );
		};

		struct logger_stream : public std::ostringstream
		{
		protected:
			logger_impl& impl_;
			logger_context context_;

		public:
			logger_stream( logger_impl &impl, const logger_context& context )
				: impl_( impl )
				, context_( context )
			{

			}

			~logger_stream()
			{
				flush();
			}

			xgc_void flush()
			{
				impl_.write( context_, "%s", str().c_str() );
			}

			logger_stream& operator << ( logger_stream& ( __cdecl *_Pfn )( logger_stream& ) )
			{
				return _Pfn( *this );
			}
		};

		///
		/// \brief 立即写入 
		/// \date 1/5/2018
		/// \author albert.xu
		///
		XGC_INLINE logger_stream& __cdecl flush( logger_stream& ss )
		{
			ss.flush();
			return ss;
		}

		///
		/// \brief 通过日志名获取日志实例 
		/// \date 1/5/2018
		/// \author albert.xu
		///
		COMMON_API logger_impl& get_logger( xgc_lpcstr name );

		///
		/// \brief 新建一个日志实例，若该实例名已存在则返回对应的实例
		/// \date 1/5/2018
		/// \author albert.xu
		///
		COMMON_API logger_impl& new_logger( xgc_lpcstr name );
	}
}

#endif

