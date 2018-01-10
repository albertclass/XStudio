#pragma once
#ifndef __XQUEST_H__
#define __XQUEST_H__

namespace xgc
{
	///
	/// \brief  任务目标事件类型 
	/// \date 12/27/2017
	/// \author albert.xu
	///
	enum eQuestObjectiveEvent
	{
		evt_objective_finished,
	};

	class XQuestObjective : public XObject
	{
	public:
		DECLARE_XCLASS();

		static xAttrIndex Index;		///< 配置ID
		static xAttrIndex TargetName;	///< 目标对象名字
		static xAttrIndex Count;		///< 当前计数
		static xAttrIndex Total;		///< 完成计数
		static xAttrIndex Finished;		///< 完成标志

		enum objective_type
		{
			type_kill,	// 杀死怪物
			type_item,	// 得到物品
		};

		XQuestObjective();
		~XQuestObjective();

		///
		/// \brief 增加计数,若计数和完成计数相同则任务目标达成. 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		xgc_void incCount( xgc_long nInc = 1 );

		///
		/// \brief 判断目标是否完成
		/// \date 12/27/2017
		/// \author albert.xu
		///
		XGC_INLINE xgc_bool isFinished()const
		{
			return getValue<xgc_bool>( Finished );
		}

		///
		/// \brief 击杀 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		xgc_void onKill( XObjectEvent &evt );

		///
		/// \brief 捡取物品
		/// \date 12/27/2017
		/// \author albert.xu
		///
		xgc_void onPick( XObjectEvent &evt );

		///
		/// \brief 对话 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		xgc_void onTalk( XObjectEvent &evt );

		///
		/// \brief 属性变更时调用 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		virtual xgc_void OnValueChanged( xAttrIndex nAttr );
	};

	enum eQuestEvent
	{
		evt_quest_finished,
	};

	class XQuest :	public XObjectNode
	{
		friend class XQuestObjective;
	public:
		XQuest(void);
		~XQuest(void);

		DECLARE_XCLASS();
	protected:
		///
		/// \brief 增加子节点后调用
		/// \author albert.xu
		/// \date 8/3/2009
		///
		virtual xgc_void OnInsertChild( XObject* pChild, xgc_lpvoid lpContext ) override;
		
		///
		/// \brief 删除子节点后调用
		/// \author albert.xu
		/// \date 8/3/2009
		///
		virtual xgc_void OnRemoveChild( XObject* pChild ) override;

		//---------------------------------------------------//
		// [9/15/2009 Albert]
		// Description:	通知任务目标完成
		//---------------------------------------------------//
		xgc_void OnFinish( XObjectEvent &evt );

	private:
		xgc_uint16 m_FinishMask;	// 完成掩码
		xgc_uint16 m_Mask;			// 当前掩码
	};
}
#endif // __XQUEST_H__