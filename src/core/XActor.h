#pragma once
#ifndef __XCHARACTOR_H__
#define __XCHARACTOR_H__
#include "XTeam.h"

namespace xgc
{
	//////////////////////////////////////////////////////////////////////////
	// ��ɫ��������
	extern CORE_API xAttrIndex	attrActorIndex;			///< ��ɫ��������
	extern CORE_API xAttrIndex	attrActorType;			///< ��ɫ����
	extern CORE_API xAttrIndex  attrActorHP;			///< ��ǰ����ֵ
	extern CORE_API xAttrIndex  attrActorHP_Max;		///< ��������
	extern CORE_API xAttrIndex	attrActorBeatSpeed;		///< �����ٶ�
	extern CORE_API xAttrIndex	attrActorFaintTime;		///< ѣ��ʱ��
	extern CORE_API xAttrIndex	attrActorAbnormalTime;	///< ����״̬ʱ��
	extern CORE_API xAttrIndex	attrActorBornTime;		///< ����ʱ��
	extern CORE_API xAttrIndex	attrActorGroupMask;		///< ������룬����������Ӫ
	extern CORE_API xAttrIndex  attrActorStatus;        ///< ��ɫ״̬
	extern CORE_API xAttrIndex attrActorCanMove;		///< ���ƶ�
	extern CORE_API xAttrIndex attrActorCanAttack;		///< �ɹ���
	extern CORE_API xAttrIndex attrActorCanBeHit;		///< ���ܻ�
	extern CORE_API xAttrIndex attrActorCanBeTanunt;	///< �ɳ���
	extern CORE_API xAttrIndex attrActorCanDead;		///< ������
	extern CORE_API xAttrIndex attrActorCanHurt;		///< ������

	enum class enActorState
	{
		sta_live,   ///< ����״̬
		sta_soul,   ///< ��״̬��������
		sta_dead,   ///< �������޷��������޷��ƶ����޷���������
	};

	enum enActorStatus
	{
		sta_actor_damage = 1,
		sta_actor_pain = 2,
		sta_actor_faint = 4,
		sta_actor_fall = 8,
		sta_actor_beat = 16,
		sta_actor_dead = 32,
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
		evt_actor_behit,		///< ��ɫ������
		evt_actor_enter_fight,	///< ����ս��״̬
		evt_actor_leave_fight,	///< �˳�ս��״̬
		evt_actor_faild_fight,	///< ��ɫս��ʧ��
		evt_actor_befor_dead,	///< ��ɫ����
		evt_actor_dead,			///< ��ɫ����
	};

	///
	/// \brief ��ɫ�¼�����
	/// \author albert.xu
	/// \date 2017/10/13
	///
	struct CORE_API XActorEvent
	{
		/// @var �¼�
		XObjectEvent cast;

		/// @var �����ߵĶ���ID
		xObject hAttacker;

		/// @var ������
		xgc_lpvoid lpContext;

		union
		{
			struct
			{
				/// @var ������ʽ
				xgc_long nMode;
				/// @var �˺�
				xgc_long nDamage;
				/// @var ���
				xgc_long nHate;
			}attack;

			struct
			{
				/// @var ������ʽ
				xgc_long nMode;
			}dead;
		};
	};

	/////------
	///// ��ɫ�ڳ����е�����¼�
	/////------
	//struct IActorMapEventHandler
	//{
	//	///
	//	/// ��ɫ�ڳ���������
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnActorDead( XActor *pActor, xObject hAttacker, enAttackMode eMode, xgc_lpvoid lpContext ) = 0;

	//	///
	//	/// ��ɫ�ڳ���������
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnActorRelive( XActor *pActor, xgc_lpvoid lpContext ) = 0;

	//	///
	//	/// �������ж�Ա����
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnTeamJoinActor( XActor *pActor ) = 0;

	//	///
	//	/// �������ж�Ա�뿪
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnTeamKickActor( XActor *pActor ) = 0;

	//	///
	//	/// �����ڽ�ɫ�������
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnActorEnterTeam( XActor *pActor ) = 0;

