#ifndef __REDIS_KEY_H__
#define __REDIS_KEY_H__

/// redis 集合类型
class redisConn;
class redisKey
{
protected:
	redisConn *conn_;
	/// 集合键值
	char *key_;
public:
	///
	/// \brief 构造
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisKey( redisConn *conn, xgc_lpcstr key )
		: key_( strdup( key ) )
		, conn_( conn )
	{

	}

	///
	/// \brief 构造
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisKey( const redisKey & v )
		: key_( strdup( v.key_ ) )
		, conn_( v.conn_ )
	{

	}

	///
	/// \brief 构造
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisKey( redisKey && v )
		: key_( xgc_nullptr )
		, conn_( v.conn_ )
	{
		std::swap( key_, v.key_ );
	}

	///
	/// \brief 赋值
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisKey & operator=( const redisKey &v )
	{
		key_ = strdup( v.key_ );
		conn_ = v.conn_;
		return *this;
	}

	///
	/// \brief 赋值
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisKey & operator=( redisKey && v )
	{
		std::swap( key_, v.key_ );
		conn_ = v.conn_;
		return *this;
	}

	///
	/// \brief 析构
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	~redisKey()
	{
		free( key_ );
	}

	///
	/// \brief 获取键值
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	xgc_lpcstr key()const
	{
		return key_;
	}
};

#endif // __REDIS_KEY_H__