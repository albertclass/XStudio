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
	typedef xgc::unordered_set< xObject > xObjectSet;
	typedef xgc::list< xObject > xObjectList;
	typedef xgc::vector< xObject > xObjectVec;
	typedef xgc::list< xgc_uint64 > xGlobalIdList;

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
		/// @var 是否处理 
		/// 0 - 只向本对象的观察者传递消息
		/// 1 - 向上传递消息
		/// 2 - 向下传递消息
		/// 3 - 先向上后向下传递消息
		/// < 0 消息已处理
		xgc_long	over;
	};

	///
	/// \brief 删除回调，主要是为了用模板保存类型信息，防止析构函数未能正确调用
	/// \date 12/26/2017
	/// \author xufeng04
	///
	template< class TObjectEvent >
	xgc_void DeleteEvent( XObjectEvent* p )
	{
		delete (TObjectEvent*)p;
	}

	typedef xgc_void( XObject::* XEventBind1 )( xgc_long, xgc_long );
	typedef xgc_void( XObject::* XEventBind2 )( XObjectEvent&, xgc_long, xgc_long );
	typedef xgc_void( XObject::* XEventBind3 )( XObjectEvent*, xgc_void(*)( XObjectEvent* ) );

	typedef std::function< void( XObjectEvent& ) > xNotify;

	///
	/// \brief 游戏对象基类
	/// \author albert.xu
	/// \date 2012/08/11
	///
	class CORE_API XObject : public auto_handle< XObject >
	{
	protected:
		XObject();

		virtual	~XObject();

	public:
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
		/// \brief 是否继承于某个类
		/// \author albert.xu
		/// \date 2014/06/30
		/// \param clsType 类的GUID
		/// \param nStartDeep 测试开始的继承深度
		///
		XGC_INLINE xgc_bool IsInheritFrom( const XClassInfo &cls )const
		{
			return GetRuntimeClass().IsInheritFrom( &cls );
		}

		///
		/// 获取指定继承层次的类类型ID
		/// [6/30/2014] create by albert.xu
		///
		XGC_INLINE xgc_lpcstr GetClassName()const
		{
			return GetRuntimeClass().GetClassName();
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
		///
		struct Observer
		{
			/// 开启状态
			xgc_bool close;
			/// 令牌分配
			xgc_long token;
			/// 重入计数，防止map迭代器失效
			xgc_long count;
			/// 观察者通知接口
			xgc::map< xgc_long, xgc::tuple< xNotify, xObject > > actions;
		};

		/// @var 事件观察者
		xgc::map< xgc_long, Observer > mEventSubject;
		/// @var 是否被销毁
		xgc_bool mIsDestory;
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
		// 销毁对象
		xgc_void	Destroy();

		///
		/// \brief 获取对象的键值 
		/// \date 11/16/2017
		/// \author xufeng04
		/// \return 键值
		///
		virtual xgc_long Key()const { return GetObjectID(); }

		///
		/// \brief 获取子对象数量
		/// \date 12/26/2017
		/// \author xufeng04
		///
		virtual xgc_size GetChildCount()const { return 0; }

		///
		/// \brief 枚举子对象  
		/// \date 12/26/2017
		/// \author xufeng04
		///
		virtual xObject Search( const std::function< xgc_bool( xObject ) > &Filter )const { return INVALID_OBJECT_ID; };

		/************************************************************************/
		/* 事件操作
		/************************************************************************/
		///
		/// \brief 注册事件
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_long	RegistEvent( xgc_long id, const xNotify &invoke, xObject hOwner = INVALID_OBJECT_ID );

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
		/// \brief 屏蔽事件
		/// \author albert.xu
		/// \date 2017/11/03
		///
		xgc_void	EnableEvent( xgc_long id, xgc_bool enable = true );

		///
		/// \brief 分配一个事件对象并初始化
		/// \date 12/26/2017
		/// \author xufeng04
		///
		template< class TObjectEvent >
		TObjectEvent* MakeEvent( xgc_long id, xgc_long direction = 0 )
		{
			TObjectEvent *evt = XGC_NEW TObjectEvent;

			evt->cast.id = id;
			evt->cast.over = direction;
			evt->cast.result = 0;
			evt->cast.sender = GetObjectID();
			evt->cast.target = INVALID_OBJECT_ID;

			return evt;
		}

		template<>
		XObjectEvent* MakeEvent< XObjectEvent >( xgc_long id, xgc_long direction )
		{
			XObjectEvent *evt = XGC_NEW XObjectEvent;

			evt->id     = id;
			evt->over   = direction;
			evt->result = 0;
			evt->sender = GetObjectID();
			evt->target = INVALID_OBJECT_ID;

			return evt;
		}

		///
		/// \brief 初始化一个已分配事件对象
		/// \date 12/26/2017
		/// \author xufeng04
		///
		XObjectEvent* InitEvent( XObjectEvent& evt, xgc_long id, xgc_long direction = 0 )
		{
			evt.id = id;
			evt.over = direction;
			evt.result = 0;
			evt.sender = GetObjectID();
			evt.target = INVALID_OBJECT_ID;

			return &evt;
		}

		///
		/// \brief 提交事件
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_void EmmitEvent( xgc_long id, xgc_long direction = 0 );

		///
		/// \brief 提交事件
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_void EmmitEvent( XObjectEvent& evt, xgc_long id, xgc_long direction = 0 );

		///
		/// \brief 提交事件
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_void EmmitEvent( XObjectEvent* evt, xgc_void( *DeleteIt )( XObjectEvent* ) = xgc_nullptr );

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
		/// [1/7/2014 albert.xu]
		/// 获取有符号数
		///
		template< class T, typename std::enable_if< is_numeric< T >::value, xgc_bool >::type = true >
		XGC_INLINE T getValue( xAttrIndex nAttr ) const
		{
			return getAttr( nAttr ).toNumeric< T >();
		}

		template<>
		XGC_INLINE xgc_bool getValue<xgc_bool>( xAttrIndex nAttr ) const
		{
			return getAttr( nAttr ).toBool();
		}

		///
		/// [1/7/2014 albert.xu]
		/// 设置属性值
		///
		template< class T, typename std::enable_if< is_numeric< T >::value, xgc_bool >::type = true >
		XGC_INLINE xgc_void setValue( xAttrIndex nAttr, T _Val )
		{
			getAttr( nAttr ) = _Val;
			OnValueChanged( nAttr );
		}

		template<>
		XGC_INLINE xgc_void setValue<xgc_bool>( xAttrIndex nAttr, xgc_bool _Val )
		{
			getAttr( nAttr ) = _Val;
			OnValueChanged( nAttr );
		}

		///
		/// [1/7/2014 albert.xu]
		/// 设置属性值
		///
		template< class T, typename std::enable_if< is_numeric< T >::value, xgc_bool >::type = true >
		XGC_INLINE xgc_void incValue( xAttrIndex nAttr, T inc )
		{
			getAttr( nAttr ) += inc;
			OnValueChanged( nAttr );
		}

		///
		/// [2/11/2014 albert.xu]
		/// 获取字符串
		///
		XGC_INLINE xgc_lpcstr getString( xAttrIndex nAttr ) const
		{
			return getAttr( nAttr ).toRawString( xgc_nullptr );
		}

		///
		/// [2/11/2014 albert.xu]
		/// 获取缓冲区
		///
		XGC_INLINE xgc_lpvoid getBuffer( xAttrIndex nAttr ) const
		{
			return getAttr( nAttr ).toRawBuffer();
		}

		///
		/// [1/9/2014 albert.xu]
		/// 设置字符串属性
		///
		XGC_INLINE xgc_void setString( xAttrIndex nAttr, xgc_lpcstr _Val, xgc_size nLength = -1 )
		{
			getAttr( nAttr ).setString( _Val, nLength );
			OnValueChanged( nAttr );
		}

		///
		/// [1/9/2014 albert.xu]
		/// \brief 设置缓冲区属性
		///
		XGC_INLINE xgc_void setBuffer( xAttrIndex nAttr, xgc_lpvoid _Val, xgc_size nSize )
		{
			getAttr( nAttr ).setBuffer( _Val, nSize );
			OnValueChanged( nAttr );
		}

		///
		/// [3/18/2014 albert.xu]
		/// 设置缓冲区长度
		/// 设置缓冲区长度不影响已有的缓冲区内容。
		///
		XGC_INLINE xgc_bool setBufferLength( xAttrIndex nAttr, xgc_size nSize, xgc_byte szFill = 0 )
		{
			return getAttr( nAttr ).setBufferLength( nSize, szFill );
		}

		///
		/// \brief 获取缓冲区长度
		/// \author albert.xu
		/// \date 3/17/2014
		///
		XGC_INLINE xgc_size getBufferLength( xAttrIndex nAttr )
		{
			return getAttr( nAttr ).getBufferLength();
		}

		///
		/// \brief 设置位
		/// \author albert.xu
		/// \date 2017/11/09
		///
		XGC_INLINE xgc_void SetBit( xAttrIndex nAttr, xgc_size _Bit )
		{
			getAttr( nAttr ).SetBit( _Bit );
			OnValueChanged( nAttr );
		}

		///
		/// \brief 清除位
		/// \author albert.xu
		/// \date 2017/11/09
		///
		XGC_INLINE xgc_void ClrBit( xAttrIndex nAttr, xgc_size _Bit )
		{
			getAttr( nAttr ).ClrBit( _Bit );
			OnValueChanged( nAttr );
		}

		///
		/// \brief 获取位
		/// \author albert.xu
		/// \date 2017/11/09
		///
		XGC_INLINE xgc_bool GetBit( xAttrIndex nAttr, xgc_size _Bit, xgc_bool bDefault = false ) const
		{
			return getAttr( nAttr ).GetBit( _Bit, bDefault );
		}

		///
		/// \brief 检查位
		/// \author albert.xu
		/// \date 2017/11/09
		///
		XGC_INLINE xgc_bool ChkBit( xAttrIndex nAttr, xgc_size _Bit, xgc_bool bTest ) const
		{
			return getAttr( nAttr ).CmpBit( _Bit, bTest );
		}

		///
		/// \brief 载入持久化数据 
		/// \date 11/9/2017
		/// \author xufeng04
		/// \param uVersion 数据的版本号
		/// \return 载入是否成功
		///
		virtual xgc_bool LoadObject( xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize );

		///
		/// \brief 保存持久化数据 
		/// \date 11/9/2017
		/// \author xufeng04
		/// \param uVersion 数据的版本号
		/// \return 保存了多少数据
		///
		virtual xgc_size SaveObject( xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize );

	protected:
		///
		/// [2/11/2014 albert.xu]
		/// 获取属性
		/// 该函数提供一个不调用属性设置Hook的方法，其他设置和取值函数皆调用Hook。
		///
		XAttribute getAttr( xAttrIndex nAttr, xgc_size nIndex = 0 ) const;

		///
		/// \brief 属性变化时调用
		/// \author albert.xu
		/// \date 2017/11/09
		///
		xgc_void OnValueChanged( xAttrIndex nAttr, int nIndex = 0 )
		{
			if( mAttributeInfo[nAttr].OnValueChanged )
				( this->*mAttributeInfo[nAttr].OnValueChanged )( nAttr, nIndex );
		}

		///
		/// \brief 销毁对象时调用
		/// \author albert.xu
		/// \date 2017/11/01
		virtual xgc_void OnDestroy() = 0;
	};

	///
	/// \brief 将一个句柄对象转为指针对象，需提供转换的目标类型
	/// \author albert.xu
	/// \date 4/25/2014
	///
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
}

#endif //_XOBJECT_