#ifndef _DATETIME_H_
#define _DATETIME_H_

#pragma once

#include "exports.h"

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
		error_t localtime_s( struct tm *tt, const time_t *t );
		error_t gmtime_s( struct tm *tt, const time_t *t );
		#endif

		struct COMMON_API timespan
		{
			volatile int64_t span;

			timespan()
				: span( 0 )
			{
			}

			timespan( int64_t _span )
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

			bool operator < ( const timespan& _timespan )const
			{
				return span < _timespan.span;
			}

			bool operator <= ( const timespan& _timespan )const
			{
				return span <= _timespan.span;
			}

			bool operator >( const timespan& _timespan )const
			{
				return span > _timespan.span;
			}

			bool operator >= ( const timespan& _timespan )const
			{
				return span >= _timespan.span;
			}

			bool operator == ( const timespan& _timespan )const
			{
				return span == _timespan.span;
			}

			bool operator != ( const timespan& _timespan )const
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
			static timespan from_days( xgc_real64 days )
			{ 
				return (int64_t)( days * ( 24 * 60 * 60 * 10000000ULL ) );
			}

			///
			/// 按秒初始化一个timespan对象
			/// [8/8/2014] create by albert.xu
			///
			static timespan from_hour( xgc_real64 hour )
			{ 
				return (int64_t)( hour * ( 60 * 60 * 10000000ULL ) );
			}

			///
			/// 按秒初始化一个timespan对象
			/// [8/8/2014] create by albert.xu
			///
			static timespan from_minutes( xgc_real64 minutes )
			{ 
				return (int64_t)( minutes * ( 60 * 10000000ULL ) );
			}

			///
			/// 按秒初始化一个timespan对象
			/// [8/8/2014] create by albert.xu
			///
			static timespan from_seconds( xgc_real64 seconds )
			{
				return (int64_t)( seconds * 10000000ULL );
			}

			///
			/// 按秒初始化一个timespan对象
			/// [8/8/2014] create by albert.xu
			///
			static timespan from_milliseconds( xgc_uint64 milliseconds )
			{
				return milliseconds * 10000ULL;
			}

			///
			/// 将字符串时间转换为时间片
			/// [7/23/2014] create by albert.xu
			///
			static timespan convert( xgc_lpcstr str );

			// 时间片段转为单位时间
			xgc_real64 to_days()const 
			{ 
				return span / ( 24 * 60 * 60 * 10000000. );
			}

			xgc_real64 to_hour()const
			{ 
				return span / ( 60 * 60 * 10000000. ); 
			}

			xgc_real64 to_minutes()const
			{ 
				return span / ( 60 * 10000000. ); 
			}

			xgc_real64 to_seconds()const
			{ 
				return span / 10000000.; 
			}

			xgc_uint64 to_millisecnods()const
			{ 
				return span / 10000;
			}
		};

		struct COMMON_API datetime
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
					SystemTimeToFileTime( (LPSYSTEMTIME) &st, (LPFILETIME) &storage );
					LocalFileTimeToFileTime( (LPFILETIME) &storage, (LPFILETIME) &storage );
				#endif

				#ifdef _LINUX
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

			xgc_time64 to_nonaseconds() const
			{
				if( storage.dt >= _Time_T_Diff )
					return storage.dt - _Time_T_Diff;

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
			systime to_systime() const;

			///
			/// 取当前时间
			/// [6/22/2014] create by albert.xu
			///
			systime to_utctime() const;

			///
			/// 转换为字符串
			/// [7/15/2015] create by albert.xu
			///
			xgc_lpcstr to_string( xgc_lpstr output, xgc_size size, xgc_lpcstr format = "%Y-%m-%d %H:%M:%S" ) const;

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
			static xgc_size now( xgc_lpstr output, xgc_size size, xgc_lpcstr format = "%Y-%m-%d %H:%M:%S" );

			///
			/// 获取当前时间字符串 - template 
			/// [7/15/2015] create by albert.xu
			///
			template< size_t S >
			static xgc_size now( xgc_char (&output)[S], xgc_lpcstr format = "%Y-%m-%d %H:%M:%S" )
			{
				return now( output, S, format );
			}

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
			static datetime convert( xgc_lpcstr str, datetime now = datetime::now() );
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

		XGC_INLINE timespan operator + ( const timespan& lhs, const timespan& rhs )
		{
			return timespan( lhs.span + rhs.span );
		}

		XGC_INLINE timespan operator - ( const timespan& lhs, const timespan& rhs )
		{
			return timespan( lhs.span - rhs.span );
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

		XGC_INLINE xgc_time64 current_time()
		{
			return datetime::now().to_ctime();
		}

		XGC_INLINE xgc_time64 current_milliseconds()
		{
			return datetime::now().to_milliseconds();
		}

		XGC_INLINE xgc_time64 current_microseconds()
		{
			return datetime::now().to_microseconds();
		}

		XGC_INLINE xgc_time64 current_nonaseconds()
		{
			return datetime::now().to_nonaseconds();
		}

		template< class _Duration >
		XGC_INLINE xgc_time64 ticks()
		{
			return std::chrono::duration_cast<_Duration>(
				std::chrono::steady_clock::now().time_since_epoch()
				).count();
		}

		///
		/// \brief 分析周期参数
		/// \param [in] args 周期参数
		/// \param [out] type 周期类型
		/// \param [out] data 周期参数
		/// \param [in] duration 持续时间
		/// \return 返回校准过的时间，该时间通过clock的类型和参数校准
		/// \date 1/21/2015
		/// \author albert.xu
		///
		xgc_bool COMMON_API cycle_params_parse( xgc_lpcstr args, xgc_uint16 & type, xgc_uint64 & data );

		///
		/// \brief 重新校时
		/// \param start 周期起始时间
		/// \param type 周期类型
		/// \param data 周期参数
		/// \param current 时间基准点
		/// \return 返回校准过的时间，该时间通过clock的类型和参数校准
		/// \date 1/21/2015
		/// \author albert.xu
		///
		datetime COMMON_API adjust_cycle_next( datetime start, xgc_uint16 type, xgc_uint64 data, datetime current );

		///
		/// \brief 重新校时，取当前时间的上一次更新
		/// \param start 周期起始时间
		/// \param type 周期类型
		/// \param data 周期参数
		/// \param current 时间基准点
		/// \return 返回校准过的时间，该时间通过clock的类型和参数校准
		/// \date 1/21/2015
		/// \author albert.xu
		///
		datetime COMMON_API adjust_cycle_prev( datetime start, xgc_uint16 type, xgc_uint64 data, datetime current );

		///
		/// \brief 重新校时
		/// \param start 周期起始时间
		/// \param args 事件参数
		/// \param current 时间基准点
		/// \return 返回校准过的时间，该时间通过clock的类型和参数校准
		/// \date 1/21/2015
		/// \author albert.xu
		///
		datetime COMMON_API adjust_cycle_next( datetime deadline, xgc_lpcstr args, datetime current );

		///
		/// \brief 重新校时，取当前时间的上一次更新
		/// \param start 周期起始时间
		/// \param args 事件参数
		/// \param current 时间基准点
		/// \return 返回校准过的时间，该时间通过clock的类型和参数校准
		/// \date 1/21/2015
		/// \author albert.xu
		///
		datetime COMMON_API adjust_cycle_prev( datetime deadline, xgc_lpcstr args, datetime current );

	}  // end namespace common
}  // end namespace xgc

#endif // _DATETIME_H_