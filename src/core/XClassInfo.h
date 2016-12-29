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

// �����������е�����
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
	typedef xgc_size xAttrIndex;	/// ������������
	///
	/// ������ֵ
	/// [6/30/2014] create by albert.xu
	///
	typedef xgc_uint32	xClassType;

	/************************************************************************/
	/* �б�ϵͳ������ϵͳ���Լ�����������ʵ��								*/
	/************************************************************************/
	typedef XGC::IndexType< xgc_uint64, 0 >::Type xGlobal;
	typedef XGC::IndexType< xgc_uint32, 1 >::Type xObject;
	typedef xgc_lpvoid	xPosition;

	//////////////////////////////////////////////////////////////////////////
	// �򵥱���������,�����������ͺ͸�������
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
	/// ͨ��Name��ȡIndex
	/// ------------------------------------------------ //
	CORE_API xAttrIndex GetAttrIndexByName( xgc_lpcstr lpName );

	class XObject;

	struct CORE_API XAttributeVersion
	{
		xgc_uint32 start;
		xgc_uint32 close;
	};

	class CORE_API XVariantReference;

	typedef xgc_bool( *PFN_GetAttribute )( XVariantReference&, xgc_uintptr, xAttrIndex, xAttrType, ... ); // ��ȡ����ֵ
	typedef xgc_bool( *PFN_SetAttribute )( XVariantReference&, xgc_uintptr, xAttrIndex, xAttrType, ... ); // ��������ֵ

	///
	/// ������Ϣ
	/// [6/27/2014] create by albert.xu
	///
	struct CORE_API XImplementInfo
	{
		xgc_lpcstr	name;	///< ������
		xgc_byte	flags;	///< ���Ա�־, ATTR_FLAG_SAVE - ���浽���ݿ�
		xAttrType	type;	///< ��������
		xgc_size	count;	///< ������ռ�ڴ���󳤶�(byte), �ַ����ͻ���������Ч��
		XAttributeVersion version;

		PFN_GetAttribute pfnGet; ///< ���Ի�ȡ
		PFN_SetAttribute pfnSet; ///< ��������

		xAttrIndex* attr_ptr;
	};

	///
	/// ������Ϣ
	/// [6/27/2014] create by albert.xu
	///
	struct CORE_API XAttributeInfo
	{
		xgc_lpcstr	name;	///< ������
		xgc_byte	flags;	///< ���Ա�־, ATTR_FLAG_SAVE - ���浽���ݿ�
		xAttrType	type;	///< ��������
		xgc_size	count;	///< ������ռ�ڴ���󳤶�(byte), �ַ����ͻ���������Ч��
		xgc_size	offset;	///< ����ƫ����

		PFN_GetAttribute pfnGet; ///< ���Ի�ȡ
		PFN_SetAttribute pfnSet; ///< ��������

		xAttrIndex* attr_ptr;
	};

	///
	/// [1/24/2014 albert.xu]
	/// ��ȡ���Գ���
	///
	CORE_API xgc_size AttributeLength( xAttrType eType, xgc_size nCount );

	///
	/// [1/24/2014 albert.xu]
	/// �����ַ�����ȡ�汾��Ϣ
	///
	CORE_API XAttributeVersion AttributeVersion( xgc_lpcstr lpVersion );

	struct CORE_API XClassInfo
	{
	protected:
		xClassType	mClassType[16];		///< ����̳�����
		xgc_lpcstr	mClassName;			///< ����
		xgc_size	mInheritLevel;		///< �̳����

		xgc_size	mImplementCount;	///< ���ø���
		xgc_size	mAttributeCount;	///< ���Ը���
		xgc_size	mAttributeSize;		///< ������ռ�ռ��С

		XImplementInfo * mImplementInfo;	///< ���Զ�������ָ��
		XAttributeInfo * mAttributeInfo;	///< ������Ϣ����ָ��

		const XClassInfo *mParent;			///< ���Ķ��̳е�
	public:
		///
		/// ���캯��
		/// [1/6/2014 albert.xu]
		///
		XClassInfo( xClassType nClassType, xgc_lpcstr lpClassName, XImplementInfo *pAttributeConf, const XClassInfo *pParent = xgc_nullptr );

		///
		/// ��������
		/// [1/6/2014 albert.xu]
		///
		~XClassInfo();

		///
		/// ��ȡ������
		/// [2/20/2014 albert.xu]
		///
		XGC_INLINE xClassType GetClassType()const
		{
			return mClassType[mInheritLevel-1];
		}

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
		XGC_INLINE xClassType GetBaseType( xgc_size nInheritDeep )const
		{
			XGC_ASSERT( nInheritDeep < mInheritLevel );
			return mClassType[nInheritDeep];
		} //-V591

		///
		/// ����ʱ���ͼ��, �Ƿ��Ǹ����ͻ�����͵������ࡣ
		/// [1/7/2014 albert.xu]
		///
		XGC_INLINE xgc_bool IsInheritFrom( xClassType clsType )const
		{
			// ������Ʒ�̳���gameobject �̳���Ȼ���assert
			if( mInheritLevel == 0U || xgc_size( ( (xgc_byteptr) &clsType )[3] & 0x0f ) >= mInheritLevel )
				return false;

			if( mClassType[( (xgc_byteptr) &clsType )[3] & 0x0f ] == clsType )
				return true;

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