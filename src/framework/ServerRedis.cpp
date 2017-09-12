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

redisValue::redisValue( redisContext * context, xgc_lpcstr key )
	: key_( strdup( key ) )
	, redis_( context )
{

}

///
/// \brief 执行redis命令
/// \date 2017/9/5
/// \author albert.xu
/// 
reply_ptr redisValue::exec( xgc_lpcstr fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	auto reply = exec_args( fmt, ap );
	va_end( ap );

	return std::move( reply );
}

///
/// \brief 执行redis命令
/// \date 2017/9/5
/// \author albert.xu
/// 
reply_ptr redisValue::exec_args( xgc_lpcstr fmt, va_list args )
{
	return reply_ptr( (redisReply*)redisCommand( redis_, fmt, args ), &freeReplyObject );
}

int redisValue::take( redisReply * reply, xvariant &val )
{
	int ret = REDIS_OK;

	if( xgc_nullptr == reply )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	switch( reply->type )
	{
		case REDIS_REPLY_ERROR:
		ret = REDIS_ERR;
		break;
		case REDIS_REPLY_STATUS:
		break;
		case REDIS_REPLY_INTEGER:
		val = reply->integer;
		break;
		case REDIS_REPLY_STRING:
		val.set_string( reply->str, reply->len );
		break;
	}

	return ret;
}

///
/// \brief 获取reply的返回值
/// \date 2017/9/5
/// \author albert.xu
/// 
int redisValue::take( redisReply *reply, int &val )
{
	int ret = REDIS_OK;

	if( xgc_nullptr == reply )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	if( reply->type != REDIS_REPLY_INTEGER )
		return REDIS_ERR;

	val = reply->integer;

	return REDIS_OK;
}

///
/// \brief 获取reply的返回值
/// \date 2017/9/5
/// \author albert.xu
/// 
int redisValue::take( redisReply *reply, xgc_string &val )
{
	int ret = REDIS_OK;

	if( xgc_nullptr == reply )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	if( reply->type != REDIS_REPLY_STRING )
		return REDIS_ERR;

	val.assign( reply->str, reply->len );

	return REDIS_OK;
}

////////////////////////////////////////////////////////////
/// redisList
////////////////////////////////////////////////////////////

///
/// \brief 获取列表长度
/// \date 2017/9/5
/// \author albert.xu
/// 
int redisList::length()
{
	auto reply = exec( "LLEN %s", key );

	if( xgc_nullptr == reply )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	if( reply->type != REDIS_REPLY_INTEGER )
		return REDIS_ERR;

	return reply->integer;
}

///
/// \brief 获取列表中的值
/// \date 2017/9/8
/// \author albert.xu
/// 
xgc_list< xvariant > redisList::get( int start/* = 0*/, int stop/* = -1*/ )
{
	auto reply = exec( "LRANGE %d %d", start, stop );

	xgc_list< xvariant > lst;

	if( xgc_nullptr == reply )
		return lst;

	if( reply->type == REDIS_REPLY_ERROR )
		return lst;

	if( reply->type != REDIS_REPLY_ARRAY )
		return lst;

	for( int i = 0; i < reply->elements; ++i )
	{
		auto e = reply->element[i];
		switch( e->type )
		{
			case REDIS_REPLY_INTEGER:
			lst.emplace_back( xvariant( e->integer ) );
			break;
			case REDIS_REPLY_STRING:
			lst.emplace_back( xvariant( e->str, e->len ) );
			break;
			case REDIS_REPLY_NIL:
			lst.emplace_back( xvariant() );
			break;
		}
	}

	return lst;
}

///
/// \brief 弹出一个元素
/// \date 2017/9/8
/// \author albert.xu
/// 
int redisList::lpush( xvariant &&val, bool create )
{
	auto reply = exec( "%s %s %s"
		, create ? "LPUSHX" : "LPUSH"
		, key_
		, val.get_string() );

	if( xgc_nullptr == reply )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	return REDIS_OK;
}

///
/// \brief 弹出一个元素
/// \date 2017/9/8
/// \author albert.xu
/// 
int redisList::rpush( xvariant &&val, bool create )
{
	auto reply = exec( "%s %s %s"
		, create ? "RPUSHX" : "RPUSH"
		, key_
		, val.get_string() );

	if( xgc_nullptr == reply )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	return REDIS_OK;
}

///
/// \brief 弹出一个元素
/// \date 2017/9/8
/// \author albert.xu
/// 
int redisList::lpop( bool block, xvariant &val )
{
	auto reply = exec( "%s %s"
		, block ? "BLPOP" : "LPOP"
		, key_ );

	return take( reply.get(), val );
}

