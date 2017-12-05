#pragma once
#ifndef __XQUEST_H__
#define __XQUEST_H__

namespace xgc
{
	class XQuestObjective	:	public XObject
	{
	public:
		DECLARE_XCLASS();
		enum objective_type
		{
			type_kill,	// 杀死怪物
			type_item,	// 得到物品
		};

		XQuestObjective( xgc_string strName, xgc_uint16 nFinishCount );
		~XQuestObjective();

		xgc_int32 OnKill( xObject hKiller, xObject hDead );
		xgc_int32 OnItem( xObject hSource, xgc_uintptr lParam );

		// 增加计数,若计数和完成计数相同则任务目标达成.
		xgc_bool Increment();

		xgc_uint16 getSerial()const
		{
			return m_nSerial;
		}

		xgc_void setSerial( xgc_uint16 nSerial )
		{
			m_nSerial = nSerial;
		}
	private:
		//objective_type	m_ObjectiveType;	// 目标类型
		xgc_string			m_strName;			// 对象名字
		xgc_uint16			m_nSerial;			// 序号
		xgc_uint16			m_nCount;			// 当前计数
		xgc_uint16			m_nFinishCount;		// 完成计数
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