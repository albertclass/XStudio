#pragma once
#ifndef _XPROPERTY_H_
#define _XPROPERTY_H_

#ifdef CORE_EXPORTS
#	define CORE_API __declspec(dllexport)
#else
#	define CORE_API __declspec(dllimport)
#endif	//CORE_EXPORTS

#define ATTR_FLAG_NONE	0
#define ATTR_FLAG_SAVE	1
#define ATTR_FLAG_FIELD 2
#define ATTR_FLAG_MORE	3

#define APARTMENT_SINGLE_THREAD	XGC::sync_nolock
#define APARTMENT_MULTI_THREAD XGC::sync_section
#define DECLARE_XCLASS()\
public: \
	static	const XClassInfo& GetThisClass(); \
	virtual const XClassInfo& GetRuntimeClass()const; \

#define DECLARE_POOL_OBJECT()\
	static void * operator new( xgc_size s );\
	static void * operator new( xgc_size s, xgc_uint32 block, xgc_lpcstr file, xgc_uint32 line );\
	static void operator delete( void * p );\
	static void operator delete( void * p, xgc_uint32 block, xgc_lpcstr file, xgc_uint32 line );\

#define IMPLEMENT_POOL_OBJECT( thisClass, thisThread )\
	static XGC::memory::freelist< thisClass, XGC::sync_section > thisClass##_Pool;\
	void * thisClass::operator new( xgc_size s ){ return thisClass##_Pool.alloc_memory( _NORMAL_BLOCK, __FILE__, __LINE__ ); }\
	void * thisClass::operator new( xgc_size s, xgc_uint32 block, xgc_lpcstr file, xgc_uint32 line ){ return thisClass##_Pool.alloc_memory( block, file, line ); }\
	void thisClass::operator delete( void * p ){ thisClass##_Pool.garbage_memory( p ); }\
	void thisClass::operator delete( void * p, xgc_uint32 block, xgc_lpcstr file, xgc_uint32 line ){ thisClass##_Pool.garbage_memory( p ); }\

#define BEGIN_IMPLEMENT_XCLASS( thisClass, baseClass, CLS_GUID )\
	static XImplementInfo*	__get_##thisClass##_Attribute(); \
	const XClassInfo& thisClass##::GetThisClass() \
	{ \
		static XClassInfo cls_##thisClass( CLS_GUID, #thisClass, __get_##thisClass##_Attribute(), &baseClass::GetThisClass() ); \
		return cls_##thisClass; \
	} \
	const XClassInfo& cls_##thisClass = thisClass##::GetThisClass(); \
	const XClassInfo& thisClass##::GetRuntimeClass() const \
	{ \
		return thisClass##::GetThisClass(); \
	} \
	static XImplementInfo *	__get_##thisClass##_Attribute( ) \
	{ \
		xClassType clsType = CLS_GUID; \
		thisClass* pClass = xgc_nullptr; \
		static XImplementInfo attribute[] = \
		{ \

// 定义类所持有的属性
#define IMPLEMENT_ATTRIBUTE( NAME, TYPE, FLAGS, VERSION )\
			{ \
				#NAME, \
				FLAGS, \
				TYPE, \
				1, \
				AttributeVersion( VERSION ), \
				xgc_nullptr, \
				xgc_nullptr, \
				&attr##NAME, \
			}, \

#define IMPLEMENT_ATTRIBUTE_ARRAY( NAME, TYPE, COUNT, FLAGS, VERSION )\
			{ \
				#NAME, \
				FLAGS, \
				TYPE, \
				COUNT, \
				AttributeVersion( VERSION ), \
				clsType, \
				xgc_nullptr, \
				xgc_nullptr, \
				&attr##NAME, \
			}, \

#define IMPLEMENT_ATTRIBUTE_EX( NAME, TYPE, FLAGS, VERSION, PFN_ATTR_GET, PFN_ATTR_SET )\
			{ \
			    #NAME, \
			    FLAGS, \
			    TYPE, \
			    1, \
			    AttributeVersion( VERSION ), \
				(PFN_GetAttribute)PFN_ATTR_GET, \
				(PFN_SetAttribute)PFN_ATTR_SET, \
				&attr##NAME, \
			}, \

#define IMPLEMENT_ATTRIBUTE_ARRAY_EX( NAME, TYPE, COUNT, FLAGS, VERSION, PFN_ATTR_GET, PFN_ATTR_SET )\
			{ \
			    #NAME, \
			    FLAGS, \
			    TYPE, \
			    COUNT, \
			    AttributeVersion( VERSION ), \
				(PFN_GetAttribute)PFN_ATTR_GET, \
				(PFN_SetAttribute)PFN_ATTR_SET, \
				&attr##NAME, \
			}, \

#define END_IMPLEMENT_XCLASS() \
			{ xgc_nullptr, 0, VT_VOID, 0, { 0, 0 }, xgc_nullptr, xgc_nullptr, xgc_nullptr }, \
		}; \
		return attribute; \
	} \

#define IMPLEMENT_XCLASS( thisClass, baseClass, CLS_GUID )\
	BEGIN_IMPLEMENT_XCLASS( thisClass, baseClass, CLS_GUID )\
	END_IMPLEMENT_XCLASS()\

namespace XGC
{
	typedef xgc_size xAttrIndex;	/// 属性索引类型
	///
	/// 类类型值
	/// [6/30/2014] create by albert.xu
	///
	typedef xgc_uint32	xClassType;

	/************************************************************************/
	/* 列表系统，属性系统，以及对象基础类的实现								*/
	/************************************************************************/
	typedef XGC::IndexType< xgc_uint64, 0 >::Type xGlobal;
	typedef XGC::IndexType< xgc_uint32, 1 >::Type xObject;
	typedef xgc_lpvoid	xPosition;

	//////////////////////////////////////////////////////////////////////////
	// 简单变体类型类,仅包含了整型和浮点类型
	//////////////////////////////////////////////////////////////////////////
	enum CORE_API xAttrType : xgc_uint8
	{
		VT_VOID = 0,
		VT_BOOL,
		VT_CHAR,
		VT_BYTE,
		VT_I16,
		VT_I32,
		VT_I64,
		VT_U16,
		VT_U32,
		VT_U64,
		VT_REAL,
		VT_REAL64,
		VT_STRING,
		VT_BUFFER,

		VT_TYPE_COUNT,
	};

	/// ------------------------------------------------ //
	/// [3/25/2014 jianglei.kinly]
	/// 通过Name获取Index
	/// ------------------------------------------------ //
	CORE_API xAttrIndex GetAttrIndexByName( xgc_lpcstr lpName );

	class XObject;

	struct CORE_API XAttributeVersion
	{
		xgc_uint32 start;
		xgc_uint32 close;
	};

	class CORE_API XVariantReference;

	typedef xgc_bool( *PFN_GetAttribute )( XVariantReference&, xgc_uintptr, xAttrIndex, xAttrType, ... ); // 获取属性值
	typedef xgc_bool( *PFN_SetAttribute )( XVariantReference&, xgc_uintptr, xAttrIndex, xAttrType, ... ); // 设置属性值

	///
	/// 属性信息
	/// [6/27/2014] create by albert.xu
	///
	struct CORE_API XImplementInfo
	{
		xgc_lpcstr	name;	///< 属性名
		xgc_byte	flags;	///< 属性标志, ATTR_FLAG_SAVE - 保存到数据库
		xAttrType	type;	///< 属性类型
		xgc_size	count;	///< 属性所占内存最大长度(byte), 字符串和缓冲类型有效。
		XAttributeVersion version;

		PFN_GetAttribute pfnGet; ///< 属性获取
		PFN_SetAttribute pfnSet; ///< 属性设置

		xAttrIndex* attr_ptr;
	};

	///
	/// 属性信息
	/// [6/27/2014] create by albert.xu
	///
	struct CORE_API XAttributeInfo
	{
		xgc_lpcstr	name;	///< 属性名
		xgc_byte	flags;	///< 属性标志, ATTR_FLAG_SAVE - 保存到数据库
		xAttrType	type;	///< 属性类型
		xgc_size	count;	///< 属性所占内存最大长度(byte), 字符串和缓冲类型有效。
		xgc_size	offset;	///< 属性偏移量

		PFN_GetAttribute pfnGet; ///< 属性获取
		PFN_SetAttribute pfnSet; ///< 属性设置

		xAttrIndex* attr_ptr;
	};

	///
	/// [1/24/2014 albert.xu]
	/// 获取属性长度
	///
	CORE_API xgc_size AttributeLength( xAttrType eType, xgc_size nCount );

	///
	/// [1/24/2014 albert.xu]
	/// 根据字符串获取版本信息
	///
	CORE_API XAttributeVersion AttributeVersion( xgc_lpcstr lpVersion );

	struct CORE_API XClassInfo
	{
	protected:
		xClassType	mClassType[16];		///< 对象继承类型
		xgc_lpcstr	mClassName;			///< 类名
		xgc_size	mInheritLevel;		///< 继承深度

		xgc_size	mImplementCount;	///< 配置个数
		xgc_size	mAttributeCount;	///< 属性个数
		xgc_size	mAttributeSize;		///< 属性所占空间大小

		XImplementInfo * mImplementInfo;	///< 属性定义描述指针
		XAttributeInfo * mAttributeInfo;	///< 属性信息描述指针

		const XClassInfo *mParent;			///< 从哪儿继承的
	public:
		///
		/// 构造函数
		/// [1/6/2014 albert.xu]
		///
		XClassInfo( xClassType nClassType, xgc_lpcstr lpClassName, XImplementInfo *pAttributeConf, const XClassInfo *pParent = xgc_nullptr );

		///
		/// 析构函数
		/// [1/6/2014 albert.xu]
		///
		~XClassInfo();

		///
		/// 获取类属性
		/// [2/20/2014 albert.xu]
		///
		XGC_INLINE xClassType GetClassType()const
		{
			return mClassType[mInheritLevel-1];
		}

		///
		/// 获取类名
		/// [11/13/2014] create by albert.xu
		///
		XGC_INLINE xgc_lpcstr GetClassName()const
		{
			return mClassName;
		}

		///
		/// 获取指定继承深度的基类类型
		/// [6/30/2014] create by albert.xu
		///
		XGC_INLINE xClassType GetBaseType( xgc_size nInheritDeep )const
		{
			XGC_ASSERT( nInheritDeep < mInheritLevel );
			return mClassType[nInheritDeep];
		} //-V591

		///
		/// 运行时类型检查, 是否是该类型或该类型的派生类。
		/// [1/7/2014 albert.xu]
		///
		XGC_INLINE xgc_bool IsInheritFrom( xClassType clsType )const
		{
			// 场景物品继承自gameobject 继承深度会有assert
			if( mInheritLevel == 0U || xgc_size( ( (xgc_byteptr) &clsType )[3] & 0x0f ) >= mInheritLevel )
				return false;

			if( mClassType[( (xgc_byteptr) &clsType )[3] & 0x0f ] == clsType )
				return true;

			return false;
		}

		///
		/// 检查索引是否合法
		/// [1/3/2014 albert.xu]
		///
		XGC_INLINE xgc_bool CheckIndex( xgc_size nIndex )const
		{
			return nIndex < mAttributeCount;
		}

		XGC_INLINE xAttrType GetAttributeType( xgc_size nIndex )const
		{
			XGC_ASSERT_RETURN( CheckIndex( nIndex ), VT_VOID );
			return mAttributeInfo[nIndex].type;
		}

		XGC_INLINE const XAttributeInfo* GetAttributeInfo( ) const
		{
			return mAttributeInfo;
		}

		XGC_INLINE const XImplementInfo* GetImplementInfo() const
		{
			return mImplementInfo;
		}

		XGC_INLINE xgc_size GetAttributeCount( )const
		{
			return mAttributeCount;
		}

		XGC_INLINE xgc_size GetImplementCount()const
		{
			return mImplementCount;
		}

		XGC_INLINE xgc_size GetAttributeSize()const
		{
			return mAttributeSize;
		}
	};
}
#endif // _XPROPERTY_H_