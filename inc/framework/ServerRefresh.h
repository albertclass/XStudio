#ifndef __SERVER_REFRESH_H__
#define __SERVER_REFRESH_H__

///
/// ��Ҫ�û�ʵ�֣�����ˢ���¼�
/// @param lpInvoke ���ò���
/// @param nPrevInvokeTime ��һ�ε���ʱ��
/// @param lpContext ����������
/// [1/20/2015] create by albert.xu
///
xgc_void OnServerRefresh( xgc_lpcstr lpInvoke, datetime dtPrevInvokeTime, xgc_lpvoid lpContext );

///
/// ��ʼ��ˢ�·��� 
/// @return ��ʼ�����
/// [1/16/2015] create by guqiwei.weir
///
xgc_bool InitServerRefresh( ini_reader &ini );

///
/// ����ˢ�·��������
/// [1/20/2015] create by albert.xu
///
xgc_bool LoadServerRefresh();

///
/// ���¼��
/// [1/21/2015] create by albert.xu
///
xgc_void StepServerRefresh();

///
/// ����ˢ�·��� 
/// @return ��ʼ�����
/// [1/16/2015] create by guqiwei.weir
///
xgc_void FiniServerRefresh();
#endif