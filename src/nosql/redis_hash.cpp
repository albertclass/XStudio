#include "nosql.h"
#include "redis_conn.h"
#include "redis_hash.h"


///
/// \brief 获取表长度
/// \date 2017/9/11
/// \author albert.xu
/// 
xgc_int64 redisHash::length()
{
	auto reply = conn_->exec( "HLEN %s", key_ );
	if( !reply )
		return REDIS_ERR;

	return reply.integer();
}

///
/// \brief 键值是否存在
/// \date 2017/9/11
/// \author albert.xu
/// 
xgc_int64 redisHash::exist( xgc_lpcstr key )
{
	auto reply = conn_->exec( "HEXISTS %s %s", key_, key );

	if( !reply )
		return REDIS_ERR;

	return reply.integer();
}

///
/// \brief 获取表中所有键
/// \date 2017/9/11
/// \author albert.xu
/// 
xgc_vector< xvariant > redisHash::keys()
{
	auto reply = conn_->exec( "HKEYS %s", key_ );

	xgc_vector< xvariant > lst;
	if( !reply.is_array() )
		return lst;

	for( auto it = reply.begin(); it != reply.end(); ++it )
	{
		lst.emplace_back( it->value() );
	}

	return lst;
}

///
/// \brief 键值是否存在
/// \date 2017/9/11
/// \author albert.xu
/// 
xgc_vector< xvariant > redisHash::vals()
{
	auto reply = conn_->exec( "HVALS %s", key_ );

	xgc_vector< xvariant > lst;
	if( !reply.is_array() )
		return lst;

	for( auto it = reply.begin(); it != reply.end(); ++it )
	{
		lst.emplace_back( it->value() );
	}

	return lst;
}

///
/// \brief 获取键值
/// \date 2017/9/11
/// \author albert.xu
/// 
int redisHash::get( xgc_lpcstr key, xvariant &val )
{
	auto reply = conn_->exec( "HGET %s", key_ );

	if( !reply )
		return REDIS_ERR;

	val = reply.value();
	return REDIS_OK;
}

///
/// \brief 获取键值
/// \date 2017/9/11
/// \author albert.xu
/// 
xgc_vector< xvariant > redisHash::get_more( xgc_vector< xgc_lpcstr > keys )
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

	auto reply = conn_->exec( "%s", pcmd );

	if( pcmd != cmd )
		free( pcmd );

	if( reply.is_array() )
	{
		for( auto it = reply.begin(); it != reply.end(); ++it )
		{
			lst.emplace_back( reply.value() );
		}
	}

	return lst;
}

///
/// \brief 获取键值
/// \date 2017/9/11
/// \author albert.xu
/// 
xgc_vector< xvariant > redisHash::get_all()
{
	auto reply = conn_->exec( "HGETALL %s", key_ );

	xgc_vector< xvariant > lst;
	if( !reply.is_array() )
		return lst;

	for( auto it = reply.begin(); it != reply.end(); ++it )
	{
		lst.emplace_back( it->value() );
	}

	return lst;
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

	auto reply = conn_->exec( "%s %s %s %s", cmd, key_, key, val );

	if( !reply )
		return REDIS_ERR;

	return REDIS_OK;
}

///
/// \brief 增加值
/// \date 2017/9/11
/// \author albert.xu
/// 
xgc_int64 redisHash::inc( xgc_lpcstr key, int val )
{
	auto reply = conn_->exec( "HINCRBY %s %s %d", key_, key, val );

	if( !reply )
	{
		return REDIS_ERR;
	}

	return reply.integer();
}

///
/// \brief 增加值
/// \date 2017/9/11
/// \author albert.xu
/// 
xgc_real64 redisHash::inc( xgc_lpcstr key, xgc_real64 val )
{
	auto reply = conn_->exec( "HINCRBYFLOAT %s %s %llf", key_, key, val );

	if( !reply )
	{
		return REDIS_ERR;
	}

	return reply.value().to_real64();
}