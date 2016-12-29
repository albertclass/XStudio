#include "stdafx.h"
#include "XObjectComposition.h"


namespace XGC
{
	// 无效值
	XObjectAttribute::XObjectAttribute( const XClassInfo *lpClassInfo, xgc_uintptr lpContext = 0 )
		: XObjectComposition( XCompAttribute )
		, mAttributeCount( lpClassInfo->GetAttributeCount() )
		, mImplementCount( lpClassInfo->GetImplementCount() )
		, mAttributeInfo( lpClassInfo->GetAttributeInfo() )
		, mImplementInfo( lpClassInfo->GetImplementInfo() )
		, mContext( lpContext )
	{
		FUNCTION_BEGIN;
		mAttributeHead = malloc( lpClassInfo->GetAttributeSize() );
		if( !mAttributeHead )
		{
			SYS_ERROR( "malloc %u failed", lpClassInfo->GetAttributeSize() );
			return;
		}
		memset( mAttributeHead, 0, _msize( mAttributeHead ) );
		for( xgc_size n = 0; n < mAttributeCount; ++n )
		{
			XVariantReference ref = GetAttribute( n );
			if( ref.isNumeric() )
			{
				ref = 0;
			}
		}
		FUNCTION_END;
	}

	XObjectAttribute::~XObjectAttribute()
	{
		FUNCTION_BEGIN;
		for( xAttrIndex i = 0; i < mAttributeCount; ++i )
		{
			GetAttribute( i ).Release();
		}
		free( mAttributeHead );
		mAttributeCount = 0;
		mAttributeInfo = xgc_nullptr;
		mAttributeHead = xgc_nullptr;
		FUNCTION_END;
	}

	xgc_lpcstr XObjectAttribute::GetString( xAttrIndex nAttrIndex, xgc_size nArrayIndex ) const
	{
		FUNCTION_BEGIN;
		if( mAttributeInfo[nAttrIndex].pfnGet )
		{
			xgc_lpcstr lpValue;
			if( mAttributeInfo[nAttrIndex].pfnGet( GetAttribute( nAttrIndex, nArrayIndex ), mContext, nAttrIndex, VT_STRING, &lpValue ) )
				return lpValue;
		}

		return GetAttribute( nAttrIndex, nArrayIndex ).toRawString();
		FUNCTION_END;
		return xgc_nullptr;
	}

	xgc_lpvoid XObjectAttribute::GetBuffer( xAttrIndex attrIdx ) const
	{
		FUNCTION_BEGIN;
		if( mAttributeInfo[attrIdx].pfnGet )
		{
			xgc_lpvoid	lpValue = xgc_nullptr;
			xgc_size	nSize = 0;
			if( mAttributeInfo[attrIdx].pfnGet( GetAttribute( attrIdx ), mContext, attrIdx, VT_BUFFER, &lpValue, &nSize ) )
				return lpValue;
		}

		return GetAttribute( attrIdx ).toRawBuffer();
		FUNCTION_END;
		return xgc_nullptr;
	}

	xgc_bool XObjectAttribute::SetString( xAttrIndex nAttrIndex, xgc_lpcstr attrValue, xgc_size nArrayIndex, xgc_size nLength /*= -1*/ )
	{
		FUNCTION_BEGIN;
		if( mAttributeInfo[nAttrIndex].pfnSet )
			return mAttributeInfo[nAttrIndex].pfnSet( GetAttribute( nAttrIndex ), mContext, nAttrIndex, VT_STRING, attrValue, nLength );

		GetAttribute( nAttrIndex, nArrayIndex ).setString( attrValue, nLength );
		return true;
		FUNCTION_END;
		return false;
	}

	xgc_size XObjectAttribute::GetBufferLength( xAttrIndex attrIdx )
	{
		FUNCTION_BEGIN;
		if( mAttributeInfo[attrIdx].pfnGet )
		{
			xgc_lpvoid pData = xgc_nullptr;
			xgc_size nSize = 0;

			mAttributeInfo[attrIdx].pfnGet( GetAttribute( attrIdx ), mContext, attrIdx, VT_BUFFER, &pData, &nSize );
			return nSize;
		}

		return GetAttribute( attrIdx ).getBufferLength();
		FUNCTION_END;
		return 0;
	}

	xgc_bool XObjectAttribute::SetBuffer( xAttrIndex attrIdx, xgc_lpcvoid attrValue, xgc_size nSize )
	{
		FUNCTION_BEGIN;
		if( mAttributeInfo[attrIdx].pfnSet )
			return mAttributeInfo[attrIdx].pfnSet( GetAttribute( attrIdx ), mContext, attrIdx, VT_BUFFER, attrValue, nSize );

		GetAttribute( attrIdx ).setBuffer( attrValue, nSize );
		return true;
		FUNCTION_END;
		return false;
	}

	xgc_bool XObjectAttribute::SetBufferLength( xAttrIndex attrIdx, xgc_size nSize, xgc_byte szFill /*= 0 */ )
	{
		FUNCTION_BEGIN;
		// 若只是设置缓冲区长度，则将缓冲区指针赋予空，长度不为零。
		if( mAttributeInfo[attrIdx].pfnSet )
			return mAttributeInfo[attrIdx].pfnSet( GetAttribute( attrIdx ), mContext, attrIdx, VT_BUFFER, xgc_nullptr, nSize, szFill );

		GetAttribute( attrIdx ).setBufferLength( nSize, szFill );
		return true;
		FUNCTION_END;
		return false;
	}

	xgc_bool XObjectAttribute::Load( xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize )
	{
		FUNCTION_BEGIN;
		if( uVersion )
		{
			serialization ar( lpData, uSize );

			for( xgc_size i = 0, j = 0; i < mImplementCount; ++i )
			{

				if( uVersion < mImplementInfo[i].version.start ||
					uVersion >= mImplementInfo[i].version.close )
					continue;

				if( mImplementInfo[i].flags == ATTR_FLAG_SAVE )
				{
					for( xgc_size n = 0; n < mImplementInfo[i].count; ++n )
						ar >> GetAttribute( *mImplementInfo[i].attr_ptr, n );
				}
				++j;
			}
		}
		return true;
		FUNCTION_END;

		return false;
	}

	xgc_size XObjectAttribute::Save( xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize )
	{
		FUNCTION_BEGIN;
		serialization ar( lpData, uSize );
		for( xgc_size i = 0, j = 0; i < mImplementCount; ++i )
		{
			if( uVersion < mImplementInfo[i].version.start ||
				uVersion >= mImplementInfo[i].version.close )
				continue;

			if( mImplementInfo[i].flags == ATTR_FLAG_SAVE )
			{
				for( xgc_size n = 0; n < mImplementInfo[i].count; ++n )
					ar << GetAttribute( *mImplementInfo[i].attr_ptr, n );
			}
			++j;
		}
		return ar.wd();
		FUNCTION_END;
		return 0;
	}

	xgc_void XObjectAttribute::Release()
	{
		delete this;
	}

}