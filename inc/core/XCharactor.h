#pragma once
#ifndef __XCHARACTOR_H__
#define __XCHARACTOR_H__
#include "XTeam.h"
#include "XGameObject.h"
#include "XSkill.h"
//#include "XState.h"

namespace xgc
{
	//////////////////////////////////////////////////////////////////////////
	// 角色属性索引
	extern CORE_API xAttrIndex	attrActorIndex;			///< 角色配置索引
	extern CORE_API xAttrIndex	attrActorType;			///< 角色类型
	extern CORE_API xAttrIndex  attrHP;					///< 当前生命值
	extern CORE_API xAttrIndex  attrMaxHP;				///< 生命上限
	extern CORE_API xAttrIndex	attrActorBeatSpeed;		///< 击退速度
	extern CORE_API xAttrIndex	attrActorFaintTime;		///< 眩晕时间
	extern CORE_API xAttrIndex	attrActorAbnormalTime;	///< 特殊状态时间
	extern CORE_API xAttrIndex	attrActorBornTime;		///< 出生时间
	extern CORE_API xAttrIndex	attrActorGroupMask;		///< 组别掩码，用于区分阵营
	extern CORE_API xAttrIndex  attrActorStatus;        ///< 角色状态

	class XCharactor;
	// ------------------------------------------------ //
	// [3/20/2014 jianglei.kinly]
	// 技能攻击的额外参数
	// ------------------------------------------------ //
	struct AttackExtraInfo
	{
		xgc_uint8   m_byAttackSection;
		xgc_uint8   m_byExFlag;
		xgc_real32  m_fX1;
		xgc_real32  m_fY1;
		xgc_uint16  m_wDir1;
		xgc_real32  m_fX2;
		xgc_real32  m_fY2;
		xgc_uint16  m_wDir2;
		xgc_real32  m_fX3;
		xgc_real32  m_fY3;
		xgc_uint16  m_wDir3;
		xgc_real32  m_fAimX;
		xgc_real32  m_fAimY;
		xgc_uint16  m_wAimDir;

		AttackExtraInfo()
			: m_byAttackSection( 0 )
			, m_byExFlag( 0 )
			, m_fX1( 0.0f )
			, m_fY1( 0.0f )
			, m_wDir1( 0 )
			, m_fX2( 0.0f )
			, m_fY2( 0.0f )
			, m_wDir2( 0 )
			, m_fX3( 0.0f )
			, m_fY3( 0.0f )
			, m_wDir3( 0 )
			, m_fAimX( 0.0f )
			, m_fAimY( 0.0f )
			, m_wAimDir( 0 )
		{
		}

		AttackExtraInfo& operator = ( AttackExtraInfo& attackExtraInfo )
		{
			m_byAttackSection = attackExtraInfo.m_byAttackSection;
			m_byExFlag = attackExtraInfo.m_byExFlag;
			m_fX1 = attackExtraInfo.m_fX1;
			m_fY1 = attackExtraInfo.m_fY1;
			m_wDir1 = attackExtraInfo.m_wDir1;
			m_fX2 = attackExtraInfo.m_fX2;
			m_fY2 = attackExtraInfo.m_fY2;
			m_wDir2 = attackExtraInfo.m_wDir2;
			m_fX3 = attackExtraInfo.m_fX3;
			m_fY3 = attackExtraInfo.m_fY3;
			m_wDir3 = attackExtraInfo.m_wDir3;
			m_fAimX = attackExtraInfo.m_fAimX;
			m_fAimY = attackExtraInfo.m_fAimY;
			m_wAimDir = attackExtraInfo.m_wAimDir;
			return *this;
		}
	};

	///
	/// \brief 角色事件对象
	/// \author albert.xu
	/// \date 2017/10/13
	///
	struct CORE_API XCharactorEvent
	{
		/// @var 事件
		XObjectEvent cast;

		/// @var 上下文
		xgc_lpvoid lpContext;
		/// @var 攻击者的对象ID
		xObject hAttacker;
	};

