#ifndef _XOBJECT_
#define _XOBJECT_
// http://69.197.40.84/
#define EXPORT	__declspec(dllexport)

#ifdef CORE_EXPORTS
#	define CORE_API __declspec(dllexport)
#else
#	define CORE_API __declspec(dllimport)
#endif	//CORE_EXPORTS

#include "auto_handle.h"
#include "XVariant.h"
#include "XClassInfo.h"
#include "XObjectComposition.h"

#define INVALID_OBJECT_ID	(-1)
#define INVALID_GLOBAL_ID	(-1)
#define INVALID_ATTRIBID	(-1)
#define INVALID_TYPE_VALUE	(-1)

/************************************************************************/
namespace XGC
{
	using namespace common;
	typedef xgc_unordered_set< xObject > xObjectSet;
	typedef xgc_list< xObject > xObjectList;
	typedef xgc_vector< xObject > xObjectVec;
	typedef xgc_list<xgc_uint64> xGlobalIdList;

	struct XObjectComposition;
	class CORE_API XObject : public auto_handle< XObject >
	{
	protected:
		XObject();

	public:
		virtual	~XObject();

		static	const XClassInfo& GetThisClass();
		virtual const XClassInfo& GetRuntimeClass()const;

		///
		/// 是否继承于某个类
		/// [6/30/2014] create by albert.xu
		/// @param clsType 类的GUID
		/// @param nStartDeep 测试开始的继承深度
		///
		XGC_INLINE xgc_bool IsInheritFrom( xClassType clsType )const
		{
			return GetRuntimeClass().IsInheritFrom( clsType );
		}

		///
		/// 获取指定继承层次的类类型ID
		/// [6/30/2014] create by albert.xu
		///
		XGC_INLINE xClassType GetBaseType( xgc_size nInheritDeep )const
		{
			return GetRuntimeClass().GetBaseType( nInheritDeep );
		}

		///
		/// 获取指定继承层次的类类型ID
		/// [6/30/2014] create by albert.xu
		///
		XGC_INLINE xClassType GetClassType()const
		{
			return  GetRuntimeClass().GetClassType();
		}

		///
		/// 获取指定继承层次的类类型ID
		/// [6/30/2014] create by albert.xu
		///
		XGC_INLINE xgc_lpcstr GetClassName()const
		{
			return  GetRuntimeClass().GetClassName();
		}

	private:
		XObject( const XObject& rsh );
		xgc_bool mIsDestory; ///< 是否被销毁
		xObjectList	mChildList;	///< 子物体列表

		xObject		mParentID;	///< 父物体ID

		XObjectComposition *mComposition[4]; ///< 组件系统
	public:
		/************************************************************************/
		/* 层级关系函数
		/************************************************************************/
		xObject		GetObjectID()const { return handle()._handle; }
		// 设置父对象
		xgc_void	SetParent( xObject nID ) { mParentID = nID; }
		// 得到父对象ID
		xObject		GetParent()const { return mParentID; }
		// 添加子对象
		xgc_bool	AddChild( xObject  nObj, xgc_lpcvoid lpContext = 0 );
		xgc_bool	AddChild( XObject* pObj, xgc_lpcvoid lpContext = 0 );
		// 删除子对象
		xgc_void	RemoveChild( xObject  nObj, xgc_bool bDestroy = false );
		xgc_void	RemoveChild( XObject* pObj, xgc_bool bDestroy = false );
		// 查询子对象
		xgc_bool	QueryChild( XObject* pObj )const;
		xgc_bool	QueryChild( xObject  nObj )const;
		xgc_bool	QueryChild( const std::function< xgc_bool( xObject ) >& fnFilter )const;
		/************************************************************************/
		// object composition operator

		///
		/// 添加自定义组件
		/// [7/14/2014] create by albert.xu
		///
		xgc_bool SetComposition( xgc_uint16 eType, XObjectComposition* pComp );

		///
		/// 获取组件
		/// [1/9/2014 albert.xu]
		///
		XObjectComposition* GetComposition( xgc_uint16 eType )const;

		///
		/// 获取静态属性
		/// [1/10/2014 albert.xu]
		///
		XGC_INLINE XVariantReference GetAttribute( xAttrIndex nAttrIndex, xgc_size nArrayIndex = 0 )const;

		///
		/// 获取属性的类型
		/// [1/12/2015] create by albert.xu
		///
		XGC_INLINE xAttrType GetAttrType( xAttrIndex nAttrIndex )const;

