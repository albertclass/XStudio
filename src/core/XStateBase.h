#pragma once
using namespace XGC;

#ifdef CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif

/*******************************************************************/
//! \file XStateBase.h
//! \brief ״̬����
//! 2014/06/11 by jianglei.kinly
/*******************************************************************/

namespace XGC
{
	///
	/// ״̬��������
	/// [6/11/2014] create by jianglei.kinly
	///
	extern "C"
	{
		extern CORE_API xAttrIndex attrStateIndex;             ///< buff����
		extern CORE_API xAttrIndex attrStateBuffID;            ///< ��ID
		extern CORE_API xAttrIndex attrStateType;              ///< ����
		extern CORE_API xAttrIndex attrStateIsRing;            ///< �Ƿ��ǹ⻷���buff
		extern CORE_API xAttrIndex attrStateTriggerTimeval;    ///< ����ʱ����
		extern CORE_API xAttrIndex attrStateTriggerTimes;      ///< ��������
		extern CORE_API xAttrIndex attrStateOverlap;           ///< ���Ӳ���
		extern CORE_API xAttrIndex attrStateOverlapSubTimeval; ///< �ɵ�����buff�ĳ���ʱ��
		extern CORE_API xAttrIndex attrStateOverlapType;       ///< buff����ʱ�Ƿ�ˢ��ʱ��

		// ��GS�õ�
		extern CORE_API xAttrIndex attrStateCurrTriggerTimeval; ///< �����´δ�����ʱ����
		extern CORE_API xAttrIndex attrStateCurrTriggerTimes;   ///< ��ǰ��������
		extern CORE_API xAttrIndex attrStateCurrOverlapNum;     ///< ��ǰ���Ӳ���
		extern CORE_API xAttrIndex attrStateCurrLastTime;       ///< buffʣ��ʱ�������ڷ����ͻ���
		extern CORE_API xAttrIndex attrStateCurrOverlapTimes;   ///< ��ǰ���Ӵ�����������϶�ʱ��Key��ʾ��
		extern CORE_API xAttrIndex attrStateCurrOverlapTimeval; ///< buff���ε��ӵ�ʱ�䳤�ȣ���gs�������ã�
	}

	class XGameObject;
	class CORE_API XStateBase : public XObject
	{
		DECLARE_XCLASS();
	public:
		XStateBase();
		~XStateBase( void );

		///
		/// �⻷���Buff���
		/// [7/3/2015] create by jianglei.kinly
		///
		xgc_void OnCheck();

		///
		/// ���BUFF
		/// [6/11/2014] create by jianglei.kinly
		///
		xgc_void OnAdd( xgc_uint32 overlapNum = 1, xgc_uint32 overlapTimeVal = 0, xgc_bool isUpdate = false );
		///
		/// ����������
		/// [6/11/2014] create by jianglei.kinly
		///
		xgc_void OnOverlapAdd( xgc_uint32 addNum = 1, xgc_uint32 overlapTimeVal = 0, xgc_bool bTrigger = true );
		///
		/// ����������
		/// [6/11/2014] create by jianglei.kinly
		///
		xgc_void OnOverlapSub( xgc_uint32 subNum, xgc_uint32 CurrSubKeyID );
		///
		/// timer����
		/// [6/11/2014] create by jianglei.kinly
		///
		xgc_void OnTrigger();
		///
		/// ״̬����
		/// [6/11/2014] create by jianglei.kinly
		///
		xgc_void OnDelete( xgc_uint32 subNum, xgc_bool isServerCancel = false );
		///
		/// ��ȡbuffʣ��ʱ�䣬�����ͻ����õ�
		/// [8/28/2014] create by jianglei.kinly
		///
		xgc_uint32 GetRealLastTime();
		///
		/// Trigger��Timer
		/// [8/30/2014] create by jianglei.kinly
		///
		timer_h GetStateTimer() { return m_StateTimer; };
		///
		/// Overlap��Timer
		/// [8/30/2014] create by jianglei.kinly
		///
		xgc_unordered_map<xgc_uint16, timer_h>& GetOverlapTimer() { return mMapOverlapTimer; };

		///
		/// �⻷����
		/// [7/3/2015] create by jianglei.kinly
		///
		virtual xgc_bool TriggerStateCheck() = 0;
		///
		/// �滻֮ǰ��Trigger StateAdd
		/// [6/11/2014] create by jianglei.kinly
		///
		virtual xgc_bool TriggerStateAdd( xgc_uint32 overlapNum = 1, xgc_uint32 overlapTimeVal = 0, xgc_bool isUpdate = false ) = 0;
		///
		/// �滻֮ǰ��Trigger StateTrigger
		/// [6/11/2014] create by jianglei.kinly
		///
		virtual xgc_bool TriggerStateTrigger() = 0;
		///
		/// �滻֮ǰ��Trigger StateOverlapAdd
		/// [6/11/2014] create by jianglei.kinly
		///
		virtual xgc_bool TriggerStateOverlapAdd( xgc_uint32 addNum, xgc_bool isSend = true ) = 0;
		///
		/// �滻֮ǰ��Trigger StateOverlapZero
		/// [6/11/2014] create by jianglei.kinly
		///
		virtual xgc_bool TriggerStateOverlapZero( xgc_uint32 subNum, xgc_bool isServerCancel = false ) = 0;
		///
		/// �滻֮ǰ��Trigger StateOverlapSub
		/// [6/11/2014] create by jianglei.kinly
		///
		virtual xgc_bool TriggerStateOverlapSub( xgc_uint32 subNum ) = 0;
		///
		/// �滻֮ǰ��Trigger StateDelete
		/// [6/11/2014] create by jianglei.kinly
		///
		virtual xgc_bool TriggerStateDelete( xgc_uint32 subNum, xgc_bool isServerCancel = false ) = 0;
	protected:
		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	�����ӽڵ�ǰ����
		// return :		true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		// ���ڹ⻷��buff--
		//---------------------------------------------------//
		virtual xgc_bool PreAddChild( XObject* pChild, xgc_lpcvoid lpContext ) { return true; }

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	�����ӽڵ�����
		//---------------------------------------------------//
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext ) {}

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	�����ӽڵ�ǰ����
		// return :		true - ȷ���Ƴ��ӽڵ�, false - �ӽڵ㱻���,�Ƴ��ӽڵ�ʧ��.
		//---------------------------------------------------//
		virtual xgc_bool PreRemoveChild( XObject* pChild, xgc_bool bRelease ) { return true; }

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	ɾ���ӽڵ�����,��ʱ������δ��ɾ��
		//---------------------------------------------------//
		virtual xgc_void OnRemoveChild( XObject* pChild, xgc_bool bRelease ) {}

		///
		/// ���ٶ���ʱ���� 
		/// [6/9/2014] create by albert.xu
		///
		virtual xgc_void OnDestroy() {}

	private:
		timer_h m_StateTimer;   // ״̬�ľ��
		timer_h m_StateCheckTimer; 
	public:
		Json::Value mJsonRoot;
	private:
		xgc_unordered_map<xgc_uint16, timer_h> mMapOverlapTimer;  ///< �����õĶ�ʱ��
	};
}
