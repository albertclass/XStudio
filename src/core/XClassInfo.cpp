#include "stdafx.h"
#include "XClassInfo.h"
#include "XObjectComposition.h"

namespace XGC
{
	// ------------------------------------------------ //
	// [3/25/2014 jianglei.kinly]
	// ��������-����Index��ӳ���ϵ
	// ------------------------------------------------ //
	xgc_unordered_map<xgc_string, xAttrIndex*>& getName2IndexMap()
	{
		static xgc_unordered_map<xgc_string, xAttrIndex*> Inst;
		return Inst;
	}

	// ------------------------------------------------ //
	// [3/25/2014 jianglei.kinly]
	// ͨ��Name��ȡIndex
	// ------------------------------------------------ //
	xAttrIndex GetAttrIndexByName( xgc_lpcstr lpName )
	{
		XGC_ASSERT_RETURN( lpName, -1 );
		auto& iter = getName2IndexMap().find( lpName );
		if( iter != getName2IndexMap().end() )
			return *( iter->second );
		XGC_DEBUG_MESSAGE( "û���ҵ���Ӧ��Index[%s]", lpName );
		return -1;
	}

	///
	/// [1/24/2014 albert.xu]
	/// ��ȡ���Գ���
	///
	CORE_API xgc_size AttributeLength( xAttrType eType, xgc_size nCount )
	{
		xgc_size nLength = 0;
		switch( eType )
		{
			case VT_CHAR:
			case VT_BYTE:
			case VT_BOOL:
			nLength = 1;
			break;
			case VT_I16:
			case VT_U16:
			nLength = 2;
			break;
			case VT_I32:
			case VT_U32:
			case VT_REAL:
			nLength = 4;
			break;
			case VT_I64:
			case VT_U64:
			nLength = 8;
			break;
			case VT_STRING:
			case VT_BUFFER:
			nLength = sizeof( xgc_lpvoid );
			break;
			default:
			XGC_DEBUG_MESSAGE( "δ֪���������͡�" );
			break;
		}

		return XGC_ALIGNMENT( nLength * nCount, 4 );
	}

	///
	/// [1/24/2014 albert.xu]
	/// �����ַ�����ȡ�汾��Ϣ
	///
	XAttributeVersion AttributeVersion( xgc_lpcstr lpVersion )
	{
		XAttributeVersion xVersion = { 0, 0 };
		xgc_lpstr _next = xgc_nullptr;
		xVersion.start = str2numeric< xgc_uint32 >( lpVersion, &_next );
		if( _next && ( _next = strchr( _next, '-' ) ) )
			xVersion.close = str2numeric< xgc_uint32 >( _next + 1, &_next );
		else
			xVersion.close = std::numeric_limits< xgc_uint32 >::max();

		return xVersion;
	}

