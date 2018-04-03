#pragma once
#ifndef __XCHARACTOR_H__
#define __XCHARACTOR_H__
#include "XTeam.h"
#include "XGameObject.h"

namespace xgc
{
	enum class enActorState
	{
		sta_live,   ///< 正常状态
		sta_soul,   ///< 魂状态（濒死）
		sta_dead,   ///< 死亡（无法攻击、无法移动、无法被攻击）
	};

	enum enActorStatus
	{
		stu_actor_normal = 0,		///< 正常
		stu_actor_pain   = 1 << 1,  ///< 疼痛，（硬直）
		stu_actor_faint  = 1 << 2,  ///< 眩晕，（不可移动，不可操作）
		stu_actor_fall   = 1 << 3,  ///< 置空，（不可移动，不可操作）
		stu_actor_beat   = 1 << 4,  ///< 击退，（不可操作）
		stu_actor_lie    = 1 << 5,  ///< 倒地，（不可移动，不可操作）
	};

	enum enActorAbility
	{
		/// 是否可移动 0 - 可以， 其他 - 不可以
		abl_actor_move = 0,
		/// 是否可攻击 0 - 可以， 其他 - 不可以
		abl_actor_attack,
		/// 是否可受击 0 - 可以， 其他 - 不可以
		abl_actor_be_hit,
		/// 是否可被嘲讽 0 - 可以， 其他 - 不可以
		abl_actor_be_tanunt,
		/// 是否可死亡 0 - 可以， 其他 - 不可以
		abl_actor_dead,
		/// 是否可受伤（扣血） 0 - 可以， 其他 - 不可以
		abl_actor_hurt,
		/// 角色能力个数
		abl_actor_count,
	};

	enum enActorEvent
	{
		evt_actor_relive,		///< 角色重生
		evt_actor_change_state,	///< 角色状态转换
		evt_actor_change_target,///< 角色转换目标
		evt_actor_attacked,		///< 角色攻击他人
		evt_actor_killed,		///< 角色杀死目标
		evt_actor_assists,		///< 角色杀死目标 - 助攻
		evt_actor_behit,		///< 角色被攻击
		evt_actor_enter_fight,	///< 进入战斗状态
		evt_actor_leave_fight,	///< 退出战斗状态
		evt_actor_faild_fight,	///< 角色战斗失败
		evt_actor_befor_dead,	///< 角色濒死
		evt_actor_dead,			///< 角色死亡

		evt_actor_pick,			///< 角色捡起物品
		evt_actor_drop,			///< 角色丢弃物品
		evt_actor_team_join,	///< 角色队伍中有新人加入
		evt_actor_team_kick,	///< 角色队伍中有人被踢出
		evt_actor_enter_team,	///< 角色加入队伍
		evt_actor_leave_team,	///< 角色离开队伍
		evt_actor_enter_scene,	///< 角色进入场景
		evt_actor_leave_scene,	///< 角色离开场景
	};

	class CORE_API XActor : public XObjectNode
	{
		DECLARE_XCLASS();

	public:
		///
		/// \brief 一般角色事件 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		struct CORE_API NormalEvent
		{
			/// @var 事件
			XObjectEvent cast;
			/// @var 上下文
			xgc_lpvoid lpContext;
		};

		///
		/// \brief 角色事件对象
		/// \author albert.xu
		/// \date 2017/10/13
		///
		struct CORE_API AttackEvent
		{
			/// @var 事件
			XObjectEvent cast;
			/// @var 攻击者的对象ID
			xObject hAttacker;
			/// @var 被攻击对象的ID
			xObject hTarget;
			/// @var 攻击方式
			xgc_long nMode;
			/// @var 伤害
			xgc_long nDamage;
			/// @var 仇恨
			xgc_long nHate;
			/// @var 上下文
			xgc_lpvoid lpContext;
		};

		///
		/// \brief 队伍事件 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		struct CORE_API TeamEvent
		{
			/// @var 事件
			XObjectEvent cast;
			/// @var 队伍ID
			xgc_ulong team_id;
		};

