#pragma once
#ifndef _CHANNEL_H_
#define _CHANNEL_H_
#include "auto_handle.h"

class CChannel : public auto_handle< CChannel >
{
public:
	/// �û�������Ϣ
	struct user
	{
		xgc_uint32 chat_id;
		/// ���һ�η��Ե�ʱ��
		xgc_time64 last_timestamp;
		/// �Ƿ񱻽���
		xgc_bool forbid;
		/// �Ƿ����
		xgc_bool disconnect;
	};

private:
	/// Ƶ������
	xgc::string mName;

	/// �Ƿ��Զ�ɾ��
	xgc_bool mAutoDestory;

	/// �Զ�ɾ���ӳ�
	xgc_long mAutoDestoryDelay;

	/// �Զ��߳���Ĭ���
	xgc_long mAutoKickDelay;

	/// ��������
	xgc::string mPassword;

	/// ����ɾ����ʱ��
	xgc_time64 mDestoryTime;

	/// �û��б�
	xgc::map< xgc_uint32, user > mUserConfigMap;
public:
	///
	/// \brief ����
	/// \author albert.xu
	/// \date 2017/08/03
	///
	CChannel( const xgc::string &strName );

	///
	/// \brief ����
	/// \author albert.xu
	/// \date 2017/08/03
	///
	~CChannel();

	///
	/// \brief ��ȡƵ����
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_lpcstr getName()const
	{
		return mName.c_str();
	}

	///
	/// \brief ��ȡƵ����
	/// \author albert.xu
	/// \date 2017/08/11
	///
	xgc_time64 getDestoryTime() const
	{
		return mDestoryTime;
	}

	///
	/// \brief ��ȡ�û�����
	/// \author albert.xu
	/// \date 2017/08/11
	///
	xgc_size getUserCount() const
	{
		return mUserConfigMap.size();
	}

	///
	/// \brief ����Ƶ��
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_long Enter( xgc_uint32 nChatID, xgc_lpcstr lpPassword );

	///
	/// \brief �뿪Ƶ��
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void Leave( xgc_uint32 nChatID );

	///
	/// \brief ����Ծ
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void eraseQuietUser();

	///
	/// \brief �Ƿ���Ч�û�
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_bool isValidUser( xgc_uint32 chat_id );

	///
	/// \brief ��������
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_long Chat( xgc_uint32 nChatID, xgc_lpcstr lpText, xgc_size nSize, xgc_uint32 nToken );

	///
	/// \brief ����Ƶ���еĶ���
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void ForEachUser( std::function< xgc_void( user &user ) > &&invoke )
	{
		for( auto &pair : mUserConfigMap )
			invoke( pair.second );
	}
};

#endif // _CHANNEL_H_