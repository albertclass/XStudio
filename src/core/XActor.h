#pragma once
#ifndef __XCHARACTOR_H__
#define __XCHARACTOR_H__
#include "XTeam.h"

namespace xgc
{
	//////////////////////////////////////////////////////////////////////////
	// 角色属性索引
	extern CORE_API xAttrIndex	attrActorIndex;			///< 角色配置索引
	extern CORE_API xAttrIndex	attrActorType;			///< 角色类型
	extern CORE_API xAttrIndex  attrActorHP;			///< 当前生命值
	extern CORE_API xAttrIndex  attrActorHP_Max;		///< 生命上限
	extern CORE_API xAttrIndex	attrActorBeatSpeed;		///< 击退速度
	extern CORE_API xAttrIndex	attrActorFaintTime;		///< 眩晕时间
	extern CORE_API xAttrIndex	attrActorAbnormalTime;	///< 特殊状态时间
	extern CORE_API xAttrIndex	attrActorBornTime;		///< 出生时间
	extern CORE_API xAttrIndex	attrActorGroupMask;		///< 组别掩码，用于区分阵营
	extern CORE_API xAttrIndex  attrActorStatus;        ///< 角色状态
	extern CORE_API xAttrIndex attrActorCanMove;		///< 可移动
	extern CORE_API xAttrIndex attrActorCanAttack;		///< 可攻击
	extern CORE_API xAttrIndex attrActorCanBeHit;		///< 可受击
	extern CORE_API xAttrIndex attrActorCanBeTanunt;	///< 可嘲讽
	extern CORE_API xAttrIndex attrActorCanDead;		///< 可死亡
	extern CORE_API xAttrIndex attrActorCanHurt;		///< 可受伤

	enum class enActorState
	{
		sta_live,   ///< 正常状态
		sta_soul,   ///< 魂状态（濒死）
		sta_dead,   ///< 死亡（无法攻击、无法移动、无法被攻击）
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
		evt_actor_behit,		///< 角色被攻击
		evt_actor_enter_fight,	///< 进入战斗状态
		evt_actor_leave_fight,	///< 退出战斗状态
		evt_actor_faild_fight,	///< 角色战斗失败
		evt_actor_befor_dead,	///< 角色濒死
		evt_actor_dead,			///< 角色死亡
	};

	///
	/// \brief 角色事件对象
	/// \author albert.xu
	/// \date 2017/10/13
	///
	struct CORE_API XActorEvent
	{
		/// @var 事件
		XObjectEvent cast;

		/// @var 攻击者的对象ID
		xObject hAttacker;

		/// @var 上下文
		xgc_lpvoid lpContext;

		union
		{
			struct
			{
				/// @var 攻击方式
				xgc_long nMode;
				/// @var 伤害
				xgc_long nDamage;
				/// @var 仇恨
				xgc_long nHate;
			}attack;

			struct
			{
				/// @var 攻击方式
				xgc_long nMode;
			}dead;
		};
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
	//	virtual xgc_void OnActorDead( XActor *pActor, xObject hAttacker, enAttackMode eMode, xgc_lpvoid lpContext ) = 0;

	//	///
	//	/// 角色在场景内死亡
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnActorRelive( XActor *pActor, xgc_lpvoid lpContext ) = 0;

	//	///
	//	/// 队伍内有队员进入
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnTeamJoinActor( XActor *pActor ) = 0;

	//	///
	//	/// 队伍内有队员离开
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnTeamKickActor( XActor *pActor ) = 0;

	//	///
	//	/// 场景内角色加入队伍
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnActorEnterTeam( XActor *pActor ) = 0;

	//	///
	//	/// 场景内角色离开队伍
	//	/// [9/25/2014] create by albert.xu
	//	///
	//	virtual xgc_void OnActorLeaveTeam( XActor *pActor ) = 0;

	//	///
	//	/// 场景内角色下线
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorOffline( XActor *pActor ) = 0;

	//	///
	//	/// 场景内角色离开场景
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorLeaveMap( XActor *pActor ) = 0;

	//	///
	//	/// 暴落物品
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnBurstGoods( XActor *pActor, xObject objId ) = 0;

	//	///
	//	/// 场景物品销毁
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnDestroySceneGoods( xObject objScene, xObject objGoods ) = 0;

	//	///
	//	/// 丢弃物品
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorDropGoods( XActor *pActor, xObject objId ) = 0;

	//	///
	//	/// 拾取物品
	//	/// [9/25/2014] create by zhangyupeng
	//	///
	//	virtual xgc_void OnActorPickUpGoods( XActor *pActor, xObject objId ) = 0;

	//	///
	//	/// 释放对象
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
		enActorState getStatus()const 
		{ 
			return enActorState( getValue<xgc_byte>( attrActorStatus ) ); \
		}

		///
		/// \brief 设置角色状态
		/// \date 8/24/2009 
		/// \author albert.xu
		///
		xgc_void SetState( enActorState eStatus, timespan tsDuration, xgc_int32 nMode = 0 );

		///
		/// \brief 设置生物状态
		/// \date 5/31/2014 
		/// \author jianglei.kinly
		///
		xgc_void SetState( enActorState eStatus );

		///
		/// \brief 判断当前状态
		/// \date 9/26/2014
		/// \author albert.xu
		///
		xgc_bool isState( enActorState eStatus )const 
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
		
			incValue( attrActorCanMove + eAbility, bEnable ? -1 : +1 );
		}

		///
		/// \brief 重置能力计数
		/// \date 6/22/2014
		/// \author jianglei.kinly
		///
		xgc_void resetAbility( enActorAbility eAbility )
		{
			XGC_ASSERT_RETURN( eAbility < abl_actor_count, XGC_NONE );

			setValue( attrActorCanMove + eAbility, 0 );
		}

		///
		/// \brief 能力是否可用
		/// \date 6/22/2014
		/// \author jianglei.kinly
		///
		xgc_bool isActivedAbility( enActorAbility eAbility )
		{
			XGC_ASSERT_RETURN( eAbility < abl_actor_count, false );

			return 0 == getValue< xgc_int32 >( attrActorCanMove + eAbility );
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
		///
		/// \brief 重置角色状态 
		/// \author albert.xu
		/// \date 12/23/2010
		///
		xgc_void ResetActorState( enActorState eStatus );

	protected:
		timer_h			mResetStatusTimerHandler;

	private:
		xgc_bool		mFightState;		///< 战斗状态
		xgc_real32		mRadius;

		enActorState	mActorRestonState;	///< 角色重置状态

		xObject			mAttacker;			///< 攻击者
		xObject			mFriend;			///< 载具或者伴生对象
		xObject			mTarget;			///< 当前目标

		XVector3		mBornPoint;

		XTeamPtr		mTeamPtr;

		datetime		mLastAttackTime;    ///< 上一次尝试攻击其他角色
		datetime		mLastAttackedTime;  ///< 上一次被其他角色尝试攻击
	};
}
#endif //__XCHARACTOR_H__