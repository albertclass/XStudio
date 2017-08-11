#pragma once
#ifndef _APPLICATION_H_
#define _APPLICATION_H_
class CApplication
{
private:
	/// �Ƿ�������
	xgc_bool mRunning = true;

	/// ��Ϸ���������ӵ�ַ
	xgc_char mGameAddr[64];
	/// ��Ϸ���������Ӷ˿�
	xgc_uint16 mGamePort;

	/// ������������ӵ�ַ
	xgc_char mChatAddr[64];
	/// ������������Ӷ˿�
	xgc_uint16 mChatPort;

public:
	CApplication();

	~CApplication();

	///
	/// \brief ��ȡ��Ϸ���������ӵ�ַ
	///
	/// \author albert.xu
	/// \date 2017/08/09
	///
	xgc_lpcstr getGameAddr() const
	{
		return mGameAddr;
	}

	///
	/// \brief ��ȡ��Ϸ���������ӵ�ַ
	///
	/// \author albert.xu
	/// \date 2017/08/09
	///
	xgc_uint16 getGamePort() const
	{
		return mGamePort;
	}

	///
	/// \brief ��ȡ������������ӵ�ַ
	///
	/// \author albert.xu
	/// \date 2017/08/09
	///
	xgc_lpcstr getChatAddr() const
	{
		return mChatAddr;
	}

	///
	/// \brief ��ȡ������������ӵ�ַ
	///
	/// \author albert.xu
	/// \date 2017/08/09
	///
	xgc_uint16 getChatPort() const
	{
		return mChatPort;
	}

	///
	/// \brief ���÷�����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_bool Setup( xgc_lpcstr lpConfigFile );

	///
	/// \brief ���з�����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Run( int argc, char *argv[] );

	///
	/// \brief ���������
	///
	/// \author albert.xu
	/// \date 2017/08/10
	///
	xgc_void Clear();
};

extern CApplication theApp;
#endif // _APPLICATION_H_