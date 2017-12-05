/*******************************************************************/
//! \file XCoreDefine.h
//! \brief 预定义常量和枚举值
//! 2014/06/30 by Albert.xu
/*******************************************************************/
#ifndef _XCORE_DEFINE_H_
#define _XCORE_DEFINE_H_

#ifdef CORE_EXPORTS
#	define CORE_API __declspec(dllexport)
#else
#	define CORE_API __declspec(dllimport)
#endif

#define NODE_TYPE_XBAG		1
#define NODE_TYPE_XSKILL	2
#define NODE_TYPE_XSTATUS	3

/// 触发器事件类型
enum eTriggerEvent : xgc_uint16
{
	Trigger_UnknownEvent,
	Trigger_ObjectEvent,		///< 对象触发事件
	Trigger_SceneEvent,			///< 场景触发事件
	Trigger_ActorEvent,			///< 角色触发事件
	Trigger_ActorEvent_Skill,	///< 技能触发事件
	Trigger_ActorEvent_State,   ///< buff触发的事件
	Trigger_PetEvent,			///< 宠物触发的事件
	Trigger_VitalityEvent,		///< 活跃度系统触发的事件
	Trigger_LevelUpEvent,		///< 升级指南事件
	Trigger_EventCount,

	Trigger_CustomEvent = 0x8000, ///< 自定义事件
};

enum eObjectTriggerCode : xgc_uint16
{
	TriggerCode_EnterEyeshot,	///< 进入视野
	TriggerCode_LeaveEyeshot,	///< 离开视野

	TriggerCode_BornInMap,		///< 出生
	TriggerCode_EnterMap,		///< 进入地图
	TriggerCode_LeaveMap,		///< 离开地图

	TriggerCode_Move,			///< 移动事件
	TriggerCode_Teleport,		///< 传送事件
};

enum eActorTriggerCode : xgc_uint16
{
	TriggerCode_ActorDead,			///< 角色死亡
	TriggerCode_ActorRelive,		///< 角色重生
	TriggerCode_ActorStateChange,	///< 角色状态转换
	TriggerCode_ActorTargetChange,	///< 角色转换目标
	TriggerCode_ActorUnderAttack,	///< 角色被攻击
	TriggerCode_ActorEnterFightState,	///< 进入战斗状态
	TriggerCode_ActorExitFightState,	///< 退出战斗状态
	TriggerCode_ActorFightFailed,			///< 角色战斗失败
	TriggerCode_ActorOnDead,			///< 死亡,但是还没有设置死亡状态
};

enum eSceneTriggerCode : xgc_uint16
{
	TriggerCode_SceneOpen,			///< 场景开启
	TriggerCode_SceneStart,			///< 场景开始
	TriggerCode_SceneFinish,		///< 场景结束
	TriggerCode_SceneClose,			///< 场景关闭
	TriggerCode_CounterOverflow,	///< 计数器上溢
	TriggerCode_CounterUnderflow,	///< 计数器下溢
	TriggerCode_CounterChanged,		///< 计数器值改变
	TriggerCode_SpoorerOver,		///< 定时器溢出
	TriggerCode_SwitchTurnOn,		///< 开关打开
	TriggerCode_SwitchTurnOff,		///< 开关关闭
	TriggerCode_EnterTripArea,		///< 目标进入陷阱区域
	TriggerCode_LeaveTripArea,		///< 目标离开陷阱区域
};

enum eSkillTriggerCode : xgc_uint16
{
	TriggerCode_SkillBegin,		    ///< 技能开始释放
	TriggerCode_SkillCasting,	    ///< 技能吟唱开始
	TriggerCode_SkillCooldown,	    ///< 技能冷却结束
	TriggerCode_SkillAttack,	    ///< 技能攻击
	TriggerCode_SkillAttackStep,    ///< 技能攻击（一次技能释放可多次触发)
	TriggerCode_SkillOver,		    ///< 技能释放完毕
};

