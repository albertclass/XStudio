#pragma once
#ifndef _USER_H_
#define _USER_H_
#include "auto_handle.h"

class CChannel;
class CClientSession;
class CUser : public auto_handle< CUser >
{
private:
	/// ��ɫ����ϷID
	xgc_uint64 mUserID;
	/// ��ɫ�ǳ�
	xgc_string mNick;
	/// ��ɫ�����ķ�����
	xgc_string mServer;
	/// ��ɫ����������
	xgc_string mExtra;
	/// ��ɫToken
	xgc_char mToken[64];
	/// ForEachChannel protected
	xgc_bool mInProtected;

	CClientSession *mClientSession;

	/// �����Ƶ���б�
	xgc_set< xgc_uint32 > mChannelSet;
public:
	CUser( xgc_uint64 nUserID );
	~CUser();

	///
	/// \brief ��ɫ����
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void online( CClientSession* pSession )
	{
		mClientSession = pSession;
	}

	///
	/// \brief ��ɫ����
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void offline()
	{
		mClientSession = xgc_nullptr;
	}

	///
	/// \brief ��ɫ�ǳ�
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void logout();

	///
	/// \brief ���ý�ɫ������
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void setServer( const xgc_string &strServer )
	{
		mServer = strServer;
	}

	///
	/// \brief ���ý�ɫ��
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void setNickName( const xgc_string &strNick )
	{
		mNick = strNick;
	}

	///
	/// \brief ���ý�ɫ��չ����
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void setExtra( const xgc_string &strExtra )
	{
		mExtra = strExtra;
	}

	///
	/// \brief ��ȡ��ɫ��
	/// \author albert.xu
	/// \date 2017/08/03
	///
	const xgc_string& getNickName()const
	{
		return mNick;
	}

	///
	/// \brief ��ȡ��ɫ��չ����
	/// \author albert.xu
	/// \date 2017/08/03
	///
	const xgc_string& getExtra()const
	{
		return mExtra;
	}

	///
	/// \brief ��ȡ��ɫToken
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_lpcstr genToken();

	///
	/// \brief ����ɫToken
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_bool checkToken( const xgc_string &strToken );

	///
	/// \brief ��ɫ����Ƶ��
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void onEnterChannel( CChannel* pChannel );

	///
	/// \brief ��ɫ�뿪Ƶ��
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void onLeaveChannel( CChannel* pChannel );

	///
	/// \brief ������ɫ�����Ƶ��
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void forEachChannel( const std::function< void( xgc_uint32 ) > &_Pred )
	{
		mInProtected = true;
		for( auto channel_id : mChannelSet )
			_Pred( channel_id );
		mInProtected = false;
	}

	///
	/// \brief �������ݸ���ɫ
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void Send( xgc_uint16 msgid, ::google::protobuf::Message& msg );
};

#endif // _USER_H_