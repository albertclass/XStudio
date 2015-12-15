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
�����ڴ�Ľṹ����ͼ��ʾ��
ͷ��3��int����ͷβ����󳤶���Ϣ��β������߼������ݡ�
--------------------------------------------
| xgc_uint32 | xgc_uint32 | xgc_uint32  | char[shmLen] |
--------------------------------------------
|  head  |  tail  | maxsize | shmBuf       |
--------------------------------------------
�������ڴ棬��һ��һд���̼������������ȫͨ�š�

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

		/// ���������ڴ棬����Ѵ��ڣ���attach��ȥ \///
		xgc_bool Create( xgc_lpcstr shmName, xgc_size size = Default_ShmBuffer_Size, xgc_lpcstr shmLogPath = ".", xgc_bool isCreateFile = false );

		///
		/// �Ƿ�ҽ�
		/// [9/3/2014] create by albert.xu
		///
		xgc_bool IsCreated()const{ return mCreated; }

		/// �����ڴ�����ɲ���ϵͳ����ά��,����map������handle���رպ� \///
		/// ϵͳ���Զ�ɾ����. \///
		xgc_void Destroy();

		xgc_string& GetName() { return mShmName; }

		///
		/// [12/16/2013 albert.xu]
		/// д���ݵ�������
		///
		xgc_bool Write( const xBuffer& buf, xgc_bool isOverWrite = false );

		///
		/// [12/16/2013 albert.xu]
		/// �ӻ�����������
		///
		xgc_bool Read( xBuffer& buf, xgc_size &readSize );

		//-----------------------------------//
		// [12/16/2013 albert.xu]
		// ǿ��д�ļ�
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