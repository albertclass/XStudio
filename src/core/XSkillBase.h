#pragma once
using namespace XGC;

#ifdef CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif

namespace XGC
{
	//////////////////////////////////////////////////////////////////////////
	// ������������
	extern "C"
	{
		extern CORE_API xAttrIndex	attrSkillIndex;		  // ������������
		extern CORE_API xAttrIndex  attrSkillMainId;      // ������ID
		extern CORE_API xAttrIndex  attrSkillUseTime;     // ����ʱ��
		extern CORE_API xAttrIndex  attrSkillKeepTime;    // ����ʱ��
		extern CORE_API xAttrIndex  attrSkillCoolDown;    // ��ȴʱ��
		extern CORE_API xAttrIndex  attrSkillRepeat;      // �ظ����� 
		/*extern CORE_API xAttrIndex  attrSkillRepeatDelay; // �ظ�ʱ����*/

		extern CORE_API xAttrIndex  attrSkillCurrCoolTs;  // ��ǰ��ȴʱ��
		extern CORE_API xAttrIndex  attrSkillCurrRepeat;  // ��ǰ�ظ����������ֶܷι���ʹ�ã�
	}
	enum EN_CASTING_STATUS
	{
		en_casting_init,   // ��ʼ
		en_casting_in,     // ������
		en_casting_out,    // ��������
	};
	class XGameObject;
	class CORE_API XSkillBase : public XObject
	{
		DECLARE_XCLASS();
	public:
		XSkillBase();
		~XSkillBase( void );

		//---------------------------------------------------//
		// [12/29/2010 Albert]
		// Description:	��ʼʹ�ü��� 
		//---------------------------------------------------//
		xgc_bool CastSkill();

		//---------------------------------------------------//
		// [12/3/2010 Albert]
		// Description:	����Ŀ��
		// return : �����ͷ���Ҫʹ�õ�ʱ��
		//---------------------------------------------------//
		xgc_void Attack( xObject hTarget );

		//---------------------------------------------------//
		// [12/6/2010 Albert]
		// Description:	���ü����ͷ�״̬ 
		//---------------------------------------------------//
		xgc_void MarkCasting();

		//---------------------------------------------------//
		// [12/11/2010 Albert]
		// Description:	���ü�����ȴ״̬ 
		//---------------------------------------------------//
		xgc_void MarkCooldown();

		// ------------------------------------------------ //
		// [5/22/2014 jianglei.kinly]
		// ���ù���CD
		// ------------------------------------------------ //
		virtual xgc_void MarkPublicCD() = 0;

		//---------------------------------------------------//
		// [12/6/2010 Albert]
		// Description:	�����Ƿ������� 
		//---------------------------------------------------//
		xgc_bool IsCasting()const
		{
			return mCasting != INVALID_TIMER_HANDLE && mCastingStatus == en_casting_in;
		}
		//---------------------------------------------------//
		// [12/6/2010 Albert]
		// Description:	�����Ƿ���������
		//---------------------------------------------------//
		xgc_bool IsCastingEnd()const
		{
			return mCasting == INVALID_TIMER_HANDLE && mCastingStatus == en_casting_out;
		}

		///
		/// ģ�����жϾ����Ƿ���� 500ms
		/// [8/17/2014] create by jianglei.kinly
		///
		xgc_bool IsCastingEndFuzzy()const;

		// ------------------------------------------------ //
		// [3/12/2014 jianglei.kinly]
		// �ж��Ƿ���������ʼ״̬
		// ------------------------------------------------ //
		xgc_bool IsCastingInit()const
		{
			return mCastingStatus == en_casting_init;
		}

		//---------------------------------------------------//
		// [12/11/2010 Albert]
		// Description:	�����Ƿ���ȴ�� 
		//---------------------------------------------------//
		xgc_bool IsCooldown()const
		{
			return mCooldown != INVALID_TIMER_HANDLE;
		}

		///
		/// �����Ƿ���ȴ�� ��ģ����300ms
		/// [8/17/2014] create by jianglei.kinly
		///
		xgc_bool IsCooldownFuzzy()const;

		/////
		/// [12/27/2010 Albert]
		/// Description:	�����Ƿ����
		/////
		xgc_bool IsOver()const
		{
			return mLifetime == INVALID_TIMER_HANDLE && mAttack == INVALID_TIMER_HANDLE;
		}

		/////
		/// [12/6/2010 Albert]
		/// Description : �������״̬ 
		/// bTriger     : �Ƿ�ִ�в���
		/////
		xgc_void ClearCasting( bool bTrigger = false );

		/////
		/// [12/6/2010 Albert]
		/// Description:	�����ȴ״̬ 
		/////
		xgc_real32 ClearCooldown( bool bTrigger = false, bool bSendMsg = false );

		/////
		/// [12/6/2010 Albert]
		/// Description:	����ͷ�״̬
		/////
		xgc_void ClearLifetime( xgc_bool isNormalEnd, bool bTrigger = false );