	XClassInfo::XClassInfo( xClassType nClassType, xgc_lpcstr lpClassName, XImplementInfo *pImplementInfo, const XClassInfo *pParent /*= xgc_nullptr */ )
		: mInheritLevel( 0 )
		, mAttributeCount( 0 )
		, mAttributeSize( 0 )
		, mImplementInfo( xgc_nullptr )
		, mAttributeInfo( xgc_nullptr )
		, mClassName( lpClassName )
		, mParent( pParent )
	{
		// �������Ը���
		xgc_size nAttributeSize  = 0;
		xgc_size nAttributeCount = 0;
		xgc_size nImplementCount = 0;	///< ���ø���

		// �ȼ������Ը���
		while( pImplementInfo[nImplementCount].name )
		{
			if( XCompAttributeVersion >= pImplementInfo[nImplementCount].version.start &&
				XCompAttributeVersion <  pImplementInfo[nImplementCount].version.close )
			{
				nAttributeSize += AttributeLength( pImplementInfo[nImplementCount].type, pImplementInfo[nImplementCount].count );
				
				auto ib = getName2IndexMap().insert( std::make_pair( pImplementInfo[nImplementCount].name, pImplementInfo[nImplementCount].attr_ptr ) );
				XGC_ASSERT_MESSAGE( ib.second, "�����ظ��ļ�ֵ%s", pImplementInfo[nImplementCount].name );

				++nAttributeCount;
			}

			++nImplementCount;
		}

		if( pParent )
		{
			mInheritLevel   = pParent->mInheritLevel;

			mAttributeCount = pParent->mAttributeCount + nAttributeCount;
			mImplementCount = pParent->mImplementCount + nImplementCount;

			mAttributeSize  = pParent->mAttributeSize  + nAttributeSize;

			for( xgc_size i = 0; i < mInheritLevel; ++i )
			{
				mClassType[i] = pParent->mClassType[i];
			}
		}

		XGC_ASSERT_MESSAGE( ( ( (xgc_byteptr) &nClassType )[3] & 0x0f ) == mInheritLevel,
			"�������˵����û�úÿ�CLSID����ɷ�ʽ��CLSID�ĵ�23λ~27λ�����������ļ̳���ȡ�" );

		mClassType[mInheritLevel++] = nClassType;

		XGC_ASSERT_MESSAGE( XGC_CHECK_ARRAY_INDEX( mClassType, mInheritLevel ),
			"�̳еĲ��̫���ˣ��Ѿ�����������������" );

		// �����������Զ���
		if( mImplementCount )
		{
			mImplementInfo = (XImplementInfo*) malloc( mImplementCount * sizeof( XImplementInfo ) );

			if( pParent && pParent->mAttributeInfo )
			{
				memcpy_s( mImplementInfo, _msize( mImplementInfo )
					, pParent->mImplementInfo, pParent->mImplementCount * sizeof( XImplementInfo ) );
			}

			memcpy_s( mImplementInfo + pParent->mAttributeCount, _msize( mImplementInfo ) - pParent->mImplementCount * sizeof( XImplementInfo )
				, pImplementInfo, nImplementCount * sizeof( XImplementInfo ) );
		}

		// �����������Զ���
		if( mAttributeCount )
		{
			mAttributeInfo = (XAttributeInfo*) malloc( mAttributeCount * sizeof( XAttributeInfo ) );

			xgc_size nAttrIdx = 0;
			xgc_size nAttrPos = 0;
			if( pParent && pParent->mAttributeInfo )
			{
				memcpy_s( mAttributeInfo, _msize( mAttributeInfo )
					, pParent->mAttributeInfo, pParent->mAttributeCount * sizeof( XAttributeInfo ) );

				nAttrIdx = pParent->mAttributeCount;
				nAttrPos = pParent->mAttributeSize;
			}

			for( auto j = 0; j < nImplementCount; ++j )
			{
				if( XCompAttributeVersion < pImplementInfo[j].version.start )
					continue;

				if( XCompAttributeVersion >= pImplementInfo[j].version.close )
					continue;

				mAttributeInfo[nAttrIdx].name     = pImplementInfo[j].name;
				mAttributeInfo[nAttrIdx].flags    = pImplementInfo[j].flags;
				mAttributeInfo[nAttrIdx].type     = pImplementInfo[j].type;
				mAttributeInfo[nAttrIdx].count    = pImplementInfo[j].count;
				mAttributeInfo[nAttrIdx].offset   = nAttrPos;
				mAttributeInfo[nAttrIdx].pfnGet   = pImplementInfo[j].pfnGet;
				mAttributeInfo[nAttrIdx].pfnSet   = pImplementInfo[j].pfnSet;
				mAttributeInfo[nAttrIdx].attr_ptr = pImplementInfo[j].attr_ptr;

				// ��������������
				mAttributeInfo[nAttrIdx].attr_ptr[0] = nAttrIdx++;

				// �������Ի��峤��
				nAttrPos += AttributeLength( pImplementInfo[j].type, pImplementInfo[j].count );
			}
		}
	}

	XClassInfo::~XClassInfo()
	{
		FUNCTION_BEGIN;
		free( mAttributeInfo );
		mAttributeInfo = xgc_nullptr;
		FUNCTION_END;
	}

}