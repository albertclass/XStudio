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
	redisSet( const redisKey & rhs )
		: redisKey( rhs )
	{

	}

	///
	/// \brief 构造
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisSet( redisKey && rhs )
		: redisKey( std::forward< redisKey >( rhs ) )
	{

	}


};
#endif // __REDIS_SET_H__