	//	///
	//	/// �����ڽ�ɫ�뿪����
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnActorLeaveTeam( XActor *pActor ) = 0;

	//	///
	//	/// �����ڽ�ɫ����
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorOffline( XActor *pActor ) = 0;

	//	///
	//	/// �����ڽ�ɫ�뿪����
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorLeaveMap( XActor *pActor ) = 0;

	//	///
	//	/// ������Ʒ
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnBurstGoods( XActor *pActor, xObject objId ) = 0;

	//	///
	//	/// ������Ʒ����
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnDestroySceneGoods( xObject objScene, xObject objGoods ) = 0;

	//	///
	//	/// ������Ʒ
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorDropGoods( XActor *pActor, xObject objId ) = 0;

	//	///
	//	/// ʰȡ��Ʒ
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorPickUpGoods( XActor *pActor, xObject objId ) = 0;

	//	///
	//	/// �ͷŶ���
	//	/// [11/20/2014] create by albert.xu
	//	///
	//	virtual xgc_void Release() = 0;
	//};

	class CORE_API XActor : public XObjectNode
	{
		DECLARE_XCLASS();
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
		enActorState getStatus()const 
		{ 
			return enActorState( getValue<xgc_byte>( attrActorStatus ) ); \
		}

		///
		/// \brief ���ý�ɫ״̬
		/// \date 8/24/2009 
		/// \author albert.xu
		///
		xgc_void SetState( enActorState eStatus, timespan tsDuration, xgc_int32 nMode = 0 );

		///
		/// \brief ��������״̬
		/// \date 5/31/2014 
		/// \author jianglei.kinly
		///
		xgc_void SetState( enActorState eStatus );

		///
		/// \brief �жϵ�ǰ״̬
		/// \date 9/26/2014
		/// \author albert.xu
		///
		xgc_bool isState( enActorState eStatus )const 
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
		
			incValue( attrActorCanMove + eAbility, bEnable ? -1 : +1 );
		}

		///
		/// \brief ������������
		/// \date 6/22/2014
		/// \author jianglei.kinly
		///
		xgc_void resetAbility( enActorAbility eAbility )
		{
			XGC_ASSERT_RETURN( eAbility < abl_actor_count, XGC_NONE );

			setValue( attrActorCanMove + eAbility, 0 );
		}

		///
		/// \brief �����Ƿ����
		/// \date 6/22/2014
		/// \author jianglei.kinly
		///
		xgc_bool isActivedAbility( enActorAbility eAbility )
		{
			XGC_ASSERT_RETURN( eAbility < abl_actor_count, false );

			return 0 == getValue< xgc_int32 >( attrActorCanMove + eAbility );
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
		/// get team object ptr
		/// [11/12/2012 Albert.xu]
		///
		XTeamPtr getTeam() 
		{ 
			return mTeamPtr; 
		}

		///
		/// set team object ptr
		/// [11/22/2012 Albert.xu]
		///
		xgc_void setTeam( XTeamPtr team ) 
		{ 
			mTeamPtr = team; 
		}

		///
		/// get team object ptr const version
		/// [11/22/2012 Albert.xu]
		///
		const XTeamPtr getTeam()const 
		{ 
			return mTeamPtr; 
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

	public:
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
		/// \brief ���ý�ɫ״̬ 
		/// \author albert.xu
		/// \date 12/23/2010
		///
		xgc_void ResetActorState( enActorState eStatus );

	protected:
		timer_h			mResetStatusTimerHandler;

	private:
		xgc_bool		mFightState;		///< ս��״̬
		xgc_real32		mRadius;

		enActorState	mActorRestonState;	///< ��ɫ����״̬

		xObject			mAttacker;			///< ������
		xObject			mFriend;			///< �ؾ߻��߰�������
		xObject			mTarget;			///< ��ǰĿ��

		XVector3		mBornPoint;

		XTeamPtr		mTeamPtr;

		datetime		mLastAttackTime;    ///< ��һ�γ��Թ���������ɫ
		datetime		mLastAttackedTime;  ///< ��һ�α�������ɫ���Թ���
	};
}
#endif //__XCHARACTOR_H__