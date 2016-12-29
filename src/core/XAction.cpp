#include "StdAfx.h"
#include "XAction.h"
#include "destructor.h"
#include "json/json.h"

namespace XGC
{
	XActionNode::~XActionNode()
	{
		delete mNode;
		mNode = xgc_nullptr;
		DestroyContainer( mLstChild );
		mLstChild.clear();
	}

	EnBasicActionRes XActionNode::TriggerSelectActionNode( XActionNode* selectNode, xObject hSource, xObject hTarget, xgc_intptr wParam, xgc_intptr lParam, Json::Value* JsonRoot, xgc_bool isOnlyDoModifyAttr )
	{
		xgc_int32 selectres = selectNode->mNode->mFuncPtr.selectFunc( 
			hSource, hTarget, wParam, lParam, selectNode->mNode->mParams.data(), selectNode->mNode->mParams.length(), JsonRoot );
		xgc_unordered_map< xgc_int32, xgc_vector<XActionNode*> >::iterator it = selectNode->mLstChild.find( selectres );
		if( selectNode->mLstChild.end() != it )
		{
			xgc_vector<XActionNode*>& vecNode = it->second;
			for( auto& itNode : vecNode )
			{
				if( TriggerActionNode( itNode, hSource, hTarget, wParam, lParam, JsonRoot, isOnlyDoModifyAttr ) == enBasicAction_failed )
					return enBasicAction_failed;
			}
		}
		else
		{
			// 没有找到，做default对应的逻辑，key = std::numeric_limits<xgc_int32>::max()
			xgc_unordered_map< xgc_int32, xgc_vector<XActionNode*> >::iterator it02 = selectNode->mLstChild.find( std::numeric_limits<xgc_int32>::max() );
			if( it02 != selectNode->mLstChild.end() )
			{
				xgc_vector<XActionNode*>& vecNode = it02->second;
				for( auto& itNode : vecNode )
				{
					if( TriggerActionNode( itNode, hSource, hTarget, wParam, lParam, JsonRoot, isOnlyDoModifyAttr ) == enBasicAction_failed )
						return enBasicAction_failed;
				}
			}
		}
		return enBasicAction_none;
	}
	EnBasicActionRes XActionNode::TriggerBasicActionNode( XActionNode* basicNode, xObject hSource, xObject hTarget, xgc_intptr wParam, xgc_intptr lParam, Json::Value* JsonRoot, xgc_bool isOnlyDoModifyAttr )
	{
		//////////////////////////////////////////////////////////////////////////
		// 下面是技能升级的处理、del状态的处理
		// 如果是回滚属性操作，只处理修改属性的函数
		if( isOnlyDoModifyAttr )
		{
			Json::Value root = *JsonRoot;
			Json::Value updateIt = root[UPDATE_SKILL_MAP_PROP_UNIQUE];
			Json::Value delIt = root[DELETE_STATE_MAP_PROP_UNIQUE];
			if( !updateIt.isNull() )
			{
				if( updateIt.asBool() )
				{
					BasicActionFunc func01 = (BasicActionFunc) GetProcAddress( ::GetModuleHandle( nullptr ), "ModifyTargetAttribute" );
					BasicActionFunc func02 = (BasicActionFunc) GetProcAddress( ::GetModuleHandle( nullptr ), "ModifySelfAttribute" );
					if( basicNode->mNode->mFuncPtr.basicFunc != func01 && basicNode->mNode->mFuncPtr.basicFunc != func02 )
					{
						return enBasicAction_success;
					}
				}
			}
			else if( !delIt.isNull() )
			{
				if( delIt.asBool() )
				{
					BasicActionFunc func01 = (BasicActionFunc) GetProcAddress( ::GetModuleHandle( nullptr ), "ModifyTargetAttribute" );
					BasicActionFunc func02 = (BasicActionFunc) GetProcAddress( ::GetModuleHandle( nullptr ), "ModifySelfAttribute" );
					if( basicNode->mNode->mFuncPtr.basicFunc != func01 && basicNode->mNode->mFuncPtr.basicFunc != func02 )
					{
						return enBasicAction_success;
					}
				}
			}
		}
		//////////////////////////////////////////////////////////////////////////
		return basicNode->mNode->mFuncPtr.basicFunc( hSource, hTarget, wParam, lParam, basicNode->mNode->mParams.data(), basicNode->mNode->mParams.length(), JsonRoot );
	}
	EnBasicActionRes XActionNode::TriggerActionNode( XActionNode* node, xObject hSource, xObject hTarget, xgc_intptr wParam, xgc_intptr lParam, Json::Value* JsonRoot, xgc_bool isOnlyDoModifyAttr )
	{
		switch( node->mActionType )
		{
			case enActionNode_select:
			{
				return TriggerSelectActionNode( node, hSource, hTarget, wParam, lParam, JsonRoot, isOnlyDoModifyAttr );
			}
			break;
			case enActionNode_basic:
			{
				return TriggerBasicActionNode( node, hSource, hTarget, wParam, lParam, JsonRoot, isOnlyDoModifyAttr );
			}
			break;
		}
		return enBasicAction_none;
	}
	//////////////////////////////////////////////////////////////////////////
	// Action
	XAction::XAction( BasicActionFunc Functor )
		: mFunctor( Functor )
		, mParams( xgc_nullptr, 0 )
	{

	}

