#include "defines.h"
#include "datetime.h"
#include "xutility.h"

namespace xgc
{
	namespace common
	{

#ifdef __GNUC__
		error_t localtime_s( tm * tt, const time_t * t )
		{
			auto ptt = localtime( t );
			if( tt && ptt )
			{
				*tt = *ptt;
				return 0;
			}

			return -1;
		}

		error_t gmtime_s( tm * tt, const time_t * t )
		{
			auto ptt = gmtime( t );
			if( tt && ptt )
			{
				*tt = *ptt;
				return 0;
			}

			return -1;
		}
#endif

		///
		/// 将字符串时间转换为时间片
		/// [7/23/2014] create by albert.xu
		///

		timespan timespan::convert( xgc_lpcstr str )
		{
			uint16_t time[] = { 0, 0, 0, 0 };
			xgc_lpcstr cursor = str + strlen( str );
			xgc_lpcstr splitc = ":. ";

			xgc_size it = 0;
			while( cursor > str )
			{
				xgc_lpcstr pc = strchr( splitc, *cursor );

				if( pc ) switch( *pc )
				{
					case '.':
					it = XGC_RNG( it, 0, 0 );
					time[it++] = str2numeric<uint16_t>( cursor + 1, xgc_nullptr, 10 );
					break;
					case ' ':
					it = 3;
					case ':':
					it = XGC_RNG( it, 1, 3 );
					time[it++] = str2numeric<uint16_t>( cursor + 1, xgc_nullptr, 10 );
					break;
				}

				--cursor;
			}

			if( it < XGC_COUNTOF( time ) )
			{
				time[it] = str2numeric<uint16_t>( cursor, xgc_nullptr, 10 );
			}

			return timespan( int64_t( ( ( ( ( time[3] * 60 + time[2] ) * 60 ) + time[1] ) * 1000 + time[0] ) * 10000ULL ) );
		}
	
		///
		/// 取当前时间
		/// [6/22/2014] create by albert.xu
		///

		systime datetime::to_systime() const
		{
			time_t t = to_ctime();

			tm gtime = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

			if( 0 != localtime_s( &gtime, &t ) )
				return { 0,0,0,0,0,0,0,0 };

			return {
				uint16_t( gtime.tm_year + 1900 ),
				uint16_t( gtime.tm_mon + 1 ),
				uint16_t( gtime.tm_wday ),
				uint16_t( gtime.tm_mday ),
				uint16_t( gtime.tm_hour ),
				uint16_t( gtime.tm_min ),
				uint16_t( gtime.tm_sec ),
				uint16_t( to_milliseconds() - t * 1000 )
			};
		}

		///
		/// 取当前时间
		/// [6/22/2014] create by albert.xu
		///

		systime datetime::to_utctime() const
		{
			time_t t = to_ctime();

			tm gtime = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

			if( 0 != gmtime_s( &gtime, &t ) )
				return { 0,0,0,0,0,0,0,0 };

			return {
				uint16_t( gtime.tm_year + 1900 ),
				uint16_t( gtime.tm_mon + 1 ),
				uint16_t( gtime.tm_wday ),
				uint16_t( gtime.tm_mday ),
				uint16_t( gtime.tm_hour ),
				uint16_t( gtime.tm_min ),
				uint16_t( gtime.tm_sec ),
				uint16_t( to_milliseconds() - t * 1000 )
			};
		}

		///
		/// 转换为字符串
		/// [7/15/2015] create by albert.xu
		///

		xgc_lpcstr datetime::to_string( xgc_lpstr output, xgc_size size, xgc_lpcstr format ) const
		{
			tm stm;
			time_t tt = (time_t)to_ctime();
			localtime_s( &stm, &tt );
			strftime( output, size, format, &stm );

			return output;
		}

		datetime datetime::now()
		{
			filetime ft;
#if defined(_WINDOWS)
			GetSystemTimeAsFileTime( (LPFILETIME)&ft );
#elif defined(_LINUX)
			struct timeval v;
			gettimeofday( &v, xgc_nullptr );
			ft.dt = v.tv_sec * 10000000ULL + v.tv_usec * 10ULL + _Time_T_Diff;
#endif

			return ft;
		}

		xgc_size datetime::now( xgc_lpstr output, xgc_size size, xgc_lpcstr format )
		{
			tm stm;
			time_t tt = (time_t)now().to_ctime();
			localtime_s( &stm, &tt );
			return strftime( output, size, format, &stm );
		}

		datetime datetime::relative_time( timespan span )
		{
			return datetime::now() + span;
		}

		///
		/// 将字符串转化为日期时间
		/// [7/23/2014] create by albert.xu
		/// @param str 日期时间格式的字符串。格式匹配必须右对齐，YYYY-MM-DD hh::mm::ss.ms
		///

		datetime datetime::convert( xgc_lpcstr str, datetime now )
		{
			systime st = now.to_systime();

			if( str )
			{
				xgc_lpcstr cursor = str + strlen( str );
				xgc_lpcstr splitc = "-:. ";

				uint16_t *ptr[] = { &st.milliseconds, &st.seconds, &st.minute, &st.hour, &st.day, &st.month, &st.year };
				xgc_size idt = 0;
				while( cursor > str )
				{
					xgc_lpcstr pc = strchr( splitc, *cursor );

					if( pc ) switch( *pc )
					{
						case '.':
						idt = XGC_RNG( idt, 0, 0 );
						*ptr[idt++] = (uint16_t)strtoul( cursor + 1, xgc_nullptr, 10 );
						break;
						case ' ':
						idt = 3;
						case ':':
						idt = XGC_RNG( idt, 1, 3 );
						*ptr[idt++] = (uint16_t)strtoul( cursor + 1, xgc_nullptr, 10 );
						break;
						case '-':
						idt = XGC_RNG( idt, 4, 6 );
						*ptr[idt++] = (uint16_t)strtoul( cursor + 1, xgc_nullptr, 10 );
						break;
					}

					--cursor;
				}

				if( idt < XGC_COUNTOF( ptr ) )
				{
					*ptr[idt] = (uint16_t)strtoul( cursor, xgc_nullptr, 10 );
				}
			}
			return datetime( st );
		}
	}
}