#pragma once
#ifndef _TIMER_MANAGER_H_
#define _TIMER_MANAGER_H_
#include <functional>
#include <chrono>

#include "defines.h"
#include "exports.h"
#include "auto_handle.h"
#include "datetime.h"

namespace xgc
{
	namespace common
	{
		// 定时器句柄
		typedef pool_handle< pool_handle_base > timer_t;

		# define INVALID_TIMER_HANDLE _my_invalid_handle_value_
		# define DURATION_FOREVER timespan(0xefffffffffffffffULL)

		# define TIMER_PRECISION 100
		/*!
		 * \brief 事件ID，用于标识定时器事件类型
		 *
		 * \author xufeng04
		 * \date 十一月 2015
		 */
		enum en_event_t
		{
			e_start,
			e_finish,
			e_create,
			e_insert,
			e_delete,
		};

		typedef std::function < xgc_void( timer_t, intptr_t& ) > timer_cb;

		template class COMMON_API std::function < xgc_void( timer_t ) > ;
		template class COMMON_API std::function < xgc_void( timer_t, en_event_t ) > ;

		extern xgc_void(*timer_event_dispatcher)( timer_t, en_event_t );

		/// 
		/// \brief 设置事件接口
		/// 
		/// \author xufeng04
		/// \date 十一月 2015
		/// 
		xgc_void set_event_dispatcher( xgc_void( *cb )(timer_t, en_event_t) );

		//////////////////////////////////////////////////////////////////////////
		/// 定时器事件接口
		//////////////////////////////////////////////////////////////////////////
		struct COMMON_API timer_event : public auto_handle < timer_event >
		{
			enum timer_type
			{ 
				e_unknown, 
				e_week, 
				e_moon, 
				e_rept,
			};

			friend class timer;
		public:
			///
			/// 默认构造
			/// [1/21/2015] create by albert.xu
			///
			timer_event()
				: auto_handle()
				, type_( e_unknown )
				, time_( 0 )
				, data_( 0 )
				, user_( 0 )
				, name_( xgc_nullptr )
				, call_( xgc_nullptr )
			{
				timer_event_dispatcher( handle(), e_create );
			};
			
			///
			/// [7/28/2008] Write by Albert.xu
			/// 构造时间事件对象
			/// @param eType 类型
			/// @param cb 函数指针或者重载operator()的结构体
			/// @param nRepeat 重复次数, 至少执行一次.
			/// @param nParam 参数
			///
			timer_event( xgc_uint16 type, const timer_cb &call, xgc_time64 over, xgc_uint64 data, xgc_intptr user = 0, xgc_lpcstr name = "noname" )
				: auto_handle()
				, type_( type )
				, time_( 0 )
				, over_( over )
				, data_( data )
				, user_( user )
				, name_( name )
				, call_( call )
			{
				timer_event_dispatcher( handle(), e_create );
			};

			///
			/// 析构
			/// [1/21/2015] create by albert.xu
			///
			~timer_event()
			{
				type_ = e_unknown;
				time_ = 0;
				data_ = 0;
				user_ = 0;
				name_ = xgc_nullptr;
				call_ = xgc_nullptr;

				timer_event_dispatcher( handle(), e_delete );
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

			/*!
			 * \brief 执行时间
			 *
			 * \author xufeng04
			 * \date 十一月 2015
			 */
			xgc_time64 get_time()const
			{
				return time_;
			}

			/*!
			* \brief 结束时间
			*
			* \author xufeng04
			* \date 十一月 2015
			*/
			xgc_time64 get_over()const
			{
				return time_;
			}

			/*!
			 * \brief 获取事件名
			 *
			 * \author xufeng04
			 * \date 十一月 2015
			 */
			xgc_lpcstr get_name()const
			{
				return name_;
			}

			/*!
			 * \brief 获取用户数据
			 *
			 * \author xufeng04
			 * \date 十一月 2015
			 */
			xgc_intptr get_user()const
			{
				return user_;
			}

		private:
			xgc_uint16 type_;	///< 定时器类型
			xgc_uint64 data_;	///< 类型参数，根据时钟类型不同含义不同
			xgc_time64 time_;	///< 定时器执行时间
			xgc_time64 over_;	///< 定时器结束时间
			xgc_lpcstr name_;	///< 事件名（用于调试）
			xgc_intptr user_;	///< 用户数据

			timer_cb call_;	///< 回调函数
		};

		template< class clock >
		xgc_time64 timer_tickcount()
		{
			return std::chrono::duration_cast< std::chrono::duration< xgc_time64, std::milli > >( clock::now().time_since_epoch() ).count();
		}

		extern COMMON_API xgc_time64 (*steady_tickcount)();
		extern COMMON_API xgc_time64 (*system_tickcount)();

		class COMMON_API timer
		{
		protected:
			typedef xgc_list< timer_t >		event_s;
			typedef xgc_vector< event_s >	wheel_s;

			///< 时间轮
			wheel_s	time_wheel_;

