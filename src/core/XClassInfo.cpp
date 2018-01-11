#include "XHeader.h"
#include "XClassInfo.h"

#define XAttributeVersion 20171227

namespace xgc
{
	///
	/// [1/24/2014 albert.xu]
	/// �����ַ�����ȡ�汾��Ϣ
	///
	XAttrVersion ConvertVersion( xgc_lpcstr lpVersion )
	{
		XAttrVersion xVersion = { 0, 0 };
		xgc_lpstr _next = xgc_nullptr;
		xVersion.start = str2numeric< xgc_uint32 >( lpVersion, &_next );
		if( _next && ( _next = strchr( _next, '-' ) ) )
			xVersion.close = str2numeric< xgc_uint32 >( _next + 1, &_next );
		else
			xVersion.close = std::numeric_limits< xgc_uint32 >::max();

		return xVersion;
	}

	XClassInfo::XClassInfo( xgc_lpcstr lpClassName, XAttributeImpl *pImplementInfo, const XClassInfo *pParent /*= xgc_nullptr */ )
		: mClassName( lpClassName )
		, mAttributeCount( 0 )
		, mImplementCount( 0 )
		, mImplementInfo( xgc_nullptr )
		, mAttributeInfo( xgc_nullptr )
		, x_implement( xgc_nullptr )
		, x_attribute( xgc_nullptr )
		, mAttributeSize( 0 )
		, mParent( pParent )
	{
		// �������Ը���
		xgc_size nAttributeSize = 0;
		xgc_size nAttributeCount = 0;
		xgc_size nImplementCount = 0;	///< ���ø���

		// �ȼ������Ը���
		while( pImplementInfo[nImplementCount].impl.name )
		{
			if( XAttributeVersion >= pImplementInfo[nImplementCount].version.start &&
				XAttributeVersion < pImplementInfo[nImplementCount].version.close )
			{
				nAttributeSize += XAttribute::Type2Size( pImplementInfo[nImplementCount].impl.type ) * pImplementInfo[nImplementCount].impl.count;

				auto ib = mName2Index.insert( std::make_pair( pImplementInfo[nImplementCount].impl.name, pImplementInfo[nImplementCount].impl.attr_ptr ) );
				XGC_ASSERT_MESSAGE( ib.second, "�����ظ��ļ�ֵ%s", pImplementInfo[nImplementCount].impl.name );

				++nAttributeCount;
			}

			++nImplementCount;
		}

		if( pParent )
		{
			mAttributeCount = pParent->mAttributeCount + nAttributeCount;
			mImplementCount = pParent->mImplementCount + nImplementCount;

			mAttributeSize = pParent->mAttributeSize + nAttributeSize;

			for( auto it : pParent->mName2Index )
			{
				auto ib = mName2Index.insert( it );
				XGC_ASSERT_MESSAGE( ib.second, "�����ظ��ļ�ֵ%s", it.first.c_str() );
			}
		}

		// �����������Զ���
		mImplementInfo = (XAttributeImpl**)malloc( mImplementCount * sizeof( XAttributeImpl* ) );

		if( pParent && pParent->mImplementInfo )
		{
			// ���Ƹ����е� mImplementInfo ָ����ָ�����ݣ�ָ���б�
			memcpy( mImplementInfo, pParent->mImplementInfo, pParent->mImplementCount * sizeof( XAttributeImpl* ) );
		}

		// ��������Զ���
		x_implement = (XAttributeImpl *)malloc( nImplementCount * sizeof( XAttributeImpl ) );
		memcpy( x_implement, pImplementInfo, nImplementCount * sizeof( XAttributeImpl ) );

		// ������Զ���ָ���б�
		for( xgc_size i = pParent->mImplementCount, j = 0; i < nImplementCount; ++i, ++j )
		{
			mImplementInfo[i] = x_implement + j;
		}

		// �����������Զ���
		mAttributeInfo = (XAttributeInfo**)malloc( mAttributeCount * sizeof( XAttributeInfo* ) );

		xgc_size nAttrIdx = 0;
		xgc_size nAttrPos = 0;
		
		if( pParent && pParent->mAttributeInfo )
		{
			// ���Ƹ����������Ϣָ���б�
			memcpy( mAttributeInfo, pParent->mAttributeInfo, pParent->mAttributeCount * sizeof( XAttributeInfo* ) );

			nAttrIdx = pParent->mAttributeCount;
			nAttrPos = pParent->mAttributeSize;
		}

		for( auto j = 0; j < nImplementCount; ++j )
		{
			if( XAttributeVersion < pImplementInfo[j].version.start )
				continue;

			if( XAttributeVersion >= pImplementInfo[j].version.close )
				continue;

			// ��ʼ�������б�
			x_attribute[j].impl           = pImplementInfo[j].impl;
			x_attribute[j].offset         = nAttrPos;
			x_attribute[j].cycle_change   = 0;
			x_attribute[j].OnValueChanged = xgc_nullptr;

			// ��������������
			x_attribute[j].impl.attr_ptr[0] = nAttrIdx++;

			// ��ָ��ָ�򱾵������б����ȷ��ַ
			mAttributeInfo[nAttrIdx] = x_attribute + j;

			// �������Ի��峤��
			nAttrPos += XAttribute::Type2Size( pImplementInfo[j].impl.type ) * pImplementInfo[j].impl.count;
		}
	}

	XClassInfo::~XClassInfo()
	{
		free( x_attribute );
		free( x_implement );

		x_attribute = xgc_nullptr;
		x_implement = xgc_nullptr;

		free( mAttributeInfo );
		free( mImplementInfo );

		mAttributeInfo = xgc_nullptr;
		mImplementInfo = xgc_nullptr;
	}
}