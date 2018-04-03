#pragma once
#ifndef __XCHARACTOR_H__
#define __XCHARACTOR_H__
#include "XTeam.h"
#include "XGameObject.h"

namespace xgc
{
	enum class enActorState
	{
		sta_live,   ///< ����״̬
		sta_soul,   ///< ��״̬��������
		sta_dead,   ///< �������޷��������޷��ƶ����޷���������
	};

	enum enActorStatus
	{
		stu_actor_normal = 0,		///< ����
		stu_actor_pain   = 1 << 1,  ///< ��ʹ����Ӳֱ��
		stu_actor_faint  = 1 << 2,  ///< ѣ�Σ��������ƶ������ɲ�����
		stu_actor_fall   = 1 << 3,  ///< �ÿգ��������ƶ������ɲ�����
		stu_actor_beat   = 1 << 4,  ///< ���ˣ������ɲ�����
		stu_actor_lie    = 1 << 5,  ///< ���أ��������ƶ������ɲ�����
	};

	enum enActorAbility
	{
		/// �Ƿ���ƶ� 0 - ���ԣ� ���� - ������
		abl_actor_move = 0,
		/// �Ƿ�ɹ��� 0 - ���ԣ� ���� - ������
		abl_actor_attack,
		/// �Ƿ���ܻ� 0 - ���ԣ� ���� - ������
		abl_actor_be_hit,
		/// �Ƿ�ɱ����� 0 - ���ԣ� ���� - ������
		abl_actor_be_tanunt,
		/// �Ƿ������ 0 - ���ԣ� ���� - ������
		abl_actor_dead,
		/// �Ƿ�����ˣ���Ѫ�� 0 - ���ԣ� ���� - ������
		abl_actor_hurt,
		/// ��ɫ��������
		abl_actor_count,
	};

	enum enActorEvent
	{
		evt_actor_relive,		///< ��ɫ����
		evt_actor_change_state,	///< ��ɫ״̬ת��
		evt_actor_change_target,///< ��ɫת��Ŀ��
		evt_actor_attacked,		///< ��ɫ��������
		evt_actor_killed,		///< ��ɫɱ��Ŀ��
		evt_actor_assists,		///< ��ɫɱ��Ŀ�� - ����
		evt_actor_behit,		///< ��ɫ������
		evt_actor_enter_fight,	///< ����ս��״̬
		evt_actor_leave_fight,	///< �˳�ս��״̬
		evt_actor_faild_fight,	///< ��ɫս��ʧ��
		evt_actor_befor_dead,	///< ��ɫ����
		evt_actor_dead,			///< ��ɫ����

		evt_actor_pick,			///< ��ɫ������Ʒ
		evt_actor_drop,			///< ��ɫ������Ʒ
		evt_actor_team_join,	///< ��ɫ�����������˼���
		evt_actor_team_kick,	///< ��ɫ���������˱��߳�
		evt_actor_enter_team,	///< ��ɫ�������
		evt_actor_leave_team,	///< ��ɫ�뿪����
		evt_actor_enter_scene,	///< ��ɫ���볡��
		evt_actor_leave_scene,	///< ��ɫ�뿪����
	};

	class CORE_API XActor : public XObjectNode
	{
		DECLARE_XCLASS();

	public:
		///
		/// \brief һ���ɫ�¼� 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		struct CORE_API NormalEvent
		{
			/// @var �¼�
			XObjectEvent cast;
			/// @var ������
			xgc_lpvoid lpContext;
		};

		///
		/// \brief ��ɫ�¼�����
		/// \author albert.xu
		/// \date 2017/10/13
		///
		struct CORE_API AttackEvent
		{
			/// @var �¼�
			XObjectEvent cast;
			/// @var �����ߵĶ���ID
			xObject hAttacker;
			/// @var �����������ID
			xObject hTarget;
			/// @var ������ʽ
			xgc_long nMode;
			/// @var �˺�
			xgc_long nDamage;
			/// @var ���
			xgc_long nHate;
			/// @var ������
			xgc_lpvoid lpContext;
		};

		///
		/// \brief �����¼� 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		struct CORE_API TeamEvent
		{
			/// @var �¼�
			XObjectEvent cast;
			/// @var ����ID
			xgc_ulong team_id;
		};

		//////////////////////////////////////////////////////////////////////////
		// ��ɫ��������
		static xAttrIndex Index;		///< ��ɫ��������
		static xAttrIndex Type;			///< ��ɫ����
		static xAttrIndex HP;			///< ��ǰ����ֵ
		static xAttrIndex HP_Max;		///< ��������
		static xAttrIndex BeatSpeed;	///< �����ٶ�
		static xAttrIndex FaintTime;	///< ѣ��ʱ��
		static xAttrIndex AbnormalTime;	///< ����״̬ʱ��
		static xAttrIndex BornTime;		///< ����ʱ��
		static xAttrIndex GroupMask;	///< ������룬����������Ӫ
		static xAttrIndex State;		///< ��ɫ״̬
		static xAttrIndex Status;		///< ��ɫ����
		static xAttrIndex StatusTime;	///< ��ɫ�쳣״̬����ʱ��
		static xAttrIndex CanMove;		///< ���ƶ�
		static xAttrIndex CanAttack;	///< �ɹ���
		static xAttrIndex CanBeHit;		///< ���ܻ�
		static xAttrIndex CanBeTanunt;	///< �ɳ���
		static xAttrIndex CanDead;		///< ������
		static xAttrIndex CanHurt;		///< ������

