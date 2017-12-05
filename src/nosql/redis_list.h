#ifndef __REDIS_LIST_H__
#define __REDIS_LIST_H__
#include "redis_key.h"
class redisConn;

///
/// \brief redis �б��װ
/// \date 2017/9/11
/// \author albert.xu
/// 
class redisList : public redisKey
{
public:
	///
	/// \brief ����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisList( const redisKey & rhs )
		: redisKey( rhs )
	{

	}

	///
	/// \brief ����
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	redisList( redisKey && rhs )
		: redisKey( std::forward< redisKey >( rhs ) )
	{

	}

	///
	/// \brief ��ȡ�б���
	/// \date 2017/9/5
	/// \author albert.xu
	/// 
	xgc_int64 length();

	///
	/// \brief ��ȡ�б��е�ֵ
	/// \date 2017/9/8
	/// \author albert.xu
	/// 
	xgc::list< xvariant > get( int start = 0, int stop = -1 );

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
	xgc_int64 remove( xgc_lpcstr match, int count );

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
#endif // __REDIS_LIST_H__