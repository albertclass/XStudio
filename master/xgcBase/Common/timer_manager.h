#pragma once
#ifndef _TIMER_MANAGER_H_
#define _TIMER_MANAGER_H_
#if (_MSC_VER >= 1500) && (_MSC_FULL_VER >= 150030729) && !defined( _USE_BOOST )
#include <functional>
using namespace std;
#if (_MSC_VER >= 1600)
// #pragma message("timer_manager using std::function")
using namespace std::placeholders;
#else
// #pragma message("timer_manager using std::tr1::function")
using namespace std::tr1;
using namespace std::tr1::placeholders;
#endif
#else
// #pragma message("timer_manager using boost::function")
#include "boost\function.hpp"
#include "boost\bind.hpp"
using namespace boost;
#endif

#include "defines.h"
#include "exports.h"

#include "allocator.h"
#include "datetime.h"
#include "synchronous.h"
#include "freelist.h"
#include "auto_handle.h"

namespace XGC
{
	namespace common
	{
		//////////////////////////////////////////////////////////////////////////
		// 定时器管理类
		// base		:	基准时间
		//////////////////////////////////////////////////////////////////////////
		typedef pool_handle< pool_handle_base > timer_t;

#define INVALID_TIMER_HANDLE _my_invalid_handle_value_
#define INFINITE_TIMER_REPEATE (0XFFFFFFFFU)
		enum enEventType
		{
			eTimerEvent_Start,
			eTimerEvent_Finish,
			eTimerEvent_Create,
			eTimerEvent_Insert,
			eTimerEvent_Remove,
		};

		typedef function < xgc_void( timer_t ) > TimerCallback;
		typedef function < xgc_void( timer_t, enEventType ) > TimerEventCallback;

		template class COMMON_API function < xgc_void( timer_t ) > ;
		template class COMMON_API function < xgc_void( timer_t, enEventType ) > ;

		enum class event_type : xgc_byte { unknown, timer, clock_week, clock_moon, clock_rept };

		//////////////////////////////////////////////////////////////////////////
		/// 定时器事件接口
		//////////////////////////////////////////////////////////////////////////
		struct COMMON_API timer_event : public auto_handle < timer_event >
		{
			friend class timer_manager;
		protected:
			///
			/// 默认构造
			/// [1/21/2015] create by albert.xu
			///
			timer_event()
				: auto_handle()
				, type( event_type::unknown )
				, caller( xgc_nullptr )
				, param( 0 )
				, index( 0 )
				, repeat( 0 )
			{
			};
			
			///
			/// [7/28/2008] Write by Albert.xu
			/// 构造时间事件对象
			/// @param eType 类型
			/// @param cb 函数指针或者重载operator()的结构体
			/// @param nRepeat 重复次数, 至少执行一次.
			/// @param nParam 参数
			///
			timer_event( event_type eType, const TimerCallback &cb, xgc_uint32 nRepeat, xgc_uint32 nParam, xgc_lpcstr pName )
				: auto_handle()
				, type( eType )
				, caller( cb )
				, param( nParam )
				, index( 0 )
				, repeat( nRepeat )
				, name( pName ? pName : "" )
			{

			};

			///
			/// 析构
			/// [1/21/2015] create by albert.xu
			///
			~timer_event()
			{
				type = event_type::unknown;
				caller = xgc_nullptr;
				repeat = 0;
				param = 0;
				index = 0;
			}

			///
			/// 禁止拷贝构造
			/// [1/21/2015] create by albert.xu
			///
			timer_event( const timer_event &src ) = delete;

			///
			/// 禁止右值引用构造
			/// [1/21/2015] create by albert.xu
			///
			timer_event( timer_event &&src ) = delete;

			///
			/// 禁止赋值操作
			/// [1/21/2015] create by albert.xu
			///
			timer_event& operator=( const timer_event &src ) = delete;
		public:
			XGC_INLINE timer_t GetHandle()const
			{
				return handle();
			}

