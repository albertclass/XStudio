#ifndef __REDIS_SET_H__
#define __REDIS_SET_H__
#include "redis_key.h"
class redisConn;

class redisSet : public redisKey
{
public:
	///
	/// \brief ����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisSet( const redisKey & rhs );

	///
	/// \brief ����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisSet( redisKey && rhs );

	///
	/// \brief ���Ԫ��
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	int add( xgc_lpcstr fmt, ... )const;

	///
	/// \brief �Ƴ�Ԫ��
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	int remove( xgc_lpcstr fmt, ... )const;

	///
	/// \brief �����ڵ�Ԫ�ظ���
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	int count()const;

	///
	/// \brief Ԫ���Ƿ����
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	int exist( xgc_lpcstr member )const;

	///
	/// \brief ��ȡ�����ڵ�Ԫ��
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	int members( xgc_vector< xvariant > &lst ) const;

	///
	/// \brief �ƶ�Ԫ�ص���������
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	int moveto( xgc_lpcstr destination, xgc_lpcstr member );

	///
	/// \brief ����Ӽ����е���һ��Ԫ��
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	int pop( xvariant &val ) const;

	///
	/// \brief �����ȡcount��Ԫ�أ�count < 0 Ԫ�������ظ�
	/// \date 2017/9/13
	/// \author albert.xu
	/// 
	size_t rand( xvariant vals[], int count = 0 );
};
#endif // __REDIS_SET_H__