#pragma once

#ifdef CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif	//CORE_EXPORTS
#pragma warning( disable:4251 )

/*********************************************************************/
/*
	Create:     2006/01/041:2006   17:46
	FileName:   XEffectBase.h
	Author:     Albert xu
	*/
/*********************************************************************/
namespace XGC
{
	// ------------------------------------------------ //
	// [5/23/2014 jianglei.kinly]
	// ��������������Ҫ�õ��ı�ʾΪ������ʹ��ͬ���ַ������������ط�
	// ------------------------------------------------ //
	#define UPDATE_SKILL_MAP_PROP_UNIQUE "Update_Skill_Map_Prop_Unique"
	// ------------------------------------------------ //
	// [5/23/2014 jianglei.kinly]
	// ɾ��״̬�õĹؼ��֣������������ط�ʹ��ͬ���ַ���
	// ------------------------------------------------ //
	#define DELETE_STATE_MAP_PROP_UNIQUE "Delete_State_Map_Prop_Unique"

	using namespace common;
	enum eStatus { status_append, status_active, status_remove, timing_count };

	typedef xgc_unordered_map< xgc_string, std::pair< xAttrType, XVariant > > ActionPropMap;
	// ����ָ������ָ��
	class CORE_API XAction;
	enum ActionReturn { action_none, action_success, action_failed, action_break, action_jumpnext, action_delay };
	typedef ActionReturn( *StatusFunc )( xObject hDest, xgc_int32 nFlag, xgc_uintptr& pUserdata, xgc_lpcvoid pBuf, xgc_size nSize );
	
	//////////////////////////////////////////////////////////////////////////
	// Begin
	// ------------------------------------------------ //
	// [5/9/2014 jianglei.kinly]
	// �µ�Action����ʽ
	// ------------------------------------------------ //
	// ��ͨ ��ʽAction�ķ���ֵ
	enum EnBasicActionRes { enBasicAction_none, enBasicAction_break, enBasicAction_success, enBasicAction_failed, enBasicAction_delay, enBasicAction_delete, };
	// Action�ڵ�����
	enum EnActionNodeType { enActionNode_select, enActionNode_basic, enActionNode_Error, };
	// lParam->Skill/State xObject 
	// wParam->TypeSkill/TypeState Index
	// Switch ��ʽ��Action
	typedef xgc_int32( *SelectActionFunc )( xObject hSource, xObject hTarget, xgc_uintptr wParam, xgc_uintptr lParam, xgc_lpvoid pBuf, xgc_size nSize, Json::Value* );
	// ��ͨ ��ʽ��Action
	typedef EnBasicActionRes( *BasicActionFunc )( xObject hSource, xObject hTarget, xgc_uintptr wParam, xgc_uintptr lParam, xgc_lpvoid pBuf, xgc_size nSize, Json::Value* );

	// һ���ڵ�
	struct CORE_API XActionNode
	{
		union unActionNodeFuncPtr
		{
			SelectActionFunc selectFunc;
			BasicActionFunc basicFunc;
		};
		struct stNode
		{
			unActionNodeFuncPtr mFuncPtr;  // �ýڵ�ָ��
			xBuffer mParams;               // �ýڵ����л�����
			stNode( unActionNodeFuncPtr funcptr, xBuffer param )
				: mFuncPtr(funcptr)
				, mParams(param)
			{
			}
		};

		XActionNode()
			: mActionType( enActionNode_Error )
			, mNode( xgc_nullptr )
		{

		}
		~XActionNode();
		EnActionNodeType mActionType;      // ��ǰ�ڵ�����
		stNode* mNode;                      // ��ǰ�ڵ�
		xgc_unordered_map< xgc_int32, xgc_vector<XActionNode*> > mLstChild; // ��ǰ�ڵ��ӽڵ㣨select���ͽڵ���Ч��key->caseֵ value->action�б�

		// ------------------------------------------------ //
		// [5/9/2014 jianglei.kinly]
		// Select ģʽ�� node ����ʽ
		// ------------------------------------------------ //
		EnBasicActionRes TriggerSelectActionNode( XActionNode* selectNode, xObject hSource, xObject hTarget, xgc_intptr wParam, xgc_intptr lParam, Json::Value*, xgc_bool isOnlyDoModifyAttr );
		// ------------------------------------------------ //
		// [5/9/2014 jianglei.kinly]
		// Basic ģʽ�� node ����ʽ
		// ------------------------------------------------ //
		EnBasicActionRes TriggerBasicActionNode( XActionNode* basicNode, xObject hSource, xObject hTarget, xgc_intptr wParam, xgc_intptr lParam, Json::Value*, xgc_bool isOnlyDoModifyAttr );
		// ------------------------------------------------ //
		// [5/9/2014 jianglei.kinly]
		// node �������
		// ------------------------------------------------ //
		EnBasicActionRes TriggerActionNode( XActionNode* node, xObject hSource, xObject hTarget, xgc_intptr wParam, xgc_intptr lParam, Json::Value*, xgc_bool isOnlyDoModifyAttr = true );

		// End
		//////////////////////////////////////////////////////////////////////////
	};

