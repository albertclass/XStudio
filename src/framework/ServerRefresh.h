#ifndef __SERVER_REFRESH_H__
#define __SERVER_REFRESH_H__

///
/// ��ʼ��ˢ�·��� 
/// @return ��ʼ�����
/// [1/16/2015] create by guqiwei.weir
///
xgc_bool InitServerRefresh( ini_reader &ini );

///
/// ���¼��
/// [1/21/2015] create by albert.xu
///
xgc_void ExecServerRefresh( datetime now );

///
/// ����ˢ�·��� 
/// @return ��ʼ�����
/// [1/16/2015] create by guqiwei.weir
///
xgc_void FiniServerRefresh();

#endif