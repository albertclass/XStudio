#pragma once
#ifndef _USER_MGR_H_
#define _USER_MGR_H_
class CUser;
class CUserMgr
{
	friend CUserMgr& getUserMgr();

private:
	/// ��ϷIDӳ���
	xgc::unordered_map< xgc_uint64, xgc_uint32 > mUserMap;

	/// �ǳ�ӳ���
	xgc::unordered_map< xgc::string, xgc_uint32 > mNickMap;

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
	xgc_long UserLogin( xgc_uint64 nUserID, const xgc::string &strServer, const xgc::string &strNick, const xgc::string &strExtra );

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

	///
	/// \brief ��ȡƵ����
	/// \author albert.xu
	/// \date 2017/08/15
	///
	xgc_size getUserCount() const
	{
		return mUserMap.size();
	}
};

CUserMgr& getUserMgr();

#endif // _USER_MGR_H_