	enum ActorStatus
	{
		ActorStatus_Born,	///< 出生状态
		ActorStatus_Live,   ///< 正常状态
		ActorStatus_Soul,   ///< 魂状态（濒死）
		ActorStatus_Dead,   ///< 死亡（无法攻击、无法移动、无法被攻击）
		ActorStatus_Relive, ///< 复活状态
	};

	enum ActorAttackMode
	{
		en_HurtCalc_Skill_Common = 1,   ///< 技能普通
		en_HurtCalc_State_Common = 2,   ///< 状态普通
		en_HurtCalc_Skill_Extern = 3,   ///< 技能额外
		en_HurtCalc_Skill_Treat = 4,    ///< 技能治疗
		en_HurtCalc_State_Treat = 5,    ///< 状态治疗
		en_HurtCalc_Rebound = 6,        ///< 反弹

		en_HurtCreateDebuff = 7,        ///< 减益buff

		en_HurtCalc_Skill_Treat_InDuJie = 8, ///< 渡劫中使用的加血技能

		en_HurtCalc_Else,           ///< 其他
	};

	enum ActorEvent
	{
		ActorEvent_Damage = 1,
		ActorEvent_Pain = 2,
		ActorEvent_Faint = 4,
		ActorEvent_Fall = 8,
		ActorEvent_Beat = 16,
		ActorEvent_Dead = 32,
	};

	enum eCharactorEvent
	{
		evt_actor_dead,			///< 角色死亡
		evt_actor_relive,		///< 角色重生
		evt_actor_change_state,	///< 角色状态转换
		evt_actor_change_target,///< 角色转换目标
		evt_actor_under_attack,	///< 角色被攻击
		evt_actor_enter_fight,	///< 进入战斗状态
		evt_actor_leave_fight,	///< 退出战斗状态
		evt_actor_faild_fight,	///< 角色战斗失败
		evt_actor_dead_befor,	///< 死亡,但是还没有设置死亡状态
	};

	///
	/// 活动阵营颜色
	/// [5/6/2015] create by jianglei.kinly
	///
	enum en_activeCampColor : xgc_uint16
	{
		en_activeCamp_none = 0,
		en_activeCamp_red = 1,
		en_activeCamp_purple = 2,
		en_activeCamp_blue = 3,

		en_activeCamp_max,
	};

	/////------
	///// 角色在场景中的相关事件
	/////------
	//struct IActorMapEventHandler
	//{
	//	///
	//	/// 角色在场景内死亡
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnActorDead( XCharactor *pActor, xObject hAttacker, ActorAttackMode eMode, xgc_lpvoid lpContext ) = 0;

	//	///
	//	/// 角色在场景内死亡
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnActorRelive( XCharactor *pActor, xgc_lpvoid lpContext ) = 0;

	//	///
	//	/// 队伍内有队员进入
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnTeamJoinActor( XCharactor *pActor ) = 0;

	//	///
	//	/// 队伍内有队员离开
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnTeamKickActor( XCharactor *pActor ) = 0;

	//	///
	//	/// 场景内角色加入队伍
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnActorEnterTeam( XCharactor *pActor ) = 0;

	//	///
	//	/// 场景内角色离开队伍
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnActorLeaveTeam( XCharactor *pActor ) = 0;

	//	///
	//	/// 场景内角色下线
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorOffline( XCharactor *pActor ) = 0;

	//	///
	//	/// 场景内角色离开场景
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorLeaveMap( XCharactor *pActor ) = 0;

	//	///
	//	/// 暴落物品
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnBurstGoods( XCharactor *pActor, xObject objId ) = 0;

	//	///
	//	/// 场景物品销毁
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnDestroySceneGoods( xObject objScene, xObject objGoods ) = 0;

	//	///
	//	/// 丢弃物品
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorDropGoods( XCharactor *pActor, xObject objId ) = 0;

	//	///
	//	/// 拾取物品
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorPickUpGoods( XCharactor *pActor, xObject objId ) = 0;

