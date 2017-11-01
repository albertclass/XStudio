#ifndef __REDIS_SET_H__
#define __REDIS_SET_H__
#include "redis_key.h"
class redisConn;

class redisSet : public redisKey
{
public:
	///
	/// \brief 构造
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisSet( const redisKey & rhs );

	///
	/// \brief 构造
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisSet( redisKey && rhs );

	///
	/// \brief 添加元素
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	int add( xgc_lpcstr fmt, ... )const;

	///
	/// \brief 移除元素
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	int remove( xgc_lpcstr fmt, ... )const;

	///
	/// \brief 集合内的元素个数
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	int count()const;

	///
	/// \brief 元素是否存在
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	int exist( xgc_lpcstr member )const;

	///
	/// \brief 提取集合内的元素
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	int members( xgc_vector< xvariant > &lst ) const;

	///
	/// \brief 移动元素到其他集合
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	int moveto( xgc_lpcstr destination, xgc_lpcstr member );

	///
	/// \brief 随机从集合中弹出一个元素
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	int pop( xvariant &val ) const;

	///
	/// \brief 随机捡取count个元素，count < 0 元素允许重复
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	size_t rand( xvariant vals[], int count = 0 );
};
#endif // __REDIS_SET_H__