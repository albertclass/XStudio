#ifndef __REDIS_KEY_H__
#define __REDIS_KEY_H__

/// redis ��������
class redisConn;
class redisKey
{
protected:
	redisConn *conn_;
	/// ���ϼ�ֵ
	char *key_;
public:
	///
	/// \brief ����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisKey( redisConn *conn, xgc_lpcstr key )
		: key_( strdup( key ) )
		, conn_( conn )
	{

	}

	///
	/// \brief ����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisKey( const redisKey & v )
		: key_( strdup( v.key_ ) )
		, conn_( v.conn_ )
	{

	}

	///
	/// \brief ����
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
	/// \brief ��ֵ
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
	/// \brief ��ֵ
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
	/// \brief ����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	~redisKey()
	{
		free( key_ );
	}

	///
	/// \brief ��ȡ��ֵ
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	xgc_lpcstr key()const
	{
		return key_;
	}
};

#endif // __REDIS_KEY_H__