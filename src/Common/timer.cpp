///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file timer.cpp
/// \brief 文件简介
/// \author xufeng04
/// \date 十二月 2015
///
/// 定时器，使用时间轮算法，该算法兼顾性能和可用性，可选择两种不同的时间生成策略
/// 1. 基于系统时间的
/// 2. 基于固定时间的
///
///////////////////////////////////////////////////////////////
#include <numeric>
#include "timer.h"
#include "xutility.h"
#include "destructor.h"

namespace xgc
{
	namespace common
	{
		typedef timer_event _type;
		typedef pool_handle< pool_handle_base > _handle_type;
		auto_handle_pool< _type, _handle_type > *
		auto_handle_pool< _type, _handle_type >::pInstance = xgc_nullptr;

		COMMON_API xgc_time64 (*steady_tickcount)(void) = timer_tickcount< std::chrono::steady_clock >;
		COMMON_API xgc_time64 (*system_tickcount)(void) = timer_tickcount< std::chrono::system_clock >;

		static xgc_void event_dispatcher( timer_t h, en_event_t e )
		{
			//xgc_lpcstr tname = "unknowe";
			//switch( e )
			//{
			//case e_start : tname = "start" ; break;
			//case e_finish: tname = "finish"; break;
			//case e_create: tname = "create"; break;
			//case e_insert: tname = "insert"; break;
			//case e_delete: tname = "delete"; break;
			//}

			//printf( "%08x - %s\n", h._handle, tname );
		};

		// 默认的事件分发
		xgc_void(*timer_event_dispatcher)( timer_t, en_event_t ) = event_dispatcher;

		/// 
		/// \brief 设置事件接口
		/// 
		/// \author xufeng04
		/// \date 十一月 2015
		/// 
		xgc_void set_event_dispatcher( xgc_void( *cb )(timer_t, en_event_t) )
		{
			timer_event_dispatcher = cb ? cb : event_dispatcher;
		}

		///
		/// \brief 时间轮定义
		/// 使用联合来将一个64位时间值转化为
		///
		/// \author albert.xu
		/// \date 2015/12/04 14:44
		///
		union timer_clock
		{
			/// 时间值
			xgc_time64 time;
			/// 时间轮
			struct
			{
				xgc_uint64 m7 : 12;
				xgc_uint64 m6 : 10;
				xgc_uint64 m5 : 10;
				xgc_uint64 m4 : 8;
				xgc_uint64 m3 : 8;
				xgc_uint64 m2 : 6;
				xgc_uint64 m1 : 6;
				xgc_uint64 m0 : 4;
			};
		};

		/// 构造函数
		timer::timer( xgc_time64 (*get_tickcount)() )
			: tickcount_( 0 )
			, get_tickcount_( get_tickcount )
		{
			// 时间轮算法 ：
			// 将时间分为多个桶，每个桶的大小为2的次幂，
			// 这样，只要按位取即可通过时间值来获得对应的索引。
			// 这里将桶分为16,64,64,256,256,1024,1024,4096共6800个
			time_wheel_.resize( 0x1a90 );
			tickcount_ = get_tickcount_() / TIMER_PRECISION;
		}

		datetime timer::now()
		{
			return datetime::from_milliseconds( tickcount_ * TIMER_PRECISION );
		}

		/// 析构函数
		timer::~timer( void )
		{
			time_wheel_.clear();
		}

		///
		/// \brief 删除定时器事件
		///
		/// \param handle 定时器句柄
		/// \return 下次执行的时间间隔
		///
		/// \author albert.xu
		/// \date 2015/12/04 14:37
		///
		timespan timer::remove( timer_t handle )
		{
			std::unique_ptr< timer_event > evtptr( timer_event::handle_exchange( handle ) );

			if( evtptr )
			{
				return timespan::from_milliseconds( (evtptr->time_ - tickcount_) * TIMER_PRECISION );
			}

			return timespan( 0 );
		}

