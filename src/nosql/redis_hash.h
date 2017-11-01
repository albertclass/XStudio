#ifndef __REDIS_HASH_H__
#define __REDIS_HASH_H__
#include "redis_key.h"
class redisConn;
///
/// \brief redis ��ֵ�Է�װ
/// \date 2017/9/11
/// \author albert.xu
/// 
class redisHash : public redisKey
{
public:
	///
	/// \brief ����
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	redisHash( const redisKey & rhs )
		: redisKey( rhs )
	{

	}

	///
	/// \brief ����
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	redisHash( redisKey && rhs )
		: redisKey( std::forward< redisKey >( rhs ) )
	{

	}

	///
	/// \brief ��ȡ����
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_int64 length();

	///
	/// \brief ��ֵ�Ƿ����
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_int64 exist( xgc_lpcstr key );

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
	int get( xgc_lpcstr key, xvariant &val);

	///
	/// \brief ��ȡֵ
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_vector< xvariant > get_more( xgc_vector< xgc_lpcstr > keys );

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
	xgc_int64 inc( xgc_lpcstr key, int val );

	///
	/// \brief ����ֵ
	/// \date 2017/9/11
	/// \author albert.xu
	/// 
	xgc_real64 inc( xgc_lpcstr key, xgc_real64 val );
};

#endif // __REDIS_HASH_H__