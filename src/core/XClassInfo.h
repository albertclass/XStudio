#pragma once
#ifndef __XCLASS_H__
#define __XCLASS_H__

#include "XAttribute.h"

#define ATTR_FLAG_NONE	0
#define ATTR_FLAG_SAVE	1
#define ATTR_FLAG_FIELD 2
#define ATTR_FLAG_MORE	3

#define DECLARE_XCLASS() \
public: \
	static	const XClassInfo& GetThisClass(); \
	virtual const XClassInfo& GetRuntimeClass()const; \

#define DECLARE_POOL_OBJECT() \
	static void * operator new( xgc_size s ); \
	static void * operator new( xgc_size s, xgc_uint32 block, xgc_lpcstr file, xgc_uint32 line ); \
	static void operator delete( void * p ); \
	static void operator delete( void * p, xgc_uint32 block, xgc_lpcstr file, xgc_uint32 line ); \

#define IMPLEMENT_POOL_OBJECT( thisClass, thisThread ) \
	static XGC::memory::freelist< thisClass, std::mutex > thisClass##_Pool; \
	void * thisClass::operator new( xgc_size s ){ return thisClass##_Pool.alloc_memory( _NORMAL_BLOCK, __FILE__, __LINE__ ); } \
	void * thisClass::operator new( xgc_size s, xgc_uint32 block, xgc_lpcstr file, xgc_uint32 line ){ return thisClass##_Pool.alloc_memory( block, file, line ); } \
	void thisClass::operator delete( void * p ){ thisClass##_Pool.garbage_memory( p ); } \
	void thisClass::operator delete( void * p, xgc_uint32 block, xgc_lpcstr file, xgc_uint32 line ){ thisClass##_Pool.garbage_memory( p ); } \

#define IMPLEMENT_XCLASS_BEGIN( thisClass, baseClass ) \
	template< class T > \
	static const XClassInfo& __get_##thisClass(); \
	static const XClassInfo& __loc_##thisClass = __get_##thisClass<thisClass>(); \
	const XClassInfo& thisClass##::GetRuntimeClass() const \
	{ \
		return __loc_##thisClass; \
	} \
	const XClassInfo& thisClass##::GetThisClass() \
	{ \
		return __loc_##thisClass; \
	} \
	template< class T > \
	static const XClassInfo& __get_##thisClass() \
	{ \
		auto thisClassName = #thisClass; \
		auto baseClassInfo = &baseClass::GetThisClass(); \
		static XAttributeImpl __attribute[] = { \

#define IMPLEMENT_ATTRIBUTE( NAME, TYPE, FLAGS, VERSION )\
			{ { TYPE, #NAME, FLAGS, 1, &T::##NAME, }, ConvertVersion( VERSION ) }, \

#define IMPLEMENT_ATTRIBUTE_ARRAY( NAME, TYPE, COUNT, FLAGS, VERSION )\
			{ { TYPE, #NAME, FLAGS | 0x80, COUNT, &T::##NAME, }, ConvertVersion( VERSION ) }, \

#define IMPLEMENT_XCLASS_END() \
			{ { VT_VOID, xgc_nullptr, 0, 0, xgc_nullptr }, { 0, 0 } }, \
		}; \
		static XClassInfo __cls__( thisClassName, __attribute, baseClassInfo ); \
		return __cls__; \
	} \

#define REGIST_ATTR_LISTENER( cls, attr, memberFunc )\
	static xgc_bool XGC_LINEID( cls ) = cls##::GetThisClass().SetAttrListener( cls##::attr, &##cls##::##memberFunc )\

namespace xgc
{
	class XObject;

	/// 属性索引类型
	typedef xgc_size xAttrIndex;

	/// 属性版本信息
	struct CORE_API XAttrVersion
	{
		xgc_uint32 start;
		xgc_uint32 close;
	};

	///
	/// \brief 属性定义
	/// \author create by albert.xu
	/// \date [6/27/2014]
	///
	struct CORE_API XAttributeBase
	{
		xAttrType	type;	///< 属性类型

		xgc_lpcstr	name;	///< 属性名
		xgc_byte	flags;	///< 属性标志, ATTR_FLAG_SAVE - 保存到数据库
		xgc_size	count;	///< 数组长度

		xAttrIndex  *attr_ptr;
	};

	///
	/// \brief 属性信息
	/// \author create by albert.xu
	/// \date [6/27/2014]
	///
	struct CORE_API XAttributeImpl
	{
		///< 属性定义
		XAttributeBase impl;
		///< 属性版本
		XAttrVersion version;
	};

	typedef xgc_void( XObject::* PFN_OnValueChanged )( xAttrIndex nAttr );
	///
	/// 属性信息
	/// [6/27/2014] create by albert.xu
	///
	struct CORE_API XAttributeInfo
	{
		///< 属性定义
		XAttributeBase impl;
		///< 属性偏移量
		xgc_size offset;
		///< 允许循环触发
		xgc_long cycle_change;
		///< 属性监听
		PFN_OnValueChanged OnValueChanged;
	};

	/************************************************************************/
	/* 列表系统，属性系统，以及对象基础类的实现								*/
	/************************************************************************/
	typedef xgc_lpvoid	xPosition;

	class XObject;

	///
	/// [1/24/2014 albert.xu]
	/// 根据字符串获取版本信息
	///
	CORE_API XAttrVersion ConvertVersion( xgc_lpcstr lpVersion );

	///
	/// \brief 根据字符串获取版本信息
	/// \author albert.xu
	/// \date 1/24/2014
	///
	class CORE_API XClassInfo
	{
	protected:
		xgc_lpcstr	mClassName;			///< 类名

		xgc_size	mImplementCount;	///< 配置个数
		xgc_size	mAttributeCount;	///< 属性个数
		xgc_size	mAttributeSize;		///< 属性所占空间大小

		XAttributeImpl * __implement;	///< 本地属性定义描述指针
		XAttributeInfo * __attribute;	///< 本地属性信息描述指针

		XAttributeImpl ** mImplementInfo;	///< 属性定义描述指针
		XAttributeInfo ** mAttributeInfo;	///< 属性信息描述指针

		//////////////////////////////////////////////////////////////////////////
		/*
			mImplementInfo
			+---------------+
			| impl_ptr[0]   |---+     Local
			| impl_ptr[1]   |   |    +-----------+
			| impl_ptr[2]   |   +--->| impl[0]   |
			| impl_ptr[3]   |		 | impl[1]   |
			| impl_ptr[4]   |		 | impl[2]   |
			| impl_ptr[5]   |---+    | ...       |
			| impl_ptr[6]   |	|	 | impl[n]   |
			| ...           |   |    +-----------+
			| impl_ptr[n]   |   |
			+---------------+   |     Parent Local
							    |    +-----------+
							 	+--->| impl[0]   |
							 		 | impl[1]   |
							 		 | impl[2]   |
							 		 | ...       |
							 		 | impl[n]   |
							 		 +-----------+
		*/
		//////////////////////////////////////////////////////////////////////////
		const XClassInfo *mParent;			///< 从哪儿继承的

		/// @var 用名字查索引
		xgc::unordered_map< xgc::string, xAttrIndex* > mName2Index;

	public:
		///
		/// 构造函数
		/// [1/6/2014 albert.xu]
		///
		XClassInfo( xgc_lpcstr lpClassName, XAttributeImpl *pAttributeImpl, const XClassInfo *pParent = xgc_nullptr );

		///
		/// 析构函数
		/// [1/6/2014 albert.xu]
		///
		~XClassInfo();

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
		XGC_INLINE const XClassInfo* GetBaseClass()const
		{
			return mParent;
		} //-V591

		///
		/// 运行时类型检查, 是否是该类型或该类型的派生类。
		/// [1/7/2014 albert.xu]
		///
		XGC_INLINE xgc_bool IsInheritFrom( const XClassInfo *cls )const
		{
			const XClassInfo *cur = this;
			while( cur )
			{
				if( cur == cls )
					return true;

				cur = cur->GetBaseClass();
			}

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
			return mAttributeInfo[nIndex]->impl.type;
		}

		XGC_INLINE xgc_bool IsArrayType( xgc_size nIndex )const
		{
			XGC_ASSERT_RETURN( CheckIndex( nIndex ), false );
			return mAttributeInfo[nIndex]->impl.count != 1;
		}

		XGC_INLINE xgc_size GetArrayLength( xgc_size nIndex )const
		{
			XGC_ASSERT_RETURN( CheckIndex( nIndex ), 0 );
			return mAttributeInfo[nIndex]->impl.count;
		}

		XGC_INLINE XAttributeInfo * const * GetAttributeInfo() const
		{
			return mAttributeInfo;
		}

		XGC_INLINE XAttributeImpl * const * GetImplementInfo() const
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

		/// 
		/// \brief 通过Name获取Index
		/// \date 3/25/2014 
		/// \author jianglei.kinly
		///
		XGC_INLINE xAttrIndex GetAttrIndexByName( xgc_lpcstr lpName )
		{
			XGC_ASSERT_RETURN( lpName, -1 );
			auto& iter = mName2Index.find( lpName );
			if( iter != mName2Index.end() )
				return *( iter->second );
			XGC_DEBUG_MESSAGE( "没有找到对应的Index[%s]", lpName );
			return -1;
		}

		///
		/// \brief 比较两个类类型是否相等 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		XGC_INLINE xgc_bool operator == ( const XClassInfo& cls )const { return mClassName == cls.mClassName; }

		///
		/// \brief 比较两个类类型是否不等 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		XGC_INLINE xgc_bool operator != ( const XClassInfo& cls )const { return mClassName != cls.mClassName; }

		///
		/// \brief 设置属性监听函数 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		template< class T, typename std::enable_if< std::is_base_of< XObject, T >::value, bool >::type = true >
		xgc_bool SetAttrListener( xAttrIndex nAttr, xgc_void( T::* OnValueChanged )( xAttrIndex ), xgc_long cycle_change = 1 )const
		{
			if( nAttr < mAttributeSize )
				return false;

			mAttributeInfo[nAttr]->cycle_change = cycle_change;
			mAttributeInfo[nAttr]->OnValueChanged = static_cast<PFN_OnValueChanged>( OnValueChanged );
			return true;
		}
	};
}
#endif // __XCLASS_H__