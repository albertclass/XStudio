#include "nosql.h"
#include "redis_conn.h"
#include "redis_set.h"

///
/// \brief 构造
/// \date 2017/9/5
/// \author albert.xu
/// 

redisSet::redisSet( const redisKey & rhs )
	: redisKey( rhs )
{

}

///
/// \brief 构造
/// \date 2017/9/5
/// \author albert.xu
/// 

redisSet::redisSet( redisKey && rhs )
	: redisKey( std::forward< redisKey >( rhs ) )
{

}

///
/// \brief 添加元素
/// \date 2017/9/13
/// \author albert.xu
/// 

int redisSet::add( xgc_lpcstr fmt, ... ) const
{
	char cmd[256];
	int cpy = sprintf_s( cmd, "SADD %s %s", key_, fmt );
	if( cpy < 0 )
		return REDIS_ERR;

	va_list args;
	va_start( args, fmt );
	auto reply = conn_->exec_args( cmd, args );
	va_end( args );

	if( !reply )
		return REDIS_ERR;

	return (int)reply.integer();
}

///
/// \brief 移除元素
/// \date 2017/9/13
/// \author albert.xu
/// 

int redisSet::remove( xgc_lpcstr fmt, ... ) const
{
	char cmd[256];
	int cpy = sprintf_s( cmd, "SREM %s %s", key_, fmt );
	if( cpy < 0 )
		return REDIS_ERR;

	va_list args;
	va_start( args, fmt );
	auto reply = conn_->exec_args( cmd, args );
	va_end( args );

	if( !reply )
		return REDIS_ERR;

	return (int)reply.integer();
}

///
/// \brief 集合内的元素个数
/// \date 2017/9/13
/// \author albert.xu
/// 

int redisSet::count() const
{
	auto reply = conn_->exec( "SCARD %s", key_ );

	if( !reply )
		return REDIS_ERR;

	return (int)reply.integer();
}

///
/// \brief 元素是否存在
/// \date 2017/9/13
/// \author albert.xu
/// 

int redisSet::exist( xgc_lpcstr member ) const
{
	auto reply = conn_->exec( "SISMEMBER %s %s", key_, member );

	if( !reply )
		return REDIS_ERR;

	return (int)reply.integer();
}

///
/// \brief 提取集合内的元素
/// \date 2017/9/13
/// \author albert.xu
/// 

int redisSet::members( xgc_vector<xvariant>& lst ) const
{
	auto reply = conn_->exec( "SMEMBERS %s", key_ );

	if( !reply )
		return REDIS_ERR;

	if( !reply.is_array() )
		return REDIS_ERR;

	for( auto it = reply.begin(); it != reply.end(); ++it )
		lst.emplace_back( it->value() );

	return REDIS_OK;
}

///
/// \brief 移动元素到其他集合
/// \date 2017/9/13
/// \author albert.xu
/// 

int redisSet::moveto( xgc_lpcstr destination, xgc_lpcstr member )
{
	auto reply = conn_->exec( "SMOVE %s %s %s", key_, destination, member );

	if( !reply )
		return REDIS_ERR;

	return (int)reply.integer();
}

///
/// \brief 随机从集合中弹出一个元素
/// \date 2017/9/13
/// \author albert.xu
/// 

int redisSet::pop( xvariant & val ) const
{
	auto reply = conn_->exec( "SPOP %s", key_ );

	if( !reply )
		return REDIS_ERR;

	val = reply.value();

	return REDIS_OK;
}

///
/// \brief 随机捡取count个元素，count < 0 元素允许重复
/// \date 2017/9/13
/// \author albert.xu
/// 

size_t redisSet::rand( xvariant vals[], int count )
{
	redisData reply;
	if( count == 0 )
	{
		reply = conn_->exec( "SRANDMEMBER %s", key_ );
		if( reply.is_error() )
			return REDIS_ERR;

		vals[0] = reply.value();

		return 1;
	}

	reply = conn_->exec( "SRANDMEMBER %s %d", key_, count );
	if( reply.is_error() )
		return REDIS_ERR;

	if( !reply.is_array() )
		return REDIS_ERR;

	int c = abs( count );
	for( int i = 0; i < c; ++i )
	{
		vals[i] = reply[i].value();
	}

	return reply.count();
}
