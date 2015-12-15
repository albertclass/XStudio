#include <numeric>
#include "timer_manager.h"
#include "destructor.h"

typedef XGC::common::timer_event _type;
typedef XGC::common::pool_handle< XGC::common::pool_handle_base > _handle_type;
XGC::common::auto_handle_pool< _type, _handle_type > *
XGC::common::auto_handle_pool< _type, _handle_type >::pInstance = xgc_nullptr;

namespace XGC
{
	namespace memory
	{
		freelist< common::timer_event, common::sync_section > pool_object< common::timer_event, common::sync_section >::pool;
	}

	namespace common
	{
		union timer_clock
		{
			xgc_time64 time;
			struct
			{
				xgc_uint32 m4 : 8;
				xgc_uint32 m3 : 6;
				xgc_uint32 m2 : 6;
				xgc_uint32 m1 : 6;
				xgc_uint32 m0 : 6;

				xgc_uint32 mh;
			};
		};

		timer_manager::timer_manager( TimerEventCallback fnCallback )
			: mCurrentList( 0x1000 ) // 当前列表不可为空
			, mEventCallback( fnCallback )
			, mTimeTickCount( GetTickCount64() )
			, mTimeSeconds( current_time() )
		{
			// 一定要设置为2的次幂
			mTimerVec.resize( 0x1000 );
			// 时间轮算法 ：
			// 将时间分为多个桶，每个桶的大小为2的次幂，
			// 这样，只要按位取即可通过时间值来获得对应的索引。
			// 这里将桶分为256,64,64,64,64共512个
			mWheelVec.resize( 0x200 );
		}

		timer_manager::~timer_manager( void )
		{
			mTimerVec.clear();
			mWheelVec.clear();

		}

		///
		/// 插入定时器
		/// @param function	回调函数,可以是成员函数,仿函数或函数指针
		/// @param repeat 重复次数
		/// @param timer 间隔时间
		/// @param return 定时器句柄
		///
		timer_t timer_manager::insert_event( const TimerCallback &function, xgc_uint32 repeat, xgc_real64 interval, xgc_real64 delay, xgc_lpcstr name )
		{
			timer_event* evtptr = XGC_NEW timer_event( event_type::timer, function, repeat, xgc_uint32( interval * 100 ), name );
			if( evtptr )
			{
				_insert_timer( evtptr, xgc_uint32( delay * 100.0 ) );
				return evtptr->handle();
			}

			return INVALID_TIMER_HANDLE;
		}

		xgc_bool timer_manager::_parse_clock_param( timer_event &event, xgc_lpcstr args )
		{
			if( xgc_nullptr == args || args[0] == 0 || strcmp( "once", args ) == 0 )
			{
				event.type   = event_type::clock_rept;
				event.repeat = 1;
				event.param  = 0;

				return true;
			}

			xgc_lpcstr cmdstr = args;
			xgc_lpcstr params = strchr( args, ':' );

			XGC_ASSERT_RETURN( params, false, "非法的参数格式" );

			xgc_size cmdlen = params - cmdstr;
			++params;

			if( strncmp( "week", cmdstr, cmdlen ) == 0 )
			{
				event.type        = event_type::clock_week;
				event.repeat      = INFINITE_TIMER_REPEATE;
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
						event.param = XGC_SETBIT( event.param, bit % 7 );
					}

					// 取下一个分段
					token = next + strspn( next, delim );
				}

				return true;
			}
			
			if( strncmp( "moon", cmdstr, cmdlen ) == 0 || strncmp( "month", cmdstr, cmdlen ) == 0 )
			{
				event.type        = event_type::clock_moon;
				event.repeat      = INFINITE_TIMER_REPEATE;
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
						event.param = XGC_SETBIT( event.param, bit );
					}

