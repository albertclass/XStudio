/*******************************************************************/
//! \file XCoreDefine.h
//! \brief Ԥ���峣����ö��ֵ
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

/// �������¼�����
enum eTriggerEvent : xgc_uint16
{
	Trigger_UnknownEvent,
	Trigger_ObjectEvent,		///< ���󴥷��¼�
	Trigger_SceneEvent,			///< ���������¼�
	Trigger_ActorEvent,			///< ��ɫ�����¼�
	Trigger_ActorEvent_Skill,	///< ���ܴ����¼�
	Trigger_ActorEvent_State,   ///< buff�������¼�
	Trigger_PetEvent,			///< ���ﴥ�����¼�
	Trigger_VitalityEvent,		///< ��Ծ��ϵͳ�������¼�
	Trigger_LevelUpEvent,		///< ����ָ���¼�
	Trigger_EventCount,

	Trigger_CustomEvent = 0x8000, ///< �Զ����¼�
};

enum eObjectTriggerCode : xgc_uint16
{
	TriggerCode_EnterEyeshot,	///< ������Ұ
	TriggerCode_LeaveEyeshot,	///< �뿪��Ұ

	TriggerCode_BornInMap,		///< ����
	TriggerCode_EnterMap,		///< �����ͼ
	TriggerCode_LeaveMap,		///< �뿪��ͼ

	TriggerCode_Move,			///< �ƶ��¼�
	TriggerCode_Teleport,		///< �����¼�
};

enum eActorTriggerCode : xgc_uint16
{
	TriggerCode_ActorDead,			///< ��ɫ����
	TriggerCode_ActorRelive,		///< ��ɫ����
	TriggerCode_ActorStateChange,	///< ��ɫ״̬ת��
	TriggerCode_ActorTargetChange,	///< ��ɫת��Ŀ��
	TriggerCode_ActorUnderAttack,	///< ��ɫ������
	TriggerCode_ActorEnterFightState,	///< ����ս��״̬
	TriggerCode_ActorExitFightState,	///< �˳�ս��״̬
	TriggerCode_ActorFightFailed,			///< ��ɫս��ʧ��
	TriggerCode_ActorOnDead,			///< ����,���ǻ�û����������״̬
};

enum eSceneTriggerCode : xgc_uint16
{
	TriggerCode_SceneOpen,			///< ��������
	TriggerCode_SceneStart,			///< ������ʼ
	TriggerCode_SceneFinish,		///< ��������
	TriggerCode_SceneClose,			///< �����ر�
	TriggerCode_CounterOverflow,	///< ����������
	TriggerCode_CounterUnderflow,	///< ����������
	TriggerCode_CounterChanged,		///< ������ֵ�ı�
	TriggerCode_SpoorerOver,		///< ��ʱ�����
	TriggerCode_SwitchTurnOn,		///< ���ش�
	TriggerCode_SwitchTurnOff,		///< ���عر�
	TriggerCode_EnterTripArea,		///< Ŀ�������������
	TriggerCode_LeaveTripArea,		///< Ŀ���뿪��������
};

enum eSkillTriggerCode : xgc_uint16
{
	TriggerCode_SkillBegin,		    ///< ���ܿ�ʼ�ͷ�
	TriggerCode_SkillCasting,	    ///< ����������ʼ
	TriggerCode_SkillCooldown,	    ///< ������ȴ����
	TriggerCode_SkillAttack,	    ///< ���ܹ���
	TriggerCode_SkillAttackStep,    ///< ���ܹ�����һ�μ����ͷſɶ�δ���)
	TriggerCode_SkillOver,		    ///< �����ͷ����
};

enum eStateTriggerCode : xgc_uint16
{
	TriggerCode_StateAppend,        ///< buff������
	TriggerCode_StateDelete,        ///< buff����֪ͨ
	TriggerCode_StateOverlapAdd,    ///< ��������
	TriggerCode_StateOverlapSub,    ///< ���Ӽ���
	TriggerCode_StateOverlapZero,   ///< ���Ӳ���==0
};

enum ePetTriggerCode : xgc_uint16
{
	TriggerCode_BeAttacked = 1,		///< �Լ�����
	TriggerCode_HostAttack = 2,		///< ����ս����
	TriggerCode_HostBeAttacked = 3,	///< ���˱���
	TriggerCode_Jump = 4,			///< ����ͼ
	TriggerCode_HostLeave = 5,		///< ������ʧ
	TriggerCode_TargetLeave = 6,	///< Ŀ����ʧ
	TriggerCode_Follow = 10,		///< ����
	TriggerCode_Attack = 11,		///< ����
	TriggerCode_MagicAttack = 12,	///< ħ������
	TriggerCode_ChangeMode = 20,	///< �ı�ģʽ
	TriggerCode_RegisterMgic = 30,	///< ע��ħ��
	TriggerCode_UnRegisterMgic = 31,///< ��ע��ħ��
	TriggerCode_Destroy = 99,		///< ����
	TriggerCode_HunYou = 98,		///< ����
	TriggerCode_ZuoJi = 97,			///< ����
	TriggerCode_EnterADV = 96,		///< ��������
	TriggerCode_LeaveADV = 95,		///< �뿪����
	TriggerCode_MonsterBeg = 94,	///< ��������
	TriggerCode_EnterRest = 100,	///< ��Ϣ
	TriggerCode_EnterFight = 101,	///< ��ս
	TriggerCode_Wedding = 110,		///< ����Ѳ��
	TriggerCode_Feed = 200,			///< ιʳ
	TriggerCode_FeedZhenYuan = 201,	///< ιʳ
	TriggerCode_FeedAndFight = 202,	///< ��ιʳ�ٳ�ս
};

enum eVitalitTriggerCode : xgc_uint16
{
	TriggerCode_RCRW = 1008,		///< ��������
	TriggerCode_PJTR = 7001,		///< ��������
	TriggerCode_MRDL = 7002,		///< ��ÿ�յ�½
};

///
/// ����ʱ��overlap��ʱ�����µķ�ʽ
/// [6/11/2014] create by jianglei.kinly
///
enum EN_OVERLAPTYPE
{
	en_overlaptype_update = 1, ///< ����ʱ��ˢ��
	en_overlaptype_unupdate,   ///< ����ʱ��ÿ�㵥������
	en_overlaptype_accumulate, ///< ����ʱ���ۻ�ʱ��
};

///
/// buff����-ö��
/// [6/17/2014] create by jianglei.kinly
///
enum enStateType : xgc_uint16
{
	en_stateType_speedDown        = 1 << 0,      ///< 1������
	en_stateType_hurt             = 1 << 1,      ///< 2���˺�
	en_stateType_control          = 1 << 2,      ///< 4������
	en_stateType_propDown         = 1 << 3,      ///< 8�����Խ���
	en_stateType_otherDown        = 1 << 4,      ///< 16����������
							  	   	     
	en_stateType_speedUp          = 1 << 5,      ///< 32������
	en_stateType_propUp           = 1 << 6,      ///< 64����������
							  	   	     
	en_stateType_recover          = 1 << 7,      ///< 128���ظ���
	en_stateType_otherUp          = 1 << 8,      ///< 256����������
							  	   	     
	en_stateType_stateClock       = 1 << 9,      ///< 512���౶���鵤�� ����clockʵ�֣�
							  	   	     
	en_stateType_ride             = 1 << 12,     ///< ����

	en_stateType_DamageAbsorption = 1 << 13,     ///< �˺�������


	en_stateType_all = 0xFFFF,
};

typedef xgc_uint64 xGlobal;
typedef xgc_uint32 xObject;

#endif // _XCORE_DEFINE_H_