	// ------------------------------------------------ //
	// [5/13/2014 jianglei.kinly]
	// �������ֻ��BasicActionFunc
	// ------------------------------------------------ //
	class CORE_API XAction
	{
	public:
		XAction() = delete;
		XAction( const BasicActionFunc Functor );
		XAction( const BasicActionFunc Functor, xBuffer Params );
		XAction( const BasicActionFunc Functor, xgc_lpvoid pData, xgc_size nSize );
		XAction( const XAction& rhs );

		virtual ~XAction() {}
		XAction& operator=( const XAction& rhs ) = delete;
	public:
		/////
		/// ִ�ж��� 
		/// [11/18/2010 Albert]
		/////
		virtual EnBasicActionRes Execute( xObject hSender, xObject hReciver, xgc_uintptr wParam, xgc_uintptr lParam, Json::Value* );

		/////
		/// [11/26/2010 Albert]
		/// Description:	��ȡ�������� 
		/////
		xBuffer GetParams()const { return mParams; }

	protected:
		BasicActionFunc	mFunctor;
		xBuffer	mParams;
	};

	///
	/// ��Ϊ��
	/// [7/25/2014] create by albert.xu
	///
	class CORE_API XActionTree : public XAction
	{
	public:
		typedef xgc_multimap< xgc_uint32, XAction* > ChildrensMap;
	private:
		 ChildrensMap	mChildrens;	///< �ӽڵ㼯��
	public:
		XActionTree() = delete;
		XActionTree( const BasicActionFunc Functor )
			: XAction( Functor )
		{

		}

		XActionTree( const BasicActionFunc Functor, xBuffer Params )
			: XAction( Functor, Params )
		{

		}

		XActionTree( const BasicActionFunc Functor, xgc_lpvoid pData, xgc_size nSize )
			: XAction( Functor, pData, nSize )
		{
		}

		XActionTree( const XActionTree& rhs )
			: XAction( rhs )
		{
			// DeepCopy
		}

		virtual ~XActionTree();

		///
		/// ���һ���ӽڵ�
		/// [7/25/2014] create by albert.xu
		/// @param nKey ��������
		/// @param nWeight Ȩ�أ��������
		/// @param pAction �ڵ�ָ��
		///
		xgc_void AppendChildren( xgc_uint32 nKey, XAction* pAction )
		{
			mChildrens.insert( std::make_pair( nKey, pAction ) );
		}

		///
		/// ɾ��һ���ӽڵ�
		/// [7/25/2014] create by albert.xu
		/// @param nKey ��������
		/// @return �ڵ�ָ��
		///
		xgc_void RemoveChildren( xgc_uint32 nKey )
		{
			auto itr = mChildrens.equal_range( nKey );
			for( auto it = itr.first; it != itr.second; ++it )
			{
				SAFE_DELETE( it->second );
			}
			mChildrens.erase( itr.first, itr.second );
		}

	private:
		/////
		/// ִ�ж��� 
		/// [11/18/2010 Albert]
		/////
		virtual EnBasicActionRes Execute( xObject hSource, xObject hTarget, xgc_uintptr wParam, xgc_uintptr lParam, Json::Value* ) override;
	};

	class CORE_API XStatusInstance
	{
	public:
		XStatusInstance();
		~XStatusInstance();

		//---------------------------------------------------//
		// [12/14/2010 Albert]
		// Description:	������� 
		//---------------------------------------------------//
		void AppendEffect( const StatusFunc& Functor, xBuffer Params );

		//---------------------------------------------------//
		// [11/19/2010 Albert]
		// Description:	ִ��״̬ 
		//---------------------------------------------------//
		void DoStatus( xObject nIdentity, xgc_uintptr& pUserdata, eStatus eFlag );
	private:
		struct effect
		{
			xBuffer	mParams;
			StatusFunc	mFunctor;
		};

		typedef xgc_vector< effect > EffectContainer;

		EffectContainer	m_EffectContainer;
	};

	class CORE_API XStatus : public XObject
	{
	public:
		XStatus( XStatusInstance* pInstance )
			: mInstance( pInstance )
			, mUpdateTimerHandler( INVALID_TIMER_HANDLE )
		{
		}

		~XStatus();

		//---------------------------------------------------//
		// [12/14/2010 Albert]
		// Description:	����״̬ 
		//---------------------------------------------------//
		xgc_void OnUpdate( timer_t handle );

		//---------------------------------------------------//
		// [12/15/2010 Albert]
		// Description:	״̬���� 
		//---------------------------------------------------//
		xgc_void OnStatusOver( timer_t handle );

		//---------------------------------------------------//
		// [9/13/2009 Albert]
		// Description:	��װ��ʱ��
		//---------------------------------------------------//
		xgc_void InstallTimer( float fLifetime, float fInterval, float fDelay = 0 );

		//---------------------------------------------------//
		// [11/19/2010 Albert]
		// Description:	ִ��״̬ 
		//---------------------------------------------------//
		XGC_INLINE xgc_void DoStatus( eStatus eFlag )
		{
			mInstance->DoStatus( GetParent(), m_Userdata, eFlag );
		}

	private:
		XStatusInstance* mInstance;
		timer_t mUpdateTimerHandler;
		timer_t mLifetimeHandler;

		union
		{
			xgc_uint32	u;
			xgc_int32	n;
			xgc_real32 f;
			xgc_uintptr	m_Userdata;
		};
	};
}
