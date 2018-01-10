#include "nosql.h"
#include "redis_conn.h"

xvariant redisData::value() const
{
	if( reply_->type == REDIS_REPLY_NIL )
		return xvariant();

	if( reply_->type == REDIS_REPLY_STRING )
		return xvariant( reply_->str, reply_->len );

	if( reply_->type == REDIS_REPLY_INTEGER )
		return xvariant( reply_->integer );

	return xvariant();
}

redisData redisData::operator[]( int i )
{
	if( i < 0 || i >= reply_->elements )
		return redisData();

	return redisData( reply_->element[i] );
}

redisConn::redisConn()
	: host_( xgc_nullptr )
	, port_( 0 )
{

}

redisConn::redisConn( xgc_lpcstr host, xgc_uint16 port )
	: host_( strdup( host ) )
	, port_( port )
{

}

redisConn::~redisConn()
{
	free( (void*)host_ );
	host_ = xgc_nullptr;

	redisFree( redis_ );
	redis_ = xgc_nullptr;
}

xgc_bool redisConn::connect( xgc_lpcstr host, xgc_uint16 port, int timeout )
{
	free( (void*)host_ );
	host_ = strdup( host );
	port_ = port_;

	return connect( timeout );
}

xgc_bool redisConn::connect( int timeout )
{
	if( timeout > 0 )
	{
		timeval tv;
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = timeout % 1000 * 1000;

		redis_ = redisConnectWithTimeout( host_, port_, tv );
	}
	else
	{
		redis_ = redisConnect( host_, port_ );
	}

	return redis_ != xgc_nullptr;
}

///
/// \brief 执行redis命令
/// \date 2017/9/5
/// \author albert.xu
/// 

redisData redisConn::exec( xgc_lpcstr fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	auto reply = exec_args( fmt, ap );
	va_end( ap );

	return reply;
}

///
/// \brief 执行redis命令
/// \date 2017/9/12
/// \author albert.xu
/// 

redisData redisConn::exec_args( xgc_lpcstr fmt, va_list args )
{
	return (redisReply*)redisCommand( redis_, fmt, args );
}

///
/// \brief 选择数据库
/// \date 2017/9/12
/// \author albert.xu
/// 

xgc_bool redisConn::select( int db )
{
	redisData reply = exec( "SELECT %d", db );
	if( reply )
		return true;

	return false;
}

///
/// \brief 获取字符串参数
/// \date 2017/9/5
/// \author albert.xu
/// 

int redisConn::get_value( xgc_lpcstr key, xvariant & val )
{
	auto reply = exec( "GET %s", key );
	if( reply )
	{
		val = reply.value();
		return REDIS_OK;
	}

	return REDIS_ERR;
}

///
/// \brief 设置字符串参数
/// \date 2017/9/5
/// \author albert.xu
/// 

int redisConn::set_value( xgc_lpcstr key, const xvariant & val )
{
	auto reply = exec( "SET %s %s", key, val.to_string().c_str() );
	if( reply )
	{
		return REDIS_OK;
	}

	return REDIS_ERR;
}