	//	///
	//	/// 释放对象
	//	/// [11/20/2014] create by albert.xu
	//	///
	//	virtual xgc_void Release() = 0;
	//};

	typedef struct
	{
		xgc_int32 iHateValue;
		xgc_time64 tUpdateTime;
	} HateInfo;

	class CORE_API XCharactor : public XGameObject
	{
		DECLARE_XCLASS();
		friend class XGameMap;
		friend class XTeam;
		typedef xgc_unordered_map< xObject, HateInfo >	HateMap;

	protected:
		// SkillMainId, 位置
		typedef xgc_unordered_map< xgc_uint32, xObject > SkillMap;

		///< 状态存储的KEY，buffType, buffMainID
		typedef xgc_map< std::pair< xgc_uint32, xgc_uint32 >, xgc_unordered_set<xObject> > StateMap;
	public:
		XCharactor( void );
		~XCharactor( void );

		///
		/// 根据PK模式判断是否是友方
		/// [6/13/2014] create by jianglei.kinly
		///
		virtual xgc_bool CheckIsFriend( xObject xobj ) { return false; }

		//////
		/// 受到攻击 [11/19/2010 Albert]（被攻击入口）
		//////
		xgc_void UnderAttack( xObject hAttacker, ActorAttackMode eMode, xgc_lpvoid lpContext );

		//////
		/// 获取仇恨最高的角色 [11/19/2010 Albert]
		//////
		XGC_INLINE xObject GetAttacker()const { return mAttacker; }

		//////
		/// 获取正在攻击的目标 
		/// [12/27/2010 Albert]
		//////
		XGC_INLINE xObject GetTarget()const { return mTarget; }

		//////
		/// 设置当前攻击的目标, 远程兵种不需要设置位置 
		/// [12/27/2010 Albert]
		//////
		XGC_INLINE xObject SetTarget( xObject hTarget )
		{
			xObject hOldTarget = mTarget;
			mTarget = hTarget;
			return hOldTarget;
		}

		////
		/// 获取伴生对象
		/// [11/23/2012 Albert.xu]
		////
		XGC_INLINE xObject GetFriend()const { return mFriend; }

		////
		/// 设置伴生对象
		/// [11/23/2012 Albert.xu]
		////
		XGC_INLINE xgc_void SetFriend( xObject hObject ) { mFriend = hObject; }

		////
		/// 获取重生点
		/// [9/28/2012 Albert.xu]
		////
		const XVector3& GetBornPoint()const { return mBornPoint; }

		////
		/// 获取重生点
		/// [1/7/2014 albert.xu]
		////
		xgc_void GetBornPoint( float &x, float &y, float &z ) { x = mBornPoint.x; y = mBornPoint.y; z = mBornPoint.z; }

		////
		/// 设置出生点
		/// [10/12/2012 Albert.xu]
		////
		xgc_void SetBornPoint( float x, float y, float z ) { mBornPoint.x = x; mBornPoint.y = y; mBornPoint.z = z; }

		//////
		/// 角色死亡（死亡入口，被攻击后调用）
		/// [8/21/2009 Albert]
		//////
		xgc_void Dead( xObject hAttacker, ActorAttackMode eMode, xgc_lpvoid lpContext );

		//////
		/// 角色重生（复活入口） 
		/// [12/28/2010 Albert]
		//////
		xgc_void Relive( xgc_lpvoid lpContext );

		//////
		/// 获取角色状态
		/// [8/24/2009 Albert]
		//////
		ActorStatus GetActorStatus()const { return ActorStatus( getValue<xgc_byte>( attrActorStatus ) ); }

		//////
		/// 设置角色状态
		/// [8/24/2009 Albert]
		//////
		xgc_void SetActorStatus( ActorStatus eStatus, xgc_real32 fTime, xgc_int32 nMode = 0 );

		/// ------------------------------------------------ ///
		/// 设置生物状态
		/// [5/31/2014 jianglei.kinly]
		/// ------------------------------------------------ ///
		xgc_void SetActorStatus( ActorStatus eStatus );

