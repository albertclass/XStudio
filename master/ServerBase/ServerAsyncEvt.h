#ifndef __SERVER_ASYNCEVT_H__
#define __SERVER_ASYNCEVT_H__

///
/// ��ʼ��ˢ�·��� 
/// @return ��ʼ�����
/// [1/16/2015] create by zhangyupeng
///
xgc_bool InitServerAsyncEvt();

///
/// Ͷ���첽�¼��¼�
/// [1/20/2015] create by zhangyupeng
///
xgc_void PostServerEvent( const function<xgc_void()>& func );

///
/// �����첽�¼��¼� 
/// @return ��ʼ�����
/// [1/16/2015] create by zhangyupeng
///
xgc_void StepServerEvent();

///
/// ����ǰ�����������������¼� 
/// @return ��ʼ�����
/// [1/16/2015] create by zhangyupeng
///
xgc_void FiniServerAsyncEvt();

#endif