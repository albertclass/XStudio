#include "nosql.h"
#include "redis_conn.h"
#include "redis_list.h"
////////////////////////////////////////////////////////////
/// redisList
////////////////////////////////////////////////////////////

///
/// \brief 获取列表长度
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_int64 redisList::length()
{
	auto reply = conn_->exec( "LLEN %s", key_ );

	if( !reply )
		return REDIS_ERR;

	return reply.integer();
}

///
/// \brief 获取列表中的值
/// \date 2017/9/8
/// \author albert.xu
/// 
xgc_list< xvariant > redisList::get( int start/* = 0*/, int stop/* = -1*/ )
{
	auto reply = conn_->exec( "LRANGE %d %d", start, stop );

	xgc_list< xvariant > lst;

	if( !reply || !reply.is_array() )
		return lst;

	for( auto it = reply.begin(); it != reply.end(); ++it )
	{
		lst.emplace_back( it->value() );
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
	auto reply = conn_->exec( "%s %s %s"
		, create ? "LPUSHX" : "LPUSH"
		, key_
		, val.get_string() );

	if( !reply )
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
	auto reply = conn_->exec( "%s %s %s"
		, create ? "RPUSHX" : "RPUSH"
		, key_
		, val.get_string() );

	if( !reply )
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
	auto reply = conn_->exec( "%s %s"
		, block ? "BLPOP" : "LPOP"
		, key_ );

	if( !reply )
		return REDIS_ERR;
	
	val = reply.value();
	return REDIS_OK;
}

///
/// \brief 弹出一个元素
/// \date 2017/9/8
/// \author albert.xu
/// 
int redisList::rpop( bool block, xvariant &val )
{
	auto reply = conn_->exec( "%s %s"
		, block ? "BRPOP" : "RPOP"
		, key_ );

	if( !reply )
		return REDIS_ERR;

	val = reply.value();
	return REDIS_OK;
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
xgc_int64 redisList::remove( xgc_lpcstr match, int count )
{
	auto reply = conn_->exec( "LREM %s %s %d", key_, match, count );

	if( !reply )
		return REDIS_ERR;

	return reply.integer();
}

///
/// \brief 设置列表中元素的值
/// \date 2017/9/8
/// \author albert.xu
/// 
int redisList::set( int index, xgc_lpcstr fmt, ... )
{
	char buf[256];

	va_list args;
	va_start( args, fmt );
	int cpy = sprintf_s( buf, "LSET %s %d %s", key_, index, fmt );
	if( cpy < 0 )
		return REDIS_ERR;

	auto reply = conn_->exec_args( fmt, args );
	va_end( args );

	if( !reply )
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
	auto reply = conn_->exec( "LTRIM %s %d %d", key_, start, stop );

	if( !reply )
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
	auto reply = conn_->exec( "LINDEX %s %d", key_, index );

	if( !reply )
		return xvariant( "Nil" );

	return reply.value();
}

xvariant redisList::poppush( xgc_lpcstr source, xgc_lpcstr target, int timeout )
{
	redisData reply;
	if( timeout >= 0 )
	{
		reply = conn_->exec( "BRPOPPUSH %s %s %d", source, target, timeout );
	}
	else
	{
		reply = conn_->exec( "RPOPPUSH %s %s", source, target );
	}

	return reply.value();
}
