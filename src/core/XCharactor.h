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
	extern CORE_API xAttrIndex  attrHP;					///< ��ǰ����ֵ
	extern CORE_API xAttrIndex  attrMaxHP;				///< ��������
	extern CORE_API xAttrIndex	attrActorBeatSpeed;		///< �����ٶ�
	extern CORE_API xAttrIndex	attrActorFaintTime;		///< ѣ��ʱ��
	extern CORE_API xAttrIndex	attrActorAbnormalTime;	///< ����״̬ʱ��
	extern CORE_API xAttrIndex	attrActorBornTime;		///< ����ʱ��
	extern CORE_API xAttrIndex	attrActorGroupMask;		///< ������룬����������Ӫ
	extern CORE_API xAttrIndex  attrActorStatus;        ///< ��ɫ״̬

	enum class enActorState
	{
		sta_live,   ///< ����״̬
		sta_soul,   ///< ��״̬��������
		sta_dead,   ///< �������޷��������޷��ƶ����޷���������
	};

	enum enAttackMode
	{
		en_HurtCalc_Skill_Common = 1,   ///< ������ͨ
		en_HurtCalc_State_Common = 2,   ///< ״̬��ͨ
		en_HurtCalc_Skill_Extern = 3,   ///< ���ܶ���
		en_HurtCalc_Skill_Treat = 4,    ///< ��������
		en_HurtCalc_State_Treat = 5,    ///< ״̬����
		en_HurtCalc_Rebound = 6,        ///< ����

		en_HurtCreateDebuff = 7,        ///< ����buff

		en_HurtCalc_Skill_Treat_InDuJie = 8, ///< �ɽ���ʹ�õļ�Ѫ����

		en_HurtCalc_Else,           ///< ����
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
		abl_actor_move,
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
		evt_actor_dead,			///< ��ɫ����
		evt_actor_relive,		///< ��ɫ����
		evt_actor_change_state,	///< ��ɫ״̬ת��
		evt_actor_change_target,///< ��ɫת��Ŀ��
		evt_actor_attacked,		///< ��ɫ��������
		evt_actor_behit,		///< ��ɫ������
		evt_actor_enter_fight,	///< ����ս��״̬
		evt_actor_leave_fight,	///< �˳�ս��״̬
		evt_actor_faild_fight,	///< ��ɫս��ʧ��
		evt_actor_dead_befor,	///< ����,���ǻ�û����������״̬
	};

	///
	/// \brief ��ɫ�¼�����
	/// \author albert.xu
	/// \date 2017/10/13
	///
	struct CORE_API XCharactorEvent
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
				enAttackMode eMode;
				/// @var �˺�
				xgc_long nDamage;
				/// @var ���
				xgc_long nHate;
			}attack;

			struct
			{
				/// @var ������ʽ
				enAttackMode eMode;
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
	//	virtual xgc_void OnActorDead( XCharactor *pActor, xObject hAttacker, enAttackMode eMode, xgc_lpvoid lpContext ) = 0;

	//	///
	//	/// ��ɫ�ڳ���������
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnActorRelive( XCharactor *pActor, xgc_lpvoid lpContext ) = 0;

	//	///
	//	/// �������ж�Ա����
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnTeamJoinActor( XCharactor *pActor ) = 0;

	//	///
	//	/// �������ж�Ա�뿪
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnTeamKickActor( XCharactor *pActor ) = 0;

	//	///
	//	/// �����ڽ�ɫ�������
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnActorEnterTeam( XCharactor *pActor ) = 0;

	//	///
	//	/// �����ڽ�ɫ�뿪����
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnActorLeaveTeam( XCharactor *pActor ) = 0;

	//	///
	//	/// �����ڽ�ɫ����
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorOffline( XCharactor *pActor ) = 0;

	//	///
	//	/// �����ڽ�ɫ�뿪����
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorLeaveMap( XCharactor *pActor ) = 0;

	//	///
	//	/// ������Ʒ
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnBurstGoods( XCharactor *pActor, xObject objId ) = 0;

	//	///
	//	/// ������Ʒ����
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnDestroySceneGoods( xObject objScene, xObject objGoods ) = 0;

	//	///
	//	/// ������Ʒ
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorDropGoods( XCharactor *pActor, xObject objId ) = 0;

	//	///
	//	/// ʰȡ��Ʒ
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorPickUpGoods( XCharactor *pActor, xObject objId ) = 0;

	//	///
	//	/// �ͷŶ���
	//	/// [11/20/2014] create by albert.xu
	//	///
	//	virtual xgc_void Release() = 0;
	//};

	typedef struct
	{
		xgc_int32 iHateValue;
		xgc_time64 tUpdateTime;
	} HateInfo;

	class CORE_API XCharactor : public XObjectNode
	{
		DECLARE_XCLASS();
		friend class XGameMap;
		friend class XTeam;
		typedef xgc_unordered_map< xObject, HateInfo >	HateMap;

	public:
		XCharactor( void );
		~XCharactor( void );

		///
		/// \brief �ܵ�����
		/// \author albert.xu
		/// \date 11/19/2010
		///
		xgc_void UnderAttack( xObject hAttacker, enAttackMode eMode, xgc_lpvoid lpContext );

		///
		/// \brief ��ȡ�����ߵĽ�ɫ [11/19/2010 Albert]
		///
		XGC_INLINE xObject GetAttacker()const { return mAttacker; }

		///
		/// \brief ��ȡ���ڹ�����Ŀ�� 
		/// \author albert.xu
		/// \date 12/27/2010
		///
		XGC_INLINE xObject GetTarget()const { return mTarget; }

		///
		/// ���õ�ǰ������Ŀ��, Զ�̱��ֲ���Ҫ����λ�� 
		/// [12/27/2010 Albert]
		///
		XGC_INLINE xObject SetTarget( xObject hTarget )
		{
			xObject hOldTarget = mTarget;
			mTarget = hTarget;
			return hOldTarget;
		}

		///
		/// ��ȡ��������
		/// [11/23/2012 Albert.xu]
		///
		XGC_INLINE xObject GetFriend()const { return mFriend; }

		///
		/// ���ð�������
		/// [11/23/2012 Albert.xu]
		///
		XGC_INLINE xgc_void SetFriend( xObject hObject ) { mFriend = hObject; }

		///
		/// ��ȡ������
		/// [9/28/2012 Albert.xu]
		///
		const XVector3& GetBornPoint()const { return mBornPoint; }

		///
		/// ��ȡ������
		/// [1/7/2014 albert.xu]
		///
		xgc_void GetBornPoint( float &x, float &y, float &z ) 
		{ 
			x = mBornPoint.x; 
			y = mBornPoint.y; 
			z = mBornPoint.z; 
		}

		///
		/// ���ó�����
		/// [10/12/2012 Albert.xu]
		///
		xgc_void SetBornPoint( float x, float y, float z ) 
		{ 
			mBornPoint.x = x; 
			mBornPoint.y = y; 
			mBornPoint.z = z; 
		}

		///
		/// ��ɫ������������ڣ�����������ã�
		/// [8/21/2009 Albert]
		///
		xgc_void Dead( xObject hAttacker, enAttackMode eMode, xgc_lpvoid lpContext );

		//////
		/// ��ɫ������������ڣ� 
		/// [12/28/2010 Albert]
		//////
		xgc_void Relive( xgc_lpvoid lpContext );

		///
		/// ��ȡ��ɫ״̬
		/// [8/24/2009 Albert]
		///
		enActorState getStatus()const 
		{ 
			return enActorState( getValue<xgc_byte>( attrActorStatus ) ); \
		}

		///
		/// ���ý�ɫ״̬
		/// [8/24/2009 Albert]
		///
		xgc_void SetState( enActorState eStatus, timespan tsDuration, xgc_int32 nMode = 0 );

		///
		/// ��������״̬
		/// [5/31/2014 jianglei.kinly]
		///
		xgc_void SetState( enActorState eStatus );

		///
		/// �жϵ�ǰ״̬
		/// [9/26/2014] create by albert.xu
		///
		xgc_bool isState( enActorState eStatus )const 
		{ 
			return eStatus == getStatus(); 
		}

		///
		/// ���ò��ܹ���
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void SetEnjoinAttack()
		{
			mStatusFlags.bEnjoinAttack += 1;
		}

		///
		/// ���ò��ܱ�����
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void SetEnjoinUnderAttack()
		{
			mStatusFlags.bEnjoinUnderAttack += 1;
		}

		///
		/// ���ò����ƶ�
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void SetEnjoinMove()
		{
			mStatusFlags.bEnjoinMove += 1;
			SetMoveFlag( false );
		}

		///
		/// ���ñ�����
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void SetBeTaunt()
		{
			mStatusFlags.bBeTaunt += 1;
		}

		///
		/// ���ò���
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void SetNeverDie()
		{
			mStatusFlags.bNeverDie += 1;
		}

		///
		/// ��������Buff
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void SetImmunity( enStateType eType )
		{
			mStatusFlags.nImmunity |= eType;
		}

		///
		/// ��Ѫʱ�����
		/// [8/21/2014] create by jianglei.kinly
		///
		xgc_void SetBeHurt()
		{
			mStatusFlags.bEnjoinBeHurt += 1;
		}

		///
		/// ����Ϊ���״̬
		/// [12/23/2014] create by jianglei.kinly
		///
		xgc_void SetSoul()
		{
			mStatusFlags.bSoul = true;
		}

		///
		/// ��������buff & debuff
		/// [7/10/2015] create by jianglei.kinly
		///
		xgc_void SetImmunityBuff()
		{
			mStatusFlags.nImmunityBuffDebuff |= 1;   // xxxx1
		}

		xgc_void SetImmunityDeBuff()
		{
			mStatusFlags.nImmunityBuffDebuff |= 2;   // xxx1x
		}

		///
		/// ������״̬
		/// [6/18/2014] create by jianglei.kinly
		///
		///
		/// �����ò��ܹ���
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void AntiSetEnjoinAttack()
		{
			if( mStatusFlags.bEnjoinAttack > 0 )
				mStatusFlags.bEnjoinAttack -= 1;
		}

		///
		/// �����ò��ܱ�����
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void AntiSetEnjoinUnderAttack()
		{
			if( mStatusFlags.bEnjoinUnderAttack > 0 )
				mStatusFlags.bEnjoinUnderAttack -= 1;
		}

		///
		/// �����ò����ƶ�
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void AntiSetEnjoinMove()
		{
			if( mStatusFlags.bEnjoinMove > 0 )
				mStatusFlags.bEnjoinMove -= 1;
			if( mStatusFlags.bEnjoinMove == 0 )
				SetMoveFlag( true );
		}

		///
		/// �����ñ�����
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void AntiSetBeTaunt()
		{
			if( mStatusFlags.bBeTaunt > 0 )
				mStatusFlags.bBeTaunt -= 1;
		}

		///
		/// �����ò���
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void AntiSetNeverDie()
		{
			if( mStatusFlags.bNeverDie > 0 )
				mStatusFlags.bNeverDie -= 1;
		}

		///
		/// ����������Buff
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void AntiSetImmunity( enStateType eType )
		{
			mStatusFlags.nImmunity &= ~eType;
		}

		///
		/// �����ÿ�Ѫʱ�����
		/// [8/21/2014] create by jianglei.kinly
		///
		xgc_void AntiSetBeHurt()
		{
			if( mStatusFlags.bEnjoinBeHurt > 0 )
				mStatusFlags.bEnjoinBeHurt -= 1;
		}

		///
		/// ���������״̬
		/// [12/23/2014] create by jianglei.kinly
		///
		xgc_void AntiSetSoul()
		{
			mStatusFlags.bSoul = false;
		}

		///
		/// ��������buff & debuff
		/// [7/10/2015] create by jianglei.kinly
		///
		xgc_void AntiSetImmunityBuff()
		{
			mStatusFlags.nImmunityBuffDebuff &= ~1;   // xxxxx0
		}
		///
		/// ��������buff & debuff
		/// [7/10/2015] create by jianglei.kinly
		///
		xgc_void AntiSetImmunityDeBuff()
		{
			mStatusFlags.nImmunityBuffDebuff &= ~2;   // xxxx0x
		}

		///
		/// �Ƿ�ɹ���
		/// [6/18/2014] create by jianglei.kinly
		///
		virtual xgc_bool CanAttack()const
		{
			return mStatusFlags.bEnjoinAttack == 0;
		}

		///
		/// �Ƿ�ɱ�����
		/// [6/18/2014] create by jianglei.kinly
		///
		xgc_bool CanUnderAttack()const
		{
			return mStatusFlags.bEnjoinUnderAttack == 0;
		}

		///
		/// �Ƿ���ƶ�
		/// [6/18/2014] create by jianglei.kinly
		///
		xgc_bool CanMove()const
		{
			return mStatusFlags.bEnjoinMove == 0;
		}

		///
		/// �Ƿ��Ǳ�����״̬
		/// [6/18/2014] create by jianglei.kinly
		///
		xgc_bool IsBeTaunt()const
		{
			return mStatusFlags.bBeTaunt != 0;
		}

		///
		/// �Ƿ���
		/// [6/18/2014] create by jianglei.kinly
		///
		xgc_bool IsNeverDie()const
		{
			return mStatusFlags.bNeverDie != 0;
		}

		///
		/// �ܷ�����������͵�Buff
		/// [6/20/2014] create by jianglei.kinly
		///
		xgc_bool IsImmunityState( enStateType stateType ) const
		{
			return ( mStatusFlags.nImmunity & stateType ) == 0 ? false : true;
		}

		xgc_bool IsImmunityStateBuff() const
		{
			return ( mStatusFlags.nImmunityBuffDebuff & 1 ) == 0 ? false : true;
		}

		xgc_bool IsImmunityStateDeBuff() const
		{
			return ( mStatusFlags.nImmunityBuffDebuff & 2 ) == 0 ? false : true;
		}

		///
		/// �Ƿ���Կ�Ѫ
		/// [8/21/2014] create by jianglei.kinly
		///
		xgc_bool IsBeHurt()const
		{
			return mStatusFlags.bEnjoinBeHurt == 0;
		}

		///
		/// �Ƿ������״̬
		/// [12/23/2014] create by jianglei.kinly
		///
		xgc_bool IsSoul() const
		{
			return mStatusFlags.bSoul;
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

		/// 
		/// �������б�
		/// [24/4/2014 zhangyupeng]
		/// 
		xgc_void UpdateHateMap( xObject hAttacker, xgc_int32 nHateValue );

		/// 
		/// \brief Copy����б�,ԭ��ѭ������update�����Ǽ���ʱ���֮��update����ʱ����£�hatemap����һ��
		/// [24/4/2014 zhangyupeng]
		/// 
		xgc_void CopyHateMap( xObject hTarget );

		/// 
		/// \brief �ӳ���б�ɾ��
		/// [24/4/2014 zhangyupeng]
		/// 
		xgc_bool DeleteFromHateMap( xObject hAttacker = INVALID_OBJECT_ID );

		/// 
		/// \brief ���ɾ�����Ƿ��������⺬��Ķ��������С�׻���
		/// [24/4/2014 zhangyupeng]
		/// 
		xgc_void CheckSpecialHate( xObject hAttacker );

		///
		/// \brief ��ȡĿ���ڳ���б��еĳ��ֵ
		/// [24/4/2014 zhangyupeng]
		///
		xgc_int32 GetHateValue( xObject hAttacker );

		/// 
		/// ��ȡ���ĳ��ֵ
		/// [24/4/2014 zhangyupeng]
		/// 
		xgc_int32 GetMaxHateValue() { return mMaxHateValue; }

		/// 
		/// ��ȡ��С�ĳ��ֵ
		/// [24/4/2014 zhangyupeng]
		/// 
		xgc_int32 GetMinHateValue() { return mMinHateValue; }

		/// 
		/// ��ȡ���ĳ��Ŀ��
		/// [24/4/2014 zhangyupeng]
		/// 
		xObject GetMaxHateTarget() { return mMaxHateTarget; }

		/// 
		/// ��ȡ��С�ĳ��ֵ
		/// [24/4/2014 zhangyupeng]
		/// 
		xObject GetMinHateTarget() { return mMinHateTarget; }

		/// 
		/// ѡȡxTarget����һ��Ŀ��
		/// [24/4/2014 zhangyupeng]
		/// 
		xObject GetRandomTarget();

		/// 
		/// ѡȡxTarget����һ��Ŀ��
		/// [24/4/2014 zhangyupeng]
		/// 
		xObject GetNextAsTarget( xObject xTarget );

		/// 
		/// �������б�
		/// [24/4/2014 zhangyupeng]
		/// 
		xgc_void ClearHateMap();

		/// 
		/// ��õ�һ�������Ҹ�Ŀ��
		/// [24/4/2014 zhangyupeng]
		/// 
		xObject GetFirstAttackMeTarget() { return mFirstAttackMeTarget; }

		/// 
		/// ���õ�һ�������Ҹ�Ŀ��
		/// [24/4/2014 zhangyupeng]
		/// 
		xgc_void SetFirstAttackMeTarget( xObject xTarget ) { mFirstAttackMeTarget = xTarget; }
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
		xgc_uint32		mEyeshotGroup;	///< ��Ұ��
		timer_h			mResetStatusTimerHandler;
		xObject			mFirstAttackMeTarget;	///< ��һ�������ҵ����,һ���ڳ���б��У����ǵ�һ���������б��Ŀ�겻һ�����׻���
		struct
		{
			xgc_uint16	bEnjoinAttack;      ///< �Ƿ��ܹ�����0�����Թ����������������Թ�����
			xgc_uint16	bEnjoinUnderAttack; ///< �Ƿ��ܱ�������0�����Ա������������������Ա�������
			xgc_uint16	bEnjoinMove;        ///< �Ƿ����ƶ���0�������ƶ����������������ƶ���
			xgc_uint16	bBeTaunt;           ///< �Ƿ񱻳���0��û�������������������ˣ�
			xgc_uint16	bNeverDie;          ///< �Ƿ���������������ֵ��ͽ���1����0�������ߣ����������ߣ�
			xgc_uint16	bEnjoinBeHurt;      ///< �Ƿ��ܵ��˺�ʱ�򲻿�Ѫ��������Ϣ����0���ǣ��������ǣ�
			xgc_uint32	nImmunity;          ///< ���ߵ�buff���ͣ���λ����A��Buff ���� B��Buff����������Bf��type��
			xgc_bool    bSoul;              ///< �Ƿ������״̬��false:����  true:�ǣ�
			xgc_uint16  nImmunityBuffDebuff; ///< ���ߵ�buff ��1������buff 2������debuff��
		}mStatusFlags;	///< ��ǰ״̬��־λ

	protected:
		HateMap			mHateMap;
		xObject			mMaxHateTarget;		///< ���ֵ�������
		xObject			mMinHateTarget;		///< ���ֵ��С�����
		xgc_int32		mMaxHateValue;
		xgc_int32		mMinHateValue;

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