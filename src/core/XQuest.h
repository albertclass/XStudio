#pragma once
#ifndef __XQUEST_H__
#define __XQUEST_H__

namespace xgc
{
	class XQuestObjective	:	public XObject
	{
	public:
		DECLARE_XCLASS();

		static xAttrIndex Index;			// 配置ID
		static xAttrIndex TargetName;		// 目标对象名字
		static xAttrIndex Serial;			// 序号
		static xAttrIndex Count;			// 当前计数
		static xAttrIndex FinishCount;		// 完成计数

		enum objective_type
		{
			type_kill,	// 杀死怪物
			type_item,	// 得到物品
		};

		XQuestObjective();
		~XQuestObjective();

		xgc_void OnKill( xObject hKiller, xObject hDead );
		xgc_void OnItem( xObject hSource, xgc_uintptr lParam );
		xgc_void OnTalk( xObject hSource, xgc_uintptr lParam );

		// 增加计数,若计数和完成计数相同则任务目标达成.
		xgc_bool Increment();
	};

	class XQuest :	public XObjectNode
	{
		friend class XQuestObjective;
	public:
		XQuest(void);
		~XQuest(void);

		DECLARE_XCLASS();
	protected:
		//---------------------------------------------------//
		// [9/15/2009 Albert]
		// Description:	添加任务目标时调用
		//---------------------------------------------------//
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext );

		//---------------------------------------------------//
		// [9/15/2009 Albert]
		// Description:	任务完成时调用
		//---------------------------------------------------//
		virtual xgc_void OnQuestFinished( ) = 0;

		//---------------------------------------------------//
		// [9/15/2009 Albert]
		// Description:	通知任务目标完成
		//---------------------------------------------------//
		xgc_void ObjectiveNotify( xgc_uint16 nSerial );

	private:
		xgc_uint16	m_FinishMask;	// 完成掩码
		xgc_uint16 m_Mask;			// 当前掩码
	};
}
#endif // __XQUEST_H__