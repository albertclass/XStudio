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

namespace xgc
{
	namespace message
	{
		extern network_t g_session;

		///
		/// \brief �����Ƿ�Ͽ�
		///
		/// \author albert.xu
		/// \date 2016/03/14 18:31
		///
		xgc_bool is_disconnect();

		///
		/// \brief ��ʼ����Ϣӳ���
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:37
		///
		xgc_void init_deliver_table();

		///
		/// \brief �ͻ��˽�������
		///
		/// \author albert.xu
		/// \date 2016/02/23 15:07
		///
		xgc_void OnConnect( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

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
		/// \brief ������֪ͨ�ļ��б�
		///
		/// \author albert.xu
		/// \date 2016/08/05 15:51
		///
		xgc_void OnFileList( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief ��ʼ����
		///
		/// \author albert.xu
		/// \date 2016/03/14 14:38
		///
		xgc_void OnStart( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief ֹͣ����
		///
		/// \author albert.xu
		/// \date 2016/03/14 14:38
		///
		xgc_void OnStop( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief �����ļ�
		///
		/// \author albert.xu
		/// \date 2016/04/12 21:47
		///
		xgc_void OnNewFile( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );

		///
		/// \brief �����ļ�
		///
		/// \author albert.xu
		/// \date 2016/04/12 21:47
		///
		xgc_void OnFileData( network_t session, xgc_lpcstr data, xgc_size size, xgc_lpvoid userdata );
	}
}
#endif // _PROCESSOR_H_