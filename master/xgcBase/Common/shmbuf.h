/********************************************************************
	created:	2010/04/14 9:18
	filename: 	shmbufimpl.h
	author:		lcj
	purpose:
	*********************************************************************/
#ifndef _SHMBUF_WIN32_H_INCLUDED_
#define _SHMBUF_WIN32_H_INCLUDED_

#include "defines.h"
#include "xbuffer.h"
/*
共享内存的结构如下图所示：
头部3个int放置头尾和最大长度信息，尾部存放逻辑层数据。
--------------------------------------------
| xgc_uint32 | xgc_uint32 | xgc_uint32  | char[shmLen] |
--------------------------------------------
|  head  |  tail  | maxsize | shmBuf       |
--------------------------------------------
本共享内存，在一读一写进程间可做到无锁安全通信。

*/
namespace XGC
{
	const xgc_size Default_ShmBuffer_Size = 1024 * 1024;
	class COMMON_API ShmBuffer
	{
	public:
		ShmBuffer();
		~ShmBuffer();

		//////////////////////////////////////////////////////////////////////////

		/// 创建共享内存，如果已存在，则attach上去 \///
		xgc_bool Create( xgc_lpcstr shmName, xgc_size size = Default_ShmBuffer_Size, xgc_lpcstr shmLogPath = ".", xgc_bool isCreateFile = false );

		///
		/// 是否挂接
		/// [9/3/2014] create by albert.xu
		///
		xgc_bool IsCreated()const{ return mCreated; }

		/// 共享内存对象由操作系统自行维护,所有map到它的handle都关闭后， \///
		/// 系统会自动删除它. \///
		xgc_void Destroy();

		xgc_string& GetName() { return mShmName; }

		///
		/// [12/16/2013 albert.xu]
		/// 写数据到缓冲区
		///
		xgc_bool Write( const xBuffer& buf, xgc_bool isOverWrite = false );

		///
		/// [12/16/2013 albert.xu]
		/// 从缓冲区读数据
		///
		xgc_bool Read( xBuffer& buf, xgc_size &readSize );

		//-----------------------------------//
		// [12/16/2013 albert.xu]
		// 强制写文件
		//-----------------------------------//
		xgc_bool ForceWriteToFile( xgc_lpcstr pszFilename );
	private:
		struct Infomation;

		xgc_handle	hMapFile;
		xgc_lpvoid	pShm;

		xBuffer		mShm;
		xBuffer		mShmBuffer;

		xgc_string	mShmName;
		xgc_bool	mCreated;

		Infomation	*pInfomation;

		xgc_string  mLogPath;
	};

	COMMON_API ShmBuffer* CreateShmBuffer( xgc_lpcstr shmName, xgc_size size, xgc_lpcstr shmLogPath, xgc_bool isCreateFile = false );
}
#endif // _SHMBUF_WIN32_H_INCLUDED_