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
	// 技能升级更新需要用到的标示为，请勿使用同样字符串用在其他地方
	// ------------------------------------------------ //
	#define UPDATE_SKILL_MAP_PROP_UNIQUE "Update_Skill_Map_Prop_Unique"
	// ------------------------------------------------ //
	// [5/23/2014 jianglei.kinly]
	// 删除状态用的关键字，请勿在其他地方使用同样字符串
	// ------------------------------------------------ //
	#define DELETE_STATE_MAP_PROP_UNIQUE "Delete_State_Map_Prop_Unique"

	using namespace common;
	enum eStatus { status_append, status_active, status_remove, timing_count };

	typedef xgc_unordered_map< xgc_string, std::pair< xAttrType, XVariant > > ActionPropMap;
	// 定义指向函数的指针
	class CORE_API XAction;
	enum ActionReturn { action_none, action_success, action_failed, action_break, action_jumpnext, action_delay };
	typedef ActionReturn( *StatusFunc )( xObject hDest, xgc_int32 nFlag, xgc_uintptr& pUserdata, xgc_lpcvoid pBuf, xgc_size nSize );
	
	//////////////////////////////////////////////////////////////////////////
	// Begin
	// ------------------------------------------------ //
	// [5/9/2014 jianglei.kinly]
	// 新的Action处理方式
	// ------------------------------------------------ //
	// 普通 方式Action的返回值
	enum EnBasicActionRes { enBasicAction_none, enBasicAction_break, enBasicAction_success, enBasicAction_failed, enBasicAction_delay, enBasicAction_delete, };
	// Action节点类型
	enum EnActionNodeType { enActionNode_select, enActionNode_basic, enActionNode_Error, };
	// lParam->Skill/State xObject 
	// wParam->TypeSkill/TypeState Index
	// Switch 方式的Action
	typedef xgc_int32( *SelectActionFunc )( xObject hSource, xObject hTarget, xgc_uintptr wParam, xgc_uintptr lParam, xgc_lpvoid pBuf, xgc_size nSize, Json::Value* );
	// 普通 方式的Action
	typedef EnBasicActionRes( *BasicActionFunc )( xObject hSource, xObject hTarget, xgc_uintptr wParam, xgc_uintptr lParam, xgc_lpvoid pBuf, xgc_size nSize, Json::Value* );

	// 一个节点
	struct CORE_API XActionNode
	{
		union unActionNodeFuncPtr
		{
			SelectActionFunc selectFunc;
			BasicActionFunc basicFunc;
		};
		struct stNode
		{
			unActionNodeFuncPtr mFuncPtr;  // 该节点指针
			xBuffer mParams;               // 该节点序列化参数
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
		EnActionNodeType mActionType;      // 当前节点类型
		stNode* mNode;                      // 当前节点
		xgc_unordered_map< xgc_int32, xgc_vector<XActionNode*> > mLstChild; // 当前节点子节点（select类型节点有效：key->case值 value->action列表）

		// ------------------------------------------------ //
		// [5/9/2014 jianglei.kinly]
		// Select 模式的 node 处理方式
		// ------------------------------------------------ //
		EnBasicActionRes TriggerSelectActionNode( XActionNode* selectNode, xObject hSource, xObject hTarget, xgc_intptr wParam, xgc_intptr lParam, Json::Value*, xgc_bool isOnlyDoModifyAttr );
		// ------------------------------------------------ //
		// [5/9/2014 jianglei.kinly]
		// Basic 模式的 node 处理方式
		// ------------------------------------------------ //
		EnBasicActionRes TriggerBasicActionNode( XActionNode* basicNode, xObject hSource, xObject hTarget, xgc_intptr wParam, xgc_intptr lParam, Json::Value*, xgc_bool isOnlyDoModifyAttr );
		// ------------------------------------------------ //
		// [5/9/2014 jianglei.kinly]
		// node 处理入口
		// ------------------------------------------------ //
		EnBasicActionRes TriggerActionNode( XActionNode* node, xObject hSource, xObject hTarget, xgc_intptr wParam, xgc_intptr lParam, Json::Value*, xgc_bool isOnlyDoModifyAttr = true );

		// End
		//////////////////////////////////////////////////////////////////////////
	};

	// ------------------------------------------------ //
	// [5/13/2014 jianglei.kinly]
	// 这个里面只有BasicActionFunc
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
		/// 执行动作 
		/// [11/18/2010 Albert]
		/////
		virtual EnBasicActionRes Execute( xObject hSender, xObject hReciver, xgc_uintptr wParam, xgc_uintptr lParam, Json::Value* );

		/////
		/// [11/26/2010 Albert]
		/// Description:	获取动作参数 
		/////
		xBuffer GetParams()const { return mParams; }

	protected:
		BasicActionFunc	mFunctor;
		xBuffer	mParams;
	};

	///
	/// 行为树
	/// [7/25/2014] create by albert.xu
	///
	class CORE_API XActionTree : public XAction
	{
	public:
		typedef xgc_multimap< xgc_uint32, XAction* > ChildrensMap;
	private:
		 ChildrensMap	mChildrens;	///< 子节点集合
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
		/// 添加一个子节点
		/// [7/25/2014] create by albert.xu
		/// @param nKey 数字索引
		/// @param nWeight 权重，用于随机
		/// @param pAction 节点指针
		///
		xgc_void AppendChildren( xgc_uint32 nKey, XAction* pAction )
		{
			mChildrens.insert( std::make_pair( nKey, pAction ) );
		}

		///
		/// 删除一个子节点
		/// [7/25/2014] create by albert.xu
		/// @param nKey 数字索引
		/// @return 节点指针
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
		/// 执行动作 
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
		// Description:	添加作用 
		//---------------------------------------------------//
		void AppendEffect( const StatusFunc& Functor, xBuffer Params );

		//---------------------------------------------------//
		// [11/19/2010 Albert]
		// Description:	执行状态 
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
		// Description:	更新状态 
		//---------------------------------------------------//
		xgc_void OnUpdate( timer_t handle );

		//---------------------------------------------------//
		// [12/15/2010 Albert]
		// Description:	状态结束 
		//---------------------------------------------------//
		xgc_void OnStatusOver( timer_t handle );

		//---------------------------------------------------//
		// [9/13/2009 Albert]
		// Description:	安装定时器
		//---------------------------------------------------//
		xgc_void InstallTimer( float fLifetime, float fInterval, float fDelay = 0 );

		//---------------------------------------------------//
		// [11/19/2010 Albert]
		// Description:	执行状态 
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