					// 取下一个分段
					token = next + strspn( next, delim );
				}

				return true;
			}
			
			if( strncmp( "rept", cmdstr, cmdlen ) == 0 || strncmp( "repeat", cmdstr, cmdlen ) == 0 )
			{
				xgc_lpstr next = xgc_nullptr;

				event.type   = event_type::clock_rept;
				// 执行的间隔时间
				event.param  = str2numeric< decltype( event.param ) >( params, &next );

				// 分割符号判定
				if( 0 == *next )
				{
					event.repeat = INFINITE_TIMER_REPEATE;
				}
				else if( strchr( " ,:", *next ) )
				{
					event.repeat = str2numeric< decltype( event.repeat ) >( next + 1 );
				}
				else
				{
					// 非法的参数格式
					return false;
				}

				return true;
			}
		
			return false;
		}

		timer_t timer_manager::insert_clock( const TimerCallback &function, xgc_time64 deadline, xgc_lpcstr params, xgc_lpcstr name )
		{
			timer_event event;

			if( _parse_clock_param( event, params ) )
			{
				timer_event* evtptr = XGC_NEW timer_event( event.type, function, event.repeat, event.param, name );
				XGC_ASSERT_RETURN( evtptr, INVALID_TIMER_HANDLE );

				_insert_clock( evtptr, deadline );
				return evtptr->handle();
			}

			return INVALID_TIMER_HANDLE;
		}

		//------------------------------------------------//
		// [7/24/2008] Write by Albert.xu
		// Description	:	得到事件对象
		// handle	:	事件句柄
		// return	:	事件对象,未找到返回NULL
		//------------------------------------------------//
		timer_event* timer_manager::get_event( timer_t handle )const
		{
			return timer_event::handle_exchange( handle );
		}

		//------------------------------------------------//
		// [7/24/2008] Write by Albert.xu
		// Description	:	删除事件
		// event	:	事件对象
		//------------------------------------------------//
		xgc_real32 timer_manager::remove_event( timer_event* evtptr )
		{
			xgc_real32 fSeconds = 0.0f;
			if( evtptr )
			{
				switch( evtptr->type )
				{
					case event_type::timer:
					fSeconds = ( evtptr->index - mCurrentList ) * 0.01f;
					break;
					case event_type::clock_rept:
					case event_type::clock_week:
					case event_type::clock_moon:
					fSeconds = ( evtptr->index - current_time() ) * 1.0f;
					break;
				}
				SAFE_DELETE( evtptr );
			}

			return fSeconds;
		}

		///
		/// 获取剩余时间
		/// [7/9/2014] create by jianglei.kinly
		///
		xgc_real64 timer_manager::remaining_seconds( timer_t handle )
		{
			xgc_real64 fSeconds = 0.0;
			timer_event* evtptr = timer_event::handle_exchange( handle );
			if( evtptr && evtptr->handle() == handle )
			{
				switch( evtptr->type )
				{
					case event_type::timer:
					fSeconds = ( evtptr->index - mCurrentList ) * 0.01;
					break;
					case event_type::clock_rept:
					case event_type::clock_week:
					case event_type::clock_moon:
					fSeconds = ( evtptr->index - current_time() ) * 1.0;
					break;
				}
			}

			return fSeconds;
		}

		///
		/// 增加获取repeat类timer的时间
		/// [8/29/2014] create by jianglei.kinly
		///
		xgc_real64 timer_manager::remaining_all_seconds( timer_t handle )
		{
			xgc_real64 fSeconds = 0.0;
			timer_event* evtptr = timer_event::handle_exchange( handle );
			if( evtptr && evtptr->handle() == handle )
			{
				XGC_ASSERT_RETURN( evtptr->repeat, fSeconds );
				switch( evtptr->type )
				{
					case event_type::timer:
					fSeconds = ( evtptr->repeat - 1 ) * evtptr->param * 0.01 + ( evtptr->index - mCurrentList ) * 0.01;
					break;
					case event_type::clock_rept:
					case event_type::clock_week:
					case event_type::clock_moon:
					fSeconds = ( evtptr->index - current_time() ) * 1.0f;
					break;
				}
			}

			return fSeconds;
		}

		/////
		/// [11/29/2010 Albert]
		/// Description:	暂停更新 
		/////
		xgc_void timer_manager::pause_event( timer_t handle )
		{
			if( handle == INVALID_TIMER_HANDLE )
				return;

			timer_event* evtptr = get_event( handle );
			if( evtptr )
				evtptr->index = -1;
		}

		/////
		/// [11/29/2010 Albert]
		/// Description:	恢复更新 
		/////
		xgc_bool timer_manager::resume_event( timer_t handle, xgc_real32 delay )
		{
			if( handle == INVALID_TIMER_HANDLE )
				return false;

			timer_event* evtptr = get_event( handle );
			XGC_ASSERT_RETURN( evtptr, false );

			_insert_timer( evtptr, xgc_uint32( delay * 100.0f ) );
			return true;
		}

		///function	:	超时处理函数
		///return	:	无
		xgc_bool timer_manager::timer_step( xgc_uint64 froce )
		{
			xgc_bool actived = false;
			xgc_uint64 nTickCount = froce ? froce : GetTickCount64();
			while( nTickCount > mTimeTickCount )
			{
				mTimeTickCount += 10;

				actived = true;
				CTimerEventList& timer_list = mTimerVec[mCurrentList & 0xfff];
				CTimerEventList::iterator iter = timer_list.begin();
				while( iter != timer_list.end() )
				{
					timer_t handle = *iter;
					timer_event* evtptr = get_event( handle );
					if( !evtptr )
					{
						iter = timer_list.erase( iter );
						continue;
					}

					if( evtptr->index == mCurrentList )
					{
						timer_t handle = evtptr->handle();
						if( mEventCallback )
						{
							mEventCallback( handle, eTimerEvent_Start );
							evtptr->caller( handle );
							mEventCallback( handle, eTimerEvent_Finish );
						}
						else
						{
							evtptr->caller( handle );
						}

						// [9/18/2008 Albert]
						// 有可能已经被删除了, 这里需要判断是否被删除.
						if( get_event( handle ) )
						{
							if( evtptr->repeat != INFINITE_TIMER_REPEATE )
							{
								--evtptr->repeat;
							}

							if( evtptr->repeat && evtptr->param )
							{
								_insert_timer( evtptr, evtptr->param );
							}
							else
							{
								remove_event( evtptr );
							}
						}
						iter = timer_list.erase( iter );
					}
					else
					{
						++iter;
					}
				}

				++mCurrentList;
			}

			return actived;
		}

		xgc_void timer_manager::_clock_step_list( xgc_list< timer_t >& lst )
		{
			auto iter = lst.begin();
			while( iter != lst.end() )
			{
				timer_t handle = *iter;

				// 有效性检查
				timer_event* evtptr = get_event( handle );
				if( xgc_nullptr == evtptr )
				{
					iter = lst.erase( iter );
					continue;
				}

				// 到时间了则执行
				if( evtptr->index <= (xgc_uint32) mTimeSeconds )
				{
					if( mEventCallback )
					{
						mEventCallback( handle, eTimerEvent_Start );
						evtptr->caller( handle );
						mEventCallback( handle, eTimerEvent_Finish );
					}
					else
					{
						evtptr->caller( handle );
					}

					// 检查对象是否还有效
					if( evtptr == get_event( handle ) )
					{
						if( evtptr->repeat != INFINITE_TIMER_REPEATE )
						{
							--evtptr->repeat;
						}

						if( evtptr->repeat && evtptr->param )
						{
							_insert_clock( evtptr, (xgc_time64) evtptr->index );
						}
						else
						{
							remove_event( evtptr );
						}
					}
				}
				else
				{
					_insert_clock( evtptr, (xgc_time64) evtptr->index );
				}

				++iter;
			}

			lst.clear();
		}

		xgc_bool timer_manager::clock_step( xgc_time64 froce )
		{
			timer_clock now;
			now.time = froce ? froce : current_time();
			while( mTimeSeconds <= now.time )
			{
				timer_clock &ts = (timer_clock&) mTimeSeconds;
				if( ts.m4 )
				{
					_clock_step_list( mWheelVec[ts.m4] );
				}
				else if( ts.m3 )
				{
					_clock_step_list( mWheelVec[ts.m3 + 0x100] );
				}
				else if( ts.m2 )
				{
					_clock_step_list( mWheelVec[ts.m2 + 0x140] );
				}
				else if( ts.m1 )
				{
					_clock_step_list( mWheelVec[ts.m1 + 0x180] );
				}
				else if( ts.m0 )
				{
					_clock_step_list( mWheelVec[ts.m0 + 0x1c0] );
				}

				++mTimeSeconds;
			}
			return true;
		}

		///
		/// [7/24/2008] Write by Albert.xu
		/// Description	:	插入对象到定时器列表中
		///	handle	:	时间事件对象句柄
		///
		xgc_void timer_manager::_insert_timer( timer_event* evtptr, xgc_uint32 delay )
		{
			XGC_ASSERT_RETURN( evtptr, xgc_void( 0 ) );
			// XGC_ASSERT( delay != 0 && evtptr->GetRepeat() != INFINITE_TIMER_REPEATE );

			evtptr->index = delay + mCurrentList;
			mTimerVec[evtptr->index & 0xfff].push_back( evtptr->handle() );
		}

		xgc_time64 timer_manager::adjust_clock_upper_bound( xgc_time64 deadline, timer_event* evtptr, xgc_time64 current )
		{
			datetime now = datetime::from_ctime( current ? current : mTimeSeconds );

			switch( evtptr->type )
			{
				case event_type::clock_week:
				{
					datetime set = datetime::from_ctime( deadline );
					if( set <= now )
						set += timespan::from_days( ( now - set ).to_days() + 1 );

					xgc_uint16 dayofweek = set.to_systime().dayofweak;
					for( xgc_uint16 cur = 0; cur < 7; ++cur )
					{
						if( 0 == XGC_GETBIT( evtptr->param, ( dayofweek + cur ) % 7 ) )
							continue;

						set += timespan::from_days( cur );
						break;
					}

					deadline = set.to_ctime();
				}
				break;
				case event_type::clock_moon:
				{
					datetime set = datetime::from_ctime( deadline );
					if( set <= now )
						set += timespan::from_days( ( now - set ).to_days() + 1 );

					xgc_uint16 day = set.to_systime().day;
					for( xgc_uint16 cur = 0; cur < 31; ++cur )
					{
						if( 0 == XGC_GETBIT( evtptr->param, ( day - 1 + cur ) % 31 + 1 ) )
							continue;

						set += timespan::from_days( cur );
						break;
					}

					deadline = set.to_ctime();
				}
				break;
				case event_type::clock_rept:
				{
					datetime set = datetime::from_ctime( deadline );
					if( set <= now )
					{
						if( evtptr->param )
							set += timespan::from_seconds( ( ( now - set ).to_seconds() + evtptr->param ) / evtptr->param * evtptr->param );
						else
							set = now;
					}
					deadline = set.to_ctime();
				}
				break;
			}

			return deadline;
		}

		xgc_time64 timer_manager::adjust_clock_lower_bound( xgc_time64 deadline, timer_event* evtptr, xgc_time64 current )
		{
			datetime now = datetime::from_ctime( current ? current : mTimeSeconds );

			switch( evtptr->type )
			{
				case event_type::clock_week:
				{
					datetime set = datetime::from_ctime( deadline );
					if( set <= now )
						set += timespan::from_days( ( now - set ).to_days() );

					xgc_uint16 dayofweek = set.to_systime().dayofweak;
					for( xgc_uint16 cur = 0; cur < 7; ++cur )
					{
						if( 0 == XGC_GETBIT( evtptr->param, 6 - ( dayofweek + cur ) % 7 ) )
							continue;

						set -= timespan::from_days( cur );
						break;
					}

					deadline = set.to_ctime();
				}
				break;
				case event_type::clock_moon:
				{
					datetime set = datetime::from_ctime( deadline );
					if( set <= now )
						set += timespan::from_days( ( now - set ).to_days() );

					xgc_uint16 day = set.to_systime().day;
					for( xgc_uint16 cur = 0; cur < 31; ++cur )
					{
						if( 0 == XGC_GETBIT( evtptr->param, ( day + 30 - cur ) % 31 + 1 ) )
							continue;

						set -= timespan::from_days( cur );
						break;
					}

					deadline = set.to_ctime();
				}
				break;
				case event_type::clock_rept:
				{
					datetime set = datetime::from_ctime( deadline );
					if( set <= now )
					{
						if( evtptr->param )
							set += timespan::from_seconds( ( ( now - set ).to_seconds() + evtptr->param ) / evtptr->param * evtptr->param );
						else
							set = now;
					}

					deadline = set.to_ctime();
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
		xgc_time64 timer_manager::adjust_clock_upper_bound( xgc_time64 deadline, xgc_lpcstr args, xgc_time64 current )
		{
			timer_event event;
			if( _parse_clock_param( event, args ) )
			{
				return adjust_clock_upper_bound( deadline, &event, current );
			}

			return 0;
		}

		///
		/// 重新校时，取当前时间的上一次更新
		/// @param deadline 结束时间
		/// @param args 事件参数
		/// @param current 时间基准点
		/// @return 返回校准过的时间，该时间通过clock的类型和参数校准
		/// [1/21/2015] create by albert.xu
		///
		xgc_time64 timer_manager::adjust_clock_lower_bound( xgc_time64 deadline, xgc_lpcstr args, xgc_time64 current )
		{
			timer_event event;
			if( _parse_clock_param( event, args ) )
			{
				return adjust_clock_lower_bound( deadline, &event, current );
			}

			return 0;
		}

		///
		/// 获取指定时间执行的事件
		/// [9/7/2015] create by albert.xu
		///
		xgc_list< timer_t > timer_manager::get_event_list( datetime dt )const
		{
			xgc_list< timer_t > lst;

			timer_clock t1 = { dt.to_ctime() };
			timer_clock t2 = { mTimeSeconds };

			xgc_size id = 0;
			if( t1.m0 != t2.m0 )
			{
				id = 0x1c0 + t1.m0;
			}
			else if( t1.m1 != t2.m1 )
			{
				id = 0x180 + t1.m1;
			}
			else if( t1.m2 != t2.m2 )
			{
				id = 0x140 + t1.m2;
			}
			else if( t1.m3 != t2.m3 )
			{
				id = 0x100 + t1.m3;
			}
			else
			{
				id = t1.m4;
			}

			for each( auto it in mWheelVec[id] )
			{
				auto evt = get_event( it );
				if( evt && evt->GetIndex() == t1.time )
					lst.push_back( it );
			}

			return lst;
		}

		///
		/// 获取指定时间执行的事件
		/// [9/7/2015] create by albert.xu
		///
		xgc_list< timer_t > timer_manager::get_event_list( xgc_lpcstr pname )const
		{
			xgc_list< timer_t > lst;
			for each( auto &wheel in mWheelVec )
			{
				for each( auto &it in wheel )
				{
					auto evt = get_event( it );

					if( evt && strstr( evt->GetEvtName(), pname ) )
						lst.push_back( it );
				}
			}

			return lst;
		}

		///
		/// 插入定时器
		/// [12/9/2014] create by albert.xu
		///
		xgc_void timer_manager::_insert_clock( timer_event* evtptr, xgc_time64 deadline )
		{
			deadline = adjust_clock_upper_bound( deadline, evtptr );
			XGC_ASSERT_MESSAGE( deadline >= mTimeSeconds, "插入的时间已经过时。%I64u,%I64u", deadline, mTimeSeconds );

			timer_clock t1 = { deadline };
			timer_clock t2 = { mTimeSeconds };

			evtptr->index = (xgc_uint32) deadline;

			if( t1.m0 != t2.m0 )
			{
				mWheelVec[0x1c0 + t1.m0].push_back( evtptr->handle() );
			}
			else if( t1.m1 != t2.m1 )
			{
				mWheelVec[0x180 + t1.m1].push_back( evtptr->handle() );
			}
			else if( t1.m2 != t2.m2 )
			{
				mWheelVec[0x140 + t1.m2].push_back( evtptr->handle() );
			}
			else if( t1.m3 != t2.m3 )
			{
				mWheelVec[0x100 + t1.m3].push_back( evtptr->handle() );
			}
			else
			{
				mWheelVec[t1.m4].push_back( evtptr->handle() );
			}
		}
	}
}