enum eStateTriggerCode : xgc_uint16
{
	TriggerCode_StateAppend,        ///< buff附加上
	TriggerCode_StateDelete,        ///< buff结束通知
	TriggerCode_StateOverlapAdd,    ///< 叠加增加
	TriggerCode_StateOverlapSub,    ///< 叠加减少
	TriggerCode_StateOverlapZero,   ///< 叠加层数==0
};

enum ePetTriggerCode : xgc_uint16
{
	TriggerCode_BeAttacked = 1,		///< 自己被打
	TriggerCode_HostAttack = 2,		///< 主人战斗中
	TriggerCode_HostBeAttacked = 3,	///< 主人被打
	TriggerCode_Jump = 4,			///< 跳地图
	TriggerCode_HostLeave = 5,		///< 主人消失
	TriggerCode_TargetLeave = 6,	///< 目标消失
	TriggerCode_Follow = 10,		///< 跟随
	TriggerCode_Attack = 11,		///< 攻击
	TriggerCode_MagicAttack = 12,	///< 魔法攻击
	TriggerCode_ChangeMode = 20,	///< 改变模式
	TriggerCode_RegisterMgic = 30,	///< 注册魔法
	TriggerCode_UnRegisterMgic = 31,///< 反注册魔法
	TriggerCode_Destroy = 99,		///< 销毁
	TriggerCode_HunYou = 98,		///< 魂佑
	TriggerCode_ZuoJi = 97,			///< 坐骑
	TriggerCode_EnterADV = 96,		///< 进入奇遇
	TriggerCode_LeaveADV = 95,		///< 离开奇遇
	TriggerCode_MonsterBeg = 94,	///< 怪物求饶
	TriggerCode_EnterRest = 100,	///< 休息
	TriggerCode_EnterFight = 101,	///< 出战
	TriggerCode_Wedding = 110,		///< 婚礼巡游
	TriggerCode_Feed = 200,			///< 喂食
	TriggerCode_FeedZhenYuan = 201,	///< 喂食
	TriggerCode_FeedAndFight = 202,	///< 先喂食再出战
};

enum eVitalitTriggerCode : xgc_uint16
{
	TriggerCode_RCRW = 1008,		///< 好友评价
	TriggerCode_PJTR = 7001,		///< 好友评价
	TriggerCode_MRDL = 7002,		///< 评每日登陆
};

///
/// 叠加时候overlap定时器更新的方式
/// [6/11/2014] create by jianglei.kinly
///
enum EN_OVERLAPTYPE
{
	en_overlaptype_update = 1, ///< 叠加时候刷新
	en_overlaptype_unupdate,   ///< 叠加时候每层单独计算
	en_overlaptype_accumulate, ///< 叠加时候累积时间
};

///
/// buff类型-枚举
/// [6/17/2014] create by jianglei.kinly
///
enum enStateType : xgc_uint16
{
	en_stateType_speedDown        = 1 << 0,      ///< 1、减速
	en_stateType_hurt             = 1 << 1,      ///< 2、伤害
	en_stateType_control          = 1 << 2,      ///< 4、控制
	en_stateType_propDown         = 1 << 3,      ///< 8、属性降低
	en_stateType_otherDown        = 1 << 4,      ///< 16：其他减益
							  	   	     
	en_stateType_speedUp          = 1 << 5,      ///< 32、加速
	en_stateType_propUp           = 1 << 6,      ///< 64、属性增加
							  	   	     
	en_stateType_recover          = 1 << 7,      ///< 128、回复类
	en_stateType_otherUp          = 1 << 8,      ///< 256、其他增益
							  	   	     
	en_stateType_stateClock       = 1 << 9,      ///< 512、多倍经验丹类 （用clock实现）
							  	   	     
	en_stateType_ride             = 1 << 12,     ///< 坐骑

	en_stateType_DamageAbsorption = 1 << 13,     ///< 伤害吸收类


	en_stateType_all = 0xFFFF,
};

typedef xgc_uint64 xGlobal;
typedef xgc_uint32 xObject;

#endif // _XCORE_DEFINE_H_