#ifndef _XOBJECT_
#define _XOBJECT_

#include "XAttribute.h"
#include "XClassInfo.h"

#define INVALID_OBJECT_ID	xObject(-1)
#define INVALID_GLOBAL_ID	xGlobal(-1)
#define INVALID_ATTRIBID	(-1)
#define INVALID_TYPE_VALUE	(-1)

/************************************************************************/
namespace xgc
{
	using namespace common;
	typedef xgc_unordered_set< xObject > xObjectSet;
	typedef xgc_list< xObject > xObjectList;
	typedef xgc_vector< xObject > xObjectVec;
	typedef xgc_list< xgc_uint64 > xGlobalIdList;

	///
	/// \brief 对象事件基类
	/// \author albert.xu
	/// \date 2017/10/12
	///
	struct CORE_API XObjectEvent
	{
		/// @var 事件ID
		xgc_long	id;
		/// @var 事件的发送者
		xObject		sender;
		/// @var 事件的接受者
		xObject		target;
		/// @var 事件令牌
		xgc_long	token;
		/// @var 返回值 -1 - err, 0 - pending, 1 - ok
		xgc_long	result;
	};

	typedef xgc_void( XObject::* XEventBind1 )( XObjectEvent& );
	typedef xgc_void( XObject::* XEventBind2 )( xgc_long, XObjectEvent& );

	typedef std::function< void( XObjectEvent& ) > xAction;

	///
	/// \brief 游戏对象基类
	/// \author albert.xu
	/// \date 2012/08/11
	///
	class CORE_API XObject : public auto_handle< XObject >
	{
	protected:
		XObject();

	public:
		virtual	~XObject();

		///
		/// \brief 获取类信息
		/// \author albert.xu
		/// \date 2017/10/09
		///
		static	const XClassInfo& GetThisClass();

		///
		/// \brief 获取类运行时信息
		/// \author albert.xu
		/// \date 2017/10/09
		///
		virtual const XClassInfo& GetRuntimeClass()const;

		///
		/// \brief 对象初始化
		/// \author albert.xu
		/// \date 2017/10/09
		///
		bool InitObject();