		/// 
		/// \brief 获取剩余执行时间
		/// 
		/// \author xufeng04
		/// \date 十一月 2015
		/// 
		timespan timer::get_remain_over( timer_t handle )
		{
			timer_event* evtptr = timer_event::handle_exchange( handle );
			if( evtptr )
			{
				return timespan::from_milliseconds( (evtptr->over_ - tickcount_) * TIMER_PRECISION );
			}

			return timespan( 0 );
		}

		/// 
		/// \brief 获取下次执行时间
		/// 
		/// \author xufeng04
		/// \date 十一月 2015
		/// 
		timespan timer::get_remain_exec( timer_t handle )
		{
			timer_event* evtptr = timer_event::handle_exchange( handle );
			if( evtptr )
			{
				return timespan::from_milliseconds( (evtptr->time_ - tickcount_) * TIMER_PRECISION );
			}

			return timespan( 0 );
		}
		
		///
		/// \brief 暂停更新 
		///
		xgc_void timer::pause( timer_t handle )
		{
			if( handle == INVALID_TIMER_HANDLE )
				return;

			timer_event* evtptr = timer_event::handle_exchange( handle );
			if( evtptr )
				evtptr->time_ = -1;
		}

		///
		/// \brief 恢复更新 
		///
		xgc_bool timer::resume( timer_t handle, timespan delay )
		{
			if( handle == INVALID_TIMER_HANDLE )
				return false;

			timer_event* evtptr = timer_event::handle_exchange( handle );
			XGC_ASSERT_RETURN( evtptr, false );

			insert_once( evtptr, delay.to_millisecnods() / TIMER_PRECISION, true );
			return true;
		}

		xgc_void timer::step_list( xgc_list< timer_t >& lst )
		{
			auto iter = lst.begin();
			while( iter != lst.end() )
			{
				timer_t handle = *iter;

				// 有效性检查
				timer_event* evtptr = timer_event::handle_exchange( handle );

				xgc_bool adjust = false;
				// 到时间了则执行
				if( evtptr && evtptr->time_ == tickcount_ )
				{
					timer_event_dispatcher( handle, e_start );
					evtptr->call_( handle, evtptr->user_ );
					timer_event_dispatcher( handle, e_finish );

					adjust = true;
				}

				// 检查对象是否还有效
				if( evtptr && evtptr == timer_event::handle_exchange( handle ) )
				{
					insert_once( evtptr, evtptr->time_, adjust );
				}

				++iter;
			}

			lst.clear();
		}

		xgc_bool timer::step( xgc_time64 froce )
		{
			froce = ( froce == 0 ? get_tickcount_() / TIMER_PRECISION : froce / TIMER_PRECISION );

			while( tickcount_ <= froce )
			{
				timer_clock ts = { tickcount_ };

				if( ts.m7 )
				{
					step_list( time_wheel_[ts.m7] );
				}
				else if( ts.m6 )
				{
					step_list( time_wheel_[ts.m6 + 0x1000] );
				}
				else if( ts.m5 )
				{
					step_list( time_wheel_[ts.m5 + 0x1400] );
				}
				else if( ts.m4 )
				{
					step_list( time_wheel_[ts.m4 + 0x1800] );
				}
				else if( ts.m3 )
				{
					step_list( time_wheel_[ts.m3 + 0x1900] );
				}
				else if( ts.m2 )
				{
					step_list( time_wheel_[ts.m2 + 0x1a00] );
				}
				else if( ts.m1 )
				{
					step_list( time_wheel_[ts.m1 + 0x1a40] );
				}
				else if( ts.m0 )
				{
					step_list( time_wheel_[ts.m0 + 0x1a80] );
				}

				tickcount_ += 1;
			}
			return true;
		}

