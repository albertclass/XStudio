#pragma once
#ifndef _XGAME_OBJECT_H_
#define _XGAME_OBJECT_H_

namespace xgc
{
	//////////////////////////////////////////////////////////////////////////
	// 基础对象类， 所有屏幕上的对象都从此类派生
	extern CORE_API xAttrIndex attrObjectName;		///< 对象名称
	extern CORE_API xAttrIndex attrObjectAlias;		///< 对象名称
	extern CORE_API xAttrIndex attrObjectFlags;		///< 对象标记位

	enum class CORE_API VisualMode
	{
		eEnter,	/// 进入（从无到有）
		eLeave,	/// 离开（从有到无）
		eAppear, /// 出现（隐身到可视）
		eVanish, /// 消失（可视到隐身）
		eShowOther,	/// 显示周围的场景对象
		eHideOther,	/// 隐藏周围的场景对象
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

		// 直接世界坐标
		XGC_INLINE xgc_void GetPosition( xgc_real32 fPos[3] )const
		{
			fPos[0] = mPosition[0];
			fPos[1] = mPosition[1];
			fPos[2] = mPosition[2];
		}

		// 直接世界坐标
		XGC_INLINE xgc_void GetPosition( XVector3& Position )const
		{
			Position = mPosition;
		}

		// 直接世界坐标
		XGC_INLINE const XVector3& GetPosition()const
		{
			return mPosition;
		}

		// 直接世界坐标
		XGC_INLINE xgc_void GetPosition( float &x, float &y, float &z )const
		{
			x = mPosition.x;
			y = mPosition.y;
			z = mPosition.z;
		}

		//////////////////////////////////////////////////////////////////////////
		// 对象的方向
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
		// 用角度值设置方向坐标
		// ------------------------------------------------ //
		XGC_INLINE xgc_void SetDirection( const xgc_real32 fAngle )
		{
			float fRad = DEG2RAD( fAngle );
			SetDirection( XMath::Cos( fRad ), -XMath::Sin( fRad ), 0 );
		}

		// 直接世界坐标
		XGC_INLINE xgc_void GetDirection( xgc_real32 fPos[3] )const
		{
			fPos[0] = mDirection[0];
			fPos[1] = mDirection[1];
			fPos[2] = mDirection[2];
		}

		// 直接世界坐标
		XGC_INLINE xgc_void GetDirection( XVector3& Position )const
		{
			Position = mDirection;
		}

		// 直接世界坐标
		XGC_INLINE const XVector3& GetDirection()const
		{
			return mDirection;
		}

		/// ------------------------------------------------ //
		/// 获取当前方向的角度值
		/// [4/18/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		XGC_INLINE const xgc_uint16 GetDirectionAngle()const
		{
			return  mDirection.y < 0 ? 
				(xgc_uint16) ( XMath::ACos( mDirection.DotProduct( XVector3( 1, 0, 0 ) ) ) * 180 / XMath::ONE_PI ) : 
				(xgc_uint16) ( 360 - XMath::ACos( mDirection.DotProduct( XVector3( 1, 0, 0 ) ) ) * 180 / XMath::ONE_PI );
		}

		///
		/// 设置是否可见
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
		/// 判定是否可见
		/// [1/17/2014 albert.xu]
		///
		XGC_INLINE xgc_bool IsVisible()
		{
			return GetBit( attrObjectFlags, Flag_NotifyEyeshot, true );
		}

		///
		/// 设置移动标志
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
		/// 获取移动标志 true - 可移动, false - 不可移动
		/// [1/17/2014 albert.xu]
		///
		XGC_INLINE xgc_bool GetMoveFlag()
		{
			return GetBit( attrObjectFlags, Flag_CanMove, true );
		}

		///
		/// 获取栅栏设置标志
		/// [6/28/2014] create by albert.xu
		///
		XGC_INLINE xgc_bool GetBarrierFlag()
		{
			return GetBit( attrObjectFlags, Flag_Barrier, false );
		}

		///
		/// 设置栅栏设置标志
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
		/// 获取霸体护甲
		/// [8/5/2014] create by jianglei.kinly
		///
		XGC_INLINE xgc_bool GetBaTiFlag()
		{
			return GetBit( attrObjectFlags, Flag_BaTi, false );
		}

