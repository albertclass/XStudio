#pragma once
#ifndef _TIMER_MANAGER_H_
#define _TIMER_MANAGER_H_
#include "defines.h"
#include "exports.h"
#include "auto_handle.h"

namespace xgc
{
	namespace common
	{
		using std::function;
		
		// ��ʱ�����
		typedef pool_handle< pool_handle_base > timer_h;

		# define INVALID_TIMER_HANDLE _my_invalid_handle_value_
		# define DURATION_FOREVER timespan(0xefffffffffffffffULL)

		# define TIMER_PRECISION 100
		/*!
		 * \brief �¼�ID�����ڱ�ʶ��ʱ���¼�����
		 *
		 * \author xufeng04
		 * \date ʮһ�� 2015
		 */
		enum en_event_t
		{
			e_start,
			e_finish,
			e_create,
			e_insert,
			e_delete,
		};

		typedef std::function < xgc_void( timer_h, xgc_lpvoid ) > timer_cb;

		#if defined(_WINDOWS)
		template class COMMON_API std::function < xgc_void( timer_h ) > ;
		template class COMMON_API std::function < xgc_void( timer_h, en_event_t ) > ;
		#endif
		
		extern xgc_void(*timer_event_dispatcher)( timer_h, en_event_t );

		/// 
		/// \brief �����¼��ӿ�
		/// 
		/// \author xufeng04
		/// \date ʮһ�� 2015
		/// 
		xgc_void set_event_dispatcher( xgc_void( *cb )(timer_h, en_event_t) );

		//////////////////////////////////////////////////////////////////////////
		/// ��ʱ���¼��ӿ�
		//////////////////////////////////////////////////////////////////////////
		struct COMMON_API timer_event : public auto_handle < timer_event >
		{
			enum timer_type
			{ 
				e_unknown, 
				e_week, 
				e_moon,
				e_year,
				e_rept,
			};

			friend class timer;
		public:
			///
			/// Ĭ�Ϲ���
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
			/// ����ʱ���¼�����
			/// @param eType ����
			/// @param cb ����ָ���������operator()�Ľṹ��
			/// @param nRepeat �ظ�����, ����ִ��һ��.
			/// @param nParam ����
			///
			timer_event( xgc_uint16 type, const timer_cb &call, xgc_time64 over, xgc_uint64 data, xgc_lpvoid user = xgc_nullptr, xgc_lpcstr name = "noname" )
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
			/// ����
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

			/*!
			 * \brief ִ��ʱ��
			 *
			 * \author xufeng04
			 * \date ʮһ�� 2015
			 */
			xgc_time64 get_time()const
			{
				return time_;
			}

			/*!
			* \brief ����ʱ��
			*
			* \author xufeng04
			* \date ʮһ�� 2015
			*/
			xgc_time64 get_over()const
			{
				return time_;
			}

			/*!
			 * \brief ��ȡ�¼���
			 *
			 * \author xufeng04
			 * \date ʮһ�� 2015
			 */
			xgc_lpcstr get_name()const
			{
				return name_;
			}

			/*!
			 * \brief ��ȡ�û�����
			 *
			 * \author xufeng04
			 * \date ʮһ�� 2015
			 */
			xgc_lpvoid get_user()const
			{
				return user_;
			}

		private:
			xgc_uint16 type_;	///< ��ʱ������
			xgc_uint64 data_;	///< ���Ͳ���������ʱ�����Ͳ�ͬ���岻ͬ
			xgc_time64 time_;	///< ��ʱ��ִ��ʱ��
			xgc_time64 over_;	///< ��ʱ������ʱ��
			xgc_lpcstr name_;	///< �¼��������ڵ��ԣ�
			xgc_lpvoid user_;	///< �û�����

			timer_cb call_;	///< �ص�����
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
			typedef xgc_list< timer_h >		event_s;
			typedef xgc_vector< event_s >	wheel_s;

			///< ʱ����
			wheel_s	time_wheel_;

			///< ��һ��ִ��ʱ��ʱ��
			xgc_time64 tickcount_;

			///< ��ȡ��ǰʱ��
			xgc_time64 (*get_tickcount_)();
		public:
			timer( xgc_time64 (*get_tickcount)() = system_tickcount );
			~timer();

			///
			/// \brief ȡ��ʱ����ǰʱ��
			///
			/// \author albert.xu
			/// \date 2015/12/04 16:40
			///
			datetime now();

			///
			/// \brief ���붨ʱ���¼�
			/// \param function �¼��ص�
			/// \param deadline ʱ�䷢����ʱ��
			/// \param param �¼�����,�����������ͼ�����������Ҫ�Ĳ���
			///
			timer_h insert( const timer_cb &cb, datetime start, timespan duration, xgc_lpcstr args, xgc_lpvoid userdata = xgc_nullptr, xgc_lpcstr name = xgc_nullptr );

			///
			/// \brief ���붨ʱ���¼�
			/// \param function �¼��ص�
			/// \param deadline ʱ�䷢����ʱ��
			/// \param param �¼�����,�����������ͼ�����������Ҫ�Ĳ���
			///
			timer_h insert( const timer_cb &cb, datetime start, timespan duration, xgc_uint16 type, xgc_uint64 data, xgc_lpvoid userdata = xgc_nullptr, xgc_lpcstr name = xgc_nullptr );

			///
			/// \brief ���Ӽ���
			/// [7/17/2014] create by albert.xu
			///
			xgc_bool step( xgc_time64 froce = 0 );

			///
			/// \brief ɾ���¼�
			/// [7/24/2008] Write by Albert.xu
			/// @param handle �¼����
			///
			timespan remove( timer_h handle );

			/// 
			/// \brief ��ȡʣ�����ʱ��
			/// 
			/// \author xufeng04
			/// \date ʮһ�� 2015
			/// 
			timespan get_remain_over( timer_h handle );

			/// 
			/// \brief ��ȡ�´�ִ��ʱ��
			/// 
			/// \author xufeng04
			/// \date ʮһ�� 2015
			/// 
			timespan get_remain_exec( timer_h handle );

			///
			/// \brief ��ͣ���� 
			/// \date [11/29/2010 Albert]
			///
			xgc_void pause( timer_h handle );

			///
			/// \brief �ָ����� 
			/// \param handle �¼����
			/// \param delay �ӳٻָ�ʱ�䣬�Ե�ǰʱ��Ϊ��׼
			/// \return �Ƿ�ָ��ɹ�
			/// \date [11/29/2010 Albert]
			///
			xgc_bool resume( timer_h handle, timespan delay = timespan( 0 ) );

			///
			/// \brief ��ȡָ��ʱ��ִ�е��¼�
			/// \date [9/7/2015] create by albert.xu
			///
			xgc_list< timer_h > get_event_list( datetime stime )const;

			///
			/// \brief ��ȡָ��ʱ��ִ�е��¼�
			/// \date [9/7/2015] create by albert.xu
			///
			xgc_list< timer_h > get_event_list( xgc_lpcstr pname )const;

		private:
			///
			/// \brief ִ��һ�������ڵ��¼�
			/// [7/18/2014] create by albert.xu
			///
			xgc_void step_list( xgc_list< timer_h >& lst );

			///
			/// \brief ������󵽶�ʱ���б���
			/// [7/18/2014] create by albert.xu
			///
			xgc_void insert_once( timer_event* evtptr, xgc_time64 deadline, xgc_bool adjust = false );
		};
	}
}

#endif // _TIMER_MANAGER_H_