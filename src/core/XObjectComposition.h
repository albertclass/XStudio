#pragma once
#ifndef _XOBJECT_COMPOSION_H_
#define _XOBJECT_COMPOSION_H_
#define EXPORT	__declspec(dllexport)
#ifdef CORE_EXPORTS
#	define CORE_API __declspec(dllexport)
#else
#	define CORE_API __declspec(dllimport)
#endif	//CORE_EXPORTS
namespace XGC
{
	#define XCompAttribute	0
	#define XCompAttributeVersion 20150803U

	struct XAttributeInfo;
	struct CORE_API XObjectComposition
	{
		XObjectComposition( xgc_uint16 nType )
			: mType( nType )
		{
		}

		virtual xgc_bool Load( xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize ) = 0;
		virtual xgc_size Save( xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize ) = 0;

		virtual xgc_void Release() = 0;

		xgc_uint16	mType;
	};

	class CORE_API XObjectAttribute : public XObjectComposition
	{
		friend class XObject;
	private:
		const XAttributeInfo * mAttributeInfo = xgc_nullptr;	// 该层次的类的属性描述指针
		const XImplementInfo * mImplementInfo = xgc_nullptr;

		xgc_lpvoid		mAttributeHead = xgc_nullptr;

		xgc_size		mAttributeCount = 0;
		xgc_size		mImplementCount = 0;
		xgc_uintptr		mContext = 0;
	public:
		XObjectAttribute( const XClassInfo *lpClassInfo, xgc_uintptr lpContext );
		~XObjectAttribute();

	public:
		///
		/// [1/7/2014 albert.xu]
		/// 检查索引是否合法
		///
		XGC_INLINE xgc_bool CheckIndex( xgc_size nAttr, xgc_size nIndex = 0 )const
		{
			if( nAttr >= mAttributeCount )
				return false;

			if( nIndex >= mAttributeInfo[nAttr].count )
				return false;

			return true;
		}

		///
		/// [1/7/2014 albert.xu]
		/// 获取指定属性的类型
		///
		XGC_INLINE xAttrType GetAttributeType( xgc_size nIndex )const
		{
			XGC_ASSERT_RETURN( CheckIndex( nIndex ), VT_VOID );
			return mAttributeInfo[nIndex].type;
		}

		///
		/// [1/7/2014 albert.xu]
		/// 获取属性个数
		///
		XGC_INLINE xgc_size GetAttributeCount()const
		{
			return mAttributeCount;
		}


		///
		/// 获取属性个数
		/// [9/12/2014] create by albert.xu
		///
		XGC_INLINE xgc_size GetArrayCount( xAttrIndex attrIdx ) const
		{
			XGC_ASSERT_THROW( CheckIndex( attrIdx ), std::logic_error( "get attribute, index out of bound." ) );
			return mAttributeInfo[attrIdx].count;
		}

		///
		/// [2/11/2014 albert.xu]
		/// 获取属性
		/// 该函数提供一个不调用属性设置Hook的方法，其他设置和取值函数皆调用Hook。
		///
		XGC_INLINE XVariantReference GetAttribute( xAttrIndex attrIdx, xgc_size nIndex = 0 ) const
		{
			XGC_ASSERT_THROW( CheckIndex( attrIdx ), std::logic_error( "get attribute, index out of bound." ) );
			xgc_lpstr lpValue = (xgc_lpstr) mAttributeHead + mAttributeInfo[attrIdx].offset;
			return XVariantReference( mAttributeInfo[attrIdx].type, 
				lpValue + nIndex * XVariant::Type2Size( mAttributeInfo[attrIdx].type ) );
		}

		///
		/// [1/7/2014 albert.xu]
		/// 获取有符号数
		///
		template< class T, typename enable_if< is_numeric< T >::value, xgc_bool >::type = true >
		T GetValue( xAttrIndex attrIdx, xgc_size nIndex = 0 ) const
		{
			XGC_ASSERT_THROW( CheckIndex( attrIdx ), std::logic_error( "get attribute, index out of bound." ) );
			FUNCTION_BEGIN;
			if( mAttributeInfo[attrIdx].pfnGet )
			{
				T _Value;
				if( mAttributeInfo[attrIdx].pfnGet(
					GetAttribute( attrIdx ),
					(xObject)mContext,
					attrIdx,
					(xAttrType) XVariant::Type2Value< T >::Value, &_Value ) )
					return _Value;
			}

			return GetAttribute( attrIdx, nIndex ).toNumeric< T >();
			FUNCTION_END;
			return 0;
		}

