#pragma once
#ifndef _XGAME_OBJECT_H_
#define _XGAME_OBJECT_H_

namespace xgc
{
	//////////////////////////////////////////////////////////////////////////
	// ���������࣬ ������Ļ�ϵĶ��󶼴Ӵ�������
	extern CORE_API xAttrIndex attrObjectName;		///< ��������
	extern CORE_API xAttrIndex attrObjectAlias;		///< ��������
	extern CORE_API xAttrIndex attrObjectFlags;		///< ������λ

	enum class CORE_API VisualMode
	{
		eEnter,	/// ���루���޵��У�
		eLeave,	/// �뿪�����е��ޣ�
		eAppear, /// ���֣��������ӣ�
		eVanish, /// ��ʧ�����ӵ�����
		eShowOther,	/// ��ʾ��Χ�ĳ�������
		eHideOther,	/// ������Χ�ĳ�������
	};

	enum CORE_API eGameObjectEvent
	{
		evt_born,
		evt_dead,
		evt_move,
		evt_enter_vision,
		evt_leave_vision,
		evt_appear,
		evt_vanish,
		evt_enter_map,
		evt_leave_map,
		evt_teleport,
	};

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
				SetBit( attrObjectFlags, Flag_NotifyEyeshot );
			else
				ClrBit( attrObjectFlags, Flag_NotifyEyeshot );
		}

		///
		/// �ж��Ƿ�ɼ�
		/// [1/17/2014 albert.xu]
		///
		XGC_INLINE xgc_bool IsVisible()
		{
			return GetBit( attrObjectFlags, Flag_NotifyEyeshot, true );
		}

		///
		/// �����ƶ���־
		/// [1/17/2014 albert.xu]
		///
		XGC_INLINE xgc_void SetMoveFlag( xgc_bool bEnable = true )
		{
			if( bEnable )
				SetBit( attrObjectFlags, Flag_CanMove );
			else
				ClrBit( attrObjectFlags, Flag_CanMove );
		}

		///
		/// ��ȡ�ƶ���־ true - ���ƶ�, false - �����ƶ�
		/// [1/17/2014 albert.xu]
		///
		XGC_INLINE xgc_bool GetMoveFlag()
		{
			return GetBit( attrObjectFlags, Flag_CanMove, true );
		}

		///
		/// ��ȡդ�����ñ�־
		/// [6/28/2014] create by albert.xu
		///
		XGC_INLINE xgc_bool GetBarrierFlag()
		{
			return GetBit( attrObjectFlags, Flag_Barrier, false );
		}

		///
		/// ����դ�����ñ�־
		/// [6/28/2014] create by albert.xu
		///
		XGC_INLINE xgc_void SetBarrierFlag( xgc_bool bEnable = true )
		{
			if( bEnable )
				SetBit( attrObjectFlags, Flag_Barrier );
			else
				ClrBit( attrObjectFlags, Flag_Barrier );
		}

		///
		/// ��ȡ���廤��
		/// [8/5/2014] create by jianglei.kinly
		///
		XGC_INLINE xgc_bool GetBaTiFlag()
		{
			return GetBit( attrObjectFlags, Flag_BaTi, false );
		}

		///
		/// ���ð��廤��
		/// [8/5/2014] create by jianglei.kinly
		///
		XGC_INLINE xgc_void SetBaTiFlag( xgc_bool bEnable = true )
		{
			if( bEnable )
				SetBit( attrObjectFlags, Flag_BaTi );
			else
				ClrBit( attrObjectFlags, Flag_BaTi );
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

	protected:
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

		///
		/// [8/3/2009 Albert]
		/// ���������Ұ
		/// @param pObject ������Ұ�Ķ���ָ��
		///
		virtual xgc_void OnEnterEyeshot( XGameObject* pObject, VisualMode eMode )
		{
			EmmitEvent( evt_enter_vision );
		}

		///
		/// [8/3/2009 Albert]
		/// Description:	�����뿪��Ұ
		/// pObject	:	�뿪��Ұ�Ķ���ָ��
		///
		virtual xgc_void OnLeaveEyeshot( XGameObject* pObject, VisualMode eMode )
		{
			EmmitEvent( evt_leave_vision );
		}

		///
		/// [2/17/2014 baomin]
		/// ��������Ұ���ƶ�
		/// @param vOldPosition �ϵ�����λ��
		/// @param lpContext ������
		///
		virtual xgc_void OnMove( const XVector3& vOldPosition, xgc_uintptr lpContext )
		{
			EmmitEvent( evt_move );
		}

		///
		/// [2/17/2014 baomin]
		/// ������
		/// @param nStep ���ý��� 0 - ����ǰ, 1 - ������, 2 - ���ͺ�
		/// @param iNewArea Ŀ�������
		/// @param iOldArea ԭ�ȵ�����
		/// @param lpContext ������
		///
		virtual xgc_void OnTeleport( xgc_uint16 nStep, const XVector3& vOldPosition, xgc_uintptr lpContext )
		{
			EmmitEvent( evt_teleport );
		}

		///
		/// ���볡�� 
		/// [11/8/2010 Albert]
		///
		virtual xgc_bool PreEnterMap( XGameMap* pScene ) 
		{ 
			return true;
		}

		///
		/// �뿪���� 
		/// [11/8/2010 Albert]
		///
		virtual xgc_bool PreLeaveMap( XGameMap* pScene ) 
		{ 
			return true;
		}

		///
		/// ��ʼ�����볡������
		/// [11/8/2010 Albert]
		///
		virtual xgc_void OnBornInMap( XGameMap* pScene ) 
		{
			EmmitEvent( evt_born );
		}

		///
		/// ���볡�� 
		/// [11/8/2010 Albert]
		///
		virtual xgc_void OnEnterMap( XGameMap* pScene ) 
		{
			EmmitEvent( evt_enter_map );
		}

		/////
		/// �뿪���� 
		/// [11/8/2010 Albert]
		/////
		virtual xgc_void OnLeaveMap( XGameMap* pScene ) 
		{
			EmmitEvent( evt_leave_map );
		}

		///
		/// \brief ��������ʱ����
		/// \date 11/10/2017
		/// \author xufeng04
		///
		xgc_void OnDestroy() override {}
	private:
		/// @var λ�ã���������
		XVector3 mPosition;	
		/// @var ��������
		XVector3 mDirection; 
		/// @var �ٶȣ�����
		XVector3 mSpeed;		
	};
}
#endif // _XGAME_OBJECT_H_