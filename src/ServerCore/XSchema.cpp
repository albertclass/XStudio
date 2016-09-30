#include "stdafx.h"
#include "XSchema.h"

namespace XGC
{
	xgc_bool XSchema::IsEnumsByType( const xgc_string& str )
	{
		if( strcasecmp( str.c_str(), "I8" ) == 0
			|| strcasecmp( str.c_str(), "U8" ) == 0
			|| strcasecmp( str.c_str(), "I16" ) == 0
			|| strcasecmp( str.c_str(), "U16" ) == 0
			|| strcasecmp( str.c_str(), "I32" ) == 0
			|| strcasecmp( str.c_str(), "U32" ) == 0
			|| strcasecmp( str.c_str(), "I64" ) == 0
			|| strcasecmp( str.c_str(), "U64" ) == 0
			|| strcasecmp( str.c_str(), "Real" ) == 0
			|| strcasecmp( str.c_str(), "String" ) == 0
			|| strcasecmp( str.c_str(), "Attr" ) == 0 )
		{
			return false;
		}
		return true;
	}

	///
	/// 根据配置的具体值获取对应的eSchemaType的值
	/// [7/28/2014] create by jianglei.kinly
	///
	xgc_string XSchema::GetEnumValue( const stSchemaArg& arg, const xgc_string& value )
	{
		if( arg.strEnumName.empty() )
			return xgc_string();
		auto itEnum = mSchemaEnums.find( arg.strEnumName );
		if( itEnum == mSchemaEnums.end() )
			return xgc_string();

		auto itItem = itEnum->second.strEnumValues.find( value );
		if( itItem == itEnum->second.strEnumValues.end() )
			return xgc_string();

		return itItem->second;
	}

	///
	/// 字符串转xAttrType,仅判断下面四种类型
	///   VT_I32,
	///   VT_U32,
	///   VT_REAL,
	///   VT_STRING,
	/// [7/28/2014] create by jianglei.kinly
	///
	eSchemaType XSchema::str2enschematype( const xgc_string& str, xgc_bool& isEnum )
	{
		auto FnGetESchemaType = []( xgc_lpcstr strType )->eSchemaType
		{
			if( strcasecmp( strType, "I8" ) == 0 )
				return eI8;
			else if( strcasecmp( strType, "U8" ) == 0 )
				return eU8;
			else if( strcasecmp( strType, "I16" ) == 0 )
				return eI16;
			else if( strcasecmp( strType, "U16" ) == 0 )
				return eU16;
			else if( strcasecmp( strType, "I32" ) == 0 )
				return eI32;
			else if( strcasecmp( strType, "U32" ) == 0 )
				return eU32;
			else if( strcasecmp( strType, "I64" ) == 0 )
				return eI64;
			else if( strcasecmp( strType, "U64" ) == 0 )
				return eU64;
			else if( strcasecmp( strType, "Real" ) == 0 )
				return eReal;
			else if( strcasecmp( strType, "String" ) == 0 )
				return eString;
			else if( strcasecmp( strType, "Attr" ) == 0 )
				return eAttr;
			return eUnknow;
		};
		if( !IsEnumsByType( str ) )
		{
			isEnum = false;
			return FnGetESchemaType( str.c_str() );
		}
		else
		{
			isEnum = true;
			auto it = mSchemaEnums.find( str );
			if( it != mSchemaEnums.end() )
			{
				auto &eType = it->second;
				return FnGetESchemaType( eType.strType.c_str() );
			}
		}
		XGC_ASSERT_RETURN( false, eUnknow );
	}

	xgc_bool XSchema::ArgIsRight( const xgc_string &strName, const xgc_unordered_map<xgc_string, xgc_string>& args, common::serialization& res )
	{
		FUNCTION_BEGIN;
		auto& vec = GetSchemaArg( strName );
		for( auto it = vec.begin(); it != vec.end(); ++it )
		{
			xgc_string strValue;
			auto it2 = args.find( it->strName.c_str() );
			if( it2 == args.end() )
			{
				strValue = it->strEnumName.empty() ? it->strValue : GetEnumValue( *it, it->strValue );
				XGC_ASSERT_RETURN( !strValue.empty() || strValue != "", false );
			}
			else
			{
				// action 有配置，取配置值
				strValue = it->strEnumName.empty() ? it2->second : GetEnumValue( *it, it2->second );
				XGC_ASSERT_RETURN( !strValue.empty() || strValue != "", false );
			}
			// 公式或值类型不支持有默认值情况
			switch( it->enType )
			{
				case eI8:
				res << xgc_int8( strtol( strValue.c_str(), xgc_nullptr, 10 ) );
				break;
				case eU8:
				res << xgc_uint8( strtoul( strValue.c_str(), xgc_nullptr, 10 ) );
				break;
				case eI16:
				res << xgc_int16( strtol( strValue.c_str(), xgc_nullptr, 10 ) );
				break;
				case eU16:
				res << xgc_uint16( strtoul( strValue.c_str(), xgc_nullptr, 10 ) );
				break;
				case eI32:
				res << xgc_int32( strtol( strValue.c_str(), xgc_nullptr, 10 ) );
				break;
				case eU32:
				res << xgc_uint32( strtoul( strValue.c_str(), xgc_nullptr, 10 ) );
				break;
				case eI64:
				res << xgc_int64( strtoll( strValue.c_str(), xgc_nullptr, 20 ) );
				break;
				case eU64:
				res << xgc_uint64( strtoull( strValue.c_str(), xgc_nullptr, 20 ) );
				break;
				case eReal:
				res << xgc_real32( atof( strValue.c_str() ) );
				break;
				case eString:
				res << strValue.c_str();
				break;
				case eAttr:
				{
					xAttrIndex attrIndex = GetAttrIndexByName( strValue.c_str() );
					res << attrIndex;
				}
				break;
				default:
				XGC_ASSERT_RETURN( false, false );
				break;
			}
		}

		return true;
		FUNCTION_END;
		return false;
	}
};