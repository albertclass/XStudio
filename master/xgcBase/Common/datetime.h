#pragma once
#ifndef __COMMON_DATETIME_H_
#define __COMMON_DATETIME_H_

#include <time.h>

#include "defines.h"
#include "exports.h"

namespace XGC
{
	namespace common
	{
		const xgc_uint64 _Time_T_Diff = 116444736000000000i64;
		struct COMMON_API systime
		{
			xgc_uint16 year;
			xgc_uint16 month;
			xgc_uint16 dayofweak;
			xgc_uint16 day;
			xgc_uint16 hour;
			xgc_uint16 minute;
			xgc_uint16 seconds;
			xgc_uint16 milliseconds;
		};

		union COMMON_API filetime
		{
			volatile xgc_uint64 qwDateTime;
			
			struct _
			{
				volatile xgc_uint32 loDateTime;
				volatile xgc_uint32 hiDateTime;
			};
		};

		struct timespan
		{
			volatile xgc_int64 span;

			timespan()
				: span( 0 )
			{
			}

			explicit timespan( xgc_int64 _span )
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

			timespan& operator *= ( xgc_real64 scale )
			{
				span = xgc_int64( span * scale );
				return *this;
			}

			timespan& operator /= ( xgc_real64 scale )
			{
				span = xgc_int64( span / scale );
				return *this;
			}

			timespan& operator *= ( xgc_int32 scale )
			{
				span *= scale;
				return *this;
			}

			timespan& operator /= ( xgc_int32 scale )
			{
				span /= scale;
				return *this;
			}

			///
			/// �����ʼ��һ��timespan����
			/// [8/8/2014] create by albert.xu
			///
			static timespan from_days( xgc_int32 days )
			{ 
				return timespan( days * ( 24 * 60 * 60 * 10000000ULL ) );
			}
			///
			/// �����ʼ��һ��timespan����
			/// [8/8/2014] create by albert.xu
			///
			static timespan from_hour( xgc_int32 hour )
			{ 
				return timespan( hour * ( 60 * 60 * 10000000ULL ) );
			}			
			///
			/// �����ʼ��һ��timespan����
			/// [8/8/2014] create by albert.xu
			///
			static timespan from_minutes( xgc_int32 minutes )
			{ 
				return timespan( minutes * ( 60 * 10000000ULL ) );
			}

			///
			/// �����ʼ��һ��timespan����
			/// [8/8/2014] create by albert.xu
			///
			static timespan from_seconds( xgc_int32 seconds )
			{
				return timespan( seconds * 10000000ULL );
			}

			///
			/// �����ʼ��һ��timespan����
			/// [8/8/2014] create by albert.xu
			///
			static timespan from_millseconds( xgc_int64 seconds )
			{
				return timespan( seconds * 10000ULL );
			}

			///
			/// ���ַ���ʱ��ת��Ϊʱ��Ƭ
			/// [7/23/2014] create by albert.xu
			///
			static timespan convert( xgc_lpcstr str )
			{
				xgc_uint16 time[] = { 0, 0, 0, 0 };
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
						time[it++] = (xgc_uint16) strtoul( cursor + 1, xgc_nullptr, 10 );
						break;
						case ' ':
						it = 3;
						case ':':
						it = XGC_RNG( it, 1, 3 );
						time[it++] = (xgc_uint16) strtoul( cursor + 1, xgc_nullptr, 10 );
						break;
					}

					--cursor;
				}

				if( it < xgc_countof( time ) )
				{
					time[it] = (xgc_uint16) strtoul( cursor, xgc_nullptr, 10 );
				}

				return timespan( xgc_int64( ( ( ( ( time[3] * 60 + time[2] ) * 60 ) + time[1] ) * 1000 + time[0] ) * 10000ULL ) );
			}

			// ʱ��Ƭ��תΪ��λʱ��
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

