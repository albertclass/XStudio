#ifndef __REDIS_CONN_H__
#define __REDIS_CONN_H__
#include "redis_key.h"
///
/// \brief redis异常
/// \author albert.xu
/// \date 2017/08/23
///
class redisExecption : public std::exception
{
	typedef std::exception base;
public:
	redisExecption()throw( );

	redisExecption( char const* _Message, int err )throw( );

	explicit redisExecption( char const* const _Message ) throw( );

	redisExecption( redisExecption const &_Other );
};

class redisData;

///
/// \brief 数据迭代器
/// \author albert.xu
/// \date 2017/08/23
///
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

///
/// \brief 数据封装
/// \author albert.xu
/// \date 2017/08/23
///
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

	xgc_size count()const
	{
		return reply_->elements;
	}

	xvariant value()const;

	redisDataIter begin()const
	{
		return reply_->element;
	}

	redisDataIter end()const
	{
		return reply_->element + reply_->elements;
	}

	redisData operator[]( int i );
};

XGC_INLINE redisData redisDataIter::operator*()
{
	return reply_[0];
}

XGC_INLINE redisData* redisDataIter::operator->()
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
	redisConn();

	redisConn( xgc_lpcstr host, xgc_uint16 port );

	~redisConn();

	xgc_bool connect( xgc_lpcstr host, xgc_uint16 port, int timeout = 0 );

	xgc_bool connect( int timeout = 0 );

	///
	/// \brief 执行redis命令
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisData exec( xgc_lpcstr fmt, ... );

	///
	/// \brief 执行redis命令
	/// \date 2017/9/12
	/// \author albert.xu
	/// 
	redisData exec_args( xgc_lpcstr fmt, va_list args );

	///
	/// \brief 选择数据库
	/// \date 2017/9/12
	/// \author albert.xu
	/// 
	xgc_bool select( int db );

	///
	/// \brief 获取字符串参数
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	int get_value( xgc_lpcstr key, xvariant &val );

	///
	/// \brief 设置字符串参数
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	int set_value( xgc_lpcstr key, const xvariant &val );

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