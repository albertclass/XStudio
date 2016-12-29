#pragma once
#include "stdafx.h"
#include "XGameObject.h"
#include "XStateObject.h"

namespace XGC
{
	XStateObject::XStateObject( const GuardFunctor& Fn, xgc_uintptr pUserdata, StateDestructor fnDestructor )
		: mGuardFunctor( Fn )
		, mUserdata( pUserdata )
		, mDestructor( fnDestructor )
		, mParent( xgc_nullptr )
		, mRoot( xgc_nullptr )
		, mCurrentSubIndex( 0 )
		, mStatePool( xgc_nullptr )
		, mStateCount( 0 )
	{
	}

	XStateObject::~XStateObject(void)
	{
		if( mDestructor )
			mDestructor( mUserdata );
		mUserdata = 0;

		for( xgc_uint32 i = 0; i < mStateCount; ++i )
			SAFE_DELETE( mStatePool[i] );
		free( mStatePool );
	}

	xgc_bool XStateObject::InitSubState( xgc_uint8 nStateCount )
	{
		mStatePool = (XStateObject**)malloc( sizeof(XStateObject*)*nStateCount );
		memset( mStatePool, 0, sizeof(mStatePool) );

		mStateCount = nStateCount;
		return true;
	}

	xgc_real64 XStateObject::GetStateLifetime()const
	{
		return ( current_millisecond() - mTimeStamp ) * 1000.0f;
	}

	//--------------------------------------------------------//
	//	created:	22:1:2010   14:10
	//	filename: 	State
	//	author:		Albert.xu
	//
	//	purpose:	无条件迁移状态
	//--------------------------------------------------------//
	xgc_bool XStateObject::Switch( xgc_uint8 nIndex, XGameObject& Object, StateParams& Params )
	{
		if( nIndex >= mStateCount )
			return false;

		XStateObject *pNewState = mStatePool[nIndex];
		XStateObject *pOldState = GetCurSubState();
		
		if( pNewState == NULL )
			return false;

		if( pNewState != pOldState )
		{
			if( pOldState )
				pOldState->OnLeave( Object );

			mCurrentSubIndex = nIndex;

			pNewState->OnEnter( Object, Params );
		}

		pNewState->OnActive( Object, Params );
		return true;
	}

	//---------------------------------------------------//
	// [12/18/2010 Albert]
	// Description:	更新状态 
	//---------------------------------------------------//
	xgc_void XStateObject::UpdateState( XGameObject& Object )
	{
		if( GetCurSubState() )
			GetCurSubState()->UpdateState( Object );

		if( mGuardFunctor )
			mGuardFunctor( Object, mParent, GetParams() );
	}
}