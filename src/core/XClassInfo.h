#pragma once
#ifndef __XCLASS_H__
#define __XCLASS_H__

#include "XAttribute.h"

#define ATTR_FLAG_NONE	0
#define ATTR_FLAG_SAVE	1
#define ATTR_FLAG_FIELD 2
#define ATTR_FLAG_MORE	3

#define DECLARE_XCLASS()\
public: \
	static	const XClassInfo& GetThisClass(); \
	virtual const XClassInfo& GetRuntimeClass()const;\

#define DECLARE_POOL_OBJECT()\
	static void * operator new( xgc_size s );\
	static void * operator new( xgc_size s, xgc_uint32 block, xgc_lpcstr file, xgc_uint32 line );\
	static void operator delete( void * p );\
	static void operator delete( void * p, xgc_uint32 block, xgc_lpcstr file, xgc_uint32 line );\

#define IMPLEMENT_POOL_OBJECT( thisClass, thisThread )\
	static XGC::memory::freelist< thisClass, std::mutex > thisClass##_Pool;\
	void * thisClass::operator new( xgc_size s ){ return thisClass##_Pool.alloc_memory( _NORMAL_BLOCK, __FILE__, __LINE__ ); }\
	void * thisClass::operator new( xgc_size s, xgc_uint32 block, xgc_lpcstr file, xgc_uint32 line ){ return thisClass##_Pool.alloc_memory( block, file, line ); }\
	void thisClass::operator delete( void * p ){ thisClass##_Pool.garbage_memory( p ); }\
	void thisClass::operator delete( void * p, xgc_uint32 block, xgc_lpcstr file, xgc_uint32 line ){ thisClass##_Pool.garbage_memory( p ); }\

#define IMPLEMENT_XCLASS_BEGIN( thisClass, baseClass )\
	static XAttributeImpl*	__get_##thisClass##_Attribute(); \
	const XClassInfo& thisClass##::GetThisClass() \
	{ \
		static XClassInfo cls_##thisClass( #thisClass, __get_##thisClass##_Attribute(), &baseClass::GetThisClass() ); \
		return cls_##thisClass; \
	} \
	const XClassInfo& cls_##thisClass = thisClass##::GetThisClass(); \
	const XClassInfo& thisClass##::GetRuntimeClass() const \
	{ \
		return thisClass##::GetThisClass(); \
	} \
	static XAttributeImpl *	__get_##thisClass##_Attribute() \
	{ \
		thisClass* pClass = xgc_nullptr; \
		static XAttributeImpl attribute[] = \
		{ \

// �����������е�����
#define IMPLEMENT_ATTRIBUTE( NAME, TYPE, FLAGS, VERSION )\
			{ { TYPE, #NAME, FLAGS, 1, &attr##NAME, }, ConvertVersion( VERSION ) }, \

#define IMPLEMENT_ATTRIBUTE_ARRAY( NAME, TYPE, COUNT, FLAGS, VERSION )\
			{ { TYPE, #NAME, FLAGS | 0x80, COUNT, &attr##NAME, }, ConvertVersion( VERSION ) }, \

#define IMPLEMENT_XCLASS_END() \
			{ { VT_VOID, xgc_nullptr, 0, 0, xgc_nullptr }, { 0, 0 } }, \
		}; \
		return attribute; \
	} \

namespace xgc
{
	class XObject;

	/// ������������
	typedef xgc_size xAttrIndex;

	/// ���԰汾��Ϣ
	struct CORE_API XAttrVersion
	{
		xgc_uint32 start;
		xgc_uint32 close;
	};

	///
	/// \brief ���Զ���
	/// \author create by albert.xu
	/// \date [6/27/2014]
	///
	struct CORE_API XAttributeBase
	{
		xAttrType	type;	///< ��������

		xgc_lpcstr	name;	///< ������
		xgc_byte	flags;	///< ���Ա�־, ATTR_FLAG_SAVE - ���浽���ݿ�
		xgc_size	count;	///< ���鳤��

		xAttrIndex  *attr_ptr;
	};

	///
	/// \brief ������Ϣ
	/// \author create by albert.xu
	/// \date [6/27/2014]
	///
	struct CORE_API XAttributeImpl
	{
		///< ���Զ���
		XAttributeBase impl;
		///< ���԰汾
		XAttrVersion version;
	};

	///
	/// ������Ϣ
	/// [6/27/2014] create by albert.xu
	///
	struct CORE_API XAttributeInfo
	{
		///< ���Զ���
		XAttributeBase impl;
		///< ����ƫ����
		xgc_size	offset;
		///< ���Լ���
		xgc_void( XObject::* OnValueChanged )( xAttrIndex nAttr, int nIndex );
	};

	/************************************************************************/
	/* �б�ϵͳ������ϵͳ���Լ�����������ʵ��								*/
	/************************************************************************/
	typedef xgc_lpvoid	xPosition;

	class XObject;

	///
	/// [1/24/2014 albert.xu]
	/// �����ַ�����ȡ�汾��Ϣ
	///
	CORE_API XAttrVersion ConvertVersion( xgc_lpcstr lpVersion );

	///
	/// \brief �����ַ�����ȡ�汾��Ϣ
	/// \author albert.xu
	/// \date 1/24/2014
	///
	class CORE_API XClassInfo
	{
	protected:
		xgc_lpcstr	mClassName;			///< ����

		xgc_size	mImplementCount;	///< ���ø���
		xgc_size	mAttributeCount;	///< ���Ը���
		xgc_size	mAttributeSize;		///< ������ռ�ռ��С

		XAttributeImpl * mImplementInfo;	///< ���Զ�������ָ��
		XAttributeInfo * mAttributeInfo;	///< ������Ϣ����ָ��

		const XClassInfo *mParent;			///< ���Ķ��̳е�

		xgc_unordered_map<xgc_string, xAttrIndex*> mName2Index;

	public:
		///
		/// ���캯��
		/// [1/6/2014 albert.xu]
		///
		XClassInfo( xgc_lpcstr lpClassName, XAttributeImpl *pAttributeImpl, const XClassInfo *pParent = xgc_nullptr );

		///
		/// ��������
		/// [1/6/2014 albert.xu]
		///
		~XClassInfo();

		///
		/// ��ȡ����
		/// [11/13/2014] create by albert.xu
		///
		XGC_INLINE xgc_lpcstr GetClassName()const
		{
			return mClassName;
		}

		///
		/// ��ȡָ���̳���ȵĻ�������
		/// [6/30/2014] create by albert.xu
		///
		XGC_INLINE const XClassInfo* GetBaseClass()const
		{
			return mParent;
		} //-V591

		///
		/// ����ʱ���ͼ��, �Ƿ��Ǹ����ͻ�����͵������ࡣ
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
		/// ��������Ƿ�Ϸ�
		/// [1/3/2014 albert.xu]
		///
		XGC_INLINE xgc_bool CheckIndex( xgc_size nIndex )const
		{
			return nIndex < mAttributeCount;
		}

		XGC_INLINE xAttrType GetAttributeType( xgc_size nIndex )const
		{
			XGC_ASSERT_RETURN( CheckIndex( nIndex ), VT_VOID );
			return mAttributeInfo[nIndex].impl.type;
		}

		XGC_INLINE xgc_bool IsArrayType( xgc_size nIndex )const
		{
			XGC_ASSERT_RETURN( CheckIndex( nIndex ), false );
			return mAttributeInfo[nIndex].impl.count != 1;
		}

		XGC_INLINE xgc_size GetArrayLength( xgc_size nIndex )const
		{
			XGC_ASSERT_RETURN( CheckIndex( nIndex ), 0 );
			return mAttributeInfo[nIndex].impl.count;
		}

		XGC_INLINE const XAttributeInfo* GetAttributeInfo() const
		{
			return mAttributeInfo;
		}

		XGC_INLINE const XAttributeImpl* GetImplementInfo() const
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

		// ------------------------------------------------ //
		// [3/25/2014 jianglei.kinly]
		// ͨ��Name��ȡIndex
		// ------------------------------------------------ //
		XGC_INLINE xAttrIndex GetAttrIndexByName( xgc_lpcstr lpName )
		{
			XGC_ASSERT_RETURN( lpName, -1 );
			auto& iter = mName2Index.find( lpName );
			if( iter != mName2Index.end() )
				return *( iter->second );
			XGC_DEBUG_MESSAGE( "û���ҵ���Ӧ��Index[%s]", lpName );
			return -1;
		}

		template< class T >
		XGC_INLINE xgc_void SetAttrListener( xAttrIndex nAttr, xgc_void( T::* OnValueChanged )( xAttrIndex, int ) )
		{
			XGC_ASSERT_RETURN( nAttr < mAttributeSize, XGC_NONE );

			auto &info = mAttributeInfo[nAttr];
			XGC_ASSERT( info.impl.attr_ptr[0] = nAttr );

			info.OnValueChanged = static_cast<xgc_void( XObject::* )( xAttrIndex, int nIndex )>( OnValueChanged );
		}
	};
}
#endif // __XCLASS_H__