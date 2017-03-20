#ifndef _DATETIME_H_
#define _DATETIME_H_

#pragma once

#include "defines.h"
#include "exports.h"
#include "xutility.h"

#include <time.h>
#include <chrono>

namespace xgc
{
	namespace common
	{
		const uint64_t _Time_T_Diff = 116444736000000000ULL;
		struct COMMON_API systime
		{
			uint16_t year;
			uint16_t month;
			uint16_t dayofweak;
			uint16_t day;
			uint16_t hour;
			uint16_t minute;
			uint16_t seconds;
			uint16_t milliseconds;
		};

		union COMMON_API filetime
		{
			volatile uint64_t dt;
			
			struct
			{
				volatile uint32_t lo;
				volatile uint32_t hi;
			};
		};

		#ifdef __GNUC__
		XGC_INLINE error_t localtime_s( struct tm *tt, const time_t *t )
		{
			auto ptt = localtime( t );
			if( tt && ptt )
			{
				*tt = *ptt;
				return 0;
			}

			return -1;
		}

		XGC_INLINE error_t gmtime_s( struct tm *tt, const time_t *t )
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

		struct timespan
		{
			volatile int64_t span;

			timespan()
				: span( 0 )
			{
			}

			explicit timespan( int64_t _span )
				: span( _span )
			{
			}

			timespan( const timespan& _timespan )
				: span( _timespan.span )
			{
			}

			timespan& operator = ( const timespan& _timespan )
			{
				span = _timespan.span;
				return *this;
			}

			timespan& operator = ( int64_t _span )
			{
				span = _span;
				return *this;
			}

			xgc_bool operator < ( const timespan& _timespan )const
			{
				return span < _timespan.span;
			}

			xgc_bool operator <= ( const timespan& _timespan )const
			{
				return span <= _timespan.span;
			}

			xgc_bool operator >( const timespan& _timespan )const
			{
				return span > _timespan.span;
			}

			xgc_bool operator >= ( const timespan& _timespan )const
			{
				return span >= _timespan.span;
			}

			xgc_bool operator == ( const timespan& _timespan )const
			{
				return span == _timespan.span;
			}

			xgc_bool operator != ( const timespan& _timespan )const
			{
				return span != _timespan.span;
			}

			timespan& operator += ( const timespan& _timespan )
			{
				span += _timespan.span;
				return *this;
			}

			timespan& operator -= ( const timespan& _timespan )
			{
				span -= _timespan.span;
				return *this;
			}

			timespan& operator *= ( double scale )
			{
				span = int64_t( span * scale );
				return *this;
			}

			timespan& operator /= ( double scale )
			{
				span = int64_t( span / scale );
				return *this;
			}

			timespan& operator *= ( int64_t scale )
			{
				span *= scale;
				return *this;
			}

			timespan& operator /= ( int64_t scale )
			{
				span /= scale;
				return *this;
			}

			///
			/// 按秒初始化一个timespan对象
			/// [8/8/2014] create by albert.xu
			///
			static timespan from_days( int64_t days )
			{ 
				return timespan( days * ( 24 * 60 * 60 * 10000000ULL ) );
			}

			///
			/// 按秒初始化一个timespan对象
			/// [8/8/2014] create by albert.xu
			///
			static timespan from_hour( int64_t hour )
			{ 
				return timespan( hour * ( 60 * 60 * 10000000ULL ) );
			}			

			///
			/// 按秒初始化一个timespan对象
			/// [8/8/2014] create by albert.xu
			///
			static timespan from_minutes( int64_t minutes )
			{ 
				return timespan( minutes * ( 60 * 10000000ULL ) );
			}

			///
			/// 按秒初始化一个timespan对象
			/// [8/8/2014] create by albert.xu
			///
			static timespan from_seconds( int64_t seconds )
			{
				return timespan( seconds * 10000000ULL );
			}

			///
			/// 按秒初始化一个timespan对象
			/// [8/8/2014] create by albert.xu
			///
			static timespan from_milliseconds( int64_t milliseconds )
			{
				return timespan( milliseconds * 10000ULL );
			}

			///
			/// 将字符串时间转换为时间片
			/// [7/23/2014] create by albert.xu
			///
			static timespan convert( xgc_lpcstr str )
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

			// 时间片段转为单位时间
			xgc_int32 to_days()const 
			{ 
				return xgc_int32( span / ( 24 * 60 * 60 * 10000000ULL ) ); 
			}