		template<>
		xgc_bool GetValue<xgc_bool>( xAttrIndex attrIdx, xgc_size nIndex ) const
		{
			XGC_ASSERT_THROW( CheckIndex( attrIdx ), std::logic_error( "get attribute, index out of bound." ) );
			FUNCTION_BEGIN;
			if( mAttributeInfo[attrIdx].pfnGet )
			{
				xgc_bool _Value;
				if( mAttributeInfo[attrIdx].pfnGet(
					GetAttribute( attrIdx ),
					mContext,
					attrIdx,
					(xAttrType) XVariant::Type2Value< xgc_bool >::Value,
					&_Value ) )
				{
					return _Value;
				}
			}

			return GetAttribute( attrIdx, nIndex ).toBool();
			FUNCTION_END;
			return false;
		}

		///
		/// [1/7/2014 albert.xu]
		/// 设置属性值
		///
		template< class T, typename enable_if< is_numeric< T >::value, xgc_bool >::type = true >
		xgc_void SetValue( xAttrIndex attrIdx, T attrValue, xgc_size nIndex = 0 )
		{
			FUNCTION_BEGIN;
			if( mAttributeInfo[attrIdx].pfnSet )
			{
				mAttributeInfo[attrIdx].pfnSet( 
					GetAttribute( attrIdx ), 
					(xObject) mContext, 
					attrIdx, 
					(xAttrType) XVariant::Type2Value< T >::Value, 
					attrValue );
				return;
			}

			GetAttribute( attrIdx, nIndex ) = attrValue;
			FUNCTION_END;
		}

		template<>
		xgc_void SetValue<xgc_bool>( xAttrIndex attrIdx, xgc_bool attrValue, xgc_size nIndex )
		{
			FUNCTION_BEGIN;
			if( mAttributeInfo[attrIdx].pfnSet )
			{
				mAttributeInfo[attrIdx].pfnSet( GetAttribute( attrIdx ),
					mContext, 
					attrIdx, 
					(xAttrType) XVariant::Type2Value< XVariant::Bool >::Value, 
					attrValue );
				return;
			}

			GetAttribute( attrIdx, nIndex ) = attrValue;
			FUNCTION_END;
		}

		///
		/// 增加数值
		/// [3/31/2015] create by albert.xu
		///
		template< class T, typename enable_if< is_numeric< T >::value, xgc_bool >::type = true >
		T IncValue( xAttrIndex attrIdx, T incValue, xgc_size nIndex = 0 )
		{
			XGC_ASSERT_THROW( CheckIndex( attrIdx ), std::logic_error( "inc attribute, index out of bound." ) );
			FUNCTION_BEGIN;
			T _Value = 0;

			if( mAttributeInfo[attrIdx].pfnGet )
			{
				mAttributeInfo[attrIdx].pfnGet(
					GetAttribute( attrIdx ),
					(xObject) mContext,
					attrIdx,
					(xAttrType) XVariant::Type2Value< T >::Value, 
					&_Value );
			}
			else
			{
				_Value = GetAttribute( attrIdx, nIndex ).toNumeric< T >();
			}

			_Value += incValue;

			if( mAttributeInfo[attrIdx].pfnSet )
			{
				mAttributeInfo[attrIdx].pfnSet( GetAttribute( attrIdx ),
					mContext,
					attrIdx,
					(xAttrType) XVariant::Type2Value< T >::Value,
					_Value );
			}
			else
			{
				GetAttribute( attrIdx, nIndex ) = _Value;
			}

			return _Value;
			FUNCTION_END;
			return 0;
		}

