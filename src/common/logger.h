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
		/// ��ʼ����־ģ��
		///
		COMMON_API xgc_bool init_logger( xgc_lpcstr path );

		///
		/// [12/16/2013 albert.xu]
		/// ��ʼ����־ģ��
		///
		COMMON_API xgc_bool init_logger( ini_reader &ini );

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

		// ��־���������
		struct COMMON_API logger_context
		{
			/// �к�
			xgc_ulong  line_;
			/// �ļ���
			xgc_lpcstr file_;
			/// ������
			xgc_lpcstr func_;
			/// ��ǩ
			xgc_lpcstr tags_;
			/// ��־
			xgc_lpcstr logs_;
			/// ��־����
			xgc_ulong  logs_len_;

			///
			/// \brief ɾ��Ĭ�Ϲ���  
			/// \date 1/5/2018
			/// \author albert.xu
			///
			logger_context();

			///
			/// \brief ����
			/// \date 1/4/2018
			/// \author albert.xu
			///
			logger_context( xgc_lpcstr file, xgc_lpcstr func, xgc_ulong line, xgc_lpcstr tags );

			///
			/// \brief �������� 
			/// \date 1/4/2018
			/// \author albert.xu
			///
			logger_context( const logger_context& ctx );

			///
			/// \brief �������� 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			~logger_context();
		};

		///
		/// [1/10/2014 albert.xu]
		/// ��־��ʽ����
		///
		class COMMON_API logger_formater
		{
		public:
			///
			/// \brief �ַ���Ƭ��
			///
			/// [01/16/2017 albert.xu]
			///
			typedef std::function< xgc_size( logger_formater*, const logger_context&, xgc_char*, xgc_size ) > format_span;

		protected:
			// ��־����
			xgc_lpstr message_;

			// ��Ϣ��ʽ������
			xgc::vector< format_span > format;

		public:
			///
			/// \brief ������־��ʽ������
			/// \author Albert.xu
			/// \date 2017/07/25
			///
			logger_formater();

			///
			/// \brief ������־��ʽ������
			/// \author Albert.xu
			/// \date 2017/07/25
			///
			~logger_formater();

			///
			/// \brief ������ʽ���ַ���
			/// \author Albert.xu
			/// \date 2017/07/25
			///
			xgc_void parse_format( xgc_lpcstr fmt );

			///
			/// \brief ��ʽ����־ 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_long parse_log( const logger_context& ctx )
			{
				return parse_message( ctx, &message_ );
			}

			///
			/// \brief ��ȡ��ʽ����־ 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_lpcvoid log()const
			{
				return message_;
			}

			///
			/// \brief �Ƿ�Ϊ��
			/// \author Albert.xu
			/// \date 2017/07/25
			///
			xgc_bool empty()const
			{
				return format.empty();
			}

			///
			/// \brief ��ʽ������ʱ��
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size datetime( xgc_char * buf, xgc_size len, xgc_lpcstr fmt );

			///
			/// \brief ��ʽ��·��
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size path( const logger_context& ctx, xgc_char * buf, xgc_size len )
			{
				return sprintf_s( buf, len, "%s", ctx.file_ );
			}

			///
			/// \brief ��ʽ���ļ�
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
			/// \brief ��ʽ������
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size func( const logger_context& ctx, xgc_char * buf, xgc_size len )
			{
				return sprintf_s( buf, len, "%s", ctx.func_ );
			}

			///
			/// \brief ��ʽ����ǩ
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size tags( const logger_context& ctx, xgc_char * buf, xgc_size len )
			{
				return sprintf_s( buf, len, "%s", ctx.tags_ );
			}

			///
			/// \brief ��ʽ���к� 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size line( const logger_context& ctx, xgc_char * buf, xgc_size len )
			{
				return sprintf_s( buf, len, "(%lu)", ctx.line_ );
			}

			///
			/// \brief ��ʽ��Ƭ��
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size span( xgc_char * buf, xgc_size len, const xgc::string & span )
			{
				return sprintf_s( buf, len, "%s", span.c_str() );
			}

			///
			/// \brief ��ʽ������
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size endl( xgc_char * buf, xgc_size len )
			{
				return sprintf_s( buf, len, "\n" );
			}

			///
			/// \brief ��ʽ����־����
			/// \date 1/5/2018
			/// \author albert.xu
			///
			XGC_INLINE xgc_size message( const logger_context& ctx, xgc_char * buf, xgc_size len )
			{
				return sprintf_s( buf, len, "%s", ctx.logs_ );
			}

		private:
			///
			/// \brief ������ʽ���ַ���
			/// \author Albert.xu
			/// \date 2017/07/25
			///
			xgc_ulong parse_message( const logger_context &ctx, xgc_char** message );
		};

		///
		/// [1/10/2014 albert.xu]
		/// ��־������
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
			/// \brief ���������������ʽ
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
			/// \brief �ر���־������
			/// \author Albert.xu
			/// \date 2017/07/27
			///
			virtual xgc_void join() = 0;

			///
			/// \brief д��־
			/// \author Albert.xu
			/// \date 2017/07/27
			///
			virtual xgc_long write( xgc_lpcvoid data, xgc_size size ) = 0;

			///
			/// \brief д��ʽ����־
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
		/// \brief ��־ʵ����
		///
		/// [01/16/2017 albert.xu]
		///
		class COMMON_API logger_impl
		{
			friend class logger;
		private:
			/// ��־��
			xgc_char logger_name[32];

			/// ����ģʽ
			xgc_long filter_mode;
			/// ���˰����ؼ��ֵ���Ϣ
			unordered_set<xgc::string> filter_include;
			/// �����ų��ؼ��ֵ���Ϣ
			unordered_set<xgc::string> filter_exclude;
			/// ���������
			vector< logger_adapter* > adapters;
			/// ��ʽ����
			logger_formater formater;
		private:
			///
			/// \brief ����ҪĬ�Ϲ��� 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			logger_impl() = delete;

			///
			/// \brief ������������ 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			logger_impl( const logger_impl & ) = delete;

			///
			/// \brief ��־ʵ������ 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			logger_impl( xgc_lpcstr name );

			///
			/// \brief ��־ʵ������ 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			~logger_impl();

		public:
			///
			/// \brief ������־��ʽ������
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_void parse_format( xgc_lpcstr format )
			{
				formater.parse_format( format );
			}

			/// 
			/// \brief ��Ӱ����ؼ���
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_void add_include( string && keyword )
			{
				filter_include.insert( keyword );
			}

			/// 
			/// \brief ��Ӱ����ؼ���
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_void add_include( const string & keyword )
			{
				filter_include.insert( keyword );
			}

			///
			/// \brief ɾ�������ؼ��� 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_void del_include( const string & keyword )
			{
				filter_include.erase( keyword );
			}

			///
			/// \brief  ����ų��ؼ���
			/// \date 1/5/2018
			/// \author albert.xu
			/// 
			xgc_void add_exclude( string && keyword )
			{
				filter_exclude.insert( keyword );
			}

			///
			/// \brief  ����ų��ؼ���
			/// \date 1/5/2018
			/// \author albert.xu
			/// 
			xgc_void add_exclude( const string & keyword )
			{
				filter_exclude.insert( keyword );
			}

			///
			/// \brief ɾ���ų��ؼ��� 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_void del_exclude( const string & keyword )
			{
				filter_exclude.erase( keyword );
			}

			///
			/// \brief ���ù���ģʽ 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_void set_filter_mode( xgc_long mode )
			{
				filter_mode = mode;
			}

			///
			/// \brief �Ƿ����������
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_bool filter( xgc_lpcstr tags );

			///
			/// \brief ��������� 
			/// \date 1/5/2018
			/// \author albert.xu
			///
			xgc_void add_adapter( logger_adapter* adapter )
			{
				adapters.push_back( adapter );
			}

			///
			/// \brief д��־
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
		/// \brief ����д�� 
		/// \date 1/5/2018
		/// \author albert.xu
		///
		XGC_INLINE logger_stream& __cdecl flush( logger_stream& ss )
		{
			ss.flush();
			return ss;
		}

		///
		/// \brief ͨ����־����ȡ��־ʵ�� 
		/// \date 1/5/2018
		/// \author albert.xu
		///
		COMMON_API logger_impl& get_logger( xgc_lpcstr name );

		///
		/// \brief �½�һ����־ʵ��������ʵ�����Ѵ����򷵻ض�Ӧ��ʵ��
		/// \date 1/5/2018
		/// \author albert.xu
		///
		COMMON_API logger_impl& new_logger( xgc_lpcstr name );
	}
}

#endif