		/// ------------------------------------------------ //
		/// [1/24/2014 jianglei.kinly]
		/// �趨���ܲ���Ч���е�״̬
		/// ------------------------------------------------ //
		xgc_void MarkAttack( timer_h handle );
		/////
		/// [12/6/2010 Albert]
		/// Description:	���ü����ͷ�״̬ 
		/////
		xgc_void ResetCasting();

		/// ------------------------------------------------ //
		/// [4/14/2014 jianglei.kinly]
		/// �滻֮ǰ��Trigger SkillBegin
		/// ------------------------------------------------ //
		virtual xgc_bool TriggerSkillBegin() = 0;

		/// ------------------------------------------------ //
		/// [4/14/2014 jianglei.kinly]
		/// �滻֮ǰ��Trigger SkillAttackStep
		/// ------------------------------------------------ //
		virtual xgc_bool TriggerSkillAttackStep( xObject hTarget ) = 0;

		/// ------------------------------------------------ //
		/// [4/14/2014 jianglei.kinly]
		/// �滻֮ǰ��Trigger SkillCasting
		/// ------------------------------------------------ //
		virtual xgc_bool TriggerSkillCasting() = 0;
		
		/// ------------------------------------------------ //
		/// [4/14/2014 jianglei.kinly]
		/// �滻֮ǰ��Trigger SkillCooldown
		/// ------------------------------------------------ //
		virtual xgc_bool TriggerSkillCooldown( bool bSendMsg ) = 0;
		
		/// ------------------------------------------------ //
		/// [4/14/2014 jianglei.kinly]
		/// �滻֮ǰ��Trigger SkillOver
		/// ------------------------------------------------ //
		virtual xgc_bool TriggerSkillOver( xgc_bool isNormalEnd ) = 0;
		
		/// ------------------------------------------------ //
		/// [4/14/2014 jianglei.kinly]
		/// �滻֮ǰ��Trigger SkillAttack
		/// ------------------------------------------------ //
		virtual xgc_bool TriggerSkillAttack() = 0;

		///
		/// ��������
		/// [7/4/2014] create by jianglei.kinly
		///
		virtual xgc_bool TriggerSkillEndCasting( xgc_bool isNormalEnd ) = 0;

		///
		/// ���ʣ����ȴʱ��
		/// [7/9/2014] create by jianglei.kinly
		///
		xgc_uint32 GetLastCoolTs();

		///
		/// ��ȡ��ǰrepeatӦ�õȴ���ʱ��
		/// [10/16/2014] create by jianglei.kinly
		///
		virtual xgc_real32 GetTriggerDelay( xgc_uint32 repeat ) = 0;
	protected:
		/// ------------------------------------------------ //
		/// [3/11/2014 jianglei.kinly]
		/// �����������ñ�־λ���ȴ��ͻ��˷�������������Ϣ
		/// ------------------------------------------------ //
		xgc_void EndCasting( timer_h handle );

		/////
		/// [12/11/2010 Albert]
		/// Description:	���ü�����ȴ״̬ 
		/////
		xgc_void ResetCooldown( timer_h handle );

		/////
		/// [12/6/2010 Albert]
		/// Description:	���ü����ͷ�״̬ 
		/////
		xgc_void ResetLifetime( timer_h handle );

	protected:
		/////
		/// �����ӽڵ�ǰ����
		/// [8/3/2009 Albert]
		/// return :		true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		/////
		virtual xgc_bool PreAddChild( XObject* pChild, xgc_lpcvoid lpContext ) { return true; }

		/////
		/// �����ӽڵ�����
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext ) {}

		/////
		/// [8/3/2009 Albert]
		/// Description:	�����ӽڵ�ǰ����
		/// return :		true - ȷ���Ƴ��ӽڵ�, false - �ӽڵ㱻���,�Ƴ��ӽڵ�ʧ��.
		//---------------------------------------------------//
		virtual xgc_bool PreRemoveChild( XObject* pChild, xgc_bool bRelease ) { return bRelease; }

		/////
		/// [8/3/2009 Albert]
		/// Description:	ɾ���ӽڵ�����,��ʱ������δ��ɾ��
		/////
		virtual xgc_void OnRemoveChild( XObject* pChild, xgc_bool bRelease ) {}

		///
		/// ���ٶ���ʱ���� 
		/// [6/9/2014] create by albert.xu
		///
		virtual xgc_void OnDestroy() {}
	public:
	private:
		timer_h mCasting;   ///< ��������
		timer_h mCooldown;  ///< ������ȴ
		timer_h mLifetime;  ///< ���ܳ���
		timer_h mAttack;    ///< �����ͷ��У��ظ���,mLifetime������ʱ��mAttack��Ҫͬʱ����

		xgc_int8 mCastingStatus;  // ����������״̬
	public:
		Json::Value mJsonRoot;
	};
}