		///
		/// 判断当前状态
		/// [9/26/2014] create by albert.xu
		///
		xgc_bool IsActorStatus( ActorStatus eStatus )const { return eStatus == GetActorStatus(); }

		///
		/// 设置状态
		/// [6/18/2014] create by jianglei.kinly
		///
		///
		/// 设置不能攻击
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void SetEnjoinAttack()
		{
			mStatusFlags.bEnjoinAttack += 1;
		}

		///
		/// 设置不能被攻击
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void SetEnjoinUnderAttack()
		{
			mStatusFlags.bEnjoinUnderAttack += 1;
		}
		///
		/// 设置不能移动
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void SetEnjoinMove()
		{
			mStatusFlags.bEnjoinMove += 1;
			SetMoveFlag( false );
		}
		///
		/// 设置被嘲讽
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void SetBeTaunt()
		{
			mStatusFlags.bBeTaunt += 1;
		}
		///
		/// 设置不死
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void SetNeverDie()
		{
			mStatusFlags.bNeverDie += 1;
		}

		///
		/// 设置免疫Buff
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void SetImmunity( enStateType eType )
		{
			mStatusFlags.nImmunity |= eType;
		}
		///
		/// 扣血时候忽略
		/// [8/21/2014] create by jianglei.kinly
		///
		xgc_void SetBeHurt()
		{
			mStatusFlags.bEnjoinBeHurt += 1;
		}
		///
		/// 设置为灵魂状态
		/// [12/23/2014] create by jianglei.kinly
		///
		xgc_void SetSoul()
		{
			mStatusFlags.bSoul = true;
		}

		///
		/// 设置阵营颜色
		/// [5/6/2015] create by jianglei.kinly
		///
		xgc_void SetCampColor( en_activeCampColor en )
		{
			mStatusFlags.enCampColor = en;
		}
		///
		/// 设置免疫buff & debuff
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
		/// 反设置状态
		/// [6/18/2014] create by jianglei.kinly
		///
		///
		/// 反设置不能攻击
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void AntiSetEnjoinAttack()
		{
			if( mStatusFlags.bEnjoinAttack > 0 )
				mStatusFlags.bEnjoinAttack -= 1;
		}
		///
		/// 反设置不能被攻击
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void AntiSetEnjoinUnderAttack()
		{
			if( mStatusFlags.bEnjoinUnderAttack > 0 )
				mStatusFlags.bEnjoinUnderAttack -= 1;
		}
		///
		/// 反设置不能移动
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
		/// 反设置被嘲讽
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void AntiSetBeTaunt()
		{
			if( mStatusFlags.bBeTaunt > 0 )
				mStatusFlags.bBeTaunt -= 1;
		}

		///
		/// 反设置不死
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void AntiSetNeverDie()
		{
			if( mStatusFlags.bNeverDie > 0 )
				mStatusFlags.bNeverDie -= 1;
		}

		///
		/// 反设置免疫Buff
		/// [6/22/2014] create by jianglei.kinly
		///
		xgc_void AntiSetImmunity( enStateType eType )
		{
			mStatusFlags.nImmunity &= ~eType;
		}

		///
		/// 反设置扣血时候忽略
		/// [8/21/2014] create by jianglei.kinly
		///
		xgc_void AntiSetBeHurt()
		{
			if( mStatusFlags.bEnjoinBeHurt > 0 )
				mStatusFlags.bEnjoinBeHurt -= 1;
		}
		///
		/// 反设置灵魂状态
		/// [12/23/2014] create by jianglei.kinly
		///
		xgc_void AntiSetSoul()
		{
			mStatusFlags.bSoul = false;
		}

