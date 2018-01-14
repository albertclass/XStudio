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
	/// \brief �¼����͵ķ��� 
	/// \date 12/27/2017
	/// \author xufeng04
	///
	enum eEventTrans : xgc_long
	{
		toSelf		= 0,
		toParent	= 1,
		toChildren	= 2,
	};

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
		/// @var �Ƿ��� 
		/// 0 - ֻ�򱾶���Ĺ۲��ߴ�����Ϣ
		/// 1 - ���ϴ�����Ϣ
		/// 2 - ���´�����Ϣ
		/// 3 - �����Ϻ����´�����Ϣ
		/// < 0 ��Ϣ�Ѵ���
		xgc_long	over;
	};

	///
	/// \brief ɾ���ص�����Ҫ��Ϊ����ģ�屣��������Ϣ����ֹ��������δ����ȷ����
	/// \date 12/26/2017
	/// \author xufeng04
	///
	template< class TObjectEvent >
	xgc_void DeleteEvent( XObjectEvent* p )
	{
		TObjectEvent* evt = XGC_CONTAINER_OF( p, TObjectEvent, cast );
		delete evt;
	}

	typedef xgc_void( XObject::* XEventBind1 )( xgc_long, xgc_long );
	typedef xgc_void( XObject::* XEventBind2 )( XObjectEvent&, xgc_long, xgc_long );
	typedef xgc_void( XObject::* XEventBind3 )( XObjectEvent*, xgc_void(*)( XObjectEvent* ) );

	typedef std::function< void( XObjectEvent& ) > xNotify;

	///
	/// \brief ��Ϸ�������
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
		/// \brief �Ƿ�̳���ĳ����
		/// \author albert.xu
		/// \date 2014/06/30
		/// \param clsType ���GUID
		/// \param nStartDeep ���Կ�ʼ�ļ̳����
		///
		XGC_INLINE xgc_bool IsInheritFrom( const XClassInfo &cls )const
		{
			return GetRuntimeClass().IsInheritFrom( &cls );
		}

		///
		/// ��ȡָ���̳в�ε�������ID
		/// [6/30/2014] create by albert.xu
		///
		XGC_INLINE xgc_lpcstr GetClassName()const
		{
			return GetRuntimeClass().GetClassName();
		}

	private:
		XObject( const XObject& rsh );

		/// @var ���������׵�ַ
		xgc_lpvoid mAttributes = xgc_nullptr;
		/// @var ����������Ϣ��ַ
		XAttributeInfo * const * mAttributeInfo = xgc_nullptr;

		///
		/// \brief �۲�����Ϣ
		///
		struct Observer
		{
			/// ����״̬
			xgc_bool close;
			/// ���Ʒ���
			xgc_long token;
			/// �����������ֹmap������ʧЧ
			xgc_long count;
			/// �۲���֪ͨ�ӿ� xgc::map< token, xgc::tuple< fnNotify, xOwner > >
			xgc::map< xgc_long, xgc::tuple< xNotify, xObject > > actions;
		};

		/// @var �¼��۲���
		xgc::map< xgc_long, Observer > mEventSubject;
		/// @var �Ƿ�����
		xgc_bool mIsDestory;
		/// @var ������ID
		xObject	mParentID;
	public:
		/************************************************************************/
		/* �㼶��ϵ����
		/************************************************************************/
		XGC_INLINE xObject GetObjectID()const { return handle()._handle; }
		// �õ�������ID
		XGC_INLINE xObject GetParent()const { return mParentID; }
		// ��ȡ������ID
		XGC_INLINE xObject GotParent( const XClassInfo &cls )const;

		// ���ø�����
		XGC_INLINE xgc_void SetParent( xObject nID ) { mParentID = nID; }
		// ���ٶ���
		xgc_void Destroy();

		///
		/// \brief ��ȡ����ļ�ֵ 
		/// \date 11/16/2017
		/// \author xufeng04
		/// \return ��ֵ
		///
		virtual xgc_long Key()const { return GetObjectID(); }

		///
		/// \brief ��ȡ�Ӷ�������
		/// \date 12/26/2017
		/// \author xufeng04
		///
		virtual xgc_size GetChildCount()const { return 0; }

		///
		/// \brief ö���Ӷ���  
		/// \date 12/26/2017
		/// \author xufeng04
		///
		virtual xObject Search( const std::function< xgc_bool( xObject ) > &Filter )const { return INVALID_OBJECT_ID; };

		/************************************************************************/
		/* �¼�����
		/************************************************************************/
		///
		/// \brief ע���¼�
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_long	RegistEvent( xgc_long id, const xNotify &invoke, xObject hOwner = INVALID_OBJECT_ID );

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
		/// \brief �����¼�
		/// \author albert.xu
		/// \date 2017/11/03
		///
		xgc_void	EnableEvent( xgc_long id, xgc_bool enable = true );

		///
		/// \brief ����һ���¼����󲢳�ʼ��
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

		template< class TObjectEvent, typename std::enable_if< std::is_same< TObjectEvent, XObjectEvent >::value, TObjectEvent >::type >
		XObjectEvent* MakeEvent( xgc_long id, xgc_long direction )
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
		/// \brief ��ʼ��һ���ѷ����¼�����
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
		/// \brief �ύ�¼�
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_void EmmitEvent( xgc_long id, xgc_long direction = 0 );

		///
		/// \brief �ύ�¼�
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_void EmmitEvent( XObjectEvent& evt, xgc_long id, xgc_long direction = 0 );

		///
		/// \brief �ύ�¼�
		/// \author albert.xu
		/// \date 2017/10/12
		///
		xgc_void EmmitEvent( XObjectEvent* evt, xgc_void( *DeleteIt )( XObjectEvent* ) = xgc_nullptr );

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
		XGC_INLINE xgc_bool isAttrValid( xAttrIndex idx )const
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
		/// [1/7/2014 albert.xu]
		/// ��ȡ�з�����
		///
		template< class T, typename std::enable_if< is_numeric< T >::value, bool >::type = true >
		XGC_INLINE T getValue( xAttrIndex nAttr ) const
		{
			return getAttr( nAttr ).toNumeric< T >();
		}

		///
		/// \brief ����VT���ͻ�ȡ����ֵ
		/// \date 12/28/2017
		/// \author albert.xu
		///
		template< xAttrType VT >
		XGC_INLINE typename XAttribute::Value2Type< VT >::type getValue( xAttrIndex nAttr )
		{
			return getValue< typename XAttribute::Value2Type< VT >::type >( nAttr );
		}

		///
		/// [1/7/2014 albert.xu]
		/// ��������ֵ
		///
		template< class T, typename std::enable_if< is_numeric< T >::value, bool >::type = true >
		XGC_INLINE xgc_void setValue( xAttrIndex nAttr, T _Val )
		{
			getAttr( nAttr ) = _Val;
			OnValueChanged( nAttr );
		}

		template< class T, typename std::enable_if< std::is_same< T, bool >::value, bool >::type = true >
		XGC_INLINE xgc_void setValue( xAttrIndex nAttr, xgc_bool _Val )
		{
			getAttr( nAttr ) = _Val;
			OnValueChanged( nAttr );
		}

		///
		/// \brief ����VT���ͻ�ȡ����ֵ
		/// \date 12/28/2017
		/// \author albert.xu
		///
		template< xAttrType VT >
		XGC_INLINE xgc_void setValue( xAttrIndex nAttr, typename XAttribute::Value2Type< VT >::type _Val )
		{
			return setValue< typename XAttribute::Value2Type< VT >::type >( nAttr, _Val );
		}

		///
		/// [1/7/2014 albert.xu]
		/// ��������ֵ
		///
		template< class T, typename std::enable_if< is_numeric< T >::value, xgc_bool >::type = true >
		XGC_INLINE xgc_void incValue( xAttrIndex nAttr, T inc )
		{
			getAttr( nAttr ) += inc;
			OnValueChanged( nAttr );
		}

		///
		/// [2/11/2014 albert.xu]
		/// ��ȡ�ַ���
		///
		XGC_INLINE xgc_lpcstr getString( xAttrIndex nAttr ) const
		{
			return getAttr( nAttr ).toRawString( xgc_nullptr );
		}

		///
		/// [2/11/2014 albert.xu]
		/// ��ȡ������
		///
		XGC_INLINE xgc_lpvoid getBuffer( xAttrIndex nAttr ) const
		{
			return getAttr( nAttr ).toRawBuffer();
		}

		///
		/// [1/9/2014 albert.xu]
		/// �����ַ�������
		///
		XGC_INLINE xgc_void setString( xAttrIndex nAttr, xgc_lpcstr _Val, xgc_size nLength = -1 )
		{
			getAttr( nAttr ).setString( _Val, nLength );
			OnValueChanged( nAttr );
		}

		///
		/// [1/9/2014 albert.xu]
		/// \brief ���û���������
		///
		XGC_INLINE xgc_void setBuffer( xAttrIndex nAttr, xgc_lpvoid _Val, xgc_size nSize )
		{
			getAttr( nAttr ).setBuffer( _Val, nSize );
			OnValueChanged( nAttr );
		}

		///
		/// [3/18/2014 albert.xu]
		/// ���û���������
		/// ���û��������Ȳ�Ӱ�����еĻ��������ݡ�
		///
		XGC_INLINE xgc_bool setBufferLength( xAttrIndex nAttr, xgc_size nSize, xgc_byte szFill = 0 )
		{
			return getAttr( nAttr ).setBufferLength( nSize, szFill );
		}

		///
		/// \brief ��ȡ����������
		/// \author albert.xu
		/// \date 3/17/2014
		///
		XGC_INLINE xgc_size getBufferLength( xAttrIndex nAttr )
		{
			return getAttr( nAttr ).getBufferLength();
		}

		///
		/// \brief ����λ
		/// \author albert.xu
		/// \date 2017/11/09
		///
		XGC_INLINE xgc_void SetBit( xAttrIndex nAttr, xgc_size _Bit )
		{
			getAttr( nAttr ).SetBit( _Bit );
			OnValueChanged( nAttr );
		}

		///
		/// \brief ���λ
		/// \author albert.xu
		/// \date 2017/11/09
		///
		XGC_INLINE xgc_void ClrBit( xAttrIndex nAttr, xgc_size _Bit )
		{
			getAttr( nAttr ).ClrBit( _Bit );
			OnValueChanged( nAttr );
		}

		///
		/// \brief ��ȡλ
		/// \author albert.xu
		/// \date 2017/11/09
		///
		XGC_INLINE xgc_bool GetBit( xAttrIndex nAttr, xgc_size _Bit, xgc_bool bDefault = false ) const
		{
			return getAttr( nAttr ).GetBit( _Bit, bDefault );
		}

		///
		/// \brief ���λ
		/// \author albert.xu
		/// \date 2017/11/09
		///
		XGC_INLINE xgc_bool ChkBit( xAttrIndex nAttr, xgc_size _Bit, xgc_bool bTest ) const
		{
			return getAttr( nAttr ).CmpBit( _Bit, bTest );
		}

		///
		/// \brief ����־û����� 
		/// \date 11/9/2017
		/// \author xufeng04
		/// \param uVersion ���ݵİ汾��
		/// \return �����Ƿ�ɹ�
		///
		virtual xgc_bool LoadObject( xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize );

		///
		/// \brief ����־û����� 
		/// \date 11/9/2017
		/// \author xufeng04
		/// \param uVersion ���ݵİ汾��
		/// \return �����˶�������
		///
		virtual xgc_size SaveObject( xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize );

	protected:
		///
		/// [2/11/2014 albert.xu]
		/// ��ȡ����
		/// �ú����ṩһ����������������Hook�ķ������������ú�ȡֵ�����Ե���Hook��
		///
		XAttribute getAttr( xAttrIndex nAttr ) const;

		///
		/// \brief ���Ա仯ʱ����
		/// \author albert.xu
		/// \date 2017/11/09
		///
		xgc_void OnValueChanged( xAttrIndex nAttr )
		{
			// ��ֹ�¼�������ѭ�������¼�
			if( mAttributeInfo[nAttr]->cycle_change &&
				mAttributeInfo[nAttr]->OnValueChanged )
			{
				--mAttributeInfo[nAttr]->cycle_change;
				( this->*mAttributeInfo[nAttr]->OnValueChanged )( nAttr );
				++mAttributeInfo[nAttr]->cycle_change;
			}
		}

		///
		/// \brief ���ٶ���ʱ����
		/// \author albert.xu
		/// \date 2017/11/01
		virtual xgc_void OnDestroy() = 0;
	};

	///
	/// \brief ��һ���������תΪָ��������ṩת����Ŀ������
	/// \author albert.xu
	/// \date 4/25/2014
	///
	template< class T, typename std::enable_if< std::is_base_of< XObject, T >::value && std::is_base_of< std::enable_shared_from_this< T >, T >::value == false, xgc_bool >::type = true >
	T* ObjectCast( xObject hObject )
	{
		XObject* pObject = XObject::handle_exchange( hObject );
		if( !pObject )
			return xgc_nullptr;

		if( !pObject->IsInheritFrom( &T::GetThisClass() ) )
			return xgc_nullptr;

		return static_cast< T* >( pObject );
	}

	template< class T, typename std::enable_if< std::is_base_of< XObject, T >::value && std::is_base_of< std::enable_shared_from_this< T >, T >::value == true, xgc_bool >::type = true >
	std::shared_ptr< T > ObjectCast( xObject hObject )
	{
		T* Pointer = static_cast<T*>( GetXObject( hObject, &T::GetThisClass() ) );

		return Pointer ? Pointer->shared_from_this() : xgc_nullptr;
	}

	template<>
	XGC_INLINE xgc_bool XObject::getValue< xgc_bool >( xAttrIndex nAttr ) const
	{
		return getAttr( nAttr ).toBool();
	}

	// ��ȡ������ID
	XGC_INLINE xObject XObject::GotParent( const XClassInfo & cls ) const
	{
		XObject *pObject = ObjectCast< XObject >( GetObjectID() );
		while( pObject && cls != pObject->GetRuntimeClass() )
		{
			pObject = ObjectCast< XObject >( pObject->GetParent() );
		}

		return pObject ? pObject->GetObjectID() : INVALID_OBJECT_ID;
	}

}

#endif //_XOBJECT_