			xgc_int32 to_hour()const 
			{ 
				return xgc_int32( span / ( 60 * 60 * 10000000ULL ) ); 
			}

			xgc_int32 to_minutes()const 
			{ 
				return xgc_int32( span / ( 60 * 10000000ULL ) ); 
			}

			xgc_int32 to_seconds()const 
			{ 
				return xgc_int32( span / 10000000ULL ); 
			}

			int64_t to_millisecnods()const 
			{ 
				return int64_t( span / 10000ULL );
			}
		};

		struct datetime
		{
			filetime storage; // is mill seconds

			///
			/// [1/7/2014 jianglei.kinly]
			/// \brief 当前时间构造
			///
			datetime()
				: storage( { _Time_T_Diff } )
			{
			}

			datetime( filetime ft )
				: storage( ft )
			{
			}

			datetime( systime st )
			{
				#ifdef _WINDOWS
					BOOL bRet = 0;
					bRet = SystemTimeToFileTime( (LPSYSTEMTIME) &st, (LPFILETIME) &storage );
					XGC_ASSERT_MESSAGE( bRet, "System Error Code = %d", GetLastError() );
					bRet = LocalFileTimeToFileTime( (LPFILETIME) &storage, (LPFILETIME) &storage );
					XGC_ASSERT_MESSAGE( bRet, "System Error Code = %d", GetLastError() );
				#elif defined( _LINUX )
					struct tm gtime = {
						st.seconds,
						st.minute,
						st.hour,
						st.day,
						st.month - 1,
						st.year - 1900,
						st.dayofweak,
						0,
						0
					};

					storage.dt = (uint64_t) mktime( &gtime ) * 10000000ULL + st.milliseconds * 10000ULL + _Time_T_Diff;
				#endif
			}

			///
			/// [1/7/2014 jianglei.kinly]
			/// \brief 拷贝构造
			///
			datetime( const datetime& _datetime )
				: storage( _datetime.storage )
			{
			}

			datetime& operator = ( const datetime& _datetime )
			{
				storage = _datetime.storage;
				return *this;
			}

			///
			/// 获取当前时间值
			/// [7/23/2014] create by albert.xu
			///
			datetime time()const
			{
				filetime ft;
				ft.dt = storage.dt % ( 24 * 60 * 60 * 10000000ULL ); 
				return datetime( ft );
			}

			///
			/// 获取当前日期值
			/// [7/23/2014] create by albert.xu
			///
			datetime date()const
			{
				filetime ft;
				ft.dt = storage.dt / ( 24 * 60 * 60 * 10000000ULL ) * ( 24 * 60 * 60 * 10000000ULL ); 
				return datetime(ft);
			}

			///
			/// 设置日期， 不改变时间
			/// [7/23/2014] create by albert.xu
			///
			xgc_void set_date( datetime dt )
			{
				storage.dt = time().to_ftime() + dt.date().to_ftime();
			}

			///
			/// 设置时间， 不改变日期
			/// [7/23/2014] create by albert.xu
			///
			xgc_void set_time( datetime dt )
			{
				storage.dt = date().to_ftime() + dt.time().to_ftime();
			}

			xgc_time64 to_ctime() const
			{
				if( storage.dt >= _Time_T_Diff )
					return ( storage.dt - _Time_T_Diff ) / 10000000ULL;
				
				return 0;
			}

			xgc_time64 to_ftime() const
			{
				return storage.dt;
			}

			xgc_time64 to_milliseconds() const
			{
				if( storage.dt >= _Time_T_Diff )
					return ( storage.dt - _Time_T_Diff ) / 10000ULL;

				return 0;
			}

			xgc_time64 to_microseconds() const
			{
				if( storage.dt >= _Time_T_Diff )
					return ( storage.dt - _Time_T_Diff ) / 10ULL;

				return 0;
			}

			static datetime from_ctime( xgc_time64 ctime )
			{
				filetime ft = { ctime * 10000000ULL + _Time_T_Diff };
				return datetime( ft );
			}

			static datetime from_ftime( xgc_time64 ftime )
			{
				filetime ft;
				ft.dt = ftime;
				return datetime( ft );
			}

			static datetime from_seconds( xgc_real64 seconds )
			{
				filetime ft = { xgc_time64( seconds * 10000000ULL ) + _Time_T_Diff };
				return datetime( ft );
			}

			static datetime from_milliseconds( uint64_t millseconds )
			{
				filetime ft = { millseconds * 10000ULL + _Time_T_Diff };
				return datetime( ft );
			}

