///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file Processor.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// ��Ϣ����
///
///////////////////////////////////////////////////////////////
#pragma once

#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

using namespace xgc::net;

namespace xgc
{
	namespace message
	{
		///
		/// \brief ��ʼ����Ϣӳ���
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:37
		///
		xgc_void InitProcessor();

		///
		/// \brief �ͻ��˽�������
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnAccept( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief �ͻ��˶Ͽ�����
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnClose( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief �ͻ������ӷ�������
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnError( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief �ͻ��˶�ʱͬ��
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnClientInfoRpt( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );
		
		///
		/// \brief �ͻ��˿�����Ӧ
		///
		/// \author albert.xu
		/// \date 2016/04/18 19:30
		///
		xgc_void OnClientStart( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );
		
		///
		/// \brief ͬ���������Դʹ�����
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnClientResNtf( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );
		
		///
		/// \brief ͬ������״̬
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnClientStatusNtf( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );
		
		///
		/// \brief ͬ���������
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnProgressNtf( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief �ļ���������
		///
		/// \author albert.xu
		/// \date 2016/04/13 17:28
		///
		xgc_void OnFilesReq( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief �ļ�������Ӧ
		///
		/// \author albert.xu
		/// \date 2016/03/15 13:48
		///
		xgc_void OnNewFileAck( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief �ļ������Ӧ
		///
		/// \author albert.xu
		/// \date 2016/04/13 17:23
		///
		xgc_void OnFileDataAck( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

	}
}
#endif // _PROCESSOR_H_