		///
		/// 反设置阵营颜色
		/// [5/6/2015] create by jianglei.kinly
		///
		xgc_void AntiSetCampColor()
		{
			mStatusFlags.enCampColor = en_activeCampColor::en_activeCamp_none;
		}
		///
		/// 设置免疫buff & debuff
		/// [7/10/2015] create by jianglei.kinly
		///
		xgc_void AntiSetImmunityBuff()
		{
			mStatusFlags.nImmunityBuffDebuff &= ~1;   // xxxxx0
		}
		///
		/// 设置免疫buff & debuff
		/// [7/10/2015] create by jianglei.kinly
		///
		xgc_void AntiSetImmunityDeBuff()
		{
			mStatusFlags.nImmunityBuffDebuff &= ~2;   // xxxx0x
		}


		///
		/// 是否可攻击
		/// [6/18/2014] create by jianglei.kinly
		///
		virtual xgc_bool CanAttack()const
		{
			return mStatusFlags.bEnjoinAttack == 0;
		}

		///
		/// 是否可被攻击
		/// [6/18/2014] create by jianglei.kinly
		///
		xgc_bool CanUnderAttack()const
		{
			return mStatusFlags.bEnjoinUnderAttack == 0;
		}

		///
		/// 是否可移动
		/// [6/18/2014] create by jianglei.kinly
		///
		xgc_bool CanMove()const
		{
			return mStatusFlags.bEnjoinMove == 0;
		}

		///
		/// 是否是被嘲讽状态
		/// [6/18/2014] create by jianglei.kinly
		///
		xgc_bool IsBeTaunt()const
		{
			return mStatusFlags.bBeTaunt != 0;
		}

		///
		/// 是否不死
		/// [6/18/2014] create by jianglei.kinly
		///
		xgc_bool IsNeverDie()const
		{
			return mStatusFlags.bNeverDie != 0;
		}

		///
		/// 能否免疫这个类型的Buff
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
		/// 是否可以扣血
		/// [8/21/2014] create by jianglei.kinly
		///
		xgc_bool IsBeHurt()const
		{
			return mStatusFlags.bEnjoinBeHurt == 0;
		}

		///
		/// 是否是灵魂状态
		/// [12/23/2014] create by jianglei.kinly
		///
		xgc_bool IsSoul() const
		{
			return mStatusFlags.bSoul;
		}

		///
		/// 获得当前阵营颜色
		/// [5/6/2015] create by jianglei.kinly
		///
		en_activeCampColor GetCampColor()
		{
			return mStatusFlags.enCampColor;
		}

		/////
		/// [8/21/2009 Albert]
		/// Description:	获取视野组
		/////
		xgc_uint32 GetEyeshotGroup()const { return mEyeshotGroup; }

		/////
		/// 获取技能个数 
		/// [11/19/2010 Albert]
		/////
		size_t GetSkillCount()const
		{
			return mSkillMap.size();
		}

		/// ------------------------------------------------ //
		/// 获取技能map
		/// [3/26/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		const SkillMap& GetAllSkill()
		{
			return mSkillMap;
		}

		///
		/// 通过技能主ID获取技能句柄
		/// [11/1/2012 Albert.xu]
		///
		xObject GetSkillByMainID( xgc_uint32 nTableMainID )const
		{
			auto result = mSkillMap.find( nTableMainID );
			if( result != mSkillMap.end() )
				return result->second;

			return INVALID_OBJECT_ID;
		}

		///
		/// 设置当前技能
		/// [11/9/2012 Albert.xu]
		///
		xgc_void SetCurrentSkill( xObject hSkill, xObject hTarget, XVector3 vCastPoint, AttackExtraInfo attackextrainfo = AttackExtraInfo() )
		{
			// CLEAR 的时候传的就是 INVALID_OBJECT_ID
			// XGC_ASSERT_RETURN( mCurrentSkill == INVALID_OBJECT_ID, xgc_void( 0 ) );
			mCurrentSkill = hSkill;
			mCurrentSkillTarget = hTarget;
			mCurrentSkillPoint = vCastPoint;
			mCurrentSkillExtraInfo = attackextrainfo;
		}

