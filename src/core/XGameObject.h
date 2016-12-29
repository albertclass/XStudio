#pragma once
#ifndef _XGAME_OBJECT_H_
#define _XGAME_OBJECT_H_
#ifdef CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif	//CORE_EXPORTS

#include <bitset>
#include "XGameMap.h"

#if (_MSC_VER >= 1500) && (_MSC_FULL_VER >= 150030729) && !defined( _USE_BOOST )
#	include <functional>
	using namespace std;
#if (_MSC_VER >= 1600)
	using namespace std::placeholders;
#else
	using namespace std::tr1;
	using namespace std::tr1::placeholders;
#endif
#else
#	include "boost\function.hpp"
#	include "boost\bind.hpp"
	using namespace boost;
#endif

namespace XGC
{
	//////////////////////////////////////////////////////////////////////////
	// ���������࣬ ������Ļ�ϵĶ��󶼴Ӵ�������
	typedef function< xgc_void( xObject, xObject, xgc_uintptr, xgc_uintptr, xgc_uint16 ) > TriggerFunctor;
	extern CORE_API xAttrIndex attrObjectName;		///< ��������
	extern CORE_API xAttrIndex attrObjectAlias;		///< ��������
	extern CORE_API xAttrIndex attrObjectFlags;		///< ������λ
	extern CORE_API xAttrIndex attrObjectRadius;	///< ����뾶
	extern CORE_API xAttrIndex attrObjectMobID;		///< ˢ����������

	class CORE_API XGameObject : public XObject
	{
	public:
		DECLARE_XCLASS();
		friend class XGameMap;
		friend class CStateMachine;
	public:
		XGameObject();
		~XGameObject();

		enum 
		{ 
			Flag_CanMove = 0, 
			Flag_Move = 1, 
			Flag_NotifyEyeshot = 2, 
			Flag_Barrier = 3, 
			Flag_BaTi = 4 
		};

		virtual xgc_void Release();

		// ֱ����������
		XGC_INLINE xgc_void GetPosition( xgc_real32 fPos[3] )const
		{
			fPos[0] = mPosition[0];
			fPos[1] = mPosition[1];
			fPos[2] = mPosition[2];
		}

		// ֱ����������
		XGC_INLINE xgc_void GetPosition( XVector3& Position )const
		{
			Position = mPosition;
		}

		// ֱ����������
		XGC_INLINE const XVector3& GetPosition()const
		{
			return mPosition;
		}

		// ֱ����������
		XGC_INLINE xgc_void GetPosition( float &x, float &y, float &z )const
		{
			x = mPosition.x;
			y = mPosition.y;
			z = mPosition.z;
		}

		//////////////////////////////////////////////////////////////////////////
		// ����ķ���
		//////////////////////////////////////////////////////////////////////////
		XGC_INLINE xgc_void SetDirection( xgc_real32 fPosX, xgc_real32 fPosY, xgc_real32 fPosZ = 0 )
		{
			mDirection.SetValue( fPosX, fPosY, fPosZ );
			if( mDirection == XVector3::ZERO )
				mDirection.x = 1.0f;
		}

		XGC_INLINE xgc_void SetDirection( const xgc_real32 fPos[3] )
		{
			mDirection[0] = fPos[0];
			mDirection[1] = fPos[1];
			mDirection[2] = fPos[2];

			if( mDirection == XVector3::ZERO )
				mDirection.x = 1.0f;
		}
		// ------------------------------------------------ //
		// [4/18/2014 jianglei.kinly]
		// �ýǶ�ֵ���÷�������
		// ------------------------------------------------ //
		XGC_INLINE xgc_void SetDirection( const xgc_real32 fAngle )
		{
			float fRad = DEG2RAD( fAngle );
			SetDirection( XMath::Cos( fRad ), -XMath::Sin( fRad ), 0 );
		}

		// ֱ����������
		XGC_INLINE xgc_void GetDirection( xgc_real32 fPos[3] )const
		{
			fPos[0] = mDirection[0];
			fPos[1] = mDirection[1];
			fPos[2] = mDirection[2];
		}