			//////////////////////////////////////////////////////////////////////////
			// [1/7/2014 jianglei.kinly]
			// 操作符 = 重载
			//////////////////////////////////////////////////////////////////////////
			XGC_INLINE xgc_bool operator == ( const datetime& rhs )const
			{
				return storage.dt == rhs.storage.dt;
			}

			XGC_INLINE xgc_bool operator != ( const datetime& rhs )const
			{
				return !( *this == rhs );
			}

			XGC_INLINE xgc_bool operator < ( const datetime& rhs )const
			{
				return storage.dt < rhs.storage.dt;
			}

			XGC_INLINE xgc_bool operator <= ( const datetime& rhs )const
			{
				return !( *this > rhs );
			}

			XGC_INLINE xgc_bool operator >( const datetime& rhs )const
			{
				return storage.dt > rhs.storage.dt;
			}

			XGC_INLINE xgc_bool operator >= ( const datetime& rhs )const
			{
				return !( *this < rhs );
			}

			///
			/// 时间 + 时间片
			/// [7/24/2014] create by albert.xu
			///
			XGC_INLINE datetime& operator += ( const timespan& rhs )
			{
				storage.dt += rhs.span;
				return *this;
			}

			///
			/// 时间 + 时间片
			/// [7/24/2014] create by albert.xu
			///
			XGC_INLINE datetime& operator -= ( const timespan& rhs )
			{
				storage.dt -= rhs.span;
				return *this;
			}

			///
			/// 取当前时间
			/// [6/22/2014] create by albert.xu
			///
			XGC_INLINE systime to_systime() const
			{
				time_t t = to_ctime();

				tm gtime = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
				
				if( 0 != localtime_s( &gtime, &t ) )
					return { 0,0,0,0,0,0,0,0 };

				return { 
					uint16_t(gtime.tm_year + 1900),
					uint16_t(gtime.tm_mon + 1),
					uint16_t(gtime.tm_wday), 
					uint16_t(gtime.tm_mday), 
					uint16_t(gtime.tm_hour), 
					uint16_t(gtime.tm_min), 
					uint16_t(gtime.tm_sec), 
					uint16_t(to_milliseconds() - t * 1000)
				};
			}

			///
			/// 取当前时间
			/// [6/22/2014] create by albert.xu
			///
			XGC_INLINE systime to_utctime() const
			{
				time_t t = to_ctime();

				tm gtime = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

				if( 0 != gmtime_s( &gtime, &t ) )
					return { 0,0,0,0,0,0,0,0 };
				
				return {
					uint16_t(gtime.tm_year + 1900),
					uint16_t(gtime.tm_mon + 1),
					uint16_t(gtime.tm_wday), 
					uint16_t(gtime.tm_mday), 
					uint16_t(gtime.tm_hour), 
					uint16_t(gtime.tm_min), 
					uint16_t(gtime.tm_sec), 
					uint16_t(to_milliseconds() - t * 1000)
				};
			}

			///
			/// 转换为字符串
			/// [7/15/2015] create by albert.xu
			///
			xgc_lpcstr to_string( xgc_lpstr output, xgc_size size, xgc_lpcstr format = "%Y-%m-%d %H:%M:%S" ) const
			{
				tm stm;
				time_t tt = (time_t)to_ctime();
				localtime_s( &stm, &tt );
				strftime( output, size, format, &stm );

				return output;
			}

			///
			/// 转换为字符串 - template
			/// [7/15/2015] create by albert.xu
			///
			template< size_t S >
			xgc_lpcstr to_string( xgc_char (&output)[S], xgc_lpcstr format = "%Y-%m-%d %H:%M:%S" ) const
			{
				tm stm;
				time_t tt = (time_t) to_ctime();
				localtime_s( &stm, &tt );
				strftime( output, S, format, &stm );

				return output;
			}

			//////////////////////////////////////////////////////////////////////////
			// [1/7/2014 jianglei.kinly]
			// 获取当前时间的datetime对象
			//////////////////////////////////////////////////////////////////////////
			static datetime now();

			///
			/// 获取当前时间字符串
			/// [7/15/2015] create by albert.xu
			///
			static xgc_lpcstr now( xgc_lpstr output, xgc_size size, xgc_lpcstr format = "%Y-%m-%d %H:%M:%S" );

			///
			/// 获取当前时间字符串 - template 
			/// [7/15/2015] create by albert.xu
			///
			template< size_t S >
			static xgc_lpcstr now( xgc_char (&output)[S], xgc_lpcstr format = "%Y-%m-%d %H:%M:%S" );