		xgc_void SetCurrentSkillAttackExtraInfo( AttackExtraInfo attackextrainfo )
		{
			mCurrentSkillExtraInfo = attackextrainfo;
		}

		xObject GetCurrentSkill()const { return mCurrentSkill; }
		xObject GetCurrentSkillTarget()const { return mCurrentSkillTarget; }
		const XVector3& GetCurrentSkillPoint()const { return mCurrentSkillPoint; }
		const AttackExtraInfo& GetCurrentSkillAttackExtraInfo()const { return mCurrentSkillExtraInfo; }

		///
		/// 设置角色碰撞半径
		/// [10/23/2012 Albert.xu]
		///
		xgc_void SetRadius( xgc_real32 fRadius ) { mRadius = fRadius; }

		///
		/// 获取角色碰撞半径
		/// [10/24/2012 Albert.xu]
		///
		xgc_real32 GetRadius()const { return mRadius; }

		///
		/// get team object ptr
		/// [11/12/2012 Albert.xu]
		///
		XTeamPtr getTeam() { return mTeamPtr; }
		xgc_void setTeam( XTeamPtr team ) { mTeamPtr = team; }

		///
		/// get team object ptr const version
		/// [11/22/2012 Albert.xu]
		///
		const XTeamPtr getTeam()const { return mTeamPtr; }

		///
		/// get team slot
		/// [11/22/2012 Albert.xu]
		///
		xgc_uint16 getTeamSlot()const { return mTeamSlot; }

		///
		/// 根据Buff类型查找buff列表
		/// [8/12/2014] create by jianglei.kinly
		///
		xObjectSet GetStateByType( xgc_uint32 type ) const
		{
			xObjectSet resSet;
			for( auto& it01 : mStateMap )
			{
				if( ( it01.first.first & type ) == type )
					resSet.insert( it01.second.begin(), it01.second.end() );
			}

			return std::move( resSet );
		}

		/// ------------------------------------------------ //
		/// 获取buff个数
		/// [2/27/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		size_t GetStateCountByType( xgc_uint32 buffType )const
		{
			return GetStateByType( buffType ).size();
		}

		/// ------------------------------------------------ //
		/// 获取buff对象列表
		/// [3/4/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		const xObjectSet& GetStateObjectList( xgc_uint32 buffID, xgc_uint32 buffType ) const
		{
			auto key = std::make_pair( buffType, buffID );
			auto it = mStateMap.find( key );
			if( it != mStateMap.end() && !it->second.empty() )
			{
				return it->second;
			}
			return static_error_res;
		}

		/// ------------------------------------------------ //
		/// 获取身上是否有某种状态
		/// [3/5/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		xgc_bool HaveSomeState( xgc_uint32 buffID, xgc_uint32 buffType )
		{
			return GetStateObjectList( buffID, buffType ).size() > 0;
		}

		///
		/// 获取所有状态
		/// [7/8/2014] create by jianglei.kinly
		///
		xObjectSet GetAllState()const
		{
			xObjectSet res;
			for( auto& it : mStateMap )
			{
				res.insert( it.second.begin(), it.second.end() );
			}
			return std::move( res );
		}

		/// ------------------------------------------------ //
		/// 插入仇恨列表
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xgc_void UpdateHateMap( xObject hAttacker, xgc_int32 nHateValue );

		/// ------------------------------------------------ //
		/// Copy仇恨列表,原来循环调用update，但是加入时间戳之后，update导致时间更新，hatemap并不一致
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xgc_void CopyHateMap( xObject xCopyTarget );

		/// ------------------------------------------------ //
		/// 清理超时怪物所有者的仇恨对象
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xgc_void CheckOwnerChange( xgc_uint32 iTimeoutLimit );

		/// ------------------------------------------------ //
		/// 从仇恨列表删除
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xgc_bool DeleteFromHateMap( xObject hAttacker = INVALID_OBJECT_ID );

		/// ------------------------------------------------ //
		/// 检测删除的是否是有特殊含义的对象，最大最小首击者
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xgc_void CheckSpecialHate( xObject hAttacker );