		//////////////////////////////////////////////////////////////////////////
		// 角色属性索引
		static xAttrIndex Index;		///< 角色配置索引
		static xAttrIndex Type;			///< 角色类型
		static xAttrIndex HP;			///< 当前生命值
		static xAttrIndex HP_Max;		///< 生命上限
		static xAttrIndex BeatSpeed;	///< 击退速度
		static xAttrIndex FaintTime;	///< 眩晕时间
		static xAttrIndex AbnormalTime;	///< 特殊状态时间
		static xAttrIndex BornTime;		///< 出生时间
		static xAttrIndex GroupMask;	///< 组别掩码，用于区分阵营
		static xAttrIndex State;		///< 角色状态
		static xAttrIndex Status;		///< 角色情形
		static xAttrIndex StatusTime;	///< 角色异常状态重置时间
		static xAttrIndex CanMove;		///< 可移动
		static xAttrIndex CanAttack;	///< 可攻击
		static xAttrIndex CanBeHit;		///< 可受击
		static xAttrIndex CanBeTanunt;	///< 可嘲讽
		static xAttrIndex CanDead;		///< 可死亡
		static xAttrIndex CanHurt;		///< 可受伤

		friend class XGameMap;
		friend class XTeam;

	public:
		XActor( void );
		~XActor( void );

		///
		/// \brief 受到攻击
		/// \author albert.xu
		/// \date 11/19/2010
		///
		xgc_void UnderAttack( xObject hAttacker, xgc_long nMode, xgc_lpvoid lpContext );

		///
		/// \brief 获取仇恨最高的角色
		/// \date  11/19/2010
		/// \author albert.xu
		///
		XGC_INLINE xObject GetAttacker()const { return mAttacker; }

		///
		/// \brief 获取正在攻击的目标 
		/// \author albert.xu
		/// \date 12/27/2010
		///
		XGC_INLINE xObject GetTarget()const { return mTarget; }

		///
		/// \brief 设置当前攻击的目标, 远程兵种不需要设置位置 
		/// \date [12/27/2010 Albert]
		///
		XGC_INLINE xObject SetTarget( xObject hTarget )
		{
			xObject hOldTarget = mTarget;
			mTarget = hTarget;
			return hOldTarget;
		}

		///
		/// \brief 获取伴生对象
		/// \date [11/23/2012 Albert.xu]
		///
		XGC_INLINE xObject GetFriend()const { return mFriend; }

		///
		/// \brief 设置伴生对象
		/// \date [11/23/2012 Albert.xu]
		///
		XGC_INLINE xgc_void SetFriend( xObject hObject ) { mFriend = hObject; }

		///
		/// \brief 获取重生点
		/// \date [9/28/2012 Albert.xu]
		///
		const XVector3& GetBornPoint()const { return mBornPoint; }

		///
		/// \brief 获取重生点
		/// \date [1/7/2014 albert.xu]
		///
		xgc_void GetBornPoint( float &x, float &y, float &z ) 
		{ 
			x = mBornPoint.x; 
			y = mBornPoint.y; 
			z = mBornPoint.z; 
		}

		///
		/// \brief 设置出生点
		/// \date [10/12/2012 Albert.xu]
		///
		xgc_void SetBornPoint( float x, float y, float z ) 
		{ 
			mBornPoint.x = x; 
			mBornPoint.y = y; 
			mBornPoint.z = z; 
		}

		///
		/// \brief 角色死亡（死亡入口，被攻击后调用）
		/// \date 8/21/2009
		/// \author albert.xu
		///
		xgc_void Dead( xObject hAttacker, xgc_long nMode, xgc_lpvoid lpContext );

		///
		/// \brief 角色重生（复活入口） 
		/// \date 12/28/2010
		/// \author albert.xu
		///
		xgc_void Relive( xgc_lpvoid lpContext );

		///
		/// \brief 获取角色状态
		/// \date 8/24/2009
		/// \author albert.xu
		///
		enActorState getState()const 
		{ 
			return enActorState( getValue<xgc_uint32>( State ) );
		}

		///
		/// \brief 设置角色状态 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		xgc_void setState( enActorState eState )
		{
			setValue< xgc_uint32 >( State, (xgc_uint32)eState );
		}

		///
		/// \brief 判断角色状态 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		xgc_bool isState( enActorState eState )
		{
			return eState == getState();
		}