		///
		/// 设置霸体护甲
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
		/// 出现在场景中（出现在别人的视野中）
		/// @param fnNotifyFilter 用于决定进入视野消息发送的条件
		/// [10/14/2014] create by albert.xu
		///
		xgc_void Appear( const std::function< xgc_bool( xObject ) > &fnNotifyFilter );

		///
		/// 消失在场景中（从别人的视野中消失）
		/// @param fnNotifyFilter 用于决定离开视野消息发送的条件
		/// [10/14/2014] create by albert.xu
		///
		xgc_void Vanish( const std::function< xgc_bool( xObject ) > &fnNotifyFilter );

		///
		/// 显示角色周围的场景对象
		/// @param fnNotifyFilter 用于决定离开视野消息发送的条件
		/// [12/5/2014] create by albert.xu
		///
		xgc_void ShowOther( const std::function< xgc_bool( xObject ) > &fnNotifyFilter );

		///
		/// 隐藏角色周围的场景对象
		/// @param fnNotifyFilter 用于决定离开视野消息发送的条件
		/// [12/5/2014] create by albert.xu
		///
		xgc_void HideOther( const std::function< xgc_bool( xObject ) > &fnNotifyFilter );

	protected:
		//////////////////////////////////////////////////////////////////////////
		// 对象位置
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
		/// 对象进入视野
		/// @param pObject 进入视野的对象指针
		///
		virtual xgc_void OnEnterEyeshot( XGameObject* pObject, VisualMode eMode )
		{
			EmmitEvent( evt_enter_vision );
		}

		///
		/// [8/3/2009 Albert]
		/// Description:	对象离开视野
		/// pObject	:	离开视野的对象指针
		///
		virtual xgc_void OnLeaveEyeshot( XGameObject* pObject, VisualMode eMode )
		{
			EmmitEvent( evt_leave_vision );
		}

		///
		/// [2/17/2014 baomin]
		/// 对象在视野内移动
		/// @param vOldPosition 老的坐标位置
		/// @param lpContext 上下文
		///
		virtual xgc_void OnMove( const XVector3& vOldPosition, xgc_uintptr lpContext )
		{
			EmmitEvent( evt_move );
		}

		///
		/// [2/17/2014 baomin]
		/// 对象传送
		/// @param nStep 调用进度 0 - 传送前, 1 - 传送中, 2 - 传送后
		/// @param iNewArea 目标的区域
		/// @param iOldArea 原先的区域
		/// @param lpContext 上下文
		///
		virtual xgc_void OnTeleport( xgc_uint16 nStep, const XVector3& vOldPosition, xgc_uintptr lpContext )
		{
			EmmitEvent( evt_teleport );
		}

		///
		/// 进入场景 
		/// [11/8/2010 Albert]
		///
		virtual xgc_bool PreEnterMap( XGameMap* pScene ) 
		{ 
			return true;
		}

		///
		/// 离开场景 
		/// [11/8/2010 Albert]
		///
		virtual xgc_bool PreLeaveMap( XGameMap* pScene ) 
		{ 
			return true;
		}

		///
		/// 初始化进入场景数据
		/// [11/8/2010 Albert]
		///
		virtual xgc_void OnBornInMap( XGameMap* pScene ) 
		{
			EmmitEvent( evt_born );
		}

		///
		/// 进入场景 
		/// [11/8/2010 Albert]
		///
		virtual xgc_void OnEnterMap( XGameMap* pScene ) 
		{
			EmmitEvent( evt_enter_map );
		}

		/////
		/// 离开场景 
		/// [11/8/2010 Albert]
		/////
		virtual xgc_void OnLeaveMap( XGameMap* pScene ) 
		{
			EmmitEvent( evt_leave_map );
		}

		///
		/// \brief 对象销毁时调用
		/// \date 11/10/2017
		/// \author xufeng04
		///
		xgc_void OnDestroy() override {}
	private:
		/// @var 位置，世界坐标
		XVector3 mPosition;	
		/// @var 方向，向量
		XVector3 mDirection; 
		/// @var 速度，标量
		XVector3 mSpeed;		
	};
}
#endif // _XGAME_OBJECT_H_