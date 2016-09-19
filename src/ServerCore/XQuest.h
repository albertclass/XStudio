#pragma once
#ifndef _XQUEST_H_
#define _XQUEST_H_

namespace XGC
{
	class XQuest;
	class XQuestObjective	:	public XObject
	{
		friend class XQuest;
	public:
		DECLARE_XCLASS();
		enum objective_type
		{
			type_kill,	// ɱ������
			type_item,	// �õ���Ʒ
		};
		XQuestObjective( xgc_string strName, xgc_uint16 nFinishCount );
		~XQuestObjective();

		xgc_int32 OnKill( xObject hKiller, xObject hDead );
		xgc_int32 OnItem( xObject hSource, xgc_uintptr lParam );

	protected:
		// ���Ӽ���,����������ɼ�����ͬ������Ŀ����.
		xgc_bool Increment();

	private:
		//objective_type	m_ObjectiveType;	// Ŀ������
		xgc_string			m_strName;			// ��������
		xgc_uint16			m_nSerial;			// ���
		xgc_uint16			m_nCount;			// ��ǰ����
		xgc_uint16			m_nFinishCount;		// ��ɼ���
	};

	class XQuest :	public XObject
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
#endif // _XQUEST_H_