			///< 上一次执行时的时间
			xgc_time64 tickcount_;

			///< 获取当前时间
			xgc_time64 (*get_tickcount_)();
		public:
			timer( xgc_time64 (*get_tickcount)() = system_tickcount );
			~timer( xgc_void );

			///
			/// \brief 取定时器当前时间
			///
			/// \author albert.xu
			/// \date 2015/12/04 16:40
			///
			datetime now();

			///
			/// \brief 插入定时器事件
			/// \param function 事件回调
			/// \param deadline 时间发生的时间
			/// \param param 事件参数,用于描述类型及该类型所需要的参数
			///
			timer_t insert( timer_cb &&function, datetime deadline, timespan duration, xgc_lpcstr args, xgc_intptr userdata = 0, xgc_lpcstr name = "noname" );

			/// 
			/// \brief 重新校时，取当前时间的下一次更新
			/// \param deadline 结束时间
			/// \param args 定时器参数
			/// \param current 时间基准点
			/// \return 返回校准过的实际执行时间，该时间通过clock的类型和参数校准
			/// [1/21/2015] create by albert.xu
			/// 
			datetime adjust_upper( datetime deadline, xgc_lpcstr args, datetime current = datetime::from_ftime( 0 ), timespan duration = timespan( 0 ) );

			/// 
			/// \brief 重新校时，取当前时间的上一次更新
			/// \param deadline 结束时间
			/// \param args 定时器参数
			/// \param current 时间基准点
			/// \return 返回校准过的实际执行时间，该时间通过clock的类型和参数校准
			/// [1/21/2015] create by albert.xu
			/// 
			datetime adjust_lower( datetime deadline, xgc_lpcstr args, datetime current = datetime::from_ftime( 0 ), timespan duration = timespan( 0 ) );

			///
			/// \brief 闹钟激活
			/// [7/17/2014] create by albert.xu
			///
			xgc_bool step( xgc_time64 froce = 0 );

			///
			/// \brief 删除事件
			/// [7/24/2008] Write by Albert.xu
			/// @param handle 事件句柄
			///
			timespan remove( timer_t handle );

			/// 
			/// \brief 获取剩余持续时间
			/// 
			/// \author xufeng04
			/// \date 十一月 2015
			/// 
			timespan get_remain_over( timer_t handle );

			/// 
			/// \brief 获取下次执行时间
			/// 
			/// \author xufeng04
			/// \date 十一月 2015
			/// 
			timespan get_remain_exec( timer_t handle );

			///
			/// \brief 暂停更新 
			/// \date [11/29/2010 Albert]
			///
			xgc_void pause( timer_t handle );

			///
			/// \brief 恢复更新 
			/// \param handle 事件句柄
			/// \param delay 延迟恢复时间，以当前时间为基准
			/// \return 是否恢复成功
			/// \date [11/29/2010 Albert]
			///
			xgc_bool resume( timer_t handle, timespan delay = timespan( 0 ) );

			///
			/// \brief 获取指定时间执行的事件
			/// \date [9/7/2015] create by albert.xu
			///
			xgc_list< timer_t > get_event_list( datetime stime )const;

			///
			/// \brief 获取指定时间执行的事件
			/// \date [9/7/2015] create by albert.xu
			///
			xgc_list< timer_t > get_event_list( xgc_lpcstr pname )const;

		private:
			/// 
			/// \brief 解释参数
			/// 
			/// \author xufeng04
			/// \date 十一月 2015
			/// 
			xgc_bool parse( xgc_lpcstr args, xgc_uint16& type, xgc_uint64& data, xgc_time64 duration );

			/// 
			/// \brief 重新校时，取当前时间的下一次更新
			/// \param deadline 结束时间
			/// \param type 定时器类型
			/// \param data 携带数据
			/// \param current 时间基准点
			/// \return 返回校准过的实际执行时间，该时间通过clock的类型和参数校准
			/// \date [1/21/2015] create by albert.xu
			/// 
			datetime adjust_upper( datetime deadline, xgc_uint16 type, xgc_uint64 data, datetime current );

			/// 
			/// \brief 重新校时，取当前时间的上一次更新
			/// \param deadline 结束时间
			/// \param type 定时器类型
			/// \param data 携带数据
			/// \param current 时间基准点
			/// \return 返回校准过的实际执行时间，该时间通过clock的类型和参数校准
			/// \date [1/21/2015] create by albert.xu
			/// 
			datetime adjust_lower( datetime deadline, xgc_uint16 type, xgc_uint64 data, datetime current );

			///
			/// \brief 执行一个链表内的事件
			/// [7/18/2014] create by albert.xu
			///
			xgc_void step_list( xgc_list< timer_t >& lst );

			///
			/// \brief 插入对象到定时器列表中
			/// [7/18/2014] create by albert.xu
			///
			xgc_void insert_once( timer_event* evtptr, xgc_time64 deadline, xgc_bool adjust = false );
		};

		void timer_manager_unit_test();
	}
}

#endif // _TIMER_MANAGER_H_