			XGC_INLINE xgc_uint32 GetIndex()const
			{
				return index;
			}

			XGC_INLINE xgc_real32 GetInterval()const
			{
				return param*0.01f;
			}

			XGC_INLINE xgc_void SetInterval( xgc_real32 _interval )
			{
				param = xgc_uint32( _interval*100.0f );
			}

			XGC_INLINE xgc_uint32 GetParam()const
			{
				return param;
			}

			XGC_INLINE xgc_void SetParam( xgc_uint32 _param )
			{
				param = _param;
			}

			XGC_INLINE xgc_uint32 GetRepeat()const
			{
				return repeat;
			}

			XGC_INLINE xgc_void SetRepeat( xgc_uint16 _repeat )
			{
				repeat = _repeat;
			}

			XGC_INLINE xgc_void SetEvtName( xgc_lpcstr _name )
			{
				name = _name;
			}

			XGC_INLINE xgc_lpcstr GetEvtName()const
			{
				return name.c_str();
			}

		private:
			event_type		type;	///< 定时器类型
			xgc_uint32		index;	///< 索引号, 代表在那个数组中
			xgc_uint32		repeat;	///< 执行次数, 执行次数为降为0 则被销毁
			xgc_uint32		param;	///< 类型参数，根据时钟类型不同含义不同
			TimerCallback	caller;	///< 回调函数

			xgc_string		name;
		};

		class COMMON_API timer_manager
		{
		private:
			typedef xgc_list< timer_t >				CTimerEventList;
			typedef xgc_vector< CTimerEventList >	CTimerEventListVec;

			CTimerEventListVec	mTimerVec;
			CTimerEventListVec	mWheelVec;

			TimerEventCallback	mEventCallback;	///< 定时事件开始回调

			// 定时器变量
			xgc_uint32	mCurrentList;	///< 下一个列表

			xgc_uint64  mTimeTickCount;		///< 上一次执行时的时间，与系统时间无关, 定时器使用
			xgc_time64  mTimeSeconds;		///< 定时器开启的时间，系统时间，时钟使用

		public:
			timer_manager( TimerEventCallback fnCallback = xgc_nullptr );
			~timer_manager( xgc_void );

		public:
			///
			/// 定时器激活
			/// [7/17/2014] create by albert.xu
			///
			xgc_bool timer_step( xgc_uint64 froce = 0 );

			///
			/// 闹钟激活
			/// [7/17/2014] create by albert.xu
			///
			xgc_bool clock_step( xgc_time64 froce = 0 );

			///
			/// 插入计时器事件
			/// @param function	回调函数,可以是成员函数,仿函数或函数指针
			/// @param repeat 重复次数
			/// @param interval 间隔时间
			/// @param delay 第一次执行的延迟时间
			/// @param return 定时器句柄
			///
			timer_t insert_event( const TimerCallback &function, xgc_uint32 repeat, xgc_real64 interval, xgc_real64 delay = 0.0f, xgc_lpcstr name = xgc_nullptr );

			///
			/// [12/7/2012 Albert.xu]
			/// 插入定时器事件
			/// @param function 事件回调
			/// @param deadline 时间发生的时间
			/// @param param 事件参数,用于描述类型及该类型所需要的参数
			///
			timer_t insert_clock( const TimerCallback &function, xgc_time64 deadline, xgc_lpcstr params = xgc_nullptr, xgc_lpcstr name = xgc_nullptr );

			///
			/// 得到事件对象
			/// [7/24/2008] Write by Albert.xu
			/// @param handle 事件句柄
			/// @param return 事件对象,未找到返回NULL
			///
			timer_event* get_event( timer_t handle )const;

			///
			/// 删除事件
			/// [7/24/2008] Write by Albert.xu
			/// @param event 事件对象
			///
			xgc_real32 remove_event( timer_event* evtptr );

			///
			/// 删除事件
			/// [7/24/2008] Write by Albert.xu
			/// @param handle 事件句柄
			///
			xgc_real32 remove_event( timer_t handle )
			{
				return remove_event( timer_event::handle_exchange( handle ) );
			}