		// ֱ����������
		XGC_INLINE xgc_void GetDirection( XVector3& Position )const
		{
			Position = mDirection;
		}

		// ֱ����������
		XGC_INLINE const XVector3& GetDirection()const
		{
			return mDirection;
		}

		/// ------------------------------------------------ //
		/// ��ȡ��ǰ����ĽǶ�ֵ
		/// [4/18/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		XGC_INLINE const xgc_uint16 GetDirectionAngle()const
		{
			return  mDirection.y < 0 ? 
				(xgc_uint16) ( XMath::ACos( mDirection.DotProduct( XVector3( 1, 0, 0 ) ) ) * 180 / XMath::ONE_PI ) : 
				(xgc_uint16) ( 360 - XMath::ACos( mDirection.DotProduct( XVector3( 1, 0, 0 ) ) ) * 180 / XMath::ONE_PI );
		}

		///
		/// �����Ƿ�ɼ�
		/// [1/17/2014 albert.xu]
		///
		XGC_INLINE xgc_void SetVisible( xgc_bool bEnable = true )
		{
			if( bEnable )
				GetAttribute( attrObjectFlags ).SetBit( Flag_NotifyEyeshot );
			else
				GetAttribute( attrObjectFlags ).ClrBit( Flag_NotifyEyeshot );
		}

		///
		/// �ж��Ƿ�ɼ�
		/// [1/17/2014 albert.xu]
		///
		XGC_INLINE xgc_bool IsVisible()
		{
			return GetAttribute( attrObjectFlags ).GetBit( Flag_NotifyEyeshot, true );
		}

		///
		/// �����ƶ���־
		/// [1/17/2014 albert.xu]
		///
		XGC_INLINE xgc_void SetMoveFlag( xgc_bool bEnable = true )
		{
			if( bEnable )
				GetAttribute( attrObjectFlags ).SetBit( Flag_CanMove );
			else
				GetAttribute( attrObjectFlags ).ClrBit( Flag_CanMove );
		}

		///
		/// ��ȡ�ƶ���־ true - ���ƶ�, false - �����ƶ�
		/// [1/17/2014 albert.xu]
		///
		XGC_INLINE xgc_bool GetMoveFlag()
		{
			return GetAttribute( attrObjectFlags ).GetBit( Flag_CanMove, true );
		}

		///
		/// ��ȡդ�����ñ�־
		/// [6/28/2014] create by albert.xu
		///
		XGC_INLINE xgc_bool GetBarrierFlag()
		{
			return GetAttribute( attrObjectFlags ).GetBit( Flag_Barrier, false );
		}

		///
		/// ����դ�����ñ�־
		/// [6/28/2014] create by albert.xu
		///
		XGC_INLINE xgc_void SetBarrierFlag( xgc_bool bEnable = true )
		{
			if( bEnable )
				GetAttribute( attrObjectFlags ).SetBit( Flag_Barrier );
			else
				GetAttribute( attrObjectFlags ).ClrBit( Flag_Barrier );
		}

		///
		/// ��ȡ���廤��
		/// [8/5/2014] create by jianglei.kinly
		///
		XGC_INLINE xgc_bool GetBaTiFlag()
		{
			return GetAttribute( attrObjectFlags ).GetBit( Flag_BaTi, false );
		}

		///
		/// ���ð��廤��
		/// [8/5/2014] create by jianglei.kinly
		///
		XGC_INLINE xgc_void SetBaTiFlag( xgc_bool bEnable = true )
		{
			if( bEnable )
				GetAttribute( attrObjectFlags ).SetBit( Flag_BaTi );
			else
				GetAttribute( attrObjectFlags ).ClrBit( Flag_BaTi );
		}

		XGC_INLINE xgc_real32 GetPosX()const { return mPosition.x; }
		XGC_INLINE xgc_real32 GetPosY()const { return mPosition.y; }
		XGC_INLINE xgc_real32 GetPosZ()const { return mPosition.z; }

		XGC_INLINE xgc_real32 GetWidth()const { return 1.0f; }
		XGC_INLINE xgc_real32 GetHeight()const { return 1.0f; }

