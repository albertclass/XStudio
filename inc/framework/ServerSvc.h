/*******************************************************************/
//! \file ServerGD.h
//! \brief GDָ��
//! 2015/01/13 by wuhailin.jerry
/*******************************************************************/

#ifndef _SERVER_GD_H
#define _SERVER_GD_H

#pragma once

namespace ServerSvc
{
	#define SVCGD		"SVCGD"		/// "GD"��Ϣ����
	#define SVCAWARD	"SVCAWARD"  /// �콱��Ϣ����  

	#define MAX_ORDERID_LEN         32  // �����Ż����û���Ϣ�ַ�����
	#define MAX_TIME_LEN            19  // ʱ�䳤�� ��ʽΪ��YYYY-MM-DD HH24:MI:SS
	#define MAX_BATCHID_LEN         10  // ���κų��� ��ʽΪ��CQ007
	#define MAX_DESCRIPTION_LEN     64  // �ɹ�ʧ�ܵ�������Ϣ����
	#define	MAX_PERSIONALINFO_LEN	256	// ��Ҹ��Ի���Ϣ��󳤶�

	#define GDNORMALC2SREQ			0x00000001   /// "GDNORMAL"��������  
	#define GDNORMALC2SRES			0x80000001   /// "GDNORMAL"���ͷ���  
	#define GDNORMALS2CREQ			0x08000001   /// "GDNORMAL"�·�����  
	#define GDNORMALS2CRES			0x88000001   /// "GDNORMAL"�·�����  

	///
	/// ��������
	/// [4/27/2015] create by albert.xu
	///
	typedef xgc_void( *fnServiceHandler )( xgc_lpvoid lpData, xgc_size nSize );

	///
	/// ��ʼ��GDָ�� 
	/// [1/13/2015] create by wuhailin.jerry
	///
	xgc_bool SvcInit( IniFile &ini );

	///
	/// ע���������Ϣ
	/// [4/27/2015] create by albert.xu
	///
	xgc_void SvcRegist( xgc_lpcstr lpSvcName, fnServiceHandler lpHandler );

	///
	/// �첽���÷�����
	/// [4/27/2015] create by albert.xu
	///
	xgc_long SvcAsynCall( xgc_lpcstr lpServiceName, xgc_lpvoid lpData, xgc_int32 nDataLen, xgc_uint32 nFlags );

	///
	/// ��ȡһ��Ψһ�ķ����¼�����
	/// [4/27/2015] create by albert.xu
	///
	xgc_int32 SvcGetSotsUniqueId( xgc_lpstr lpOrderId, xgc_int32 nLength );
}

#endif