		/// ------------------------------------------------ //
		/// 获取目标在仇恨列表中的仇恨值
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xgc_int32 GetHateValue( xObject hAttacker );

		/// ------------------------------------------------ //
		/// 获取最大的仇恨值
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xgc_int32 GetMaxHateValue() { return mMaxHateValue; }

		/// ------------------------------------------------ //
		/// 获取最小的仇恨值
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xgc_int32 GetMinHateValue() { return mMinHateValue; }

		/// ------------------------------------------------ //
		/// 获取最大的仇恨目标
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xObject GetMaxHateTarget() { return mMaxHateTarget; }

		/// ------------------------------------------------ //
		/// 获取最小的仇恨值
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xObject GetMinHateTarget() { return mMinHateTarget; }

		/// ------------------------------------------------ //
		/// 选取xTarget的下一个目标
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xObject GetRandomTarget();

		/// ------------------------------------------------ //
		/// 选取xTarget的下一个目标
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xObject GetNextAsTarget( xObject xTarget );

		/// ------------------------------------------------ //
		/// 获取最小的仇恨目标
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		const HateMap& GetHateMap() { return mHateMap; }

		/// ------------------------------------------------ //
		/// 清除仇恨列表
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xgc_void ClearHateMap();


		/// ------------------------------------------------ //
		/// 获得第一个攻击我个目标
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xObject GetFirstAttackMeTarget() { return mFirstAttackMeTarget; }

		/// ------------------------------------------------ //
		/// 设置第一个攻击我个目标
		/// [24/4/2014 zhangyupeng]
		/// ------------------------------------------------ //
		xgc_void SetFirstAttackMeTarget( xObject xTarget ) { mFirstAttackMeTarget = xTarget; }
	public:
		///
		/// 是否在战斗状态
		/// [8/29/2014] create by jianglei.kinly
		///
		xgc_bool IsFightState() { return mFightState; }

		///
		/// 进入战斗状态
		/// [8/29/2014] create by jianglei.kinly
		///
		virtual xgc_void EnterFightState( xObject hObject ) { mFightState = true; }

		///
		/// 退出战斗状态
		/// [8/29/2014] create by jianglei.kinly
		///
		virtual xgc_void ExitFightState() { mFightState = false; }

	protected:
		/////
		/// 重置角色状态 
		/// [12/23/2010 Albert]
		/////
		xgc_void ResetActorStatus( timer_h handle, ActorStatus eStatus );

		///
		/// 受击时调用
		/// [11/6/2012 Albert.xu]
		///
		virtual xgc_bool OnHit( xObject hAttacker, ActorAttackMode eMode, xgc_lpvoid lpContext, xgc_long &nDamage, xgc_long &nHatred )
		{
			return true;
		}

		///
		/// 角色死亡时调用
		/// [9/25/2014 xufeng04]
		///
		virtual xgc_void OnDead( xObject hAttacker, ActorAttackMode eMode, xgc_lpvoid lpContext )
		{
			XCharactorEvent evt;
			evt.lpContext = lpContext;
			evt.hAttacker = hAttacker;

			EmmitEvent( evt_actor_dead, evt.cast );
		}

		///
		/// 角色复活时调用
		/// [9/25/2014 xufeng04]
		///
		virtual xgc_void OnRelive( xgc_lpvoid lpContext )
		{
			XCharactorEvent evt;
			evt.lpContext = lpContext;

			EmmitEvent( evt_actor_relive, evt.cast );
		}

		///
		/// 攻击完成后调用（被攻击者）
		/// [9/26/2014] create by albert.xu
		///
		virtual xgc_void OnUnderAttacked( xObject hAttacker, ActorAttackMode eMode, xgc_lpvoid lpContext, xgc_long nDamage ) {}

		///
		/// 攻击完成后调用(攻击者)
		/// [9/26/2014] create by albert.xu
		///
		virtual xgc_void OnAttacked( xObject hUnderAttacker, ActorAttackMode eMode, xgc_lpvoid lpContext, xgc_long nDamage ) {}