		///
		/// 减少数值 有符号
		/// [3/31/2015] create by albert.xu
		///
		template< class T, typename enable_if< is_signed< T >::value, xgc_bool >::type = true >
		T DecValue( xAttrIndex attrIdx, T decValue, xgc_size nIndex = 0 )
		{
			XGC_ASSERT_THROW( CheckIndex( attrIdx ), std::logic_error( "inc attribute, index out of bound." ) );
			FUNCTION_BEGIN;
			T _Value = 0;

			if( mAttributeInfo[attrIdx].pfnGet )
			{
				mAttributeInfo[attrIdx].pfnGet(
					GetAttribute( attrIdx ),
					(xObject) mContext,
					attrIdx,
					(xAttrType) XVariant::Type2Value< T >::Value,
					&_Value );
			}
			else
			{
				_Value = GetAttribute( attrIdx, nIndex ).toNumeric< T >();
			}

			_Value -= decValue;

			if( mAttributeInfo[attrIdx].pfnSet )
			{
				mAttributeInfo[attrIdx].pfnSet( GetAttribute( attrIdx ),
					mContext,
					attrIdx,
					(xAttrType) XVariant::Type2Value< T >::Value,
					_Value );
			}
			else
			{
				GetAttribute( attrIdx, nIndex ) = _Value;
			}

			return _Value;
			FUNCTION_END;
			return 0;
		}

		///
		/// 减少数值 无符号
		/// [3/31/2015] create by albert.xu
		///
		template< class T, typename enable_if< is_unsigned< T >::value, xgc_bool >::type = true >
		T DecValue( xAttrIndex attrIdx, T decValue, xgc_size nIndex = 0 )
		{
			XGC_ASSERT_THROW( CheckIndex( attrIdx ), std::logic_error( "inc attribute, index out of bound." ) );
			FUNCTION_BEGIN;
			T _Value = 0;

			if( mAttributeInfo[attrIdx].pfnGet )
			{
				mAttributeInfo[attrIdx].pfnGet(
					GetAttribute( attrIdx ),
					(xObject) mContext,
					attrIdx,
					(xAttrType) XVariant::Type2Value< T >::Value,
					&_Value );
			}
			else
			{
				_Value = GetAttribute( attrIdx, nIndex ).toNumeric< T >();
			}

			if (_Value > decValue)
			{
				_Value -= decValue;
			}
			else
			{
				_Value = 0;
			}

			if( mAttributeInfo[attrIdx].pfnSet )
			{
				mAttributeInfo[attrIdx].pfnSet( GetAttribute( attrIdx ),
					mContext,
					attrIdx,
					(xAttrType) XVariant::Type2Value< T >::Value,
					_Value );
			}
			else
			{
				GetAttribute( attrIdx, nIndex ) = _Value;
			}

			return _Value;
			FUNCTION_END;
			return 0;
		}

		///
		/// [2/11/2014 albert.xu]
		/// 获取字符串
		///
		xgc_lpcstr GetString( xAttrIndex nAttrIndex, xgc_size nArrayIndex ) const;

		///
		/// [2/11/2014 albert.xu]
		/// 获取缓冲区
		///
		xgc_lpvoid GetBuffer( xAttrIndex attrIdx ) const;

		///
		/// [1/9/2014 albert.xu]
		/// 设置字符串属性
		///
		xgc_bool SetString( xAttrIndex nAttrIndex, xgc_lpcstr attrValue, xgc_size nArrayIndex, xgc_size nLength = -1 );

		///
		/// [1/9/2014 albert.xu]
		/// 设置缓冲区属性
		///
		xgc_bool SetBuffer( xAttrIndex attrIdx, xgc_lpcvoid attrValue, xgc_size nSize );

		///
		/// [3/18/2014 albert.xu]
		/// 设置缓冲区长度
		/// 设置缓冲区长度不影响已有的缓冲区内容。
		///
		xgc_bool SetBufferLength( xAttrIndex attrIdx, xgc_size nSize, xgc_byte szFill = 0 );

		///
		/// [3/17/2014 albert.xu]
		/// 获取缓冲区长度
		///
		xgc_size GetBufferLength( xAttrIndex attrIdx );

	private:
		///
		/// 装载存储的属性
		/// [3/17/2014 albert.xu]
		///
		virtual xgc_bool Load( xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize ) override;

		///
		/// 保存数据到缓冲
		/// [3/17/2014 albert.xu]
		///
		virtual xgc_size Save( xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize ) override;

		///
		/// 释放当前对象
		/// [7/14/2014] create by albert.xu
		///
		virtual xgc_void Release() override;
	};
}
#endif // _XOBJECT_COMPOSION_H_