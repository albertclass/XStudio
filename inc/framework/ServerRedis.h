#ifndef __SERVER_REDIS_H__
#define __SERVER_REDIS_H__
#include "xvariant.h"

typedef std::unique_ptr< redisReply, decltype( freeReplyObject )* > reply_ptr;




///
/// \brief 初始化Redis模块
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_bool InitServerRedis( ini_reader &ini );

///
/// \brief 使用数据库
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long UsingDB( int index );

///
/// \brief 获取字符串参数
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisGetStr( xgc_lpcstr key, xgc_lpstr buf, xgc_size len );

///
/// \brief 设置字符串参数
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisSetStr( xgc_lpcstr key, xgc_lpcstr val );

///
/// \brief 获取数值参数
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisGetVal( xgc_lpcstr key, xgc_int64 &val );

///
/// \brief 设置数值参数
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisSetVal( xgc_lpcstr key, xgc_int64 val );

///
/// \brief 清理Redis模块
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_void FiniServerRedis();
#endif // __SERVER_REDIS_H__