		///
		/// �����ڳ����У������ڱ��˵���Ұ�У�
		/// @param fnNotifyFilter ���ھ���������Ұ��Ϣ���͵�����
		/// [10/14/2014] create by albert.xu
		///
		xgc_void Appear( const std::function< xgc_bool( xObject ) > &fnNotifyFilter );

		///
		/// ��ʧ�ڳ����У��ӱ��˵���Ұ����ʧ��
		/// @param fnNotifyFilter ���ھ����뿪��Ұ��Ϣ���͵�����
		/// [10/14/2014] create by albert.xu
		///
		xgc_void Vanish( const std::function< xgc_bool( xObject ) > &fnNotifyFilter );

		///
		/// ��ʾ��ɫ��Χ�ĳ�������
		/// @param fnNotifyFilter ���ھ����뿪��Ұ��Ϣ���͵�����
		/// [12/5/2014] create by albert.xu
		///
		xgc_void ShowOther( const std::function< xgc_bool( xObject ) > &fnNotifyFilter );

		///
		/// ���ؽ�ɫ��Χ�ĳ�������
		/// @param fnNotifyFilter ���ھ����뿪��Ұ��Ϣ���͵�����
		/// [12/5/2014] create by albert.xu
		///
		xgc_void HideOther( const std::function< xgc_bool( xObject ) > &fnNotifyFilter );

		/////
		/// [10/12/2010 Albert]
		/// ע�ᴥ����
		/////
		xgc_void RegisteTrigger( xObject hWhoCare, xgc_uint16 nTriggerEvent, const TriggerFunctor &Call, xgc_uint16 nTriggerCode = 0xffff );

		/////
		/// [10/12/2010 Albert]
		/// ���������
		/////
		xgc_void DismissTrigger( xObject hWhoCare, xgc_uint16 nTriggerEvent = Trigger_UnknownEvent, xgc_uint16 nTriggerCode = 0xffff );

		/////
		/// [11/23/2010 Albert]
		/// Description:	�����¼� 
		/////
		xgc_void Trigger( xgc_uint16 nTriggerEvent, xgc_uintptr wParam, xgc_uintptr lParam, xgc_uint16 nCode );

	protected:
		/////
		/// [8/3/2009 Albert]
		/// ���������Ұ
		/// @param pObject ������Ұ�Ķ���ָ��
		/////
		virtual xgc_void OnEnterEyeshot( XGameObject* pObject, VisualMode eMode )
		{
			Trigger( Trigger_ObjectEvent, IsVisible(), 0, TriggerCode_EnterEyeshot );
		}

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	�����뿪��Ұ
		// pObject	:	�뿪��Ұ�Ķ���ָ��
		//---------------------------------------------------//
		virtual xgc_void OnLeaveEyeshot( XGameObject* pObject, VisualMode eMode )
		{
			Trigger( Trigger_ObjectEvent, IsVisible(), 0, TriggerCode_LeaveEyeshot );
		}

		/////
		/// [2/17/2014 baomin]
		/// ��������Ұ���ƶ�
		/// @param vOldPosition �ϵ�����λ��
		/// @param lpContext ������
		/////
		virtual xgc_void OnMove( const XVector3& vOldPosition, xgc_uintptr lpContext )
		{
			Trigger( Trigger_ObjectEvent, IsVisible(), lpContext, TriggerCode_Move );
		}

		/////
		/// [2/17/2014 baomin]
		/// ������
		/// @param nStep ���ý��� 0 - ����ǰ, 1 - ������, 2 - ���ͺ�
		/// @param iNewArea Ŀ�������
		/// @param iOldArea ԭ�ȵ�����
		/// @param lpContext ������
		/////
		virtual xgc_void OnTeleport( xgc_uint16 nStep, const XVector3& vOldPosition, xgc_uintptr lpContext )
		{
			Trigger( Trigger_ObjectEvent, nStep, lpContext, TriggerCode_Teleport );
		}

