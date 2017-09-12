#ifndef __SERVER_REDIS_H__
#define __SERVER_REDIS_H__
#include "xvariant.h"

typedef std::unique_ptr< redisReply, decltype( freeReplyObject )* > reply_ptr;

class redisExecption : public std::exception
{
	typedef std::exception base;
public:
	redisExecption()throw()
		: base()
	{

	}

	redisExecption( char const* _Message, int err )throw()
		: base( _Message, err )
	{

	}

	explicit redisExecption( char const* const _Message ) throw()
		: base( _Message )
	{

	}

	redisExecption( redisExecption const &_Other )
		: base( _Other )
	{

	}

	redisExecption& operator=( redisExecption const & _Other )
	{
		*static_cast<redisExecption*>( this ) = static_cast<redisExecption const &>( _Other );
		return *this;
	}
};
/// redis ��������
class redisValue
{
protected:
	redisContext *redis_;
	/// ���ϼ�ֵ
	char *key_;
public:
	///
	/// \brief ����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisValue( redisContext * context, xgc_lpcstr key );

	///
	/// \brief ����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisValue( const redisValue & v )
		: key_( strdup( v.key_ ) )
		, redis_( v.redis_ )
	{

	}

	///
	/// \brief ����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisValue( redisValue && v )
		: key_( xgc_nullptr )
		, redis_( v.redis_ )
	{
		std::swap( key_, v.key_ );
	}

	///
	/// \brief ��ֵ
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisValue & operator=( const redisValue &v )
	{
		key_ = strdup( v.key_ );
		redis_ = v.redis_;
		return *this;
	}

	///
	/// \brief ��ֵ
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisValue & operator=( redisValue && v )
	{
		std::swap( key_, v.key_ );
		redis_ = v.redis_;
		return *this;
	}

	///
	/// \brief ����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	~redisValue()
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

	///
	/// \brief ִ��redis����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	reply_ptr exec( xgc_lpcstr fmt, ... );

	///
	/// \brief ִ��redis����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	reply_ptr exec_args( xgc_lpcstr fmt, va_list args );

	///
	/// \brief ��ȡreply�ķ���ֵ
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	int take( redisReply *reply, xvariant &val );

	///
	/// \brief ��ȡreply�ķ���ֵ
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	int take( redisReply *reply, int &val );

	///
	/// \brief ��ȡreply�ķ���ֵ
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	int take( redisReply *reply, xgc_string &val );
};

///
/// \brief redis �б��װ
/// \date 2017/9/11
/// \author albert.xu
/// 
class redisList : public redisValue
{
public:
	redisList( redisContext * context, xgc_lpcstr key )
		: redisValue( context, key )
	{

	}

	///
	/// \brief ����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisList( const redisList & rhs )
		: redisValue( rhs )
	{

	}

	///
	/// \brief ����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisList( redisList && rhs )
		: redisValue( std::forward< redisList >( rhs ) )
	{
		
	}

	///
	/// \brief ��ֵ
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	redisList& operator=( const redisList &rhs )
	{
		*static_cast< redisValue* >( this ) = static_cast< const redisValue& >( rhs );
	}

	///
	/// \brief ��ֵ
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	redisList& operator=( redisList &&rhs )
	{
		*static_cast< redisValue* >( this ) = static_cast< const redisValue& >( rhs );
	}

	///
	/// \brief ��ȡ�б���
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	int length();

	///
	/// \brief ��ȡ�б��е�ֵ
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	xgc_list< xvariant > get( int start = 0, int stop = -1 );

	///
	/// \brief ����һ��Ԫ��
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	int lpush( xvariant &&val, bool create = true );

	///
	/// \brief ����һ��Ԫ��
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	int rpush( xvariant &&val, bool create = true );

	///
	/// \brief ����һ��Ԫ��
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	int lpop( bool block, xvariant &val );

	///
	/// \brief ����һ��Ԫ��
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	int rpop( bool block, xvariant &val );

	///
	/// \brief ɾ���б���Ԫ��
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	/// count ��ֵ���������¼��֣�
	///		count > 0 : �ӱ�ͷ��ʼ���β�������Ƴ��� value ��ȵ�Ԫ�أ�����Ϊ count ��
	///		count < 0 : �ӱ�β��ʼ���ͷ�������Ƴ��� value ��ȵ�Ԫ�أ�����Ϊ count �ľ���ֵ��
	///		count = 0 : �Ƴ����������� value ��ȵ�ֵ��
	int remove( xgc_lpcstr match, int count );

	///
	/// \brief �����б���Ԫ�ص�ֵ
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	int set( int index, xgc_lpcstr fmt, ... );

	///
	/// \brief �����б���Ԫ�ص�ֵ
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	int trim( int start = 0, int stop = -1 );

	///
	/// \brief ��ȡ�±�Ϊindex��Ԫ��
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	xvariant at( int index );

	///
	/// \brief ��ȡ�±�Ϊindex��Ԫ��
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xvariant poppush( xgc_lpcstr source, xgc_lpcstr target, int timeout = -1 );
};

///
/// \brief redis ��ֵ�Է�װ
/// \date 2017/9/11
/// \author albert.xu
/// 
class redisHash : public redisValue
{
public:
	///
	/// \brief ����
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	redisHash( redisContext * context, xgc_lpcstr key )
		: redisValue( context, key )
	{

	}

	///
	/// \brief ����
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	redisHash( const redisHash & rhs )
		: redisValue( rhs )
	{

	}

	///
	/// \brief ����
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	redisHash( redisHash && rhs )
		: redisValue( std::forward< redisHash >( rhs ) )
	{

	}

	///
	/// \brief ��ȡ����
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	int length();

	///
	/// \brief ��ֵ�Ƿ����
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	int exist( xgc_lpcstr key );

	///
	/// \brief ��ȡ�������м�ֵ
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_vector< xvariant > keys();

	///
	/// \brief ��ȡ��������ֵ
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_vector< xvariant > vals();

	///
	/// \brief ��ȡֵ
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xvariant get( xgc_lpcstr key );

	///
	/// \brief ��ȡֵ
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_vector< xvariant > get( xgc_list< xgc_lpcstr > keys );

	///
	/// \brief ��ȡֵ
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_vector< xvariant > get_all();

	///
	/// \brief ����ֵ
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	int set( xgc_lpcstr key, xgc_lpcstr val, xgc_bool create = true );

	///
	/// \brief ����ֵ
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	int inc( xgc_lpcstr key, int val );

	///
	/// \brief ����ֵ
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_real64 inc( xgc_lpcstr key, xgc_real64 val );
};

///
/// \brief ��ʼ��Redisģ��
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_bool InitServerRedis( ini_reader &ini );

///
/// \brief ʹ�����ݿ�
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long UsingDB( int index );

///
/// \brief ��ȡ�ַ�������
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisGetStr( xgc_lpcstr key, xgc_lpstr buf, xgc_size len );

///
/// \brief �����ַ�������
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisSetStr( xgc_lpcstr key, xgc_lpcstr val );

///
/// \brief ��ȡ��ֵ����
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisGetVal( xgc_lpcstr key, xgc_int64 &val );

///
/// \brief ������ֵ����
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisSetVal( xgc_lpcstr key, xgc_int64 val );

///
/// \brief ����Redisģ��
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_void FiniServerRedis();
#endif // __SERVER_REDIS_H__