			///
			/// 获取当前时间的相对时间
			/// [8/8/2014] create by albert.xu
			///
			static datetime relative_time( timespan span );

			///
			/// 将字符串转化为日期时间
			/// [7/23/2014] create by albert.xu
			/// @param str 日期时间格式的字符串。格式匹配必须右对齐，YYYY-MM-DD hh::mm::ss.ms
			///
			static datetime convert( xgc_lpcstr str, datetime now = datetime::now() )
			{
				systime st = now.to_systime();

				if (str)
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
							*ptr[idt++] = (uint16_t) strtoul( cursor + 1, xgc_nullptr, 10 );
							break;
							case ' ':
							idt = 3;
							case ':':
							idt = XGC_RNG( idt, 1, 3 );
							*ptr[idt++] = (uint16_t) strtoul( cursor + 1, xgc_nullptr, 10 );
							break;
							case '-':
							idt = XGC_RNG( idt, 4, 6 );
							*ptr[idt++] = (uint16_t) strtoul( cursor + 1, xgc_nullptr, 10 );
							break;
						}

						--cursor;
					}

					if( idt < XGC_COUNTOF( ptr ) )
					{
						*ptr[idt] = (uint16_t) strtoul( cursor, xgc_nullptr, 10 );
					}
				}
				return datetime( st );
			}
		};

		XGC_INLINE datetime operator + ( const datetime& dt, const timespan& sp )
		{
			filetime ft = { dt.storage.dt + sp.span };
			return datetime( ft );
		}

		XGC_INLINE datetime operator - ( const datetime& dt, const timespan& sp )
		{
			filetime ft = { dt.storage.dt - sp.span };
			return datetime( ft );
		}

		XGC_INLINE timespan operator - ( const datetime& lhs, const datetime& rhs )
		{
			return timespan( lhs.storage.dt - rhs.storage.dt );
		}

		XGC_INLINE timespan operator * ( const timespan& lhs, xgc_real64 rhs )
		{
			return timespan( xgc_time64( lhs.span * rhs ) );
		}

		XGC_INLINE timespan operator / ( const timespan& lhs, xgc_real64 rhs )
		{
			return timespan( xgc_time64( lhs.span / rhs ) );
		}

		XGC_INLINE timespan operator * ( const timespan& lhs, xgc_int32 rhs )
		{
			return timespan( xgc_time64( lhs.span * rhs ) );
		}

		XGC_INLINE timespan operator / ( const timespan& lhs, xgc_int32 rhs )
		{
			return timespan( xgc_time64( lhs.span / rhs ) );
		}

		XGC_INLINE datetime datetime::now()
		{
			filetime ft;
			#if defined(_WINDOWS)
			GetSystemTimeAsFileTime( (LPFILETIME) &ft );
			#elif defined(_LINUX)
			struct timeval v;
			gettimeofday( &v, xgc_nullptr );
			ft.dt = v.tv_sec * 10000000ULL + v.tv_usec * 10ULL + _Time_T_Diff;
			#endif

			return ft;
		}

		XGC_INLINE xgc_lpcstr datetime::now( xgc_lpstr output, xgc_size size, xgc_lpcstr format )
		{
			tm stm;
			time_t tt = (time_t) now().to_ctime();
			localtime_s( &stm, &tt );
			strftime( output, size, format, &stm );

			return output;
		}

		template< size_t S >
		XGC_INLINE xgc_lpcstr datetime::now( xgc_char (&output)[S], xgc_lpcstr format )
		{
			tm stm;
			time_t tt = (time_t) now().to_ctime();
			localtime_s( &stm, &tt );
			strftime( output, S, format, &stm );

			return output;
		}

		XGC_INLINE datetime datetime::relative_time( timespan span )
		{
			return datetime::now() + span;
		}

		XGC_INLINE xgc_time64 current_time()
		{
			return datetime::now().to_ctime();
		}

		XGC_INLINE xgc_time64 current_millisecond()
		{
			return datetime::now().to_milliseconds();
		}

		XGC_INLINE xgc_time64 current_microsecond()
		{
			return datetime::now().to_microseconds();
		}

		template< class _Duration >
		xgc_time64 ticks()
		{
			return std::chrono::duration_cast<_Duration>(
				std::chrono::steady_clock::now().time_since_epoch()
			).count();
		}
	}  // end namespace common
}  // end namespace xgc

#endif // _DATETIME_H_