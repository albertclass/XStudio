#ifndef __SERVER_REDIS_H__
#define __SERVER_REDIS_H__
#include "xvariant.h"

typedef std::unique_ptr< redisReply, decltype( freeReplyObject )* > reply_ptr;




///
/// \brief ��ʼ��Redisģ��
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_bool InitServerRedis( ini_reader &ini );

///
/// \brief ʹ�����ݿ�
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long UsingDB( int index );

///
/// \brief ��ȡ�ַ�������
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisGetStr( xgc_lpcstr key, xgc_lpstr buf, xgc_size len );

///
/// \brief �����ַ�������
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisSetStr( xgc_lpcstr key, xgc_lpcstr val );

///
/// \brief ��ȡ��ֵ����
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisGetVal( xgc_lpcstr key, xgc_int64 &val );

///
/// \brief ������ֵ����
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisSetVal( xgc_lpcstr key, xgc_int64 val );

///
/// \brief ����Redisģ��
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_void FiniServerRedis();
#endif // __SERVER_REDIS_H__