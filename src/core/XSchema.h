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
	/// ���Ͷ���
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
		eAttr,   ///< ����

		eUnknow, ///< error
	};
	//////////////////////////////////////////////////////////////////////////
	// [1/22/2014 jianglei.kinly]
	// Schema ����
	//////////////////////////////////////////////////////////////////////////
	struct CORE_API stSchemaArg
	{
		xgc_string      strName;     // ������
		eSchemaType     enType;      // ��������
		xgc_string      strValue;    // ����ֵ
		xgc_string      strEnumName; // ö�ٵ�����
	};

	//////////////////////////////////////////////////////////////////////////
	// [1/22/2014 jianglei.kinly]
	// XSchema ��
	// �洢����֤ schema �����б�
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
		// �Ƿ���ڸ�schema
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
		// ��ȡSchema����ָ��
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
		// ����m_umapSchema
		//  : schema_name, �����б�, �����Ƿ��滻
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
		// ���schema�����б�
		// ------------------------------------------------ //
		const xgc_vector<stSchemaArg>& GetSchemaArg( const xgc_string &name )
		{
			XGC_ASSERT( HasSchema( name ) );
			return mSchemaMap[name].second;
		}

		// ------------------------------------------------ //
		// [1/22/2014 jianglei.kinly]
		// �ж�Action�����Ƿ���ȷ�������л�����
		// ------------------------------------------------ //
		xgc_bool ArgIsRight( const xgc_string &strName, const xgc_unordered_map<xgc_string, xgc_string>& args, common::serialization& res );

		///
		/// ����Type�ж��Ƿ���ö������
		/// [7/28/2014] create by jianglei.kinly
		///
		xgc_bool IsEnumsByType( const xgc_string& str );

		///
		/// �������õľ���ֵ��ȡ��Ӧ��eSchemaType��ֵ
		/// [7/28/2014] create by jianglei.kinly
		///
		xgc_string GetEnumValue( const stSchemaArg& arg, const xgc_string& value );

		///
		/// �ַ���תxAttrType,���ж�������������
		/// [7/28/2014] create by jianglei.kinly
		///
		eSchemaType str2enschematype( const xgc_string& str, xgc_bool& isEnum );
	}; // end class XSchema
} // end namespace XGC