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
	/// \brief �����¼�����
	/// \author albert.xu
	/// \date 2017/10/12
	///
	struct CORE_API XObjectEvent
	{
		/// @var �¼�ID
		xgc_long	id;
		/// @var �¼��ķ�����
		xObject		sender;
		/// @var �¼��Ľ�����
		xObject		target;
		/// @var �¼�����
		xgc_long	token;
		/// @var ����ֵ -1 - err, 0 - pending, 1 - ok
		xgc_long	result;
	};

	typedef xgc_void( XObject::* XEventBind1 )( XObjectEvent& );
	typedef xgc_void( XObject::* XEventBind2 )( xgc_long, XObjectEvent& );

	typedef std::function< void( XObjectEvent& ) > xAction;

	///
	/// \brief ��Ϸ�������
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
		/// \brief ��ȡ����Ϣ
		/// \author albert.xu
		/// \date 2017/10/09
		///
		static	const XClassInfo& GetThisClass();

		///
		/// \brief ��ȡ������ʱ��Ϣ
		/// \author albert.xu
		/// \date 2017/10/09
		///
		virtual const XClassInfo& GetRuntimeClass()const;

		///
		/// \brief �����ʼ��
		/// \author albert.xu
		/// \date 2017/10/09
		///
		bool InitObject();

		///
		/// \brief �Ƿ�̳���ĳ����
		/// \author albert.xu
		/// \date 2014/06/30
		/// \param clsType ���GUID
		/// \param nStartDeep ���Կ�ʼ�ļ̳����
		///
		XGC_INLINE xgc_bool IsInheritFrom( const XClassInfo *cls )const
		{
			return GetRuntimeClass().IsInheritFrom( cls );
		}

		///
		/// ��ȡָ���̳в�ε�������ID
		/// [6/30/2014] create by albert.xu
		///
		XGC_INLINE xgc_lpcstr GetClassName()const
		{
			return  GetRuntimeClass().GetClassName();
		}

	private:
		XObject( const XObject& rsh );

		/// @var ���������׵�ַ
		xgc_lpvoid mAttributes = xgc_nullptr;
		/// @var ����������Ϣ��ַ
		const XAttributeInfo * mAttributeInfo = xgc_nullptr;
		/// @var �������Զ����ַ
		const XAttributeImpl * mImplementInfo = xgc_nullptr;

		///
		/// \brief �۲�����Ϣ
		struct Observer
		{
			xgc_long token;
			xgc_long count;
			xgc_map< xgc_long, xgc_tuple< xAction, xObject > > actions;
		};

		/// @var �¼��۲���
		xgc_map< xgc_long, Observer > mEventSubject;

		/// @var �Ƿ�����
		xgc_bool mIsDestory;
		/// @var �������б�
		xObjectList	mChildList;
		/// @var ������ID
		xObject	mParentID;
	public:
		/************************************************************************/
		/* �㼶��ϵ����
		/************************************************************************/
		xObject		GetObjectID()const { return handle()._handle; }
		// ���ø�����
		xgc_void	SetParent( xObject nID ) { mParentID = nID; }
		// �õ�������ID
		xObject		GetParent()const { return mParentID; }
		// ����Ӷ���
		xgc_bool	AddChild( xObject  nObj, xgc_lpcvoid lpContext = 0 );
		xgc_bool	AddChild( XObject* pObj, xgc_lpcvoid lpContext = 0 );
		// ɾ���Ӷ���
		xgc_void	RemoveChild( xObject  nObj, xgc_bool bDestroy = false );
		xgc_void	RemoveChild( XObject* pObj, xgc_bool bDestroy = false );
		// ��ѯ�Ӷ���
		xgc_bool	QueryChild( XObject* pObj )const;
		xgc_bool	QueryChild( xObject  nObj )const;
		xgc_bool	QueryChild( const std::function< xgc_bool( xObject ) >& fnFilter )const;

		/************************************************************************/
		// ���ٶ���
		xgc_void	Destroy();
		// ���������Ӷ���
		xgc_void	DestroyAllChild();
		// ��ȡ�Ӷ�������
		xgc_size	GetChildrenCount()const { return mChildList.size(); }

		/************************************************************************/
		/* �¼�����
		/************************************************************************/
		///
		/// \brief ע���¼�
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_long	RegistEvent( xgc_long id, const xAction &invoke, xObject hOwner = INVALID_OBJECT_ID );

		///
		/// \brief ɾ���¼�
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_void	RemoveEvent( xgc_long id, xgc_long token );

		///
		/// \brief ɾ���¼�
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_void	RemoveEvent( xObject hOwner );

		///
		/// \brief �ύ�¼�
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_void	EmmitEvent( xgc_long id, XObjectEvent& evt = XObjectEvent() );

		///
		/// \brief �ύ�¼�
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_void	EmmitEvent( XObjectEvent& evt );

		/************************************************************************/
		/* ���Բ���
		/************************************************************************/

		///
		/// \brief ��ȡ��������
		/// \author albert.xu
		/// \date 2017/10/09
		///
		XGC_INLINE xgc_size getAttrCount()const
		{
			return GetRuntimeClass().GetAttributeCount();
		}

		///
		/// \brief ��������Ƿ�Ϸ�
		/// \author albert.xu
		/// \date 2017/10/09
		///
		XGC_INLINE xgc_bool checkIndex( xAttrIndex idx )const
		{
			return idx < getAttrCount();
		}

		///
		/// \brief ��ȡ��������
		/// \author albert.xu
		/// \date 2017/10/09
		///
		XGC_INLINE xAttrType getAttrType( xAttrIndex idx )const
		{
			return GetRuntimeClass().GetAttributeType( idx );
		}

		///
		/// \brief �����Ƿ���������
		/// \author albert.xu
		/// \date 2017/10/09
		///
		XGC_INLINE xgc_bool isArray( xAttrIndex idx )const
		{
			return GetRuntimeClass().IsArrayType( idx );
		}

		///
		/// \brief ��ȡ�������鳤��
		/// \author albert.xu
		/// \date 2017/10/09
		///
		XGC_INLINE xgc_size getAttrArrayLength( xAttrIndex idx )const
		{
			return GetRuntimeClass().GetArrayLength( idx );
		}

		///
		/// [2/11/2014 albert.xu]
		/// ��ȡ����
		/// �ú����ṩһ����������������Hook�ķ������������ú�ȡֵ�����Ե���Hook��
		///
		XGC_INLINE XAttribute getAttr( xAttrIndex idx, xgc_size nIndex = 0 ) const
		{
			XGC_ASSERT_THROW( checkIndex( idx ), std::logic_error( "get attribute, index out of bound." ) );
			xgc_lpstr lpValue = (xgc_lpstr)mAttributes + mAttributeInfo[idx].offset;

			// �������Բ�������
			return XAttribute( mAttributeInfo[idx].impl.type,
				lpValue + nIndex * XAttribute::Type2Size( mAttributeInfo[idx].impl.type ) );
		}

		///
		/// [1/7/2014 albert.xu]
		/// ��ȡ�з�����
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
		/// ��������ֵ
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
		/// ��ȡ�ַ���
		///
		XGC_INLINE xgc_lpcstr getString( xAttrIndex idx ) const
		{
			return getAttr( idx ).toRawString( xgc_nullptr );
		}

		///
		/// [2/11/2014 albert.xu]
		/// ��ȡ������
		///
		XGC_INLINE xgc_lpvoid getBuffer( xAttrIndex idx ) const
		{
			return getAttr( idx ).toRawBuffer();
		}

		///
		/// [1/9/2014 albert.xu]
		/// �����ַ�������
		///
		XGC_INLINE xgc_void setString( xAttrIndex idx, xgc_lpcstr _Val, xgc_size nLength = -1 )
		{
			getAttr( idx ).setString( _Val, nLength );
		}

		///
		/// [1/9/2014 albert.xu]
		/// ���û���������
		///
		XGC_INLINE xgc_void setBuffer( xAttrIndex idx, xgc_lpvoid _Val, xgc_size nSize )
		{
			getAttr( idx ).setBuffer( _Val, nSize );
		}

		///
		/// [3/18/2014 albert.xu]
		/// ���û���������
		/// ���û��������Ȳ�Ӱ�����еĻ��������ݡ�
		///
		XGC_INLINE xgc_bool setBufferLength( xAttrIndex idx, xgc_size nSize, xgc_byte szFill = 0 )
		{
			return getAttr( idx ).setBufferLength( nSize, szFill );
		}

		///
		/// [3/17/2014 albert.xu]
		/// ��ȡ����������
		///
		XGC_INLINE xgc_size getBufferLength( xAttrIndex idx )
		{
			return getAttr( idx ).getBufferLength();
		}
	protected:
		/************************************************************************/
		/* ���õ�һЩ�¼���Ӧ�麯����
		/************************************************************************/
		/////
		/// �����ӽڵ�ǰ����
		/// [8/3/2009 Albert]
		/// @return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		/////
		virtual xgc_bool PreAddChild( XObject* pChild, xgc_lpcvoid lpContext ) = 0;

		/////
		/// �����ӽڵ�����
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext ) = 0;

		/////
		/// �����ӽڵ�ǰ����
		/// [8/3/2009 Albert]
		/// @return true - ȷ���Ƴ��ӽڵ�, false - �ӽڵ㱻���,�Ƴ��ӽڵ�ʧ��.
		/////
		virtual xgc_bool PreRemoveChild( XObject* pChild, xgc_bool bRelease ) = 0;

		/////
		/// ɾ���ӽڵ�����,��ʱ������δ��ɾ��
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void OnRemoveChild( XObject* pChild, xgc_bool bRelease ) = 0;

		///
		/// ���ٶ����ʱ�����
		///
		virtual xgc_void OnDestroy() = 0;
	};

	//-----------------------------------//
	// [4/25/2014 albert.xu]
	// ��һ���������תΪָ�����
	// ���ṩת����Ŀ������
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
	/// \brief ���ض�������
	/// \author albert.xu
	/// \date 2017/10/10
	///
	xgc_bool LoadObject( XObject* pObj, xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize );

	///
	/// \brief �����������
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