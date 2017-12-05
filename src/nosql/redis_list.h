#ifndef __REDIS_LIST_H__
#define __REDIS_LIST_H__
#include "redis_key.h"
class redisConn;

///
/// \brief redis 列表封装
/// \date 2017/9/11
/// \author albert.xu
/// 
class redisList : public redisKey
{
public:
	///
	/// \brief 构造
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisList( const redisKey & rhs )
		: redisKey( rhs )
	{

	}

	///
	/// \brief 构造
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisList( redisKey && rhs )
		: redisKey( std::forward< redisKey >( rhs ) )
	{

	}

	///
	/// \brief 获取列表长度
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	xgc_int64 length();

	///
	/// \brief 获取列表中的值
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	xgc::list< xvariant > get( int start = 0, int stop = -1 );

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
	xgc_int64 remove( xgc_lpcstr match, int count );

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
#endif // __REDIS_LIST_H__