		///
		/// 获取指定时间执行的事件
		/// [9/7/2015] create by albert.xu
		///
		xgc_list< timer_t > timer::get_event_list( datetime stime )const
		{
			timer_clock t1 = { stime.to_milliseconds() / TIMER_PRECISION  };
			timer_clock t2 = { tickcount_ };

			xgc_size id = 0;
			if( t1.m0 != t2.m0 )
			{
				id = 0x1a80 + t1.m0;
			}
			else if( t1.m1 != t2.m1 )
			{
				id = 0x1a40 + t1.m1;
			}
			else if( t1.m2 != t2.m2 )
			{
				id = 0x1a00 + t1.m2;
			}
			else if( t1.m3 != t2.m3 )
			{
				id = 0x1900 + t1.m3;
			}
			else if( t1.m4 != t2.m4 )
			{
				id = 0x1800 + t1.m4;
			}
			else if( t1.m5 != t2.m5 )
			{
				id = 0x1400 + t1.m5;
			}
			else if( t1.m6 != t2.m6 )
			{
				id = 0x1000 + t1.m6;
			}
			else
			{
				id = t1.m7;
			}

			return time_wheel_[id];
		}

		///
		/// 获取指定时间执行的事件
		/// [9/7/2015] create by albert.xu
		///
		xgc_list< timer_t > timer::get_event_list( xgc_lpcstr pname )const
		{
			xgc_list< timer_t > lst;
			for each( auto &wheel in time_wheel_ )
			{
				for each( auto &it in wheel )
				{
					auto evt = timer_event::handle_exchange( it );

					if( evt && strstr( evt->get_name(), pname ) )
						lst.push_back( it );
				}
			}

			return lst;
		}