		///
		/// \brief 是否继承于某个类
		/// \author albert.xu
		/// \date 2014/06/30
		/// \param clsType 类的GUID
		/// \param nStartDeep 测试开始的继承深度
		///
		XGC_INLINE xgc_bool IsInheritFrom( const XClassInfo *cls )const
		{
			return GetRuntimeClass().IsInheritFrom( cls );
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

		/// @var 对象属性首地址
		xgc_lpvoid mAttributes = xgc_nullptr;
		/// @var 对象属性信息地址
		const XAttributeInfo * mAttributeInfo = xgc_nullptr;
		/// @var 对象属性定义地址
		const XAttributeImpl * mImplementInfo = xgc_nullptr;

		///
		/// \brief 观察者信息
		struct Observer
		{
			xgc_long token;
			xgc_long count;
			xgc_map< xgc_long, xgc_tuple< xAction, xObject > > actions;
		};

		/// @var 事件观察者
		xgc_map< xgc_long, Observer > mEventSubject;

		/// @var 是否被销毁
		xgc_bool mIsDestory;
		/// @var 子物体列表
		xObjectList	mChildList;
		/// @var 父对象ID
		xObject	mParentID;
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
		// 销毁对象
		xgc_void	Destroy();
		// 销毁所有子对象
		xgc_void	DestroyAllChild();
		// 获取子对象数量
		xgc_size	GetChildrenCount()const { return mChildList.size(); }

		/************************************************************************/
		/* 事件操作
		/************************************************************************/
		///
		/// \brief 注册事件
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_long	RegistEvent( xgc_long id, const xAction &invoke, xObject hOwner = INVALID_OBJECT_ID );

		///
		/// \brief 删除事件
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_void	RemoveEvent( xgc_long id, xgc_long token );

		///
		/// \brief 删除事件
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_void	RemoveEvent( xObject hOwner );

		///
		/// \brief 提交事件
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_void	EmmitEvent( xgc_long id, XObjectEvent& evt = XObjectEvent() );

		///
		/// \brief 提交事件
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_void	EmmitEvent( XObjectEvent& evt );

		/************************************************************************/
		/* 属性操作
		/************************************************************************/

		///
		/// \brief 获取属性数量
		/// \author albert.xu
		/// \date 2017/10/09
		///
		XGC_INLINE xgc_size getAttrCount()const
		{
			return GetRuntimeClass().GetAttributeCount();
		}

		///
		/// \brief 检查索引是否合法
		/// \author albert.xu
		/// \date 2017/10/09
		///
		XGC_INLINE xgc_bool checkIndex( xAttrIndex idx )const
		{
			return idx < getAttrCount();
		}

		///
		/// \brief 获取属性类型
		/// \author albert.xu
		/// \date 2017/10/09
		///
		XGC_INLINE xAttrType getAttrType( xAttrIndex idx )const
		{
			return GetRuntimeClass().GetAttributeType( idx );
		}

		///
		/// \brief 属性是否数组类型
		/// \author albert.xu
		/// \date 2017/10/09
		///
		XGC_INLINE xgc_bool isArray( xAttrIndex idx )const
		{
			return GetRuntimeClass().IsArrayType( idx );
		}

		///
		/// \brief 获取属性数组长度
		/// \author albert.xu
		/// \date 2017/10/09
		///
		XGC_INLINE xgc_size getAttrArrayLength( xAttrIndex idx )const
		{
			return GetRuntimeClass().GetArrayLength( idx );
		}

		///
		/// [2/11/2014 albert.xu]
		/// 获取属性
		/// 该函数提供一个不调用属性设置Hook的方法，其他设置和取值函数皆调用Hook。
		///
		XGC_INLINE XAttribute getAttr( xAttrIndex idx, xgc_size nIndex = 0 ) const
		{
			XGC_ASSERT_THROW( checkIndex( idx ), std::logic_error( "get attribute, index out of bound." ) );
			xgc_lpstr lpValue = (xgc_lpstr)mAttributes + mAttributeInfo[idx].offset;

			// 生成属性操作对象
			return XAttribute( mAttributeInfo[idx].impl.type,
				lpValue + nIndex * XAttribute::Type2Size( mAttributeInfo[idx].impl.type ) );
		}

		///
		/// [1/7/2014 albert.xu]
		/// 获取有符号数
		///
		template< class T, typename std::enable_if< is_numeric< T >::value, xgc_bool >::type = true >
		XGC_INLINE T getValue( xAttrIndex idx ) const
		{
			return getAttr( idx ).toNumeric< T >();
		}

		template<>
		xgc_bool getValue<xgc_bool>( xAttrIndex idx ) const
		{
			return getAttr( idx ).toBool();
		}

		///
		/// [1/7/2014 albert.xu]
		/// 设置属性值
		///
		template< class T, typename std::enable_if< is_numeric< T >::value, xgc_bool >::type = true >
		XGC_INLINE xgc_void setValue( xAttrIndex idx, T attrValue )
		{
			getAttr( idx ) = attrValue;
		}

		template<>
		XGC_INLINE xgc_void setValue<xgc_bool>( xAttrIndex idx, xgc_bool attrValue )
		{
			getAttr( idx ) = attrValue;
		}

		///
		/// [2/11/2014 albert.xu]
		/// 获取字符串
		///
		XGC_INLINE xgc_lpcstr getString( xAttrIndex idx ) const
		{
			return getAttr( idx ).toRawString( xgc_nullptr );
		}

		///
		/// [2/11/2014 albert.xu]
		/// 获取缓冲区
		///
		XGC_INLINE xgc_lpvoid getBuffer( xAttrIndex idx ) const
		{
			return getAttr( idx ).toRawBuffer();
		}

		///
		/// [1/9/2014 albert.xu]
		/// 设置字符串属性
		///
		XGC_INLINE xgc_void setString( xAttrIndex idx, xgc_lpcstr _Val, xgc_size nLength = -1 )
		{
			getAttr( idx ).setString( _Val, nLength );
		}

		///
		/// [1/9/2014 albert.xu]
		/// 设置缓冲区属性
		///
		XGC_INLINE xgc_void setBuffer( xAttrIndex idx, xgc_lpvoid _Val, xgc_size nSize )
		{
			getAttr( idx ).setBuffer( _Val, nSize );
		}

		///
		/// [3/18/2014 albert.xu]
		/// 设置缓冲区长度
		/// 设置缓冲区长度不影响已有的缓冲区内容。
		///
		XGC_INLINE xgc_bool setBufferLength( xAttrIndex idx, xgc_size nSize, xgc_byte szFill = 0 )
		{
			return getAttr( idx ).setBufferLength( nSize, szFill );
		}

		///
		/// [3/17/2014 albert.xu]
		/// 获取缓冲区长度
		///
		XGC_INLINE xgc_size getBufferLength( xAttrIndex idx )
		{
			return getAttr( idx ).getBufferLength();
		}
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

	//-----------------------------------//
	// [4/25/2014 albert.xu]
	// 将一个句柄对象转为指针对象
	// 需提供转换的目标类型
	//-----------------------------------//
	template< class T, typename std::enable_if< std::is_base_of< XObject, T >::value && std::is_base_of< std::enable_shared_from_this< T >, T >::value == false, xgc_bool >::type = true >
	T* ObjectCast( typename xObject hObject )
	{
		XObject* pObject = XObject::handle_exchange( hObject );
		if( !pObject )
			return xgc_nullptr;

		if( !pObject->IsInheritFrom( &T::GetThisClass() ) )
			return xgc_nullptr;

		return static_cast< T* >( pObject );
	}

	template< class T, typename std::enable_if< std::is_base_of< XObject, T >::value && std::is_base_of< std::enable_shared_from_this< T >, T >::value == true, xgc_bool >::type = true >
	std::shared_ptr< T > ObjectCast( typename xObject hObject )
	{
		T* Pointer = static_cast<T*>( GetXObject( hObject, &T::GetThisClass() ) );

		return Pointer ? Pointer->shared_from_this() : xgc_nullptr;
	}

	///
	/// \brief 加载对象属性
	/// \author albert.xu
	/// \date 2017/10/10
	///
	xgc_bool LoadObject( XObject* pObj, xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize );

	///
	/// \brief 保存对象属性
	/// \author albert.xu
	/// \date 2017/10/10
	///
	xgc_size SaveObject( XObject* pObj, xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize );
}

_STD_BEGIN;
template<>
struct default_delete< xgc::XObject >
{	// default deleter for unique_ptr
	typedef default_delete<xgc::XObject> _Myt;

	default_delete() _NOEXCEPT
	{	// default construct
	}

	template<class _Ty2,
	class = typename enable_if<is_convertible<_Ty2 *, XGC::XObject *>::value,
		void>::type>
		default_delete( const default_delete<_Ty2>& ) _NOEXCEPT
	{	// construct from another default_delete
	}

	void operator()( xgc::XObject *_Ptr ) const _NOEXCEPT
	{	// delete a pointer
		static_assert( 0 < sizeof ( xgc::XObject ), "can't delete an incomplete type" );
		_Ptr->Destroy();
		delete _Ptr;
	}
};
_STD_END;

#endif //_XOBJECT_