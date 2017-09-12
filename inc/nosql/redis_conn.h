#ifndef __REDIS_CONN_H__
#define __REDIS_CONN_H__
#include "redis_key.h"

class redisExecption : public std::exception
{
	typedef std::exception base;
public:
	redisExecption()throw( );

	redisExecption( char const* _Message, int err )throw( );

	explicit redisExecption( char const* const _Message ) throw( );

	redisExecption( redisExecption const &_Other );

	redisExecption& operator=( redisExecption const & _Other );
};

class redisData;

class redisDataIter
{
friend class redisData;
private:
	redisReply **reply_;

	redisDataIter( redisReply **reply )
		: reply_( reply )
	{

	}

public:
	redisDataIter& operator++()
	{
		++reply_;
		return *this;
	}

	redisDataIter& operator--()
	{
		--reply_;
		return *this;
	}

	xgc_bool operator==( const redisDataIter &_Iter )
	{
		return reply_ == _Iter.reply_;
	}

	xgc_bool operator!=( const redisDataIter &_Iter )
	{
		return reply_ != _Iter.reply_;
	}

	redisData operator*();

	redisData* operator->();
};

class redisData
{
private:
	redisReply *reply_;

public:
	redisData()
		: reply_( xgc_nullptr )
	{

	}

	redisData( redisReply* reply )
		: reply_( reply )
	{

	}

	redisData( redisData &&_Other )
		: reply_( xgc_nullptr )
	{
		std::swap( reply_, _Other.reply_ );
	}

	redisData( const redisData &_Other ) = delete;

	redisData& operator=( redisData &&_Other )
	{
		std::swap( reply_, _Other.reply_ );
		return *this;
	}

	redisData& operator=( const redisData &_Other ) = delete;

	~redisData()
	{
		freeReplyObject( reply_ );
	}

	operator bool()
	{
		if( xgc_nullptr == reply_ )
			return false;

		return is_error();
	}

	xgc_bool is_error()const
	{
		return reply_->type == REDIS_REPLY_ERROR;
	}

	xgc_bool is_nil()const
	{
		return reply_->type == REDIS_REPLY_NIL;
	}

	xgc_bool is_string()const
	{
		return reply_->type == REDIS_REPLY_STRING;
	}

	xgc_bool is_integer()const
	{
		return reply_->type == REDIS_REPLY_INTEGER;
	}

	xgc_bool is_status()const
	{
		return reply_->type == REDIS_REPLY_STATUS;
	}

	xgc_bool is_array()const
	{
		return reply_->type == REDIS_REPLY_ARRAY;
	}

	xgc_lpcstr string()const
	{
		return reply_->str;
	}

	xgc_int64 string_len()const
	{
		return reply_->len;
	}

	xgc_int64 integer()const
	{
		return reply_->integer;
	}

	xgc_int64 status()const
	{
		return reply_->integer;
	}

	xvariant value()const
	{
		if( reply_->type == REDIS_REPLY_NIL )
			return xvariant();

		if( reply_->type == REDIS_REPLY_STRING )
			return xvariant( reply_->str, reply_->len );
		
		if( reply_->type == REDIS_REPLY_INTEGER )
			return xvariant( reply_->integer );

		return xvariant();
	}

	redisDataIter begin()const
	{
		return reply_->element;
	}

	redisDataIter end()const
	{
		return reply_->element + reply_->elements;
	}
};

redisData redisDataIter::operator*()
{
	return reply_[0];
}

redisData* redisDataIter::operator->()
{
	return (redisData*)reply_;
}

class redisConn
{
private:
	xgc_lpcstr host_;
	xgc_uint16 port_;

	redisContext *redis_;
public:
	redisConn()
		: host_( xgc_nullptr )
		, port_( 0 )
	{

	}

	redisConn( xgc_lpcstr host, xgc_uint16 port )
		: host_( strdup( host ) )
		, port_( port )
	{

	}

	~redisConn()
	{
		free( (void*)host_ );
		host_ = xgc_nullptr;

		redisFree( redis_ );
		redis_ = xgc_nullptr;
	}

	xgc_bool connect( xgc_lpcstr host, xgc_uint16 port, int timeout = 0 )
	{
		free( (void*)host_ );
		host_ = strdup( host );
		port_ = port_;

		return connect( timeout );
	}

	xgc_bool connect( int timeout = 0 )
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
	}

	///
	/// \brief 执行redis命令
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisData exec( xgc_lpcstr fmt, ... )
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
	redisData exec_args( xgc_lpcstr fmt, va_list args )
	{
		return (redisReply*)redisCommand( redis_, fmt, args );
	}

	///
	/// \brief 选择数据库
	/// \date 2017/9/12
	/// \author albert.xu
	/// 
	xgc_bool select( int db )
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
	int get_value( xgc_lpcstr key, xvariant &val )
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
	int set_value( xgc_lpcstr key, const xvariant &val )
	{
		auto reply = exec( "SET %s %s", key, val.to_string().c_str() );
		if( reply )
		{
			return REDIS_OK;
		}

		return REDIS_ERR;
	}

	///
	/// \brief 设置字符串参数
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisKey get( xgc_lpcstr key )
	{
		return redisKey( this, key );
	}
};
#endif // __REDIS_CONN_H__