		///
		/// \brief 获取角色当前情形 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		enActorStatus getStatus()const
		{
			return enActorStatus( getValue<xgc_uint32>( Status ) );
		}

		///
		/// \brief 设置角色状态
		/// \date 8/24/2009 
		/// \author albert.xu
		///
		xgc_void setStatus( enActorStatus eStatus, timespan tsDuration, xgc_int32 nMode = 0 );

		///
		/// \brief 设置生物状态
		/// \date 5/31/2014 
		/// \author jianglei.kinly
		///
		xgc_void setStatus( enActorStatus eStatus );

		///
		/// \brief 判断当前状态
		/// \date 9/26/2014
		/// \author albert.xu
		///
		xgc_bool isStatus( enActorStatus eStatus )const 
		{ 
			return eStatus == getStatus(); 
		}

		///
		/// \brief 开启/关闭能力
		/// \date 6/22/2014
		/// \author jianglei.kinly
		///
		xgc_void enableAbility( enActorAbility eAbility, xgc_bool bEnable = true )
		{
			XGC_ASSERT_RETURN( eAbility < abl_actor_count, XGC_NONE );
		
			incValue( CanMove + eAbility, bEnable ? -1 : +1 );
		}

		///
		/// \brief 重置能力计数
		/// \date 6/22/2014
		/// \author jianglei.kinly
		///
		xgc_void resetAbility( enActorAbility eAbility )
		{
			XGC_ASSERT_RETURN( eAbility < abl_actor_count, XGC_NONE );

			setValue( CanMove + eAbility, 0 );
		}

		///
		/// \brief 能力是否可用
		/// \date 6/22/2014
		/// \author jianglei.kinly
		///
		xgc_bool isAbilityActived( enActorAbility eAbility )
		{
			XGC_ASSERT_RETURN( eAbility < abl_actor_count, false );

			return 0 == getValue< xgc_int32 >( CanMove + eAbility );
		}

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
		/// 获取队伍指针
		/// [11/12/2012 Albert.xu]
		///
		XTeamPtr getTeam() 
		{ 
			return mTeamPtr; 
		}

		///
		/// 获取队伍指针 const
		/// [11/22/2012 Albert.xu]
		///
		const XTeamPtr getTeam()const
		{
			return mTeamPtr;
		}

		///
		/// 设置队伍指针
		/// [11/22/2012 Albert.xu]
		///
		xgc_void setTeam( XTeamPtr team ) 
		{ 
			mTeamPtr = team;
		}

		///
		/// 根据Buff类型查找buff列表
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
		/// 获取buff个数
		/// [2/27/2014 jianglei.kinly]
		/// 
		//size_t GetStateCountByType( xgc_uint32 buffType )const
		//{
		//	return GetStateByType( buffType ).size();
		//}

		/// 
		/// 获取buff对象列表
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
		/// 获取身上是否有某种状态
		/// [3/5/2014 jianglei.kinly]
		/// 
		//xgc_bool HaveSomeState( xgc_uint32 buffID, xgc_uint32 buffType )
		//{
		//	return GetStateObjectList( buffID, buffType ).size() > 0;
		//}

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
		///
		/// \brief 战斗计算 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		virtual xgc_void beHit( AttackEvent& evt )
		{
			EmmitEvent( evt.cast, evt_actor_behit );
		}

		///
		/// \brief 战斗计算 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		virtual xgc_void doAttack( AttackEvent& evt )
		{
			EmmitEvent( evt.cast, evt_actor_attacked );
		}

	protected:
		/// @var 状态重置的定时器句柄
		timer_h mResetStatusTimerHandler;

	private:
		/// @var 战斗状态
		xgc_bool mFightState;
		/// @var 角色碰撞半径
		xgc_real32 mRadius;
		/// @var 攻击者
		xObject	mAttacker;
		/// @var 载具或者伴生对象
		xObject	mFriend;
		/// @var 当前目标
		xObject	mTarget;
		/// @var 出生点
		XVector3 mBornPoint;
		/// @var 队伍对象
		XTeamPtr mTeamPtr;
	};
}
#endif //__XCHARACTOR_H__