#pragma once
#ifndef _USER_MGR_H_
#define _USER_MGR_H_
class CUser;
class CUserMgr
{
	friend CUserMgr& getUserMgr();

private:
	/// ��ϷIDӳ���
	xgc_unordered_map< xgc_uint64, xgc_uint32 > mUserMap;

	/// �ǳ�ӳ���
	xgc_unordered_map< xgc_string, xgc_uint32 > mNickMap;

private:
	///
	/// \brief ��ɫ����������
	/// \author albert.xu
	/// \date 2017/08/03
	///
	CUserMgr();

	///
	/// \brief ��ɫ����������
	/// \author albert.xu
	/// \date 2017/08/03
	///
	~CUserMgr();

public:
	///
	/// \brief ��ɫ��½
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_long UserLogin( xgc_uint64 nUserID, const xgc_string &strServer, const xgc_string &strNick, const xgc_string &strExtra );

	///
	/// \brief ��ɫ�ǳ�
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void UserLogout( xgc_uint64 nUserID );

	///
	/// \brief ��ɫ�ǳ�
	/// \author albert.xu
	/// \date 2017/08/03
	///
	CUser* GetUserByUID( xgc_uint64 nUserID );
};

CUserMgr& getUserMgr();

#endif // _USER_MGR_H_