		//////////////////////////////////////////////////////////////////////////
		// ����λ��
		XGC_INLINE xgc_void SetPosition( xgc_real32 fPosX, xgc_real32 fPosY, xgc_real32 fPosZ )
		{
			mPosition.SetValue( fPosX, fPosY, fPosZ );
		}

		XGC_INLINE xgc_void SetPosition( const xgc_real32 fPos[3] )
		{
			mPosition[0] = fPos[0];
			mPosition[1] = fPos[1];
			mPosition[2] = fPos[2];
		}

		/////
		/// ���볡�� 
		/// [11/8/2010 Albert]
		/////
		virtual xgc_bool PreEnterMap( XGameMap* pScene ) 
		{ 
			return true; 
		}

		/////
		/// �뿪���� 
		/// [11/8/2010 Albert]
		/////
		virtual xgc_bool PreLeaveMap( XGameMap* pScene ) 
		{ 
			return true; 
		}

		/////
		/// ��ʼ�����볡������
		/// [11/8/2010 Albert]
		/////
		virtual xgc_void OnBornInMap( XGameMap* pScene ) 
		{
			Trigger( Trigger_ObjectEvent, IsVisible(), 0, TriggerCode_BornInMap );
		}

		/////
		/// ���볡�� 
		/// [11/8/2010 Albert]
		/////
		virtual xgc_void OnEnterMap( XGameMap* pScene ) 
		{
			Trigger( Trigger_ObjectEvent, IsVisible(), 0, TriggerCode_EnterMap );
		}

		/////
		/// �뿪���� 
		/// [11/8/2010 Albert]
		/////
		virtual xgc_void OnLeaveMap( XGameMap* pScene ) 
		{
			Trigger( Trigger_ObjectEvent, IsVisible(), 0, TriggerCode_LeaveMap );
		}

		///////////////////////////////////////////////////////////////////////////
		/// XObject�ӿ�

		/////
		/// �����ӽڵ�ǰ����
		/// [8/3/2009 Albert]
		/// @return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		/////
		virtual xgc_bool PreAddChild( XObject* pChild, xgc_lpcvoid lpContext ) { return true; }

		/////
		/// �����ӽڵ�����
		/// [8/3/2009 Albert]
		/// @param pChild ��ӵ��Ӷ���ָ��
		/// @param lpContext ��Ӷ���ʱ�����Ĳ���
		/////
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext ) {}

		/////
		/// �����ӽڵ�ǰ����
		/// [8/3/2009 Albert]
		/// @param pChild ��ӵ��Ӷ���ָ��
		/// @param bRelease �Ƿ�����
		/// @return true - ȷ���Ƴ��ӽڵ�, false - �ӽڵ㱻���,�Ƴ��ӽڵ�ʧ��.
		/////
		virtual xgc_bool PreRemoveChild( XObject* pChild, xgc_bool bRelease ) { return true; }

		/////
		/// ɾ���ӽڵ�����,��ʱ������δ��ɾ��
		/// [8/3/2009 Albert]
		/// @param pChild ��ӵ��Ӷ���ָ��
		/// @param bRelease �Ƿ�����
		/////
		virtual xgc_void OnRemoveChild( XObject* pChild, xgc_bool bRelease ) {}

	private:
		XVector3	mPosition;	///< λ�ã���������
		XVector3	mDirection; ///< ��������
		XVector3	mSpeed;		///< �ٶȣ�����

		xgc_uint16	mTriggerDeep;	///< ����Trigger�У�������������ɾ�� Trigger����Trigger��ɺ�һ������

		///
		/// �������۲���
		/// [6/28/2014] create by albert.xu
		///
		struct TriggerObserver
		{
			xObject first; 
			TriggerFunctor second;
		};

		struct TriggerList { xgc_list< TriggerObserver > mTriggerList; };
		struct TriggerCode { xgc_unordered_map< xgc_uint16, TriggerList > mTriggerCode; };
		struct TriggerEvent { xgc_unordered_map< xgc_uint16, TriggerCode >	mTriggerEvent; };

		TriggerEvent mTriggerTable;
	};
}
#endif // _XGAME_OBJECT_H_