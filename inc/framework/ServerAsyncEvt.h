#ifndef __SERVER_ASYNCEVT_H__
#define __SERVER_ASYNCEVT_H__

struct EventObject
{
	int id;
	int event;
};

//template< class R, class T... >
//R Emit( const xgc_string &system, const xgc_string &event, T... )
//{
//	auto invoke = GetInvoke( system, event );
//	return invoke( std::forward< T >( ...T ) );
//}

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
xgc_void PostServerEvent( const std::function<xgc_void()>& func );

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