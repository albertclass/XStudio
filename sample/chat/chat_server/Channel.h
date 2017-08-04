#pragma once
#ifndef _CHANNEL_H_
#define _CHANNEL_H_
#include "auto_handle.h"

class CChannel : public auto_handle< CChannel >
{
private:
	/// Ƶ������
	xgc_string mName;

	/// Ƶ������

	/// �Ƿ��Զ�ɾ��
	xgc_bool mAutoDestory;

	/// �Զ�ɾ���ӳ�
	xgc_long mAutoDestoryDelay;

	/// �Զ��߳���Ĭ���
	xgc_long mAutoKickDelay;

	/// ��������
	xgc_string mPassword;

	/// �û�������Ϣ
	struct UserConfig
	{
		/// �Ƿ񱻽���
		xgc_bool forbid;
		/// �Ƿ����
		xgc_bool disconnect;
		/// ���һ�η��Ե�ʱ��
		xgc_time64 last_timestamp;
	};

	/// �û��б�
	xgc_map< xgc_uint32, UserConfig > mUserConfigMap;
public:
	///
	/// \brief ����
	/// \author albert.xu
	/// \date 2017/08/03
	///
	CChannel( const xgc_string &strName );

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
	xgc_void AliveCheck();

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
	xgc_void ForEachUser( std::function< xgc_void( xgc_uint32, xgc_bool ) > &&invoke )
	{
		for( auto &pair : mUserConfigMap )
		{
			invoke( pair.first, pair.second.forbid );
		}
	}
};

#endif // _CHANNEL_H_