		friend class XGameMap;
		friend class XTeam;

	public:
		XActor( void );
		~XActor( void );

		///
		/// \brief �ܵ�����
		/// \author albert.xu
		/// \date 11/19/2010
		///
		xgc_void UnderAttack( xObject hAttacker, xgc_long nMode, xgc_lpvoid lpContext );

		///
		/// \brief ��ȡ�����ߵĽ�ɫ
		/// \date  11/19/2010
		/// \author albert.xu
		///
		XGC_INLINE xObject GetAttacker()const { return mAttacker; }

		///
		/// \brief ��ȡ���ڹ�����Ŀ�� 
		/// \author albert.xu
		/// \date 12/27/2010
		///
		XGC_INLINE xObject GetTarget()const { return mTarget; }

		///
		/// \brief ���õ�ǰ������Ŀ��, Զ�̱��ֲ���Ҫ����λ�� 
		/// \date [12/27/2010 Albert]
		///
		XGC_INLINE xObject SetTarget( xObject hTarget )
		{
			xObject hOldTarget = mTarget;
			mTarget = hTarget;
			return hOldTarget;
		}

		///
		/// \brief ��ȡ��������
		/// \date [11/23/2012 Albert.xu]
		///
		XGC_INLINE xObject GetFriend()const { return mFriend; }

		///
		/// \brief ���ð�������
		/// \date [11/23/2012 Albert.xu]
		///
		XGC_INLINE xgc_void SetFriend( xObject hObject ) { mFriend = hObject; }

		///
		/// \brief ��ȡ������
		/// \date [9/28/2012 Albert.xu]
		///
		const XVector3& GetBornPoint()const { return mBornPoint; }

		///
		/// \brief ��ȡ������
		/// \date [1/7/2014 albert.xu]
		///
		xgc_void GetBornPoint( float &x, float &y, float &z ) 
		{ 
			x = mBornPoint.x; 
			y = mBornPoint.y; 
			z = mBornPoint.z; 
		}

		///
		/// \brief ���ó�����
		/// \date [10/12/2012 Albert.xu]
		///
		xgc_void SetBornPoint( float x, float y, float z ) 
		{ 
			mBornPoint.x = x; 
			mBornPoint.y = y; 
			mBornPoint.z = z; 
		}

		///
		/// \brief ��ɫ������������ڣ�����������ã�
		/// \date 8/21/2009
		/// \author albert.xu
		///
		xgc_void Dead( xObject hAttacker, xgc_long nMode, xgc_lpvoid lpContext );

		///
		/// \brief ��ɫ������������ڣ� 
		/// \date 12/28/2010
		/// \author albert.xu
		///
		xgc_void Relive( xgc_lpvoid lpContext );

		///
		/// \brief ��ȡ��ɫ״̬
		/// \date 8/24/2009
		/// \author albert.xu
		///
		enActorState getState()const 
		{ 
			return enActorState( getValue<xgc_uint32>( State ) );
		}

		///
		/// \brief ���ý�ɫ״̬ 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		xgc_void setState( enActorState eState )
		{
			setValue< xgc_uint32 >( State, (xgc_uint32)eState );
		}

		///
		/// \brief �жϽ�ɫ״̬ 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		xgc_bool isState( enActorState eState )
		{
			return eState == getState();
		}

		///
		/// \brief ��ȡ��ɫ��ǰ���� 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		enActorStatus getStatus()const
		{
			return enActorStatus( getValue<xgc_uint32>( Status ) );
		}

		///
		/// \brief ���ý�ɫ״̬
		/// \date 8/24/2009 
		/// \author albert.xu
		///
		xgc_void setStatus( enActorStatus eStatus, timespan tsDuration, xgc_int32 nMode = 0 );

		///
		/// \brief ��������״̬
		/// \date 5/31/2014 
		/// \author jianglei.kinly
		///
		xgc_void setStatus( enActorStatus eStatus );

		///
		/// \brief �жϵ�ǰ״̬
		/// \date 9/26/2014
		/// \author albert.xu
		///
		xgc_bool isStatus( enActorStatus eStatus )const 
		{ 
			return eStatus == getStatus(); 
		}

		///
		/// \brief ����/�ر�����
		/// \date 6/22/2014
		/// \author jianglei.kinly
		///
		xgc_void enableAbility( enActorAbility eAbility, xgc_bool bEnable = true )
		{
			XGC_ASSERT_RETURN( eAbility < abl_actor_count, XGC_NONE );
		
			incValue( CanMove + eAbility, bEnable ? -1 : +1 );
		}

