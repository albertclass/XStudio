#pragma once
#ifndef _PIPE_MANAGER_H_ 
#define _PIPE_MANAGER_H_ 
namespace net_module
{
	class CPipeManager
	{
		friend CPipeManager& getPipeMgr();
		friend class CPipeSession;
	private:
		/// 管道网络标识映射表
		std::unordered_map< NETWORK_ID, CPipeSession* >	mPipeMap;

	private:
		///
		/// \brief 私有构造
		///
		/// \author albert.xu
		/// \date 2017/02/28 15:48
		///
		CPipeManager();

		///
		/// \brief 私有析构
		///
		/// \author albert.xu
		/// \date 2017/02/28 15:48
		///
		~CPipeManager();

		///
		/// \brief 添加管道信息
		///
		/// \author albert.xu
		/// \date 2017/03/20 14:14
		///
		xgc_bool OnPipeConnect( NETWORK_ID nID, CPipeSession* pPipe );

	public:
		///
		/// \brief 建立管道连接
		///
		/// \author albert.xu
		/// \date 2017/02/28 16:19
		///
		xgc_bool PipeConnect( NETWORK_ID nID, xgc_lpcstr pAddr, xgc_uint16 nPort, xgc_uint32 nTimeout );

		///
		/// \brief 获取管道会话
		///
		/// \author albert.xu
		/// \date 2017/02/28 16:20
		///
		CPipeSession* GetPipe( NETWORK_ID nID );
	};

	CPipeManager& getPipeMgr();
}
#endif // _PIPE_MANAGER_H_ 