///
/// \brief 弹出一个元素
/// \date 2017/9/8
/// \author albert.xu
/// 
int redisList::rpop( bool block, xvariant &val )
{
	auto reply = exec( "%s %s"
		, block ? "BRPOP" : "RPOP"
		, key_ );

	return take( reply.get(), val );
}

///
/// \brief 删除列表中元素
/// \date 2017/9/8
/// \author albert.xu
/// 
/// count 的值可以是以下几种：
///		count > 0 : 从表头开始向表尾搜索，移除与 value 相等的元素，数量为 count 。
///		count < 0 : 从表尾开始向表头搜索，移除与 value 相等的元素，数量为 count 的绝对值。
///		count = 0 : 移除表中所有与 value 相等的值。
int redisList::remove( xgc_lpcstr match, int count )
{
	auto reply = exec( "LREM %s %s %d", key_, match, count );

	if( xgc_nullptr == reply )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	return REDIS_OK;
}

///
/// \brief 设置列表中元素的值
/// \date 2017/9/8
/// \author albert.xu
/// 
int redisList::set( int index, xgc_lpcstr fmt, ... )
{
	char buf[256], *pbuf = buf;

	va_list args;
	va_start( args, fmt );
	int cpy = vsprintf_s( buf, fmt, args );
	if( cpy < 0 )
	{
		while( cpy < 0 )
		{
			auto nbuf = (char*)realloc( pbuf, memsize( pbuf ) + 256 );
			if( !nbuf )
				break;

			pbuf = nbuf;

			cpy = vsprintf_s( buf, fmt, args );
		}
	}
	else
	{
		pbuf = buf;
	}

	auto reply = exec( "LSET %s %d %s", key_, index, pbuf );
	va_end( args );

	if( pbuf != buf )
		free( pbuf );

	if( xgc_nullptr == reply )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	return REDIS_OK;
}

///
/// \brief 设置列表中元素的值
/// \date 2017/9/8
/// \author albert.xu
/// 
int redisList::trim( int start /*= 0*/, int stop /*= -1*/ )
{
	auto reply = exec( "LTRIM %s %d %d", key_, start, stop );

	if( xgc_nullptr == reply )
		return REDIS_ERR;

	if( reply->type == REDIS_REPLY_ERROR )
		return REDIS_ERR;

	return REDIS_OK;
}

///
/// \brief 获取下标为index的元素
/// \date 2017/9/8
/// \author albert.xu
/// 
xvariant redisList::at( int index )
{
	auto reply = exec( "LINDEX %s %d", key_, index );

	if( xgc_nullptr == reply )
		return REDIS_ERR;

	xvariant val;
	
	take( reply.get(), val );
	return val;
}