	XAction::XAction( BasicActionFunc Functor, xBuffer Params )
		: mFunctor( Functor )
		, mParams( Params )
	{

	}

	XAction::XAction( const BasicActionFunc Functor, xgc_lpvoid pData, xgc_size nSize )
		: mFunctor( Functor )
		, mParams( pData, nSize )
	{
	}

	XAction::XAction( const XAction& rhs )
		: mFunctor( rhs.mFunctor )
		, mParams( rhs.mParams )
	{

	}

	EnBasicActionRes XAction::Execute( xObject hSender, xObject hReciver, xgc_uintptr wParam, xgc_uintptr lParam, Json::Value* JsonRoot )
	{
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( mFunctor, enBasicAction_none );

		return mFunctor( hSender, hReciver, wParam, lParam, mParams.data(), mParams.length(), JsonRoot );
		FUNCTION_END;
		return enBasicAction_failed;
	}

	XActionTree::~XActionTree()
	{
		FUNCTION_BEGIN;
		DestroyContainer( mChildrens );
		FUNCTION_END;
	}

	EnBasicActionRes XActionTree::Execute( xObject hSource, xObject hTarget, xgc_uintptr wParam, xgc_uintptr lParam, Json::Value* JsonRoot )
	{
		FUNCTION_BEGIN;
		xgc_uintptr params[3] = { 0xcdeffedc, wParam, lParam };
		wParam = reinterpret_cast<xgc_intptr>( params );
		lParam = reinterpret_cast<xgc_intptr>( &mChildrens );
		return mFunctor( hSource, hTarget, wParam, lParam, mParams.data(), mParams.length(), JsonRoot );
		FUNCTION_END;
		return enBasicAction_failed;
	}

	//////////////////////////////////////////////////////////////////////////
	// Status class
	//////////////////////////////////////////////////////////////////////////
	XStatusInstance::XStatusInstance()
	{
	}

	XStatusInstance::~XStatusInstance()
	{
	}

	//---------------------------------------------------//
	// [12/14/2010 Albert]
	// Description:	添加作用 
	//---------------------------------------------------//
	xgc_void XStatusInstance::AppendEffect( const StatusFunc& Functor, xBuffer Params )
	{
		effect e = { Params, Functor };
		m_EffectContainer.push_back( e );
	}

	//---------------------------------------------------//
	// [11/19/2010 Albert]
	// Description:	执行状态 
	//---------------------------------------------------//
	xgc_void XStatusInstance::DoStatus( xObject nIdentity, xgc_uintptr& pUserdata, eStatus eFlag )
	{
		for( EffectContainer::iterator i = m_EffectContainer.begin(); i != m_EffectContainer.end(); ++i )
		{
			i->mFunctor( nIdentity, (xgc_int32)eFlag, pUserdata, i->mParams.data(), i->mParams.length() );
		}
	}

	XStatus::~XStatus()
	{
		mInstance->DoStatus( GetParent(), m_Userdata, status_remove );

		getTimer().remove_event( mUpdateTimerHandler );
		mUpdateTimerHandler = INVALID_TIMER_HANDLE;
	}

	xgc_void XStatus::InstallTimer( float fLifetime, float fInterval, float fDelay )
	{
		DoStatus( status_append );

		if( fLifetime != 0.0f )
		{
			mLifetimeHandler = getTimer().insert_event( bind( &XStatus::OnStatusOver, this, _1 ), 1, -1, fLifetime );
		}

		if( fInterval != 0.0f )
		{
			// 插入状态作用定时器
			mUpdateTimerHandler = getTimer().insert_event( 
				bind( &XStatus::OnUpdate, this, _1 ), 
				xgc_int32((fLifetime - fDelay)/fInterval), 
				fInterval,
				fDelay );
		}

	}

	xgc_void XStatus::OnUpdate( timer_t handle )
	{
		mInstance->DoStatus( GetParent(), m_Userdata, status_active );
	}

	xgc_void XStatus::OnStatusOver( timer_t handle )
	{
		getTimer().remove_event( mUpdateTimerHandler );
		getTimer().remove_event( mLifetimeHandler );
		mUpdateTimerHandler = INVALID_TIMER_HANDLE;
		mLifetimeHandler = INVALID_TIMER_HANDLE;

		delete this;
	}
}
