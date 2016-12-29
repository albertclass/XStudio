#pragma once
#if (_MSC_VER >= 1500) && (_MSC_FULL_VER >= 150030729) && !defined( _USE_BOOST )
	#include <functional>
	using namespace std;
	#if (_MSC_VER >= 1600)
		using namespace std::placeholders;
	#else
	using namespace std::tr1;
	using namespace std::tr1::placeholders;
	#endif
#else
	#include "boost\function.hpp"
	#include "boost\bind.hpp"
	using namespace boost;
#endif

namespace XGC
{
	class XGameObject;
	class XStateObject;
	
	struct CORE_API StateParams
	{
		union
		{
			struct
			{
				xgc_byte bit7 : 1;
				xgc_byte bit6 : 1;
				xgc_byte bit5 : 1;
				xgc_byte bit4 : 1;
				xgc_byte bit3 : 1;
				xgc_byte bit2 : 1;
				xgc_byte bit1 : 1;
				xgc_byte bit0 : 1;
			};
			xgc_byte mark;
		};
	};

	typedef xgc_void( *StateDestructor )( xgc_intptr userdata );
	typedef xgc_void (*GuardFunction)( XGameObject& Object, XStateObject* pParentState, StateParams* pParams );
	typedef function< xgc_void ( XGameObject& Object, XStateObject* pParentState, StateParams* pParams ) >	GuardFunctor;// 守护条件

	class CORE_API XStateObject /*: public XObject*/
	{
	public:
		explicit XStateObject( const GuardFunctor& Fn = xgc_nullptr, xgc_uintptr pUserdata = 0, StateDestructor fnDestructor = xgc_nullptr );

		~XStateObject(void);

		//---------------------------------------------------//
		// [12/11/2010 Albert]
		// Description:	获取状态运行时间 
		//---------------------------------------------------//
		xgc_real64 GetStateLifetime()const;

		//---------------------------------------------------//
		// [12/11/2010 Albert]
		// Description:	初始化状态机 
		//---------------------------------------------------//
		xgc_bool InitSubState( xgc_uint8 nStateCount );

		//----------------------------------------//
		//
		// [10/31/2012 Albert.xu]
		// 获取自定义数据
		//----------------------------------------//
		xgc_intptr getUserdata( )const
		{ 
			return mUserdata; 
		}

		//----------------------------------------//
		//
		// [10/31/2012 Albert.xu]
		// 设置自定义数据
		//----------------------------------------//
		xgc_void setUserdata( xgc_uintptr userdata, StateDestructor fnDestructor = xgc_nullptr )
		{ 
			mUserdata = userdata; 
			mDestructor = fnDestructor; 
		}

		//--------------------------------------------------------//
		//	created:	22:1:2010   14:10
		//	filename: 	State
		//	author:		Albert.xu
		//
		//	purpose:	设置状态映射表
		//--------------------------------------------------------//
		template< class StateType >
		xgc_bool CreateSubState( xgc_uint8 nIndex, const GuardFunctor& Fn, xgc_uintptr userdata = 0 )
		{
			if( nIndex >= mStateCount ) 
				return false;

			mStatePool[nIndex] = XGC_NEW StateType( Fn );
			mStatePool[nIndex]->mParent = this;
			mStatePool[nIndex]->mRoot	= mRoot?mRoot:this;
			mStatePool[nIndex]->mUserdata = userdata;

			mCurrentSubIndex = nIndex;
			return true;
		}

		//--------------------------------------------------------//
		//	created:	22:1:2010   14:10
		//	filename: 	State
		//	author:		Albert.xu
		//
		//	purpose:	无条件迁移状态
		//--------------------------------------------------------//
		xgc_bool Switch( xgc_uint8 nIndex, XGameObject& Object, StateParams& Params );

		//---------------------------------------------------//
		// [12/18/2010 Albert]
		// Description:	更新状态 
		//---------------------------------------------------//
		xgc_void UpdateState( XGameObject& Object );

		//---------------------------------------------------//
		// [12/17/2010 Albert]
		// Description:	获取当前状态 
		//---------------------------------------------------//
		XStateObject* GetCurSubState(){ return mStatePool?mStatePool[mCurrentSubIndex]:xgc_nullptr; }

		//---------------------------------------------------//
		// [12/17/2010 Albert]
		// Description:	获取当前状态 
		//---------------------------------------------------//
		xgc_uint8 GetCurSubIndex(){ return mCurrentSubIndex; }

		//---------------------------------------------------//
		// [2/7/2012 Albert]
		// Description:	获取根状态机
		//---------------------------------------------------//
		XStateObject* GetRoot(){ return mRoot; }

		//---------------------------------------------------//
		// [2/7/2012 Albert]
		// Description:	获取父状态机
		//---------------------------------------------------//
		XStateObject* GetParent(){ return mParent; }

	protected:
		//--------------------------------------------------------//
		//	created:	21:1:2010   14:33
		//	filename: 	State
		//	author:		Albert.xu
		//
		//	purpose:	进入状态前调用
		//	pEntity:	进入状态的对象指针
		//	return:		TRUE 进入状态成功， false进入状态失败
		//--------------------------------------------------------//
		virtual xgc_bool OnEnter( XGameObject& pObject, StateParams& Params )
		{ 
			mTimeStamp = current_millisecond(); 
			return true; 
		}

		//----------------------------------------//
		//
		// [11/26/2012 Albert.xu]
		// 状态转换时调用，转换状态相同的时候也会被调用。
		//----------------------------------------//
		virtual xgc_void OnActive( XGameObject& Object, StateParams& Params ){}

		//--------------------------------------------------------//
		//	created:	21:1:2010   14:35
		//	filename: 	State
		//	author:		Albert.xu
		//
		//	purpose:	离开状态
		//	pEntity:	进入状态的对象指针
		//--------------------------------------------------------//
		virtual xgc_void OnLeave( XGameObject& Object ){}

	private:
		//--------------------------------------------------------------//
		//	created:	2:2:2010   14:37
		//	filename: 	FsmBase.h
		//	author:		Albert.xu

		//	Description : 	获取参数信息
		//--------------------------------------------------------------//
		virtual StateParams* GetParams(){ return xgc_nullptr; }

	protected:
		GuardFunctor	mGuardFunctor;
		StateDestructor	mDestructor;
		XStateObject*	mRoot;
		XStateObject*	mParent;

		xgc_uint8		mStateCount;
		xgc_uint8		mCurrentSubIndex;
		XStateObject**	mStatePool;
		xgc_uintptr		mUserdata;

		xgc_uint64		mTimeStamp;
	};
}