#include "config.h"
#include "pipe_manager.h"
#include "pipe_session.h"

namespace net_module
{
	CPipeManager::CPipeManager()
	{
	}

	CPipeManager::~CPipeManager()
	{
	}

	xgc_bool CPipeManager::PipeConnect( NETWORK_ID nID, xgc_lpcstr pAddr, xgc_uint16 nPort, xgc_uint32 nTimeout )
	{
		auto it = mPipeMap.find( nID );
		if( it != mPipeMap.end() )
			return true;

		// ·ÀÖ¹ÖØ¸´Á¬½Ó
		mPipeMap[nID] = xgc_nullptr;

		net::ConnectAsync( pAddr, nPort, nTimeout, XGC_NEW CPipeSession() );
		return true;
	}

	xgc_bool CPipeManager::OnPipeConnect( NETWORK_ID nID, CPipeSession * pPipe )
	{
		auto it = mPipeMap.find( nID );
		if( it == mPipeMap.end() || it->second == xgc_nullptr )
		{
			mPipeMap[nID] = pPipe;
			return true;
		}

		return false;
	}

	CPipeSession * CPipeManager::GetPipe( NETWORK_ID nID )
	{
		auto it = mPipeMap.find( nID );
		if( it != mPipeMap.end() )
			return it->second;

		return xgc_nullptr;
	}

	CPipeManager & getPipeMgr()
	{
		static CPipeManager inst;
		return inst;
	}
}