			///
			/// 获取剩余时间
			/// [7/9/2014] create by jianglei.kinly
			///
			xgc_real64 remaining_seconds( timer_t handle );

			///
			/// 获取总的剩余时间
			/// [8/29/2014] create by jianglei.kinly
			///
			xgc_real64 remaining_all_seconds( timer_t handle );

			///
			/// [11/29/2010 Albert]
			/// Description:	暂停更新 
			///
			xgc_void pause_event( timer_t handle );

			///
			/// 恢复更新 
			/// [11/29/2010 Albert]
			/// @param handle 事件句柄
			/// @param delay 延迟恢复时间，以当前时间为基准
			/// @return 是否恢复成功
			///
			xgc_bool resume_event( timer_t handle, xgc_real32 delay = 0.0f );

			///
			/// 重新校时，取当前时间的下一次更新
			/// @param deadline 结束时间
			/// @param evtptr 事件指针
			/// @param current 时间基准点
			/// @return 返回校准过的实际执行时间，该时间通过clock的类型和参数校准
			/// [1/21/2015] create by albert.xu
			///
			xgc_time64 adjust_clock_upper_bound( xgc_time64 deadline, timer_event* evtptr, xgc_time64 current = 0 );

			///
			/// 重新校时，取当前时间的上一次更新
			/// @param deadline 结束时间
			/// @param evtptr 事件指针
			/// @param current 时间基准点
			/// @return 返回校准过的实际执行时间，该时间通过clock的类型和参数校准
			/// [1/21/2015] create by albert.xu
			///
			xgc_time64 adjust_clock_lower_bound( xgc_time64 deadline, timer_event* evtptr, xgc_time64 current );
				
			///
			/// 重新校时，取当前时间的下一次更新
			/// @param deadline 结束时间
			/// @param args 事件参数
			/// @param current 时间基准点
			/// @return 返回校准过的时间，该时间通过clock的类型和参数校准
			/// [1/21/2015] create by albert.xu
			///
			xgc_time64 adjust_clock_upper_bound( xgc_time64 deadline, xgc_lpcstr args, xgc_time64 current = 0 );

			///
			/// 重新校时，取当前时间的上一次更新
			/// @param deadline 结束时间
			/// @param args 事件参数
			/// @param current 时间基准点
			/// @return 返回校准过的时间，该时间通过clock的类型和参数校准
			/// [1/21/2015] create by albert.xu
			///
			xgc_time64 adjust_clock_lower_bound( xgc_time64 deadline, xgc_lpcstr args, xgc_time64 current = 0 );

			///
			/// 获取指定时间执行的事件
			/// [9/7/2015] create by albert.xu
			///
			xgc_list< timer_t > get_event_list( datetime dt )const;

			///
			/// 获取指定时间执行的事件
			/// [9/7/2015] create by albert.xu
			///
			xgc_list< timer_t > get_event_list( xgc_lpcstr pname )const;
		protected:
			///
			/// 解释clock命令行
			/// [1/21/2015] create by albert.xu
			///
			xgc_bool _parse_clock_param( timer_event &event, xgc_lpcstr args );

			///
			/// 执行一个链表内的事件
			/// [7/18/2014] create by albert.xu
			///
			xgc_void _clock_step_list( xgc_list< timer_t >& lst );

			///
			/// 插入对象到计时器列表中
			/// [7/24/2008] Write by Albert.xu
			/// @param container 插入的容器
			///	@param evtptr 时间事件对象
			/// @param delay_millseconds 延迟时间（按单元计算）
			///
			xgc_void _insert_timer( timer_event* evtptr, xgc_uint32 delay = 0 );

			///
			/// 插入对象到定时器列表中
			/// [7/18/2014] create by albert.xu
			///
			xgc_void _insert_clock( timer_event* evtptr, xgc_time64 startfrom );
		};

		void timer_manager_unit_test();
	}
}

#endif // _TIMER_MANAGER_H_