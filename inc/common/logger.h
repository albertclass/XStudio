///
/// CopyRight ? 2016 X Studio
/// \file logger.h
/// \date ���� 2016
///
/// \author albert.xu windxu@126.com
/// \brief ��־֧��
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
		/// ��ʼ����־ģ��
		///
		COMMON_API xgc_bool init_logger( xgc_lpcstr path, LoggerCallback callback, xgc_lpvoid context );

		///
		/// [1/10/2014 albert.xu]
		/// ������־ϵͳ
		///
		COMMON_API xgc_void fini_logger();

		///
		/// [12/26/2013 albert.xu]
		/// ��ʼ�������ڴ���־
		///
		COMMON_API xgc_bool init_logger_shared( xgc_lpcstr path, xgc_lpcstr name );

		///
		/// [12/16/2013 albert.xu]
		/// д��־
		///
		COMMON_API xgc_void write( EExeLogLvl level, xgc_lpcstr format, ... );

		///
		/// [12/16/2013 albert.xu]
		/// д��־
		///
		COMMON_API xgc_void write( xgc_lpcstr function, xgc_int32 line, EExeLogLvl level, xgc_lpcstr format, ... );
		///
		/// [3/4/2014 albert.xu]
		/// �����ڴ���־
		///
		COMMON_API xgc_void write_shared( xgc_lpcstr function, xgc_int32 line, xgc_lpcstr format, ... );

		///
		/// [3/4/2014 albert.xu]
		/// �ļ���־
		///
		COMMON_API xgc_void write_file( xgc_lpcstr file, xgc_lpcstr format, ... );
	}

	namespace common
	{
		///
		/// [12/16/2013 albert.xu]
		/// ��ʼ����־ģ��
		///
		COMMON_API xgc_bool init_logger( xgc_lpcstr path );

		///
		/// [1/10/2014 albert.xu]
		/// ������־ϵͳ
		///
		COMMON_API xgc_void fini_logger();

		///
		/// [1/10/2014 albert.xu]
		/// д�ļ���־
		///
		COMMON_API xgc_void write_file( xgc_lpcstr file, xgc_lpcstr format, ... );

		class COMMON_API logger;
		///
		/// [1/10/2014 albert.xu]
		/// ��־������
		///
		class COMMON_API logger_adapter
		{
		public:
			virtual xgc_void write( xgc_lpvoid data, xgc_size size ) = 0;
			virtual ~logger_adapter(){}
		};

		///
		/// \brief ��־ʵ����
		///
		/// [01/16/2017 albert.xu]
		///
		class COMMON_API logger_impl
		{
		private:
			///
			/// \brief �ַ���Ƭ��
			///
			/// [01/16/2017 albert.xu]
			///
			typedef std::function< xgc_size (xgc_char*, xgc_size) > string_span;

			// ��־���������
			struct
			{
				xgc_lpcstr file;
				xgc_lpcstr func;
				xgc_lpcstr tags;
				xgc_ulong  line;

				xgc_lpcstr fmt;
				va_list	   args;
			} context;

			// ��Ϣ��ʽ������
			xgc_vector<string_span> format;

			// ���˰����ؼ��ֵ���Ϣ
			xgc_unordered_set<xgc_string> filter_include;
			// �����ų��ؼ��ֵ���Ϣ
			xgc_unordered_set<xgc_string> filter_exclude;
			// ���������
			xgc_list< logger_adapter* > adapters;

			friend class logger;
		private:
			logger_impl()
			{

			}

			~logger_impl()
			{
				for( auto it : adapters )
				{
					SAFE_DELETE(it);
				}

				adapters.clear();
				format.clear();
				filter_include.clear();
				filter_exclude.clear();
			}

			logger_impl( const logger_impl & ) = delete;
		public:
			// ��Ӱ����ؼ���
			xgc_void add_include( xgc_lpcstr keyword )
			{
				filter_include.insert( keyword );
			}

			// ɾ�������ؼ���
			xgc_void del_include( xgc_lpcstr keyword )
			{
				filter_include.erase( keyword );
			}

			// ����ų��ؼ���
			xgc_void add_exclude( xgc_lpcstr keyword )
			{
				filter_include.insert( keyword );
			}

			// ɾ���ų��ؼ���
			xgc_void del_exclude( xgc_lpcstr keyword )
			{
				filter_include.erase( keyword );
			}

			// ���������
			xgc_void add_adapter( logger_adapter *adapter )
			{
				XGC_ASSERT( adapter );
				adapters.push_back( adapter );
			}

			///
			/// \brief ������־��ʽ������
			///
			/// [01/16/2017 albert.xu]
			///
			xgc_void parse_format( xgc_lpcstr fmt );

			///
			/// \brief д��־
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

		class COMMON_API logger
		{
		private:
			xgc_unordered_map< xgc_string, logger_impl* > loggers;

		public:
			logger_impl& get( xgc_lpcstr name )
			{
				auto it = loggers.find( name );
				if( it != loggers.end() )
					return *(it->second);

				return *(logger_impl*)xgc_nullptr;
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

			xgc_void clear()
			{
				for( auto it : loggers )
				{
					SAFE_DELETE( it.second );
				}

				loggers.clear();
			}
		};

		/// get logger by logger name
		COMMON_API logger_impl& get_logger( xgc_lpcstr name );

		/// get logger by logger name
		COMMON_API logger_impl& new_logger( xgc_lpcstr name );
	}
}

#endif

