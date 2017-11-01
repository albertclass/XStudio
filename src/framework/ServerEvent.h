#ifndef __SERVER_EVENT_H__
#define __SERVER_EVENT_H__

/// �¼�����
struct EventObject
{
	/// ϵͳID
	int id;
	/// �¼�ID
	int event;
	/// ִ�н��  < 0 - ִ�д��� = 0 ִ�гɹ��� > 0 ִ�о���
	int result;
};

template< class _EventType >
_EventType* make_event( int id, int event )
{
	return XGC_NEW _EventType{ id, event, 0 };
}

///
/// \brief �¼�ע��
/// 
/// \author albert.xu
/// \date 2017/08/21
///
xgc_bool InitServerEvent();

///
/// \brief �¼�ע��
/// 
/// \author albert.xu
/// \date 2017/08/21
///
xgc_bool RegistEventListener( int id, int event, std::function< xgc_void( EventObject& ) > &&invoke );

///
/// \brief �����¼�������ִ��)
/// 
/// \author albert.xu
/// \date 2017/08/21
///
xgc_void EmitEvent( EventObject &evt );

///
/// \brief Ͷ���¼����ӳ�ִ�У�
/// 
/// \author albert.xu
/// \date 2017/08/21
///
xgc_void PostEvent( EventObject *evt );

///
/// \brief ִ���¼�
/// 
/// \author albert.xu
/// \date 2017/08/21
///
xgc_void ExecEvent();

///
/// \brief �����¼�ϵͳ
/// 
/// \author albert.xu
/// \date 2017/08/21
///
xgc_void FiniServerEvent();

#endif // __SERVER_EVENT_H__