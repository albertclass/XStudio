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
		// ��ʱ��������
		// base		:	��׼ʱ��
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
		/// ��ʱ���¼��ӿ�
		//////////////////////////////////////////////////////////////////////////
		struct COMMON_API timer_event : public auto_handle < timer_event >
		{
			friend class timer_manager;
		protected:
			///
			/// Ĭ�Ϲ���
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
			/// ����ʱ���¼�����
			/// @param eType ����
			/// @param cb ����ָ���������operator()�Ľṹ��
			/// @param nRepeat �ظ�����, ����ִ��һ��.
			/// @param nParam ����
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
			/// ����
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
			/// ��ֹ��������
			/// [1/21/2015] create by albert.xu
			///
			timer_event( const timer_event &src ) = delete;

			///
			/// ��ֹ��ֵ���ù���
			/// [1/21/2015] create by albert.xu
			///
			timer_event( timer_event &&src ) = delete;

			///
			/// ��ֹ��ֵ����
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
			event_type		type;	///< ��ʱ������
			xgc_uint32		index;	///< ������, �������Ǹ�������
			xgc_uint32		repeat;	///< ִ�д���, ִ�д���Ϊ��Ϊ0 ������
			xgc_uint32		param;	///< ���Ͳ���������ʱ�����Ͳ�ͬ���岻ͬ
			TimerCallback	caller;	///< �ص�����

			xgc_string		name;
		};

		class COMMON_API timer_manager
		{
		private:
			typedef xgc_list< timer_t >				CTimerEventList;
			typedef xgc_vector< CTimerEventList >	CTimerEventListVec;

			CTimerEventListVec	mTimerVec;
			CTimerEventListVec	mWheelVec;

			TimerEventCallback	mEventCallback;	///< ��ʱ�¼���ʼ�ص�

			// ��ʱ������
			xgc_uint32	mCurrentList;	///< ��һ���б�

			xgc_uint64  mTimeTickCount;		///< ��һ��ִ��ʱ��ʱ�䣬��ϵͳʱ���޹�, ��ʱ��ʹ��
			xgc_time64  mTimeSeconds;		///< ��ʱ��������ʱ�䣬ϵͳʱ�䣬ʱ��ʹ��

		public:
			timer_manager( TimerEventCallback fnCallback = xgc_nullptr );
			~timer_manager( xgc_void );

		public:
			///
			/// ��ʱ������
			/// [7/17/2014] create by albert.xu
			///
			xgc_bool timer_step( xgc_uint64 froce = 0 );

			///
			/// ���Ӽ���
			/// [7/17/2014] create by albert.xu
			///
			xgc_bool clock_step( xgc_time64 froce = 0 );

			///
			/// �����ʱ���¼�
			/// @param function	�ص�����,�����ǳ�Ա����,�º�������ָ��
			/// @param repeat �ظ�����
			/// @param interval ���ʱ��
			/// @param delay ��һ��ִ�е��ӳ�ʱ��
			/// @param return ��ʱ�����
			///
			timer_t insert_event( const TimerCallback &function, xgc_uint32 repeat, xgc_real64 interval, xgc_real64 delay = 0.0f, xgc_lpcstr name = xgc_nullptr );

			///
			/// [12/7/2012 Albert.xu]
			/// ���붨ʱ���¼�
			/// @param function �¼��ص�
			/// @param deadline ʱ�䷢����ʱ��
			/// @param param �¼�����,�����������ͼ�����������Ҫ�Ĳ���
			///
			timer_t insert_clock( const TimerCallback &function, xgc_time64 deadline, xgc_lpcstr params = xgc_nullptr, xgc_lpcstr name = xgc_nullptr );

			///
			/// �õ��¼�����
			/// [7/24/2008] Write by Albert.xu
			/// @param handle �¼����
			/// @param return �¼�����,δ�ҵ�����NULL
			///
			timer_event* get_event( timer_t handle )const;

			///
			/// ɾ���¼�
			/// [7/24/2008] Write by Albert.xu
			/// @param event �¼�����
			///
			xgc_real32 remove_event( timer_event* evtptr );

			///
			/// ɾ���¼�
			/// [7/24/2008] Write by Albert.xu
			/// @param handle �¼����
			///
			xgc_real32 remove_event( timer_t handle )
			{
				return remove_event( timer_event::handle_exchange( handle ) );
			}

			///
			/// ��ȡʣ��ʱ��
			/// [7/9/2014] create by jianglei.kinly
			///
			xgc_real64 remaining_seconds( timer_t handle );

			///
			/// ��ȡ�ܵ�ʣ��ʱ��
			/// [8/29/2014] create by jianglei.kinly
			///
			xgc_real64 remaining_all_seconds( timer_t handle );

			///
			/// [11/29/2010 Albert]
			/// Description:	��ͣ���� 
			///
			xgc_void pause_event( timer_t handle );

			///
			/// �ָ����� 
			/// [11/29/2010 Albert]
			/// @param handle �¼����
			/// @param delay �ӳٻָ�ʱ�䣬�Ե�ǰʱ��Ϊ��׼
			/// @return �Ƿ�ָ��ɹ�
			///
			xgc_bool resume_event( timer_t handle, xgc_real32 delay = 0.0f );

			///
			/// ����Уʱ��ȡ��ǰʱ�����һ�θ���
			/// @param deadline ����ʱ��
			/// @param evtptr �¼�ָ��
			/// @param current ʱ���׼��
			/// @return ����У׼����ʵ��ִ��ʱ�䣬��ʱ��ͨ��clock�����ͺͲ���У׼
			/// [1/21/2015] create by albert.xu
			///
			xgc_time64 adjust_clock_upper_bound( xgc_time64 deadline, timer_event* evtptr, xgc_time64 current = 0 );

			///
			/// ����Уʱ��ȡ��ǰʱ�����һ�θ���
			/// @param deadline ����ʱ��
			/// @param evtptr �¼�ָ��
			/// @param current ʱ���׼��
			/// @return ����У׼����ʵ��ִ��ʱ�䣬��ʱ��ͨ��clock�����ͺͲ���У׼
			/// [1/21/2015] create by albert.xu
			///
			xgc_time64 adjust_clock_lower_bound( xgc_time64 deadline, timer_event* evtptr, xgc_time64 current );
				
			///
			/// ����Уʱ��ȡ��ǰʱ�����һ�θ���
			/// @param deadline ����ʱ��
			/// @param args �¼�����
			/// @param current ʱ���׼��
			/// @return ����У׼����ʱ�䣬��ʱ��ͨ��clock�����ͺͲ���У׼
			/// [1/21/2015] create by albert.xu
			///
			xgc_time64 adjust_clock_upper_bound( xgc_time64 deadline, xgc_lpcstr args, xgc_time64 current = 0 );

			///
			/// ����Уʱ��ȡ��ǰʱ�����һ�θ���
			/// @param deadline ����ʱ��
			/// @param args �¼�����
			/// @param current ʱ���׼��
			/// @return ����У׼����ʱ�䣬��ʱ��ͨ��clock�����ͺͲ���У׼
			/// [1/21/2015] create by albert.xu
			///
			xgc_time64 adjust_clock_lower_bound( xgc_time64 deadline, xgc_lpcstr args, xgc_time64 current = 0 );

			///
			/// ��ȡָ��ʱ��ִ�е��¼�
			/// [9/7/2015] create by albert.xu
			///
			xgc_list< timer_t > get_event_list( datetime dt )const;

			///
			/// ��ȡָ��ʱ��ִ�е��¼�
			/// [9/7/2015] create by albert.xu
			///
			xgc_list< timer_t > get_event_list( xgc_lpcstr pname )const;
		protected:
			///
			/// ����clock������
			/// [1/21/2015] create by albert.xu
			///
			xgc_bool _parse_clock_param( timer_event &event, xgc_lpcstr args );

			///
			/// ִ��һ�������ڵ��¼�
			/// [7/18/2014] create by albert.xu
			///
			xgc_void _clock_step_list( xgc_list< timer_t >& lst );

			///
			/// ������󵽼�ʱ���б���
			/// [7/24/2008] Write by Albert.xu
			/// @param container ���������
			///	@param evtptr ʱ���¼�����
			/// @param delay_millseconds �ӳ�ʱ�䣨����Ԫ���㣩
			///
			xgc_void _insert_timer( timer_event* evtptr, xgc_uint32 delay = 0 );

			///
			/// ������󵽶�ʱ���б���
			/// [7/18/2014] create by albert.xu
			///
			xgc_void _insert_clock( timer_event* evtptr, xgc_time64 startfrom );
		};

		void timer_manager_unit_test();
	}
}

#endif // _TIMER_MANAGER_H_