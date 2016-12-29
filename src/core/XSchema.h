#pragma once

#ifdef CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif

#include "stdafx.h"
#include "serialization.h"

namespace XGC
{
	///
	/// 类型定义
	/// [7/28/2014] create by jianglei.kinly
	///
	enum CORE_API eSchemaType
	{
		eI8,     ///< int8
		eU8,     ///< unsigned int8
		eI16,    ///< int16
		eU16,    ///< unsigned int16
		eI32,    ///< int32
		eU32,    ///< unsigned int32
		eI64,    ///< int64
		eU64,    ///< unsigned int64
		eReal,   ///< float
		eString, ///< string
		eAttr,   ///< 属性

		eUnknow, ///< error
	};
	//////////////////////////////////////////////////////////////////////////
	// [1/22/2014 jianglei.kinly]
	// Schema 参数
	//////////////////////////////////////////////////////////////////////////
	struct CORE_API stSchemaArg
	{
		xgc_string      strName;     // 参数名
		eSchemaType     enType;      // 参数类型
		xgc_string      strValue;    // 具体值
		xgc_string      strEnumName; // 枚举的类型
	};

	//////////////////////////////////////////////////////////////////////////
	// [1/22/2014 jianglei.kinly]
	// XSchema 类
	// 存储、验证 schema 参数列表
	//////////////////////////////////////////////////////////////////////////
	class CORE_API XSchema
	{
	private:
		// ------------------------------------------------ //
		// [1/22/2014 jianglei.kinly]
		// SchemaName, Func ArgList
		// ------------------------------------------------ //
		xgc_unordered_map<xgc_string, std::pair<BasicActionFunc, xgc_vector<stSchemaArg> > > mSchemaMap;

	protected:
		///
		/// enums name, xAttrType, enum
		/// [7/28/2014] create by jianglei.kinly
		///
		struct EnumType
		{
			xgc_string strType;
			xgc_unordered_map<xgc_string, xgc_string> strEnumValues;
		};

		xgc_unordered_map<xgc_string, EnumType > mSchemaEnums;
	public:
		XSchema()
		{
		}

		~XSchema()
		{
		}

		// ------------------------------------------------ //
		// [1/22/2014 jianglei.kinly]
		// 是否存在该schema
		// ------------------------------------------------ //
		xgc_bool HasSchema( const xgc_string &name )
		{
			auto it = mSchemaMap.find( name );
			if( it == mSchemaMap.end() )
				return false;
			return true;
		}

		// ------------------------------------------------ //
		// [3/24/2014 jianglei.kinly]
		// 获取Schema函数指针
		// ------------------------------------------------ //
		BasicActionFunc GetSchema( const xgc_string &name )
		{
			if( HasSchema( name ) )
			{
				return mSchemaMap[name].first;
			}
			return xgc_nullptr;
		}

		// ------------------------------------------------ //
		// [1/22/2014 jianglei.kinly]
		// 插入m_umapSchema
		//  : schema_name, 参数列表, 存在是否替换
		// ------------------------------------------------ //
		xgc_bool InsertSchema( const xgc_string& strName, xgc_int32 (*func)(...), const xgc_vector<stSchemaArg>& vecArg )
		{
			if( HasSchema( strName ) )
				return false;
			mSchemaMap[strName] = std::make_pair( (BasicActionFunc)func, vecArg );
			return true;
		}

		// ------------------------------------------------ //
		// [1/22/2014 jianglei.kinly]
		// 获得schema参数列表
		// ------------------------------------------------ //
		const xgc_vector<stSchemaArg>& GetSchemaArg( const xgc_string &name )
		{
			XGC_ASSERT( HasSchema( name ) );
			return mSchemaMap[name].second;
		}

		// ------------------------------------------------ //
		// [1/22/2014 jianglei.kinly]
		// 判断Action参数是否正确，并序列化参数
		// ------------------------------------------------ //
		xgc_bool ArgIsRight( const xgc_string &strName, const xgc_unordered_map<xgc_string, xgc_string>& args, common::serialization& res );

		///
		/// 根据Type判断是否是枚举类型
		/// [7/28/2014] create by jianglei.kinly
		///
		xgc_bool IsEnumsByType( const xgc_string& str );

		///
		/// 根据配置的具体值获取对应的eSchemaType的值
		/// [7/28/2014] create by jianglei.kinly
		///
		xgc_string GetEnumValue( const stSchemaArg& arg, const xgc_string& value );

		///
		/// 字符串转xAttrType,仅判断下面四种类型
		/// [7/28/2014] create by jianglei.kinly
		///
		eSchemaType str2enschematype( const xgc_string& str, xgc_bool& isEnum );
	}; // end class XSchema
} // end namespace XGC