		/////
		/// 当加入的对象为关注的类型，则进行管理。
		/// 添加技能Child
		/// [11/19/2010 Albert]
		/////
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext );

		/////
		/// 当移除的对象为关注对象，则从列表中取出 
		/// [11/19/2010 Albert]
		/////
		virtual xgc_void OnRemoveChild( XObject* pChild, bool bRelease );

	protected:
		xgc_uint32		mEyeshotGroup;	///< 视野组
		timer_h			mResetStatusTimerHandler;
		xObject			mFirstAttackMeTarget;	///< 第一个攻击我的玩家,一定在仇恨列表中，但是第一个进入仇恨列表的目标不一定是首击者
		struct
		{
			xgc_uint16	bEnjoinAttack;      ///< 是否不能攻击（0：可以攻击，其他：不可以攻击）
			xgc_uint16	bEnjoinUnderAttack; ///< 是否不能被攻击（0：可以被攻击，其他：不可以被攻击）
			xgc_uint16	bEnjoinMove;        ///< 是否不能移动（0：可以移动，其他：不可以移动）
			xgc_uint16	bBeTaunt;           ///< 是否被嘲讽（0：没被嘲讽，其他：被嘲讽了）
			xgc_uint16	bNeverDie;          ///< 是否免疫死亡（生命值最低降到1）（0：不免疫，其他：免疫）
			xgc_uint16	bEnjoinBeHurt;      ///< 是否受到伤害时候不扣血（不发消息）（0：非，其他：是）
			xgc_uint32	nImmunity;          ///< 免疫的buff类型（按位）（A类Buff 免疫 B类Buff，这里面填Bf的type）
			xgc_bool    bSoul;              ///< 是否是灵魂状态（false:不是  true:是）
			en_activeCampColor enCampColor; ///< 阵营颜色
			xgc_uint16  nImmunityBuffDebuff; ///< 免疫的buff （1：免疫buff 2：免疫debuff）
		}mStatusFlags;	///< 当前状态标志位

	protected:
		SkillMap	    mSkillMap;
		StateMap		mStateMap;
		HateMap			mHateMap;
		xObject			mMaxHateTarget;		///< 仇恨值最大的玩家
		xObject			mMinHateTarget;		///< 仇恨值最小的玩家
		xgc_int32		mMaxHateValue;
		xgc_int32		mMinHateValue;
	private:
		ActorStatus		mActorRestonStatus;	///< 角色重置状态

		xgc_bool		mFightState;		///< 战斗状态

		xObject			mAttacker;			///< 攻击者
		xObject			mFriend;			///< 载具或者伴生对象
		xObject			mTarget;			///< 当前目标

		XVector3		mBornPoint;
		xgc_real32		mRadius;

		xObject			mCurrentSkill;          ///< 当前技能
		xObject			mCurrentSkillTarget;    ///< 当前技能攻击对象
		XVector3		mCurrentSkillPoint;     ///< 当前技能释放坐标
		AttackExtraInfo mCurrentSkillExtraInfo; ///< 当前技能额外信息

		XTeamPtr		mTeamPtr;
		xgc_uint16		mTeamSlot;

		datetime		mLastAttackTime;    ///< 上一次尝试攻击其他角色
		datetime		mLastAttackedTime;  ///< 上一次被其他角色尝试攻击

		xgc_bool		mIsIgnoreSkillCD;  ///< 是否忽略技能CD

		static xObjectSet static_error_res;  ///< 错误的
	public:
		///
		/// 设置是否忽略技能CD
		/// [7/9/2014] create by jianglei.kinly
		///
		xgc_void SetIsIgnoreSkillCD( xgc_bool isIgnore )
		{
			mIsIgnoreSkillCD = isIgnore;
		}
		xgc_bool GetIsIgnoreSkillCD()
		{
			return mIsIgnoreSkillCD;
		}
	};
}
#endif //__XCHARACTOR_H__