		///
		/// 插入定时器
		/// [12/9/2014] create by albert.xu
		///
		xgc_void timer::insert_once( timer_event* evtptr, xgc_time64 deadline, xgc_bool adjust )
		{
			// 不允许插入到当前时间点
			if( adjust )
			{
				// 重新校准时间
				deadline = adjust_upper( 
					datetime::from_milliseconds( deadline * TIMER_PRECISION ),
					evtptr->type_,
					evtptr->data_,
					datetime::from_milliseconds( tickcount_ * TIMER_PRECISION ) ).to_milliseconds() / TIMER_PRECISION;

				XGC_ASSERT_MESSAGE( deadline >= tickcount_, "插入的时间已经过时。%I64u,%I64u", deadline, tickcount_ );
			}

			// 下次执行时间在有效时间内的则插入
			if( deadline <= evtptr->over_ )
			{
				timer_clock t1 = { deadline };
				timer_clock t2 = { tickcount_ };

				evtptr->time_ = deadline;

				if( t1.m0 != t2.m0 )
				{
					time_wheel_[0x1a80 + t1.m0].push_back( evtptr->handle() );
				}
				else if( t1.m1 != t2.m1 )
				{
					time_wheel_[0x1a40 + t1.m1].push_back( evtptr->handle() );
				}
				else if( t1.m2 != t2.m2 )
				{
					time_wheel_[0x1a00 + t1.m2].push_back( evtptr->handle() );
				}
				else if( t1.m3 != t2.m3 )
				{
					time_wheel_[0x1900 + t1.m3].push_back( evtptr->handle() );
				}
				else if( t1.m4 != t2.m4 )
				{
					time_wheel_[0x1800 + t1.m4].push_back( evtptr->handle() );
				}
				else if( t1.m5 != t2.m5 )
				{
					time_wheel_[0x1400 + t1.m5].push_back( evtptr->handle() );
				}
				else if( t1.m6 != t2.m6 )
				{
					time_wheel_[0x1000 + t1.m6].push_back( evtptr->handle() );
				}
				else
				{
					time_wheel_[t1.m7].push_back( evtptr->handle() );
				}

				timer_event_dispatcher( evtptr->handle(), e_insert );
			}
			else // 否则删除
			{
				delete evtptr;
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// class timer
		//////////////////////////////////////////////////////////////////////////
		xgc_bool timer::parse( xgc_lpcstr args, xgc_uint16 &type, xgc_uint64 &data, xgc_time64 duration )
		{
			XGC_ASSERT_RETURN( xgc_nullptr != args && args[0] != 0, false );

			xgc_lpcstr cmdstr = args;
			xgc_lpcstr params = strchr( args, ':' );

			XGC_ASSERT_RETURN( params, false, "非法的参数格式" );

			xgc_size cmdlen = params - cmdstr;
			++params;

			if( strncmp( "week", cmdstr, cmdlen ) == 0 )
			{
				type = timer_event::e_week;
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
				type = timer_event::e_moon;
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
				type = timer_event::e_rept;
				// 执行的间隔时间
				if( duration )
					data = duration / str2numeric< decltype( data + 0 ) >( params );
				else
					data = 1;

				return true;
			}

			if( strncmp( "intv", cmdstr, cmdlen ) == 0 || strncmp( "interval", cmdstr, cmdlen ) == 0 )
			{
				type = timer_event::e_rept;
				// 执行的间隔时间
				data = str2numeric< decltype( data + 0 ) >( params );

				return true;
			}

			return false;
		}

		timer_t timer::insert( timer_cb &&function, datetime deadline, timespan duration, xgc_lpcstr args, xgc_intptr userdata, xgc_lpcstr name )
		{
			xgc_uint16 type = timer_event::e_unknown;
			xgc_uint64 data = 0;

			if( parse( args, type, data, duration.to_millisecnods() ) )
			{
				timer_event* evtptr = XGC_NEW timer_event( 
					type
					, std::forward< timer_cb >( function )
					, ( deadline + duration ).to_milliseconds() / TIMER_PRECISION
					, data
					, userdata
					, name );

				XGC_ASSERT_RETURN( evtptr, INVALID_TIMER_HANDLE );

				insert_once( evtptr, deadline.to_milliseconds() / TIMER_PRECISION, true );
				return evtptr->handle();
			}

			return INVALID_TIMER_HANDLE;
		}

		datetime timer::adjust_upper( datetime deadline, xgc_uint16 type, xgc_uint64 data, datetime current )
		{
			datetime now = current;

			switch( type )
			{
			case timer_event::e_week:
				{
					datetime set = deadline;
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

					deadline = set;
				}
				break;
			case timer_event::e_moon:
				{
					datetime set = deadline;
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

					deadline = set;
				}
				break;
			case timer_event::e_rept:
				{
					datetime set = deadline;
					if( set <= now )
					{
						if( data )
							set += timespan::from_milliseconds( ( ( now - set ).to_millisecnods() + data ) / data * data );
						else
							set = now;
					}
					deadline = set;
				}
				break;
			}

			return deadline;
		}

		datetime timer::adjust_lower( datetime deadline, xgc_uint16 type, xgc_uint64 data, datetime current )
		{
			datetime now = current;

			switch( type )
			{
			case timer_event::e_week:
				{
					datetime set = deadline;
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

					deadline = set;
				}
				break;
			case timer_event::e_moon:
				{
					datetime set = deadline;
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

					deadline = set;
				}
				break;
			case timer_event::e_rept:
				{
					datetime set = deadline;
					if( set <= now )
					{
						if( data )
							set += timespan::from_milliseconds( ( ( now - set ).to_millisecnods() + data ) / data * data );
						else
							set = now;
					}

					deadline = set;
				}
				break;
			}

			return deadline;
		}

		///
		/// 重新校时
		/// @return 返回校准过的时间，该时间通过clock的类型和参数校准
		/// [1/21/2015] create by albert.xu
		///
		datetime timer::adjust_upper( datetime deadline, xgc_lpcstr args, datetime current, timespan duration )
		{
			xgc_uint16 type = 0;
			xgc_uint64 data = 0;
			if( parse( args, type, data, duration.to_millisecnods() ) )
			{
				return adjust_upper( deadline, type, data, current );
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
		datetime timer::adjust_lower( datetime deadline, xgc_lpcstr args, datetime current, timespan duration )
		{
			xgc_uint16 type;
			xgc_uint64 data = 0;
			if( parse( args, type, data, duration.to_millisecnods() ) )
			{
				return adjust_lower( deadline, type, data, current );
			}

			return datetime::from_ftime( 0 );
		}
	}
}