#pragma once
using namespace XGC;

#ifdef CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif

/*******************************************************************/
//! \file XStateBase.h
//! \brief 状态基类
//! 2014/06/11 by jianglei.kinly
/*******************************************************************/

namespace XGC
{
	///
	/// 状态属性索引
	/// [6/11/2014] create by jianglei.kinly
	///
	extern "C"
	{
		extern CORE_API xAttrIndex attrStateIndex;             ///< buff索引
		extern CORE_API xAttrIndex attrStateBuffID;            ///< 主ID
		extern CORE_API xAttrIndex attrStateType;              ///< 分类
		extern CORE_API xAttrIndex attrStateIsRing;            ///< 是否是光环类的buff
		extern CORE_API xAttrIndex attrStateTriggerTimeval;    ///< 触发时间间隔
		extern CORE_API xAttrIndex attrStateTriggerTimes;      ///< 触发次数
		extern CORE_API xAttrIndex attrStateOverlap;           ///< 叠加层数
		extern CORE_API xAttrIndex attrStateOverlapSubTimeval; ///< 可叠加类buff的持续时间
		extern CORE_API xAttrIndex attrStateOverlapType;       ///< buff叠加时是否刷新时间

		// 跳GS用的
		extern CORE_API xAttrIndex attrStateCurrTriggerTimeval; ///< 距离下次触发的时间间隔
		extern CORE_API xAttrIndex attrStateCurrTriggerTimes;   ///< 当前触发次数
		extern CORE_API xAttrIndex attrStateCurrOverlapNum;     ///< 当前叠加层数
		extern CORE_API xAttrIndex attrStateCurrLastTime;       ///< buff剩余时长，用于发给客户端
		extern CORE_API xAttrIndex attrStateCurrOverlapTimes;   ///< 当前叠加次数（用来组合定时器Key标示）
		extern CORE_API xAttrIndex attrStateCurrOverlapTimeval; ///< buff当次叠加的时间长度（跳gs，上线用）
	}

	class XGameObject;
	class CORE_API XStateBase : public XObject
	{
		DECLARE_XCLASS();
	public:
		XStateBase();
		~XStateBase( void );

		///
		/// 光环类的Buff检测
		/// [7/3/2015] create by jianglei.kinly
		///
		xgc_void OnCheck();

		///
		/// 添加BUFF
		/// [6/11/2014] create by jianglei.kinly
		///
		xgc_void OnAdd( xgc_uint32 overlapNum = 1, xgc_uint32 overlapTimeVal = 0, xgc_bool isUpdate = false );
		///
		/// 叠加数增加
		/// [6/11/2014] create by jianglei.kinly
		///
		xgc_void OnOverlapAdd( xgc_uint32 addNum = 1, xgc_uint32 overlapTimeVal = 0, xgc_bool bTrigger = true );
		///
		/// 叠加数减少
		/// [6/11/2014] create by jianglei.kinly
		///
		xgc_void OnOverlapSub( xgc_uint32 subNum, xgc_uint32 CurrSubKeyID );
		///
		/// timer触发
		/// [6/11/2014] create by jianglei.kinly
		///
		xgc_void OnTrigger();
		///
		/// 状态结束
		/// [6/11/2014] create by jianglei.kinly
		///
		xgc_void OnDelete( xgc_uint32 subNum, xgc_bool isServerCancel = false );
		///
		/// 获取buff剩余时间，发给客户端用的
		/// [8/28/2014] create by jianglei.kinly
		///
		xgc_uint32 GetRealLastTime();
		///
		/// Trigger的Timer
		/// [8/30/2014] create by jianglei.kinly
		///
		timer_h GetStateTimer() { return m_StateTimer; };
		///
		/// Overlap的Timer
		/// [8/30/2014] create by jianglei.kinly
		///
		xgc_unordered_map<xgc_uint16, timer_h>& GetOverlapTimer() { return mMapOverlapTimer; };

		///
		/// 光环类检测
		/// [7/3/2015] create by jianglei.kinly
		///
		virtual xgc_bool TriggerStateCheck() = 0;
		///
		/// 替换之前的Trigger StateAdd
		/// [6/11/2014] create by jianglei.kinly
		///
		virtual xgc_bool TriggerStateAdd( xgc_uint32 overlapNum = 1, xgc_uint32 overlapTimeVal = 0, xgc_bool isUpdate = false ) = 0;
		///
		/// 替换之前的Trigger StateTrigger
		/// [6/11/2014] create by jianglei.kinly
		///
		virtual xgc_bool TriggerStateTrigger() = 0;
		///
		/// 替换之前的Trigger StateOverlapAdd
		/// [6/11/2014] create by jianglei.kinly
		///
		virtual xgc_bool TriggerStateOverlapAdd( xgc_uint32 addNum, xgc_bool isSend = true ) = 0;
		///
		/// 替换之前的Trigger StateOverlapZero
		/// [6/11/2014] create by jianglei.kinly
		///
		virtual xgc_bool TriggerStateOverlapZero( xgc_uint32 subNum, xgc_bool isServerCancel = false ) = 0;
		///
		/// 替换之前的Trigger StateOverlapSub
		/// [6/11/2014] create by jianglei.kinly
		///
		virtual xgc_bool TriggerStateOverlapSub( xgc_uint32 subNum ) = 0;
		///
		/// 替换之前的Trigger StateDelete
		/// [6/11/2014] create by jianglei.kinly
		///
		virtual xgc_bool TriggerStateDelete( xgc_uint32 subNum, xgc_bool isServerCancel = false ) = 0;
	protected:
		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	增加子节点前调用
		// return :		true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		// 用于光环类buff--
		//---------------------------------------------------//
		virtual xgc_bool PreAddChild( XObject* pChild, xgc_lpcvoid lpContext ) { return true; }

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	加入子节点后调用
		//---------------------------------------------------//
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext ) {}

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	增加子节点前调用
		// return :		true - 确认移除子节点, false - 子节点被否决,移除子节点失败.
		//---------------------------------------------------//
		virtual xgc_bool PreRemoveChild( XObject* pChild, xgc_bool bRelease ) { return true; }

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	删除子节点后调用,此时对象尚未被删除
		//---------------------------------------------------//
		virtual xgc_void OnRemoveChild( XObject* pChild, xgc_bool bRelease ) {}

		///
		/// 销毁对象时调用 
		/// [6/9/2014] create by albert.xu
		///
		virtual xgc_void OnDestroy() {}

	private:
		timer_h m_StateTimer;   // 状态的句柄
		timer_h m_StateCheckTimer; 
	public:
		Json::Value mJsonRoot;
	private:
		xgc_unordered_map<xgc_uint16, timer_h> mMapOverlapTimer;  ///< 叠加用的定时器
	};
}