			xgc_int64 to_millsecnods()const 
			{ 
				return xgc_int64( span / 10000ULL );
			}
		};

		struct datetime
		{
			filetime mTime; // is mill seconds

			//////////////////////////////////////////////////////////////////////////
			// [1/7/2014 jianglei.kinly]
			// ��ǰʱ�乹��
			//////////////////////////////////////////////////////////////////////////
			datetime()
				: mTime( { _Time_T_Diff } )
			{
			}

			datetime( filetime ft )
				: mTime( ft )
			{
			}

			datetime( systime st )
			{
				BOOL bRet = 0;
				bRet = SystemTimeToFileTime( (LPSYSTEMTIME) &st, (LPFILETIME) &mTime );
				XGC_ASSERT_MESSAGE( bRet, "System Error Code = %d", GetLastError() );
				bRet = LocalFileTimeToFileTime( (LPFILETIME) &mTime, (LPFILETIME) &mTime );
				XGC_ASSERT_MESSAGE( bRet, "System Error Code = %d", GetLastError() );
			}

			//////////////////////////////////////////////////////////////////////////
			// [1/7/2014 jianglei.kinly]
			// ��������
			//////////////////////////////////////////////////////////////////////////
			datetime( const datetime& _datetime )
				: mTime( _datetime.mTime )
			{
			}

			datetime& operator = ( const datetime& _datetime )
			{
				mTime = _datetime.mTime;
				return *this;
			}

			///
			/// ��ȡ��ǰʱ��ֵ
			/// [7/23/2014] create by albert.xu
			///
			xgc_time64 time()
			{
				return mTime.qwDateTime % ( 24 * 60 * 60 * 10000000ULL );
			}

			///
			/// ��ȡ��ǰ����ֵ
			/// [7/23/2014] create by albert.xu
			///
			xgc_time64 date()
			{
				return mTime.qwDateTime / ( 24 * 60 * 60 * 10000000ULL ) * ( 24 * 60 * 60 * 10000000ULL );
			}

			///
			/// �������ڣ� ���ı�ʱ��
			/// [7/23/2014] create by albert.xu
			///
			xgc_void setdate( datetime dt )
			{
				mTime.qwDateTime = time() + dt.date();
			}

			///
			/// ����ʱ�䣬 ���ı�����
			/// [7/23/2014] create by albert.xu
			///
			xgc_void settime( datetime dt )
			{
				mTime.qwDateTime = date() + dt.time();
			}

			xgc_time64 to_ctime() const
			{
				if( mTime.qwDateTime >= _Time_T_Diff )
					return ( mTime.qwDateTime - _Time_T_Diff ) / 10000000ULL;
				
				return 0;
			}

			xgc_time64 to_ftime() const
			{
				return mTime.qwDateTime;
			}

			xgc_time64 to_millisecond() const
			{
				if( mTime.qwDateTime >= _Time_T_Diff )
					return ( mTime.qwDateTime - _Time_T_Diff ) / 10000ULL;

				return 0;
			}

			xgc_time64 to_microsecond() const
			{
				if( mTime.qwDateTime >= _Time_T_Diff )
					return ( mTime.qwDateTime - _Time_T_Diff ) / 10ULL;

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
				ft.qwDateTime = ftime;
				return datetime( ft );
			}

			static datetime from_millseconds( xgc_uint64 millseconds )
			{
				filetime ft = { millseconds * 10000ULL + _Time_T_Diff };
				return datetime( ft );
			}

			//////////////////////////////////////////////////////////////////////////
			// [1/7/2014 jianglei.kinly]
			// ������ = ����
			//////////////////////////////////////////////////////////////////////////
			XGC_INLINE xgc_bool operator == ( const datetime& rhs )const
			{
				return CompareFileTime( (LPFILETIME) &mTime, (LPFILETIME) &rhs.mTime ) == 0;
			}

			XGC_INLINE xgc_bool operator != ( const datetime& rhs )const
			{
				return !( *this == rhs );
			}

			XGC_INLINE xgc_bool operator < ( const datetime& rhs )const
			{
				return CompareFileTime( (LPFILETIME) &mTime, (LPFILETIME) &rhs.mTime ) < 0;
			}

			XGC_INLINE xgc_bool operator <= ( const datetime& rhs )const
			{
				return !( *this > rhs );
			}

			XGC_INLINE xgc_bool operator >( const datetime& rhs )const
			{
				return CompareFileTime( (LPFILETIME) &mTime, (LPFILETIME) &rhs.mTime ) > 0;
			}

			XGC_INLINE xgc_bool operator >= ( const datetime& rhs )const
			{
				return !( *this < rhs );
			}

			///
			/// ʱ�� + ʱ��Ƭ
			/// [7/24/2014] create by albert.xu
			///
			XGC_INLINE datetime& operator += ( const timespan& rhs )
			{
				mTime.qwDateTime += rhs.span;
				return *this;
			}

			///
			/// ʱ�� + ʱ��Ƭ
			/// [7/24/2014] create by albert.xu
			///
			XGC_INLINE datetime& operator -= ( const timespan& rhs )
			{
				mTime.qwDateTime -= rhs.span;
				return *this;
			}

			///
			/// ȡ��ǰʱ��
			/// [6/22/2014] create by albert.xu
			///
			XGC_INLINE systime to_systime() const
			{
				systime st = { 0, 0, 0, 0, 0, 0, 0, 0 };
				filetime ft = { 0 };
				FileTimeToLocalFileTime( (LPFILETIME) &mTime, (LPFILETIME) &ft );
				FileTimeToSystemTime( (LPFILETIME) &ft, (LPSYSTEMTIME) &st );

				return st;
			}

			///
			/// ȡ��ǰʱ��
			/// [6/22/2014] create by albert.xu
			///
			XGC_INLINE systime to_utctime() const
			{
				systime st = { 0, 0, 0, 0, 0, 0, 0, 0 };
				filetime ft = { 0 };
				FileTimeToSystemTime( (LPFILETIME) &ft, (LPSYSTEMTIME) &st );

				return st;
			}

			///
			/// ת��Ϊ�ַ���
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
			/// ת��Ϊ�ַ��� - template
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
			// ��ȡ��ǰʱ���datetime����
			//////////////////////////////////////////////////////////////////////////
			static datetime current_time();

			///
			/// ��ȡ��ǰʱ���ַ���
			/// [7/15/2015] create by albert.xu
			///
			static xgc_lpcstr current_time( xgc_lpstr output, xgc_size size, xgc_lpcstr format = "%Y-%m-%d %H:%M:%S" );

			///
			/// ��ȡ��ǰʱ���ַ��� - template 
			/// [7/15/2015] create by albert.xu
			///
			template< size_t S >
			static xgc_lpcstr current_time( xgc_char (&output)[S], xgc_lpcstr format = "%Y-%m-%d %H:%M:%S" );

			///
			/// ��ȡ��ǰʱ������ʱ��
			/// [8/8/2014] create by albert.xu
			///
			static datetime relative_time( timespan span );

			///
			/// ���ַ���ת��Ϊ����ʱ��
			/// [7/23/2014] create by albert.xu
			/// @param str ����ʱ���ʽ���ַ�������ʽƥ������Ҷ��룬YYYY-MM-DD hh::mm::ss.ms
			///
			static datetime convert( xgc_lpcstr str, xgc_time64 sec = 0 )
			{
				systime st = datetime::from_ctime( sec ).to_systime();

				if (str)
				{
					xgc_lpcstr cursor = str + strlen( str );
					xgc_lpcstr splitc = "-:. ";

					xgc_uint16 *ptr[] = { &st.milliseconds, &st.seconds, &st.minute, &st.hour, &st.day, &st.month, &st.year };
					xgc_size idt = 0;
					while( cursor > str )
					{
						xgc_lpcstr pc = strchr( splitc, *cursor );

						if( pc ) switch( *pc )
						{
							case '.':
							idt = XGC_RNG( idt, 0, 0 );
							*ptr[idt++] = (xgc_uint16) strtoul( cursor + 1, xgc_nullptr, 10 );
							break;
							case ' ':
							idt = 3;
							case ':':
							idt = XGC_RNG( idt, 1, 3 );
							*ptr[idt++] = (xgc_uint16) strtoul( cursor + 1, xgc_nullptr, 10 );
							break;
							case '-':
							idt = XGC_RNG( idt, 4, 6 );
							*ptr[idt++] = (xgc_uint16) strtoul( cursor + 1, xgc_nullptr, 10 );
							break;
						}

						--cursor;
					}

					if( idt < XGC_COUNTOF( ptr ) )
					{
						*ptr[idt] = (xgc_uint16) strtoul( cursor, xgc_nullptr, 10 );
					}
				}
				return datetime( st );
			}
		};

		XGC_INLINE datetime operator + ( const datetime& dt, const timespan& sp )
		{
			filetime ft = { dt.mTime.qwDateTime + sp.span };
			return datetime( ft );
		}

		XGC_INLINE datetime operator - ( const datetime& dt, const timespan& sp )
		{
			filetime ft = { dt.mTime.qwDateTime - sp.span };
			return datetime( ft );
		}

		XGC_INLINE timespan operator - ( const datetime& lhs, const datetime& rhs )
		{
			return timespan( lhs.mTime.qwDateTime - rhs.mTime.qwDateTime );
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

		XGC_INLINE datetime datetime::current_time()
		{
			filetime ft;
			GetSystemTimeAsFileTime( (LPFILETIME) &ft );
			return ft;
		}

		XGC_INLINE xgc_lpcstr datetime::current_time( xgc_lpstr output, xgc_size size, xgc_lpcstr format )
		{
			tm stm;
			time_t tt = (time_t) current_time().to_ctime();
			localtime_s( &stm, &tt );
			strftime( output, size, format, &stm );

			return output;
		}

		template< size_t S >
		XGC_INLINE xgc_lpcstr datetime::current_time( xgc_char (&output)[S], xgc_lpcstr format )
		{
			tm stm;
			time_t tt = (time_t) current_time().to_ctime();
			localtime_s( &stm, &tt );
			strftime( output, S, format, &stm );

			return output;
		}

		XGC_INLINE datetime datetime::relative_time( timespan span )
		{
			return datetime::current_time() + span;
		}

		XGC_INLINE xgc_time64 current_time()
		{
			return datetime::current_time().to_ctime();
		}

		XGC_INLINE xgc_time64 current_millisecond()
		{
			return datetime::current_time().to_millisecond();
		}

		XGC_INLINE xgc_time64 current_microsecond()
		{
			return datetime::current_time().to_microsecond();
		}

	}  // end namespace common
}  // end namespace XGC

#endif // __COMMON_DATETIME_H_