		///
		/// \brief ������������
		/// \date 6/22/2014
		/// \author jianglei.kinly
		///
		xgc_void resetAbility( enActorAbility eAbility )
		{
			XGC_ASSERT_RETURN( eAbility < abl_actor_count, XGC_NONE );

			setValue( CanMove + eAbility, 0 );
		}

		///
		/// \brief �����Ƿ����
		/// \date 6/22/2014
		/// \author jianglei.kinly
		///
		xgc_bool isAbilityActived( enActorAbility eAbility )
		{
			XGC_ASSERT_RETURN( eAbility < abl_actor_count, false );

			return 0 == getValue< xgc_int32 >( CanMove + eAbility );
		}

		///
		/// ���ý�ɫ��ײ�뾶
		/// [10/23/2012 Albert.xu]
		///
		xgc_void SetRadius( xgc_real32 fRadius ) { mRadius = fRadius; }

		///
		/// ��ȡ��ɫ��ײ�뾶
		/// [10/24/2012 Albert.xu]
		///
		xgc_real32 GetRadius()const { return mRadius; }

		///
		/// ��ȡ����ָ��
		/// [11/12/2012 Albert.xu]
		///
		XTeamPtr getTeam() 
		{ 
			return mTeamPtr; 
		}

		///
		/// ��ȡ����ָ�� const
		/// [11/22/2012 Albert.xu]
		///
		const XTeamPtr getTeam()const
		{
			return mTeamPtr;
		}

		///
		/// ���ö���ָ��
		/// [11/22/2012 Albert.xu]
		///
		xgc_void setTeam( XTeamPtr team ) 
		{ 
			mTeamPtr = team;
		}

		///
		/// ����Buff���Ͳ���buff�б�
		/// [8/12/2014] create by jianglei.kinly
		///
		//xObjectSet GetStateByType( xgc_uint32 type ) const
		//{
		//	xObjectSet resSet;
		//	for( auto& it01 : mStateMap )
		//	{
		//		if( ( it01.first.first & type ) == type )
		//			resSet.insert( it01.second.begin(), it01.second.end() );
		//	}

		//	return std::move( resSet );
		//}

		/// 
		/// ��ȡbuff����
		/// [2/27/2014 jianglei.kinly]
		/// 
		//size_t GetStateCountByType( xgc_uint32 buffType )const
		//{
		//	return GetStateByType( buffType ).size();
		//}

		/// 
		/// ��ȡbuff�����б�
		/// [3/4/2014 jianglei.kinly]
		/// 
		//const xObjectSet& GetStateObjectList( xgc_uint32 buffID, xgc_uint32 buffType ) const
		//{
		//	auto key = std::make_pair( buffType, buffID );
		//	auto it = mStateMap.find( key );
		//	if( it != mStateMap.end() && !it->second.empty() )
		//	{
		//		return it->second;
		//	}
		//	return static_error_res;
		//}

		/// 
		/// ��ȡ�����Ƿ���ĳ��״̬
		/// [3/5/2014 jianglei.kinly]
		/// 
		//xgc_bool HaveSomeState( xgc_uint32 buffID, xgc_uint32 buffType )
		//{
		//	return GetStateObjectList( buffID, buffType ).size() > 0;
		//}

		///
		/// �Ƿ���ս��״̬
		/// [8/29/2014] create by jianglei.kinly
		///
		xgc_bool IsFightState() { return mFightState; }

		///
		/// ����ս��״̬
		/// [8/29/2014] create by jianglei.kinly
		///
		virtual xgc_void EnterFightState( xObject hObject ) { mFightState = true; }

		///
		/// �˳�ս��״̬
		/// [8/29/2014] create by jianglei.kinly
		///
		virtual xgc_void ExitFightState() { mFightState = false; }

	protected:
		///
		/// \brief ս������ 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		virtual xgc_void beHit( AttackEvent& evt )
		{
			EmmitEvent( evt.cast, evt_actor_behit );
		}

		///
		/// \brief ս������ 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		virtual xgc_void doAttack( AttackEvent& evt )
		{
			EmmitEvent( evt.cast, evt_actor_attacked );
		}

	protected:
		/// @var ״̬���õĶ�ʱ�����
		timer_h mResetStatusTimerHandler;

	private:
		/// @var ս��״̬
		xgc_bool mFightState;
		/// @var ��ɫ��ײ�뾶
		xgc_real32 mRadius;
		/// @var ������
		xObject	mAttacker;
		/// @var �ؾ߻��߰�������
		xObject	mFriend;
		/// @var ��ǰĿ��
		xObject	mTarget;
		/// @var ������
		XVector3 mBornPoint;
		/// @var �������
		XTeamPtr mTeamPtr;
	};
}
#endif //__XCHARACTOR_H__