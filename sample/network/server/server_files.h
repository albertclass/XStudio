#pragma once
#ifndef _SERVER_FILE_H_ 
#define _SERVER_FILE_H_ 
class CServerFiles
{
friend CServerFiles& getServerFiles();

private:
	struct fileinfo
	{
		int fd;
		/// �ļ�·��
		xgc_char path[XGC_MAX_PATH];
		/// �ļ���
		xgc_char name[XGC_MAX_FNAME];
		/// �ļ����
		xgc_uint32 sequence;
		/// �ļ���С
		xgc_uint64 size;
		/// �ļ�������
		xgc_char file[1];
	};

	/// �ļ�ӳ��� - ���к�
	std::unordered_map< xgc_uint32, fileinfo* > files_seq_;

	/// �ļ�ӳ��� - �ļ���
	std::unordered_map< xgc_string, xgc_uint32 > files_;
	/// �ļ����к�
	xgc_uint32 sequence_;

	/// �ļ������б�
	std::list< xgc_string > ignore_files;
	
private:
	CServerFiles();
	~CServerFiles();

public:
	///
	/// \brief ���ɺ����б�
	///
	/// \author albert.xu
	/// \date 2017/03/30 18:03
	///
	xgc_long GenIgnoreList( xgc_lpcstr root );

	///
	/// \brief �Ƿ��Ǻ��Ե��ļ�
	///
	/// \author albert.xu
	/// \date 2017/03/30 18:25
	///
	xgc_bool IsIgnoreFile( xgc_lpcstr path, xgc_lpcstr name );

	///
	/// \brief �����ļ��б�
	///
	/// \return file sequence
	///
	/// \author albert.xu
	/// \date 2017/03/27 15:25
	///
	xgc_long GenFileList( xgc_lpcstr root );

	///
	/// \brief ��ȡ�ļ���Ϣ
	///
	/// \return file sequence
	///
	/// \author albert.xu
	/// \date 2017/03/27 15:25
	///
	xgc_uint32 GetFileInfo( xgc_lpcstr path, xgc_lpcstr name, xgc_uint64 *length = xgc_nullptr );

	///
	/// \brief ��ȡ�ļ�����
	///
	/// \author albert.xu
	/// \date 2017/03/27 15:25
	///
	xgc_int32 GetFileData( xgc_uint32 sequence, xgc_long offset, xgc_lpvoid buffer, xgc_long length );
};

CServerFiles& getServerFiles();
#endif // _SERVER_FILE_H_ 

