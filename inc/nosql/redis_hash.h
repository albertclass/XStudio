#ifndef __REDIS_HASH_H__
#define __REDIS_HASH_H__
#include "redis_key.h"
class redisConn;
///
/// \brief redis 键值对封装
/// \date 2017/9/11
/// \author albert.xu
/// 
class redisHash : public redisKey
{
public:
	///
	/// \brief 构造
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	redisHash( const redisKey & rhs )
		: redisKey( rhs )
	{

	}

	///
	/// \brief 构造
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	redisHash( redisKey && rhs )
		: redisKey( std::forward< redisKey >( rhs ) )
	{

	}

	///
	/// \brief 获取表长度
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_int64 length();

	///
	/// \brief 键值是否存在
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_int64 exist( xgc_lpcstr key );

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
	int get( xgc_lpcstr key, xvariant &val);

	///
	/// \brief 获取值
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_vector< xvariant > get_more( xgc_vector< xgc_lpcstr > keys );

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
	xgc_int64 inc( xgc_lpcstr key, int val );

	///
	/// \brief 增加值
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_real64 inc( xgc_lpcstr key, xgc_real64 val );
};

#endif // __REDIS_HASH_H__