		///
		/// 获取字符串属性
		/// [3/3/2014 albert.xu]
		///
		XGC_INLINE xgc_lpcstr GetStrAttr( xAttrIndex nAttrIndex, xgc_size nArrayIndex = 0 )const;

		///
		/// 设置字符串属性
		/// [3/3/2014 albert.xu]
		///
		XGC_INLINE xgc_void SetStrAttr( xAttrIndex nAttrIndex, xgc_lpcstr pszValue, xgc_size nArrayIndex = 0 );

		///
		/// [1/7/2014 albert.xu]
		/// 获取有符号数
		///
		template< class T >
		XGC_INLINE T GetAttrValue( xAttrIndex nAttrIndex, xgc_size nArrayIndex = 0 ) const
		{
			XObjectAttribute* pAttr = (XObjectAttribute*) GetComposition( XCompAttribute );
			XGC_ASSERT_THROW( pAttr, std::logic_error( "属性组件未创建" ) );
			return pAttr->GetValue< T >( nAttrIndex, nArrayIndex );
		}

		///
		/// [1/7/2014 albert.xu]
		/// 设置属性值
		///
		template< class T >
		XGC_INLINE xgc_void SetAttrValue( xAttrIndex nAttrIndex, T attrValue, xgc_size nArrayIndex = 0 )
		{
			XObjectAttribute* pAttr = (XObjectAttribute*) GetComposition( XCompAttribute );
			XGC_ASSERT_THROW( pAttr, std::logic_error( "属性组件未创建" ) );
			pAttr->SetValue< T >( nAttrIndex, attrValue, nArrayIndex );
		}

		///
		/// [1/7/2014 albert.xu]
		/// Add属性值
		///
		template< class T >
		XGC_INLINE T IncAttrValue( xAttrIndex nAttrIndex, T attrValue, xgc_size nArrayIndex = 0 )
		{
			XObjectAttribute* pAttr = (XObjectAttribute*) GetComposition( XCompAttribute );
			XGC_ASSERT_THROW( pAttr, std::logic_error( "属性组件未创建" ) );
			return pAttr->IncValue< T >( nAttrIndex, attrValue, nArrayIndex );
		}

		///
		/// [1/7/2014 albert.xu]
		/// Sub属性值
		///
		template< class T >
		XGC_INLINE T DecAttrValue( xAttrIndex nAttrIndex, T attrValue, xgc_size nArrayIndex = 0 )
		{
			XObjectAttribute* pAttr = (XObjectAttribute*) GetComposition( XCompAttribute );
			XGC_ASSERT_THROW( pAttr, std::logic_error( "属性组件未创建" ) );
			return pAttr->DecValue< T >( nAttrIndex, attrValue, nArrayIndex );
		}

		///
		/// 获取数值类型属性
		/// [3/3/2014 albert.xu]
		///
		XGC_INLINE XVariant::Integer GetIntegerAttr( xAttrIndex nAttrIndex )const;

		///
		/// 设置整型属性
		/// [3/3/2014 albert.xu]
		///
		XGC_INLINE xgc_void SetIntegerAttr( xAttrIndex xIndex, XVariant::Integer _Val );

		///
		/// 获取数值类型属性
		/// [3/3/2014 albert.xu]
		///
		XGC_INLINE XVariant::Unsigned GetUnsignedAttr( xAttrIndex xIndex )const;

		///
		/// 设置整型属性
		/// [3/3/2014 albert.xu]
		///
		XGC_INLINE xgc_void SetUnsignedAttr( xAttrIndex xIndex, XVariant::Unsigned _Val );

		///
		/// 获取数值类型属性
		/// [3/3/2014 albert.xu]
		///
		XGC_INLINE XVariant::Real GetRealAttr( xAttrIndex xIndex )const;

		///
		/// 设置整型属性
		/// [3/3/2014 albert.xu]
		///
		XGC_INLINE xgc_void SetRealAttr( xAttrIndex xIndex, XVariant::Real _Val );

		///
		/// 获取Bool类型属性
		/// [3/3/2014 albert.xu]
		///
		XGC_INLINE XVariant::Bool GetBoolAttr( xAttrIndex xIndex )const;

		///
		/// 设置bool属性
		/// [3/3/2014 albert.xu]
		///
		XGC_INLINE xgc_void SetBoolAttr( xAttrIndex xIndex, XVariant::Bool _Val );

		///
		/// 获取缓冲区属性
		/// [3/17/2014 albert.xu]
		///
		XGC_INLINE xgc_lpcvoid GetBufferAttr( xAttrIndex xIndex )const;

