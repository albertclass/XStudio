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

		/// 周期模式
		enum cycle_mode : xgc_uint16
		{ 
			e_week, 
			e_month, 
			e_year, 
			e_repeat 
		};

		xgc_bool cycle_params_parse( xgc_lpcstr args, xgc_uint16 & type, xgc_uint64 & data )
		{
			XGC_ASSERT_RETURN( xgc_nullptr != args && args[0] != 0, false );

			xgc_lpcstr cmdstr = args;
			xgc_lpcstr params = strchr( args, ':' );

			XGC_ASSERT_RETURN( params, false, "非法的参数格式" );

			xgc_size cmdlen = params - cmdstr;
			++params;

			if( strncmp( "week", cmdstr, cmdlen ) == 0 )
			{
				type = cycle_mode::e_week;
				// 根据‘，’分割不连续的段
				xgc_lpcstr delim = " ,\t";
				xgc_lpcstr token = params + strspn( params, delim );
				while( *token )
				{
					xgc_lpcstr next = token + strcspn( token, delim );

					xgc_lpstr end = xgc_nullptr;
					xgc_ulong min = strtoul( token, &end, 10 );
					xgc_ulong max = min;

					// 查找范围标志
					while( end < next )
					{
						if( *end == '-' )
						{
							max = strtoul( end + 1, &end, 10 );
						}
						++end;
					}

					XGC_ASSERT( max >= min );
					// 设置位掩码
					for( xgc_ulong bit = min; bit <= max && bit <= 7; ++bit )
					{
						data = XGC_SETBIT( data, bit % 7 );
					}

					// 取下一个分段
					token = next + strspn( next, delim );
				}

				return true;
			}

			if( strncmp( "moon", cmdstr, cmdlen ) == 0 || strncmp( "month", cmdstr, cmdlen ) == 0 )
			{
				type = cycle_mode::e_month;
				// 根据‘，’分割不连续的段
				xgc_lpcstr delim = " ,\t";
				xgc_lpcstr token = params + strspn( params, delim );
				while( *token )
				{
					xgc_lpcstr next = token + strcspn( token, delim );

					xgc_lpstr end = xgc_nullptr;
					xgc_ulong min = strtoul( token, &end, 10 );
					xgc_ulong max = min;

					// 查找范围标志
					while( end < next )
					{
						if( *end == '-' )
						{
							max = strtoul( end + 1, &end, 10 );
						}
						++end;
					}

					XGC_ASSERT( max >= min );
					// 设置位掩码
					for( xgc_ulong bit = min; bit <= max && bit <= 31; ++bit )
					{
						data = XGC_SETBIT( data, bit );
					}

					// 取下一个分段
					token = next + strspn( next, delim );
				}

				return true;
			}

			if( strncmp( "rept", cmdstr, cmdlen ) == 0 || strncmp( "repeat", cmdstr, cmdlen ) == 0 )
			{
				type = cycle_mode::e_repeat;
				// 执行的间隔时间
				data = str2numeric< decltype( data + 0 ) >( params );

				return true;
			}

			return false;
		}

		datetime adjust_cycle_next( datetime start, xgc_uint16 type, xgc_uint64 data, datetime current )
		{
			datetime now = current;

			switch( type )
			{
				case cycle_mode::e_week:
				{
					datetime set = start;
					if( set <= now )
						set += timespan::from_days( ( now - set ).to_days() + 1 );

					xgc_uint16 dayofweek = set.to_systime().dayofweak % 7;
					for( xgc_uint16 cur = 0; cur < 7; ++cur )
					{
						if( 0 == XGC_CHKBIT( data, ( dayofweek + cur ) % 7 ) )
							continue;

						set += timespan::from_days( cur );
						break;
					}

					start = set;
				}
				break;
				case cycle_mode::e_month:
				{
					datetime set = start;
					if( set <= now )
						set += timespan::from_days( ( now - set ).to_days() + 1 );

					xgc_uint16 day = set.to_systime().day;
					for( xgc_uint16 cur = 0; cur < 31; ++cur )
					{
						if( 0 == XGC_CHKBIT( data, ( day - 1 + cur ) % 31 + 1 ) )
							continue;

						set += timespan::from_days( cur );
						break;
					}

					start = set;
				}
				break;
				case cycle_mode::e_repeat:
				{
					datetime set = start;
					if( set <= now )
					{
						if( data )
							set += timespan::from_milliseconds( ( ( now - set ).to_millisecnods() + data ) / data * data );
						else
							set = now;
					}
					start = set;
				}
				break;
			}

			return start;
		}

		datetime adjust_cycle_prev( datetime start, xgc_uint16 type, xgc_uint64 data, datetime current )
		{
			datetime now = current;

			switch( type )
			{
				case cycle_mode::e_week:
				{
					datetime set = start;
					if( set <= now )
						set += timespan::from_days( ( now - set ).to_days() );

					xgc_uint16 dayofweek = set.to_systime().dayofweak % 7;
					for( xgc_uint16 cur = 0; cur < 7; ++cur )
					{
						if( 0 == XGC_CHKBIT( data, 6 - ( dayofweek + cur ) % 7 ) )
							continue;

						set -= timespan::from_days( cur );
						break;
					}

					start = set;
				}
				break;
				case cycle_mode::e_month:
				{
					datetime set = start;
					if( set <= now )
						set += timespan::from_days( ( now - set ).to_days() );

					xgc_uint16 day = set.to_systime().day;
					for( xgc_uint16 cur = 0; cur < 31; ++cur )
					{
						if( 0 == XGC_CHKBIT( data, ( day + 30 - cur ) % 31 + 1 ) )
							continue;

						set -= timespan::from_days( cur );
						break;
					}

					start = set;
				}
				break;
				case cycle_mode::e_repeat:
				{
					datetime set = start;
					if( set <= now )
					{
						if( data )
							set += timespan::from_milliseconds( ( ( now - set ).to_millisecnods() + data ) / data * data );
						else
							set = now;
					}

					start = set;
				}
				break;
			}

			return start;
		}

		///
		/// 重新校时
		/// @return 返回校准过的时间，该时间通过clock的类型和参数校准
		/// [1/21/2015] create by albert.xu
		///
		datetime adjust_cycle_next( datetime start, xgc_lpcstr args, datetime current )
		{
			xgc_uint16 type = 0;
			xgc_uint64 data = 0;
			if( cycle_params_parse( args, type, data ) )
			{
				return adjust_cycle_next( start, type, data, current );
			}

			return datetime::from_ftime( 0 );
		}

		///
		/// 重新校时，取当前时间的上一次更新
		/// @param deadline 结束时间
		/// @param args 事件参数
		/// @param current 时间基准点
		/// @return 返回校准过的时间，该时间通过clock的类型和参数校准
		/// [1/21/2015] create by albert.xu
		///
		datetime adjust_cycle_prev( datetime start, xgc_lpcstr args, datetime current )
		{
			xgc_uint16 type;
			xgc_uint64 data = 0;
			if( cycle_params_parse( args, type, data ) )
			{
				return adjust_cycle_prev( start, type, data, current );
			}

			return datetime::from_ftime( 0 );
		}
	}
}