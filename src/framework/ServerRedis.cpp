#include "ServerDefines.h"
#include "ServerRedis.h"

static redisContext *__redis = xgc_nullptr;

static int __redisType( xgc_lpcstr type )
{
	if( strcasecmp( type, "list" ) )
		return 1;

	if( strcasecmp( type, "set" ) )
		return 2;

	if( strcasecmp( type, "zset" ) )
		return 3;

	if( strcasecmp( type, "hash" ) )
		return 4;

	return 0;
}


///
/// \brief 初始化Redis模块
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_bool InitServerRedis( ini_reader &ini )
{
	if( false == ini.is_exist_section( "redis" ) )
		return false;

	auto host = ini.get_item_value( "redis", "host", "127.0.0.1" );
	auto port = ini.get_item_value( "redis", "port", 6379 );

	redisContext *c = redisConnect( host, port );
	if( xgc_nullptr == c )
	{
		SYS_ERROR( "无法分配redis上下文。" );
		return false;
	}
	
	if( 0 == c->err )
	{
		SYS_ERROR( "redis 连接错误 %s", c->errstr );
		return false;
	}

	__redis = c;

	return true;
}

///
/// \brief 使用数据库
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long UsingDB( int index )
{
	reply_ptr reply = { (redisReply*)redisCommand( __redis, "SELECT %d", index ), &freeReplyObject };

	if( reply == xgc_nullptr )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	return REDIS_OK;
}

///
/// \brief 获取参数
/// \date 2017/9/5
/// \author albert.xu
/// 
static xgc_long redisGetStr( redisReply *reply, xgc_lpstr buf, xgc_size len )
{
	long cpy = 0;
	switch( reply->type )
	{
		case REDIS_REPLY_NIL:
		{
			buf[0] = 0;
			break;
		}
		case REDIS_REPLY_STRING:
		{
			cpy = sprintf_s( buf, len, "%s", reply->str );
			break;
		}
		case REDIS_REPLY_INTEGER:
		{
			cpy = numeric2str( reply->integer, buf, len );
			break;
		}
		case REDIS_REPLY_ARRAY:
		{
			for( size_t i = 0; i < reply->elements; ++i )
			{
				auto c = redisGetStr( reply, buf + cpy, len - cpy );
				if( c < 0 )
					return REDIS_ERR;

				cpy += c;
			}

			break;
		}

		default: return REDIS_ERR;
	}

	return cpy;
}

///
/// \brief 获取字符串参数
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisGetStr( xgc_lpcstr key, xgc_lpstr buf, xgc_size len )
{
	reply_ptr reply = { (redisReply*)redisCommand( __redis, "GET %s", key ), &freeReplyObject };

	if( reply == xgc_nullptr )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	return redisGetStr( reply.get(), buf, len );
}

///
/// \brief 设置参数
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisSetStr( xgc_lpcstr key, xgc_lpcstr val )
{
	reply_ptr reply = { (redisReply*)redisCommand( __redis, "SET %s %s", key, val ), &freeReplyObject };
	if( reply == xgc_nullptr )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	return REDIS_OK;
}

///
/// \brief 获取参数
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisGetVal( xgc_lpcstr key, xgc_int64 &val )
{
	reply_ptr reply = { (redisReply*)redisCommand( __redis, "GET %s", key ), &freeReplyObject };

	if( reply == xgc_nullptr )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	switch( reply->type )
	{
		case REDIS_REPLY_NIL:
		{
			val = 0;
			break;
		}
		case REDIS_REPLY_STRING:
		{
			val = str2numeric< xgc_int64 >( reply->str );
			break;
		}
		case REDIS_REPLY_INTEGER:
		{
			val = reply->integer;
			break;
		}
		case REDIS_REPLY_ARRAY:
		return REDIS_ERR;
		default:
		return REDIS_ERR;
	}

	return REDIS_OK;
}

///
/// \brief 设置参数
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisSetVal( xgc_lpcstr key, xgc_int64 val )
{
	reply_ptr reply = { (redisReply*)redisCommand( __redis, "SET %s %lld", key, val ), &freeReplyObject };

	if( reply == xgc_nullptr )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	return REDIS_OK;
}

///
/// \brief 获取数值参数
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisGet( xgc_lpcstr key, redisValue &tbl )
{
	reply_ptr reply = { (redisReply*)redisCommand( __redis, "TYPE %s", key ), &freeReplyObject };

	if( reply == xgc_nullptr )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	tbl = redisValue( __redis, key );

	return REDIS_OK;
}

///
/// \brief 清理Redis模块
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_void FiniServerRedis()
{
	if( __redis )
	{
		redisFree( __redis );
	}
}
