#pragma once
#ifndef __XQUEST_H__
#define __XQUEST_H__

namespace xgc
{
	class XQuestObjective	:	public XObject
	{
	public:
		DECLARE_XCLASS();

		static xAttrIndex Index;			// ����ID
		static xAttrIndex TargetName;		// Ŀ���������
		static xAttrIndex Serial;			// ���
		static xAttrIndex Count;			// ��ǰ����
		static xAttrIndex FinishCount;		// ��ɼ���

		enum objective_type
		{
			type_kill,	// ɱ������
			type_item,	// �õ���Ʒ
		};

		XQuestObjective();
		~XQuestObjective();

		xgc_void OnKill( xObject hKiller, xObject hDead );
		xgc_void OnItem( xObject hSource, xgc_uintptr lParam );
		xgc_void OnTalk( xObject hSource, xgc_uintptr lParam );

		// ���Ӽ���,����������ɼ�����ͬ������Ŀ����.
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
		// Description:	�������Ŀ��ʱ����
		//---------------------------------------------------//
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext );

		//---------------------------------------------------//
		// [9/15/2009 Albert]
		// Description:	�������ʱ����
		//---------------------------------------------------//
		virtual xgc_void OnQuestFinished( ) = 0;

		//---------------------------------------------------//
		// [9/15/2009 Albert]
		// Description:	֪ͨ����Ŀ�����
		//---------------------------------------------------//
		xgc_void ObjectiveNotify( xgc_uint16 nSerial );

	private:
		xgc_uint16	m_FinishMask;	// �������
		xgc_uint16 m_Mask;			// ��ǰ����
	};
}
#endif // __XQUEST_H__