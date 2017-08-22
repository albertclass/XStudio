#include "header.h"
#include "session.h"

CSession::CSession()
{

}

CSession::~CSession()
{

}

///
/// \brief ���ݰ��Ƿ�
/// \return	0 ~ �ɹ�, -1 ~ ʧ��
///
int CSession::OnParsePacket( const void* data, xgc_size size )
{
	if( size < sizeof( xgc_uint16 ) )
		return 0;

	xgc_uint16 length = ntohs( *(xgc_uint16*)data );
	if( size < length )
		return 0;

	return length;
}

///
/// \brief ���ӽ���
///
/// \author albert.xu
/// \date 2017/02/28 11:09
///
xgc_void CSession::OnAccept( net::network_t handle )
{

}

///
/// \brief ���ӽ���
///
/// \author albert.xu
/// \date 2017/02/28 11:09
///
xgc_void CSession::OnConnect( net::network_t handle )
{
	printf( "client connected. handle = %08x\n", handle );
}

///
/// \brief ���Ӵ���
///
/// \author albert.xu
/// \date 2017/02/28 11:09
///
xgc_void CSession::OnError( xgc_int16 error_type, xgc_int16 error_code )
{
	printf( "client error type = %d, code = %d\n", error_type, error_code );
}

///
/// \brief ���ӹر�
///
/// \author albert.xu
/// \date 2017/02/28 11:10
///
xgc_void CSession::OnClose()
{
	printf( "client closed\n" );
}

///
/// \brief ��������
///
/// \author albert.xu
/// \date 2017/02/28 11:10
///
xgc_void CSession::OnRecv( xgc_lpvoid data, xgc_size size )
{

}

///
/// \brief ���籣���¼�
///
/// \author albert.xu
/// \date 2017/03/03 10:41
///
xgc_void CSession::OnAlive()
{

}
