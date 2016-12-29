#include "stdafx.h"
#include "XClassInfo.h"
#include "XObjectComposition.h"

namespace XGC
{
	// ------------------------------------------------ //
	// [3/25/2014 jianglei.kinly]
	// 属性名称-属性Index的映射关系
	// ------------------------------------------------ //
	xgc_unordered_map<xgc_string, xAttrIndex*>& getName2IndexMap()
	{
		static xgc_unordered_map<xgc_string, xAttrIndex*> Inst;
		return Inst;
	}

	// ------------------------------------------------ //
	// [3/25/2014 jianglei.kinly]
	// 通过Name获取Index
	// ------------------------------------------------ //
	xAttrIndex GetAttrIndexByName( xgc_lpcstr lpName )
	{
		XGC_ASSERT_RETURN( lpName, -1 );
		auto& iter = getName2IndexMap().find( lpName );
		if( iter != getName2IndexMap().end() )
			return *( iter->second );
		XGC_DEBUG_MESSAGE( "没有找到对应的Index[%s]", lpName );
		return -1;
	}

	///
	/// [1/24/2014 albert.xu]
	/// 获取属性长度
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
			XGC_DEBUG_MESSAGE( "未知的属性类型。" );
			break;
		}

		return XGC_ALIGNMENT( nLength * nCount, 4 );
	}

	///
	/// [1/24/2014 albert.xu]
	/// 根据字符串获取版本信息
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
		// 计算属性个数
		xgc_size nAttributeSize  = 0;
		xgc_size nAttributeCount = 0;
		xgc_size nImplementCount = 0;	///< 配置个数

		// 先计算属性个数
		while( pImplementInfo[nImplementCount].name )
		{
			if( XCompAttributeVersion >= pImplementInfo[nImplementCount].version.start &&
				XCompAttributeVersion <  pImplementInfo[nImplementCount].version.close )
			{
				nAttributeSize += AttributeLength( pImplementInfo[nImplementCount].type, pImplementInfo[nImplementCount].count );
				
				auto ib = getName2IndexMap().insert( std::make_pair( pImplementInfo[nImplementCount].name, pImplementInfo[nImplementCount].attr_ptr ) );
				XGC_ASSERT_MESSAGE( ib.second, "发现重复的键值%s", pImplementInfo[nImplementCount].name );

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
			"这里断言说明你没好好看CLSID的组成方式！CLSID的第23位~27位代表了这个类的继承深度。" );

		mClassType[mInheritLevel++] = nClassType;

		XGC_ASSERT_MESSAGE( XGC_CHECK_ARRAY_INDEX( mClassType, mInheritLevel ),
			"继承的层次太深了，已经超过允许的最大上限" );

		// 若该类有属性定义
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

		// 若该类有属性定义
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

				// 计算属性索引号
				mAttributeInfo[nAttrIdx].attr_ptr[0] = nAttrIdx++;

				// 计算属性缓冲长度
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