		///
		/// 设置缓冲区属性
		/// [3/17/2014 albert.xu]
		///
		XGC_INLINE xgc_void SetBufferAttr( xAttrIndex xIndex, xgc_lpcvoid pData, xgc_size nSize );

		///
		/// 获取缓冲区属性长度
		/// [3/17/2014 albert.xu]
		///
		XGC_INLINE xgc_size GetBufferAttr_Length( xAttrIndex xIndex )const;

		///
		/// 设置缓冲区长度
		/// [3/18/2014 albert.xu]
		///
		XGC_INLINE xgc_bool SetBufferAttr_Length( xAttrIndex, xgc_size nSize, xgc_byte szFill = 0 );

		/************************************************************************/
		// 销毁对象
		xgc_void	Destroy();
		// 销毁所有子对象
		xgc_void	DestroyAllChild();
		// 获取子对象数量
		xgc_size	GetChildrenCount()const { return mChildList.size(); }

	protected:
		/************************************************************************/
		/* 设置的一些事件响应虚函数。
		/************************************************************************/
		/////
		/// 增加子节点前调用
		/// [8/3/2009 Albert]
		/// @return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		/////
		virtual xgc_bool PreAddChild( XObject* pChild, xgc_lpcvoid lpContext ) = 0;

		/////
		/// 加入子节点后调用
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext ) = 0;

		/////
		/// 增加子节点前调用
		/// [8/3/2009 Albert]
		/// @return true - 确认移除子节点, false - 子节点被否决,移除子节点失败.
		/////
		virtual xgc_bool PreRemoveChild( XObject* pChild, xgc_bool bRelease ) = 0;

		/////
		/// 删除子节点后调用,此时对象尚未被删除
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void OnRemoveChild( XObject* pChild, xgc_bool bRelease ) = 0;