xvariant redisList::poppush( xgc_lpcstr source, xgc_lpcstr target, int timeout )
{
	reply_ptr reply = xgc_nullptr;
	if( timeout >= 0 )
	{
		reply = exec( "BRPOPPUSH %s %s %d", source, target, timeout );
	}
	else
	{
		reply = exec( "RPOPPUSH %s %s", source, target );
	}

	xvariant val;

	take( reply.get(), val );
	return val;
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

///
/// \brief 获取表长度
/// \date 2017/9/11
/// \author albert.xu
/// 
int redisHash::length()
{
	auto reply = exec( "HLEN %s", key_ );

	int val = 0;
	take( reply.get(), val );
	return val;
}

///
/// \brief 键值是否存在
/// \date 2017/9/11
/// \author albert.xu
/// 
int redisHash::exist( xgc_lpcstr key )
{
	auto reply = exec( "HEXISTS %s %s", key_, key );

	int val = 0;
	take( reply.get(), val );
	return val;

	return 0;
}

///
/// \brief 获取表中所有键
/// \date 2017/9/11
/// \author albert.xu
/// 
xgc_vector< xvariant > redisHash::keys()
{
	auto reply = exec( "HKEYS %s", key_ );

	xgc_vector< xvariant > lst;
	if( reply->type != REDIS_REPLY_ARRAY )
		return lst;

	for( size_t i = 0; i < reply->elements; ++i )
	{
		auto e = reply->element[i];
		switch( e->type )
		{
			case REDIS_REPLY_INTEGER:
			lst.emplace_back( xvariant( e->integer ) );
			break;
			case REDIS_REPLY_STRING:
			lst.emplace_back( xvariant( e->str, e->len ) );
			break;
			case REDIS_REPLY_NIL:
			lst.emplace_back( xvariant() );
			break;
		}
	}
}

///
/// \brief 键值是否存在
/// \date 2017/9/11
/// \author albert.xu
/// 
xgc_vector< xvariant > redisHash::vals()
{
	auto reply = exec( "HVALS %s", key_ );

	xgc_vector< xvariant > lst;
	if( reply->type != REDIS_REPLY_ARRAY )
		return lst;

	for( size_t i = 0; i < reply->elements; ++i )
	{
		auto e = reply->element[i];
		switch( e->type )
		{
			case REDIS_REPLY_INTEGER:
			lst.emplace_back( xvariant( e->integer ) );
			break;
			case REDIS_REPLY_STRING:
			lst.emplace_back( xvariant( e->str, e->len ) );
			break;
			case REDIS_REPLY_NIL:
			lst.emplace_back( xvariant() );
			break;
		}
	}
}

///
/// \brief 获取键值
/// \date 2017/9/11
/// \author albert.xu
/// 
xvariant redisHash::get( xgc_lpcstr key )
{
	auto reply = exec( "HGET %s", key_ );

	xvariant val;
	int ret = take( reply.get(), val );
	if( ret != REDIS_OK )
	{

	}

	return val;
}

///
/// \brief 获取键值
/// \date 2017/9/11
/// \author albert.xu
/// 
xgc_vector< xvariant > redisHash::get( xgc_list< xgc_lpcstr > keys )
{
	xgc_vector< xvariant > lst;
	char cmd[256], *pcmd = cmd;
	
	xgc_size size = sizeof( cmd );

	int pos = sprintf_s( cmd, "HMGET %s", key_ );
	for( auto key : keys )
	{
		int cpy = sprintf_s( pcmd + pos, size - pos, " %s", key );
		while( cpy <= 0 )
		{
			auto pnew = (char*)realloc( pcmd == cmd ? xgc_nullptr : pcmd, size + 256 );
			if( pnew == xgc_nullptr )
				return lst;

			pcmd = pnew;
			cpy = sprintf_s( pcmd + pos, size - pos, " %s", key );
		}
	}

	auto reply = exec( "%s", pcmd );

	if( pcmd != cmd )
		free( pcmd );

	if( reply->type != REDIS_REPLY_ARRAY )
		return lst;

	for( size_t i = 0; i < reply->elements; ++i )
	{
		auto e = reply->element[i];
		switch( e->type )
		{
			case REDIS_REPLY_INTEGER:
			lst.emplace_back( xvariant( e->integer ) );
			break;
			case REDIS_REPLY_STRING:
			lst.emplace_back( xvariant( e->str, e->len ) );
			break;
			case REDIS_REPLY_NIL:
			lst.emplace_back( xvariant() );
			break;
		}
	}
}

///
/// \brief 获取键值
/// \date 2017/9/11
/// \author albert.xu
/// 
xgc_vector< xvariant > redisHash::get_all()
{
	xgc_vector< xvariant > lst;

	auto reply = exec( "HGETALL %s", key_ );

	if( reply->type != REDIS_REPLY_ARRAY )
		return lst;

	for( size_t i = 0; i < reply->elements; ++i )
	{
		auto e = reply->element[i];
		switch( e->type )
		{
			case REDIS_REPLY_INTEGER:
			lst.emplace_back( xvariant( e->integer ) );
			break;
			case REDIS_REPLY_STRING:
			lst.emplace_back( xvariant( e->str, e->len ) );
			break;
			case REDIS_REPLY_NIL:
			lst.emplace_back( xvariant() );
			break;
		}
	}
}

///
/// \brief 设置值
/// \date 2017/9/11
/// \author albert.xu
/// 
int redisHash::set( xgc_lpcstr key, xgc_lpcstr val, xgc_bool create /*= false*/ )
{
	xgc_lpcstr cmd = "HSET";
	if( false == create )
		cmd = "HSETNX";

	auto reply = exec( "%s %s %s %s", cmd, key_, key, val );

	int ret = 0;
	int err = take( reply.get(), ret );

	if( err != REDIS_OK )
		return 0;

	return ret;
}

///
/// \brief 增加值
/// \date 2017/9/11
/// \author albert.xu
/// 
int redisHash::inc( xgc_lpcstr key, int val )
{
	auto reply = exec( "HINCRBY %s %s %d", key_, key, val );

	int ret = 0;
	int err = take( reply.get(), ret );

	if( err != REDIS_OK )
	{
	}

	return ret;
}

///
/// \brief 增加值
/// \date 2017/9/11
/// \author albert.xu
/// 
xgc_real64 redisHash::inc( xgc_lpcstr key, xgc_real64 val )
{
	auto reply = exec( "HINCRBYFLOAT %s %s %llf", key_, key, val );

	xvariant ret;
	int err = take( reply.get(), ret );

	if( err != REDIS_OK )
	{
	}

	return ret.to_real64();
}