#pragma once
#ifndef __XQUEST_H__
#define __XQUEST_H__

namespace xgc
{
	///
	/// \brief  ����Ŀ���¼����� 
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

		static xAttrIndex Index;		///< ����ID
		static xAttrIndex TargetName;	///< Ŀ���������
		static xAttrIndex Count;		///< ��ǰ����
		static xAttrIndex Total;		///< ��ɼ���
		static xAttrIndex Finished;		///< ��ɱ�־

		enum objective_type
		{
			type_kill,	// ɱ������
			type_item,	// �õ���Ʒ
		};

		XQuestObjective();
		~XQuestObjective();

		///
		/// \brief ���Ӽ���,����������ɼ�����ͬ������Ŀ����. 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		xgc_void incCount( xgc_long nInc = 1 );

		///
		/// \brief �ж�Ŀ���Ƿ����
		/// \date 12/27/2017
		/// \author albert.xu
		///
		XGC_INLINE xgc_bool isFinished()const
		{
			return getValue<xgc_bool>( Finished );
		}

		///
		/// \brief ��ɱ 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		xgc_void onKill( XObjectEvent &evt );

		///
		/// \brief ��ȡ��Ʒ
		/// \date 12/27/2017
		/// \author albert.xu
		///
		xgc_void onPick( XObjectEvent &evt );

		///
		/// \brief �Ի� 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		xgc_void onTalk( XObjectEvent &evt );

		///
		/// \brief ���Ա��ʱ���� 
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
		/// \brief �����ӽڵ�����
		/// \author albert.xu
		/// \date 8/3/2009
		///
		virtual xgc_void OnInsertChild( XObject* pChild, xgc_lpvoid lpContext ) override;
		
		///
		/// \brief ɾ���ӽڵ�����
		/// \author albert.xu
		/// \date 8/3/2009
		///
		virtual xgc_void OnRemoveChild( XObject* pChild ) override;

		//---------------------------------------------------//
		// [9/15/2009 Albert]
		// Description:	֪ͨ����Ŀ�����
		//---------------------------------------------------//
		xgc_void OnFinish( XObjectEvent &evt );

	private:
		xgc_uint16 m_FinishMask;	// �������
		xgc_uint16 m_Mask;			// ��ǰ����
	};
}
#endif // __XQUEST_H__