		///
		/// 销毁对象的时候调用
		///
		virtual xgc_void OnDestroy() = 0;
	};

	XGC_INLINE XVariant::Integer XObject::GetIntegerAttr( xAttrIndex xIndex ) const
	{
		return GetAttrValue< XVariant::Integer >( xIndex, 0 );
	}

	XGC_INLINE xgc_void XObject::SetIntegerAttr( xAttrIndex xIndex, XVariant::Integer _Val )
	{
		SetAttrValue( xIndex, _Val, 0 );
	}

	XGC_INLINE XVariant::Unsigned XObject::GetUnsignedAttr( xAttrIndex xIndex ) const
	{
		return GetAttrValue< XVariant::Unsigned >( xIndex, 0 );
	}

	XGC_INLINE xgc_void XObject::SetUnsignedAttr( xAttrIndex xIndex, XVariant::Unsigned _Val )
	{
		SetAttrValue( xIndex, _Val, 0 );
	}

	XGC_INLINE XVariant::Real XObject::GetRealAttr( xAttrIndex xIndex ) const
	{
		return GetAttrValue< XVariant::Real >( xIndex, 0 );
	}

	XGC_INLINE xgc_void XObject::SetRealAttr( xAttrIndex xIndex, XVariant::Real _Val )
	{
		SetAttrValue( xIndex, _Val, 0 );
	}
	
	XGC_INLINE XVariant::Bool XObject::GetBoolAttr( xAttrIndex xIndex ) const
	{
		return GetAttrValue< XVariant::Bool >( xIndex, 0 );
	}

	XGC_INLINE xgc_void XObject::SetBoolAttr( xAttrIndex xIndex, XVariant::Bool _Val )
	{
		SetAttrValue( xIndex, _Val, 0 );
	}

	XGC_INLINE XVariantReference XObject::GetAttribute( xAttrIndex nAttrIndex, xgc_size nArrayIndex ) const
	{
		XObjectAttribute* pAttr = (XObjectAttribute*) GetComposition( XCompAttribute );
		XGC_ASSERT_THROW( pAttr, std::logic_error( "属性组件未创建" ) );
		return pAttr->GetAttribute( nAttrIndex, nArrayIndex );
	}

	///
	/// 获取属性的类型
	/// [1/12/2015] create by albert.xu
	///
	XGC_INLINE xAttrType XObject::GetAttrType( xAttrIndex nAttrIndex )const
	{
		XObjectAttribute* pAttr = (XObjectAttribute*) GetComposition( XCompAttribute );
		XGC_ASSERT_THROW( pAttr, std::logic_error( "属性组件未创建" ) );
		return pAttr->GetAttributeType( nAttrIndex );
	}

	XGC_INLINE xgc_lpcstr XObject::GetStrAttr( xAttrIndex nAttrIndex, xgc_size nArrayIndex ) const
	{
		XObjectAttribute* pAttr = (XObjectAttribute*) GetComposition( XCompAttribute );
		XGC_ASSERT_THROW( pAttr, std::logic_error( "属性组件未创建" ) );
		return pAttr->GetString( nAttrIndex, nArrayIndex );
	}

	XGC_INLINE xgc_void XObject::SetStrAttr( xAttrIndex nAttrIndex, xgc_lpcstr pszValue, xgc_size nArrayIndex )
	{
		XObjectAttribute* pAttr = (XObjectAttribute*) GetComposition( XCompAttribute );
		XGC_ASSERT_THROW( pAttr, std::logic_error( "属性组件未创建" ) );
		pAttr->SetString( nAttrIndex, pszValue, nArrayIndex, -1 );
	}

	XGC_INLINE xgc_lpcvoid XObject::GetBufferAttr( xAttrIndex xIndex ) const
	{
		XObjectAttribute* pAttr = (XObjectAttribute*) GetComposition( XCompAttribute );
		XGC_ASSERT_THROW( pAttr, std::logic_error( "属性组件未创建" ) );
		return pAttr->GetBuffer( xIndex );
	}

	XGC_INLINE xgc_void XObject::SetBufferAttr( xAttrIndex xIndex, xgc_lpcvoid pData, xgc_size nSize )
	{
		XObjectAttribute* pAttr = (XObjectAttribute*) GetComposition( XCompAttribute );
		XGC_ASSERT_THROW( pAttr, std::logic_error( "属性组件未创建" ) );
		pAttr->SetBuffer( xIndex, pData, nSize );
	}

	XGC_INLINE xgc_size XObject::GetBufferAttr_Length( xAttrIndex xIndex ) const
	{
		XObjectAttribute* pAttr = (XObjectAttribute*) GetComposition( XCompAttribute );
		XGC_ASSERT_THROW( pAttr, std::logic_error( "属性组件未创建" ) );
		return pAttr->GetBufferLength( xIndex );
	}

	XGC_INLINE xgc_bool XObject::SetBufferAttr_Length( xAttrIndex xIndex, xgc_size nSize, xgc_byte szFill /*= 0*/ )
	{
		XObjectAttribute* pAttr = (XObjectAttribute*) GetComposition( XCompAttribute );
		XGC_ASSERT_THROW( pAttr, std::logic_error( "属性组件未创建" ) );
		return pAttr->SetBufferLength( xIndex, nSize, szFill );
	}
	//-----------------------------------//
	// [4/25/2014 albert.xu]
	// 将一个句柄对象转为指针对象
	// 需提供转换的目标类型
	//-----------------------------------//
	template< class T, typename std::enable_if< std::is_base_of< XObject, T >::value && std::is_base_of< std::enable_shared_from_this< T >, T >::value == false, xgc_bool >::type = true >
	T* ObjectCast( typename xObject hObject )
	{
		return static_cast<T*>( GetXObject( hObject, T::GetThisClass().GetClassType() ) );
	}

	template< class T, typename std::enable_if< std::is_base_of< XObject, T >::value && std::is_base_of< std::enable_shared_from_this< T >, T >::value == true, xgc_bool >::type = true >
	std::shared_ptr< T > ObjectCast( typename xObject hObject )
	{
		T* Pointer = static_cast<T*>( GetXObject( hObject, T::GetThisClass().GetClassType() ) );
		if( Pointer )
			return Pointer->shared_from_this();

		return std::shared_ptr< T >( xgc_nullptr );
	}
}

_STD_BEGIN;
template<>
struct default_delete< XGC::XObject >
{	// default deleter for unique_ptr
	typedef default_delete<XGC::XObject> _Myt;

	default_delete() _NOEXCEPT
	{	// default construct
	}

	template<class _Ty2,
	class = typename enable_if<is_convertible<_Ty2 *, XGC::XObject *>::value,
		void>::type>
		default_delete( const default_delete<_Ty2>& ) _NOEXCEPT
	{	// construct from another default_delete
	}

	void operator()( XGC::XObject *_Ptr ) const _NOEXCEPT
	{	// delete a pointer
		static_assert( 0 < sizeof ( XGC::XObject ), "can't delete an incomplete type" );
		_Ptr->Destroy();
		delete _Ptr;
	}
};
_STD_END;

#endif //_XOBJECT_