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
/// redis 集合类型
class redisValue
{
protected:
	redisContext *redis_;
	/// 集合键值
	char *key_;
public:
	///
	/// \brief 构造
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisValue( redisContext * context, xgc_lpcstr key );

	///
	/// \brief 构造
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisValue( const redisValue & v )
		: key_( strdup( v.key_ ) )
		, redis_( v.redis_ )
	{

	}

	///
	/// \brief 构造
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
	/// \brief 赋值
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
	/// \brief 赋值
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
	/// \brief 析构
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	~redisValue()
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

	///
	/// \brief 执行redis命令
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	reply_ptr exec( xgc_lpcstr fmt, ... );

	///
	/// \brief 执行redis命令
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	reply_ptr exec_args( xgc_lpcstr fmt, va_list args );

	///
	/// \brief 获取reply的返回值
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	int take( redisReply *reply, xvariant &val );

	///
	/// \brief 获取reply的返回值
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	int take( redisReply *reply, int &val );

	///
	/// \brief 获取reply的返回值
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	int take( redisReply *reply, xgc_string &val );
};

///
/// \brief redis 列表封装
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
	/// \brief 构造
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisList( const redisList & rhs )
		: redisValue( rhs )
	{

	}

	///
	/// \brief 构造
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisList( redisList && rhs )
		: redisValue( std::forward< redisList >( rhs ) )
	{
		
	}

	///
	/// \brief 赋值
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	redisList& operator=( const redisList &rhs )
	{
		*static_cast< redisValue* >( this ) = static_cast< const redisValue& >( rhs );
	}

	///
	/// \brief 赋值
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	redisList& operator=( redisList &&rhs )
	{
		*static_cast< redisValue* >( this ) = static_cast< const redisValue& >( rhs );
	}

	///
	/// \brief 获取列表长度
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	int length();

	///
	/// \brief 获取列表中的值
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	xgc_list< xvariant > get( int start = 0, int stop = -1 );

	///
	/// \brief 弹出一个元素
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	int lpush( xvariant &&val, bool create = true );

	///
	/// \brief 弹出一个元素
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	int rpush( xvariant &&val, bool create = true );

	///
	/// \brief 弹出一个元素
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	int lpop( bool block, xvariant &val );

	///
	/// \brief 弹出一个元素
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	int rpop( bool block, xvariant &val );

	///
	/// \brief 删除列表中元素
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	/// count 的值可以是以下几种：
	///		count > 0 : 从表头开始向表尾搜索，移除与 value 相等的元素，数量为 count 。
	///		count < 0 : 从表尾开始向表头搜索，移除与 value 相等的元素，数量为 count 的绝对值。
	///		count = 0 : 移除表中所有与 value 相等的值。
	int remove( xgc_lpcstr match, int count );

	///
	/// \brief 设置列表中元素的值
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	int set( int index, xgc_lpcstr fmt, ... );

	///
	/// \brief 设置列表中元素的值
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	int trim( int start = 0, int stop = -1 );

	///
	/// \brief 获取下标为index的元素
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	xvariant at( int index );

	///
	/// \brief 获取下标为index的元素
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xvariant poppush( xgc_lpcstr source, xgc_lpcstr target, int timeout = -1 );
};

///
/// \brief redis 键值对封装
/// \date 2017/9/11
/// \author albert.xu
/// 
class redisHash : public redisValue
{
public:
	///
	/// \brief 构造
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	redisHash( redisContext * context, xgc_lpcstr key )
		: redisValue( context, key )
	{

	}

	///
	/// \brief 构造
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	redisHash( const redisHash & rhs )
		: redisValue( rhs )
	{

	}

	///
	/// \brief 构造
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	redisHash( redisHash && rhs )
		: redisValue( std::forward< redisHash >( rhs ) )
	{

	}

	///
	/// \brief 获取表长度
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	int length();

	///
	/// \brief 键值是否存在
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	int exist( xgc_lpcstr key );

	///
	/// \brief 获取表中所有键值
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_vector< xvariant > keys();

	///
	/// \brief 获取表中所有值
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_vector< xvariant > vals();

	///
	/// \brief 获取值
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xvariant get( xgc_lpcstr key );

	///
	/// \brief 获取值
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_vector< xvariant > get( xgc_list< xgc_lpcstr > keys );

	///
	/// \brief 获取值
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_vector< xvariant > get_all();

	///
	/// \brief 设置值
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	int set( xgc_lpcstr key, xgc_lpcstr val, xgc_bool create = true );

	///
	/// \brief 增加值
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	int inc( xgc_lpcstr key, int val );

	///
	/// \brief 增加值
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_real64 inc( xgc_lpcstr key, xgc_real64 val );
};

///
/// \brief 初始化Redis模块
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_bool InitServerRedis( ini_reader &ini );

///
/// \brief 使用数据库
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long UsingDB( int index );

///
/// \brief 获取字符串参数
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisGetStr( xgc_lpcstr key, xgc_lpstr buf, xgc_size len );

///
/// \brief 设置字符串参数
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisSetStr( xgc_lpcstr key, xgc_lpcstr val );

///
/// \brief 获取数值参数
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisGetVal( xgc_lpcstr key, xgc_int64 &val );

///
/// \brief 设置数值参数
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_long redisSetVal( xgc_lpcstr key, xgc_int64 val );

///
/// \brief 清理Redis模块
/// \date 2017/9/5
/// \author albert.xu
/// 
xgc_void FiniServerRedis();
#endif // __SERVER_REDIS_H__