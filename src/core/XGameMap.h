#pragma once
#ifndef _XGAME_MAP_H_
#define _XGAME_MAP_H_

#define INVALID_BLOCK_INDEX 0x0000ffffU

//////////////////////////////////////////////////////////////////////////
// MapBlock 格子管理
namespace xgc
{
	/// @var 场景索引
	extern CORE_API xAttrIndex attrSceneIndex;
	/// @var 场景显示名
	extern CORE_API xAttrIndex attrSceneTitle;
	/// @var 场景索引名(唯一ID)
	extern CORE_API xAttrIndex attrSceneStrName;
	/// @var 场景地图名
	extern CORE_API xAttrIndex attrSceneMapName;
	/// @var 场景标记
	extern CORE_API xAttrIndex attrSceneFlags;

	class XAction;
	class XGameObject;
	///
	/// 格子对象
	/// [6/25/2014] create by albert.xu
	///
	struct IBlockExtern
	{
		///
		/// 进入时调用
		/// [6/25/2014] create by albert.xu
		/// @param pObject 作用的对象
		///
		virtual xgc_void OnEnterBlock( XGameObject* pObject ) = 0;

		///
		/// 越过时调用
		/// [6/25/2014] create by albert.xu
		/// @param pObject 作用的对象
		///
		virtual xgc_void OnCrossBlock( XGameObject* pObject ) = 0;

		///
		/// 离开时调用
		/// [6/25/2014] create by albert.xu
		/// @param pObject 作用的对象
		///
		virtual xgc_void OnLeaveBlock( XGameObject* pObject ) = 0;

		///
		/// 释放对象，该函数应当被每个继承类重载
		/// [6/25/2014] create by albert.xu
		///
		virtual xgc_void Release() = 0;
	};

	#define COLLISION_NONE	      0	  ///< 什么也不检测
	#define COLLISION_PATH	      1	  ///< 检测路径碰撞
	#define COLLISION_DEST	      2	  ///< 检测终点碰撞
	#define COLLISION_NOMOVEFLAG  4   ///< 不检测移动标识

	#define EYESHOTAREA_FORCEfLUSH  256   ///< 视野强制刷新,9-16

	enum class CORE_API MapFlags
	{
		eCanMove = 0,
	};

	enum class CORE_API MapCell
	{
		BLOCK = 1,
		BARRIER = 2,
		SAFETY = 4,
		ATTACK = 8,
	};

	enum CORE_API eGameMapEvent
	{
		evt_map_timer,
		evt_map_clock,
		evt_map_counter_change,
		evt_map_counter_overflow,
		evt_map_counter_underflow,
		evt_map_turn_off,
		evt_map_turn_on,
	};

	/// 地图事件
	struct CORE_API XGameMapEvent
	{
		/// @var 事件
		XObjectEvent cast;
		/// @var 源的名字
		xgc_lpcstr alias;
		/// @var 计数器值
		xgc_long val;
		/// @var 计数器增量
		xgc_long inc;
	};

	/// 地图初始化参数
	struct CORE_API MapConf
	{
		/// @var 单元格大小（一个单元格的长和宽，世界坐标单位）
		fSize mCellSize;

		/// @var 单元格配置（一张地图包含的网格行列数）
		iSize mCellConf;

		/// @var 视野区域大小（一个域包含的单元格行列数）
		iSize mAreaCell;

		/// @var 视野区域（场景对象所拥有的视野）
		iSize mEyesight;

		/// 有些客户端认为场景的中心点为（0，0）点，有些认为左上角为（0，0）点，该变量用于变换坐标系
		/// @var 坐标变换（场景中心点和世界坐标中心点的差值）
		fPoint mTransfrom;
	};

	///
	/// 场景地图基类
	/// 该类主要实现了基础的碰撞管理和视野管理两个模块
	/// [6/25/2014] create by albert.xu
	///
	class CORE_API XGameMap : public XObjectNode
	{
		DECLARE_XCLASS();
	private:
		struct MapConf mMapConf;

		/// @var 区域大小（基于世界坐标单位）
		fSize mAreaSize;

		/// @var 区域配置（一张地图包含的区域行列数）
		iSize mAreaConf;

		/// @var 视野函数重入检测
		TwiceEnterGuard	mEyeshotChecker;

		/// @var 移动函数重入检测
		TwiceEnterGuard	mDynamicChecker;

		xObjectSet *mpAreas; ///< 区域列表

	public:
		typedef xgc_unordered_map< xgc_string, xgc_list< xgc_tuple< xgc_long, xAction > > > MapEventConf;
		typedef xgc_unordered_map< xgc_string, xgc_tuple< xgc_long, xgc_long, xgc_long > > MapCounter;
		typedef xgc_unordered_map< xgc_string, xgc_long > MapSwitch;
		typedef xgc_unordered_map< xgc_string, timer_h > MapTimer;
		typedef xgc_unordered_map< xgc_string, timer_h > MapClock;

	protected:
		///
		/// 地砖数据结构
		/// [6/28/2014] create by albert.xu
		///
		union MapCell
		{
			struct
			{
				xgc_uint32 block : 1;		///< 阻挡
				xgc_uint32 barrier : 1;		///< 栅栏
				xgc_uint32 safe : 1;		///< 安全区
				xgc_uint32 attack : 1;		///< 是否可攻击（限制玩家PK）
				xgc_uint32 unused : 2;		///< 未使用变量

				xgc_uint32 ext : 2;		///< 记录IBlockExtern的索引占几个字节
				xgc_uint32 data : 24;	///< 扩展属性
			};
			xgc_uint32 mask; ///< 用于初始化
		};

		MapCell *mpCells;	///< 地砖数据
		//struct IActorMapEventHandler *mpActorEventHandler; ///< 角色事件

		/// @var 地图事件
		MapEventConf mMapEventConf;
		/// @var 计数器配置
		MapCounter	mMapCounter;
		/// @var 场景内的开关对象
		MapSwitch	mMapSwitch;
		/// @var 场景内的计时器
		MapTimer	mMapTimer;
		/// @var 场景内的定时器
		MapClock	mMapClock;

		/// @var 地砖扩展信息
		xgc_vector< IBlockExtern* > mBlockExtern; 

		/// @var 客户端事件列表 < event, state >
		xgc_map< xgc_uint16, xgc_uint16 > mClientEvents;

		/// @var 服务器事件列表 < event, eventHandle >
		xgc_map< xgc_uint16, function< xgc_void( XGameMap*, xgc_uint16, xgc_uint16 ) > > mServerEvents;

		/// @var 触发器日志
		function< xgc_void( xgc_lpcstr, xgc_uint32, xObject, xObject ) > mpTriggerLogger;

		///
		/// 获取一个地图格子的指针
		/// [10/14/2014] create by albert.xu
		///
		MapCell* GetCell( xgc_int32 x, xgc_int32 y )
		{
			XGC_ASSERT_RETURN( x >= 0 && x < mMapConf.mCellConf.cx && y >= 0 && y < mMapConf.mCellConf.cy
				, xgc_nullptr
				, "scene = %s, x = %d, y = %d, map size = (%d,%d)"
				, getString( attrSceneTitle )
				, x, y
				, mMapConf.mCellConf.cx, mMapConf.mCellConf.cy );

			return mpCells + y * mMapConf.mCellConf.cx + x;
		}

	protected:
		///
		/// 基础构造函数
		/// [6/25/2014] create by albert.xu
		///
		XGameMap();

		///
		/// 析构函数
		/// [6/25/2014] create by albert.xu
		///
		~XGameMap();

		/////
		/// 初始化地图 
		/// [1/4/2011 Albert]
		/////
		xgc_void InitializeMap( const struct MapConf &Conf );

	public:
		XGC_INLINE	xgc_uint32	GetCellCountW()const { return mMapConf.mCellConf.cx; }
		XGC_INLINE	xgc_uint32	GetCellCountH()const { return mMapConf.mCellConf.cy; }
		//XGC_INLINE	xgc_uint32	GetAreaCountW()const { return mMapConf.mFieldSize.cx; }
		//XGC_INLINE	xgc_uint32	GetAreaCountH()const { return mMapConf.mFieldSize.cy; }

		//XGC_INLINE	xgc_real32	GetCellWidth()const  { return mMapConf.mCellSize.cx; }
		//XGC_INLINE	xgc_real32	GetCellHeight()const { return mMapConf.mCellSize.cy; }
		//XGC_INLINE	xgc_real32	GetAreaWidth()const  { return mMapConf.mFieldSize.cx; }
		//XGC_INLINE	xgc_real32	GetAreaHeight()const { return mMapConf.mFieldSize.cy; }
		//XGC_INLINE	xgc_real32	GetWorldWidth()const { return GetCellCountW()*GetCellWidth(); }
		//XGC_INLINE	xgc_real32	GetWorldHeight()const{ return GetCellCountH()*GetCellHeight(); }

		XGC_INLINE xgc_int32 GetEyesightX() const { return mMapConf.mEyesight.cx; }
		XGC_INLINE xgc_int32 GetEyesightY() const { return mMapConf.mEyesight.cy; }

		///
		/// 世界坐标换算为格子坐标
		/// [6/25/2014] create by albert.xu
		///
		iPoint WorldToCell( xgc_real32 rx, xgc_real32 ry )const
		{
			xgc_int32 x = xgc_int32( ( rx + mMapConf.mTransfrom.x ) / mMapConf.mCellSize.cx );
			xgc_int32 y = xgc_int32( ( ry + mMapConf.mTransfrom.y ) / mMapConf.mCellSize.cy );

			x = x < 0 ? 0 : x;
			x = x > mMapConf.mCellConf.cx ? mMapConf.mCellConf.cx : x;

			y = y < 0 ? 0 : y;
			y = y > mMapConf.mCellConf.cy ? mMapConf.mCellConf.cy : y;

			return iPoint( x, y );
		}

		///
		/// 格子坐标换算为世界坐标，取中心点位置
		/// [6/25/2014] create by albert.xu
		///
		XVector2 CellToWorld( xgc_int32 x, xgc_int32 y )const
		{
			return XVector2(
				( x + 0.5f )*mMapConf.mCellSize.cx - mMapConf.mTransfrom.x,
				( y + 0.5f )*mMapConf.mCellSize.cy - mMapConf.mTransfrom.y );
		}

		///
		/// 世界坐标换算视野管理单元索引
		/// [10/14/2014] create by albert.xu
		///
		iPoint WorldToArea( xgc_real32 x, xgc_real32 y )const
		{
			return iPoint(
				xgc_int32( ( x + mMapConf.mTransfrom.x ) / mAreaSize.cx ),
				xgc_int32( ( y + mMapConf.mTransfrom.y ) / mAreaSize.cy ) );
		}

		///
		/// 世界管理单元索引换算世界坐标（取中心点）
		/// [10/14/2014] create by albert.xu
		///
		XVector2 AreaToWorld( xgc_int32 x, xgc_int32 y )const
		{
			return XVector2(
				( x + 0.5f )*mAreaSize.cx - mMapConf.mTransfrom.x,
				( y + 0.5f )*mAreaSize.cy - mMapConf.mTransfrom.y );
		}

		///
		/// 设置扩展属性索引
		/// [6/25/2014] create by albert.xu
		/// @param ptCell 地砖的坐标
		/// @see iPoint
		///
		xgc_void SetBlockExternIdx( iPoint ptCell, xgc_uint32 nIdx );

		///
		/// 获取扩展属性索引
		/// [6/25/2014] create by albert.xu
		/// @param ptCell 地砖的坐标
		/// @see iPoint
		///
		xgc_uint32 GetBlockExternIdx( iPoint ptCell );

		///
		/// 设置地砖扩展信息
		/// [6/25/2014] create by albert.xu
		/// @param pExtern 扩展接口的指针
		/// @param ptCell 地砖的坐标
		/// @see iPoint
		/// @see IBlockExtern
		///
		xgc_void SetBlockExternInfo( xgc_uint32 nBlockExternIdx, IBlockExtern* pExtern );

		///
		/// 设置扩展信息槽个数
		/// [6/25/2014] create by albert.xu
		/// @param pExtern 扩展接口的指针
		/// @param ptCells 地砖的坐标列表
		/// @see iPoint
		/// @see IBlockExtern
		///
		xgc_void SetBlockExternSize( xgc_size nBlockExternSize );

		///
		/// 获取扩展信息槽个数
		/// [7/4/2014] create by albert.xu
		/// @return 扩展信息槽个数
		///
		xgc_size GetBlockExternSize()const;

		///
		/// 扩展扩展信息槽个数
		/// [7/4/2014] create by albert.xu
		/// @return 扩展信息槽个数
		///
		xgc_size ExpandBlockExternSize( xgc_size nExpandCount );

		///
		/// 获取地砖扩展信息
		/// [6/25/2014] create by albert.xu
		/// @param ptCell 地砖的坐标
		/// @see iPoint
		///
		IBlockExtern* GetBlockExternInfo( iPoint ptCell );

		///
		/// 获取地砖扩展信息
		/// [6/25/2014] create by albert.xu
		/// @param nBlockExternIdx 扩展信息索引号
		///
		IBlockExtern* GetBlockExternInfo( xgc_uint32 nBlockExternIdx );

		///
		/// 设置格子掩码
		/// [10/14/2014] create by albert.xu
		///
		xgc_void SetCellBlock( xgc_int32 x, xgc_int32 y, xgc_bool bBlock = true );

		/////
		/// [1/4/2011 Albert]
		/// Description:	测试是否碰撞 
		/////
		xgc_bool IsCellBlock( xgc_int32 x, xgc_int32 y, xgc_bool bTestBarrier = true );

		///
		/// 测试是否是安全区
		/// [6/16/2014] create by jianglei.kinly
		///
		xgc_bool IsCellSafety( xgc_int32 x, xgc_int32 y );

		///
		/// 地图是否允许移动
		/// [9/12/2014] create by albert.xu
		///
		xgc_bool GetMoveFlag()const
		{
			return getAttr( attrSceneFlags ).GetBit( (xgc_size)MapFlags::eCanMove, true );
		}

		///
		/// 设置地图是否允许移动
		/// [9/12/2014] create by albert.xu
		///
		xgc_void SetMoveFlag( xgc_bool bCanMove = true )const
		{
			if( bCanMove )
				getAttr( attrSceneFlags ).SetBit( (xgc_size)MapFlags::eCanMove );
			else
				getAttr( attrSceneFlags ).ClrBit( (xgc_size)MapFlags::eCanMove );
		}

		///
		/// \brief 创建计数器 
		/// \author albert.xu
		/// \date 1/5/2011
		///
		xgc_void CreateCounter( xgc_lpcstr lpCounterName, xgc_long nInit, xgc_long nMax, xgc_long nMin = 0 )
		{
			auto ib = mMapCounter.insert( std::make_pair( lpCounterName, std::make_tuple( nMin, nMax, nInit ) ) );
			XGC_ASSERT( ib.second );
		}

		///
		/// \brief 创建计数器 
		/// \author albert.xu
		/// \date 1/5/2011
		///
		xgc_void CreateSwitch( xgc_lpcstr lpSwitchName, xgc_long nInit )
		{
			auto ib = mMapSwitch.insert( std::make_pair( lpSwitchName, nInit ) );
			XGC_ASSERT( ib.second );
		}

		///
		/// \brief 获取计数器名字列表
		/// \author albert.xu
		/// \date 8/12/2014
		///
		const MapCounter& GetMapCounter()const
		{
			return mMapCounter;
		}

		///
		/// \brief 获取计数器名字列表
		/// \author albert.xu
		/// \date 8/12/2014
		///
		const MapClock& GetMapClock()const
		{
			return mMapClock;
		}

		///
		/// 获取计数器名字列表
		/// [8/12/2014] create by albert.xu
		///
		const MapTimer& GetMapTimer()const
		{
			return mMapTimer;
		}

		///
		/// 创建闹钟
		/// \author albert.xu
		/// \date 12/7/2012
		///
		xgc_void CreateClock( xObject hSender, xgc_lpcstr lpName, xgc_lpcstr lpStart, xgc_lpcstr lpParam, xgc_lpcstr lpDuration );

		///
		/// \brief 创建闹钟
		/// \author albert.xu
		/// \date 12/22/2010
		///
		xgc_void CreateTimer( xObject hSender, xgc_lpcstr lpName, xgc_lpcstr lpStart, xgc_lpcstr lpParam, xgc_lpcstr lpDuration );

		///
		/// \brief 插入触发器 
		/// \author albert.xu
		/// \date 12/22/2010
		///
		xgc_void AddEventConf( xgc_lpcstr lpSourceName, xgc_long nEventId, xAction &&doAction )
		{
			XGC_ASSERT_RETURN( lpSourceName && lpSourceName[0], xgc_void( 0 ) );
			mMapEventConf[lpSourceName].emplace_back( std::make_tuple( nEventId, doAction ) );
		}

		///
		/// 删除计数器
		/// [7/28/2014] create by albert.xu
		///
		xgc_void DeleteCounter( xgc_lpcstr lpSourceName );

		///
		/// 删除场景开关
		/// [7/28/2014] create by albert.xu
		///
		xgc_void DeleteSwitch( xgc_lpcstr lpSourceName );

		///
		/// 删除定时器
		/// [7/28/2014] create by albert.xu
		///
		xgc_void DeleteClock( xgc_lpcstr lpSourceName );

		///
		/// 删除定时器
		/// [7/28/2014] create by albert.xu
		///
		xgc_void DeleteTimer( xgc_lpcstr lpSourceName );

		/////
		/// 增加计数器 
		/// [1/5/2011 Albert]
		/////
		xgc_void IncCounter( xgc_lpcstr lpCounterName, xgc_long nInc );

		/////
		/// 设置计数器 
		/// [1/5/2011 Albert]
		/////
		xgc_void SetCounter( xgc_lpcstr lpCounterName, xgc_long nVal );

		///
		/// 获取计数器值
		/// [11/5/2014] create by albert.xu
		///
		xgc_bool GetCounter( xgc_lpcstr lpCounterName, xgc_long &nVal );

		/////
		/// 转换开关
		/// [1/5/2011 Albert]
		/////
		xgc_void TurnSwitch( xgc_lpcstr lpSwitchName, xgc_long nSwitch );

		///
		/// 客户端场景事件触发
		/// [10/2/2014] create by albert.xu
		///
		xgc_bool TriggerClientEvent( xgc_uint16 nEvent, xgc_uint16 nState );

		///
		/// 服务器场景事件触发
		/// [10/27/2014] create by albert.xu
		///
		xgc_bool TriggerServerEvent( xgc_uint16 nEvent, xgc_uint16 nState );

		///
		/// 获取客户端事件列表
		/// [10/28/2014] create by albert.xu
		///
		const decltype( mClientEvents )& GetClientEvents()const
		{
			return mClientEvents;
		}

		///
		/// 获取指定场景事件的值
		/// [4/3/2015] create by jianglei.kinly
		///
		xgc_uint16 GetClientEvent( xgc_uint16 _key )const
		{
			auto it = mClientEvents.find( _key );
			if( it != mClientEvents.end() )
				return it->second;
			return (xgc_uint16)-1;
		}

		///
		/// 调试输出文字地图
		/// [8/6/2014] create by albert.xu
		///
		xgc_void OutputStringMap( xgc_lpcstr lpFileName );

		/////
		/// [12/29/2010 Albert]
		/// Description:	 根据世界坐标获取区域
		/////
		xObjectSet* GetArea( xgc_real32 x, xgc_real32 y )const;

		/////
		/// [12/29/2010 Albert]
		/// Description:	获取区域 
		/////
		xObjectSet* GetArea( xgc_int32 x, xgc_int32 y )const;

		/////
		/// 移动对象
		/// [8/7/2009 Albert]
		/////
		xgc_bool DynamicMoveTo( XGameObject* pObject, XVector3 &vPosition, xgc_uint32 nCollistionMask = COLLISION_PATH, xgc_uintptr lpContext = 0 );

		/////
		/// 传送对象
		/// [8/7/2009 Albert]
		/////
		xgc_bool TeleportTo( XGameObject* pObject, XVector3 &vPosition, xgc_uintptr lpContext = 0 );

		/////
		/// [1/5/2011 Albert]
		/// Description:	建立触发器 
		/////
		xgc_void BuildTrigger( XGameObject* pServerObject );

		///
		/// 手动执行触发器
		/// [11/19/2012 Albert.xu]
		///
		//xgc_void ExecuteTrigger( xgc_lpcstr lpSourceName, TriggerKey tTriggerKey, xObject hSender, xObject hReciver, xgc_uintptr wParam, xgc_uintptr lParam )const;

		/////
		/// 在地图中添加场景对象 
		/// @param pObject 插入到场景中的对象
		/// @param vPosition 对象在场景中的位置
		/// [1/5/2011 Albert]
		/////
		xgc_bool Insert( XGameObject* pObject, const XVector3& vPosition );

		/////
		/// 从场景中移除对象 
		/// @param pObject 从场景中移除的对象
		/// [1/5/2011 Albert]
		/////
		xgc_void Remove( XGameObject* pObject );

		/////
		/// 视野内对象选取
		/// 注意：区域指一个视野区域，而非一个Block。视野区域是多个相邻Block的集合
		/// [3/3/2014 baomin]
		/////
		xObjectSet CaptureObjectByEyeshot( const XVector3 &vCenter, const std::function< xgc_bool( xObject ) > &fnFilter = xgc_nullptr );

		/// ------------------------------------------------ //
		/// 获取附近的对象，圆形
		/// [4/18/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		xObjectSet CaptureObjectByCircle( const XVector3 &vCenter, xgc_uint32 dwRadius, const std::function< xgc_bool( xObject ) > &fnFilter = xgc_nullptr );

		/// ------------------------------------------------ //
		/// 获取附近的对象，扇形
		/// [4/18/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		xObjectSet CaptureObjectBySector( const XVector3 &vCenter, const XVector3& dir, xgc_uint32 dwRadius, xgc_uint16 wAngle, const std::function< xgc_bool( xObject ) > &fnFilter = xgc_nullptr );

		/// ------------------------------------------------ //
		/// 圆环
		/// [4/23/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		xObjectSet CaptureObjectByAnnular( const XVector3 &vCenter, xgc_uint32 dwRadiusMax, xgc_uint32 dwRadiusMin, const std::function< xgc_bool( xObject ) > &fnFilter = xgc_nullptr );

		/// ------------------------------------------------ //
		/// 扇形环（有角度限制的圆环）
		/// [4/23/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		xObjectSet CaptureObjectBySectorRing( const XVector3 &vCenter, const XVector3& dir, xgc_uint32 dwRadiusMax, xgc_uint32 dwRadiusMin, xgc_uint16 wAngle, const std::function< xgc_bool( xObject ) > &fnFilter = xgc_nullptr );

		/// ------------------------------------------------ //
		/// 获取附近的对象，长方形
		/// [4/18/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		xObjectSet CaptureObjectByRectangle( const XVector3 &vCenter, xgc_uint32 dwXRadius, xgc_uint32 dwYRadius, const std::function< xgc_bool( xObject ) > &fnFilter = xgc_nullptr );

		/// ------------------------------------------------ //
		/// 获取方向矩形
		/// [4/25/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		xObjectSet CaptureObjectByRectangle( const XVector3 &vCenter, const XVector3& vDir, xgc_uint32 dwRadiusMax, xgc_uint32 dwRadiusMin, const std::function< xgc_bool( xObject ) > &fnFilter = xgc_nullptr );

	protected:
		///
		/// 选取区域内场景对象
		/// [9/29/2014] create by albert.xu
		///
		xObjectSet FetchObject( iRect rc, const std::function< xgc_bool( xObject ) > &fnFilter );

		///
		/// 执行触发器
		/// [11/19/2012 Albert.xu]
		///
		//xgc_void ExecuteObjectTrigger( xObject hGameObject, TriggerKey tTriggerKey, xObject hSender, xObject hReciver, xgc_uintptr wParam, xgc_uintptr lParam )const;

		/////
		/// 碰撞检测
		/// [8/3/2009 Albert]
		/// @param x0,y0 起始坐标
		/// @param x1,y1 终止坐标
		/// @param bTestBarrier 是否检测栅栏设置
		/// @return 是否有碰撞
		/////
		xgc_bool TestCollision( xgc_int32 x0, xgc_int32 y0, xgc_int32 &x1, xgc_int32 &y1, xgc_bool bTestBarrier = true );

		/////
		/// 对象所占格子发生改变时调用,若在同一视野区的时候会直接返回
		/// [8/3/2009 Albert]
		/////
		xgc_void ExchangeEyeshotArea( XGameObject *pObject, const iPoint &vOldPosition, const iPoint &vNewPosition, const iSize &iEyeshot, xgc_uint32 nCollistionMask = 0 );

		///
		/// 设置格子阻挡信息
		/// [6/28/2014] create by albert.xu
		///
		xgc_void ExchangeBlockCell( XGameObject *pObject, iPoint &iOldCell, iPoint &iNewCell );

		/////
		/// 增加子节点前调用
		/// [8/3/2009 Albert]
		/// @return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		/////
		virtual xgc_bool PreAddChild( XObject* pChild, xgc_lpcvoid lpContext ) override;

		/////
		/// 加入子节点后调用
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext ) override;

		/////
		/// 增加子节点前调用
		/// [8/3/2009 Albert]
		/// @return true - 确认移除子节点, false - 子节点被否决,移除子节点失败.
		/////
		virtual xgc_bool PreDelChild( XObject* pChild, xgc_bool bRelease ) override;

		/////
		/// 删除子节点后调用,此时对象尚未被删除
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void OnDelChild( XObject* pChild, xgc_bool bRelease ) override;

		/////
		/// 销毁地图 
		/// [1/4/2011 Albert]
		/////
		virtual xgc_void OnDestroy() override;

		/////
		/// 通知格子内的所有对象,有人进入了视野
		/// @param nObjID	:	被通知的对象ID
		/// @param pObj		:	正在移动的对象
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void NotifyEnterEyeshot( XGameObject* pObj, xObject nObjID, VisualMode eMode );

		/////
		/// 通知格子内的所有对象,有人离开了视野
		/// @param nObjID	:	被通知的对象ID
		/// @param pObj		:	正在移动的对象
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void NotifyLeaveEyeshot( XGameObject* pObj, xObject nObjID, VisualMode eMode );

		///
		/// 对象切换格子时调用
		/// [6/25/2014] create by albert.xu
		///
		virtual xgc_void OnExchangeBlock( XGameObject* pObject, iPoint &ptOldCell, iPoint &ptNewCell );
	};

	XGC_INLINE xgc_void XGameMap::SetBlockExternIdx( iPoint ptCell, xgc_uint32 nIdx )
	{
		XGC_ASSERT_RETURN( ptCell.x >= 0 &&
			ptCell.x < mMapConf.mCellConf.cx &&
			ptCell.y >= 0 &&
			ptCell.y < mMapConf.mCellConf.cy,
			xgc_void( 0 ) );

		mpCells[ptCell.y * mMapConf.mCellConf.cx + ptCell.x].data = nIdx;
	}

	XGC_INLINE xgc_uint32 XGameMap::GetBlockExternIdx( iPoint ptCell )
	{
		XGC_ASSERT_RETURN( ptCell.x >= 0 &&
			ptCell.x < mMapConf.mCellConf.cx &&
			ptCell.y >= 0 &&
			ptCell.y < mMapConf.mCellConf.cy,
			INVALID_BLOCK_INDEX );

		return mpCells[ptCell.y * mMapConf.mCellConf.cx + ptCell.x].data;
	}

	///
	/// 设置地砖扩展信息
	/// [6/25/2014] create by albert.xu
	/// @param pExtern 扩展接口的指针
	/// @param ptCell 地砖的坐标
	/// @see iPoint
	/// @see IBlockExtern
	///
	XGC_INLINE xgc_void XGameMap::SetBlockExternInfo( xgc_uint32 nBlockExternIdx, IBlockExtern* pExtern )
	{
		XGC_ASSERT_RETURN( nBlockExternIdx < (xgc_uint32) mBlockExtern.size(), xgc_void( 0 ) );
		if( mBlockExtern[nBlockExternIdx] != pExtern )
		{
			SAFE_RELEASE( mBlockExtern[nBlockExternIdx] );
			mBlockExtern[nBlockExternIdx] = pExtern;
		}
	}

	///
	/// 设置扩展信息槽个数
	/// [6/25/2014] create by albert.xu
	/// @param pExtern 扩展接口的指针
	/// @param ptCells 地砖的坐标列表
	/// @see iPoint
	/// @see IBlockExtern
	///
	XGC_INLINE xgc_void XGameMap::SetBlockExternSize( xgc_size nBlockExternSize )
	{
		XGC_ASSERT( nBlockExternSize < INVALID_BLOCK_INDEX );
		if( nBlockExternSize < mBlockExtern.size() )
		{
			for( xgc_size i = nBlockExternSize; i < mBlockExtern.size(); ++i )
			{
				SAFE_RELEASE( mBlockExtern[i] );
			}
		}
		mBlockExtern.resize( nBlockExternSize, xgc_nullptr );
	}

	///
	/// 获取扩展信息槽个数
	/// [7/4/2014] create by albert.xu
	/// @return 扩展信息槽个数
	///
	XGC_INLINE xgc_size XGameMap::GetBlockExternSize()const
	{
		return mBlockExtern.size();
	}

	///
	/// 扩展扩展信息槽个数
	/// [7/4/2014] create by albert.xu
	/// @return 扩展信息槽个数
	///
	XGC_INLINE xgc_size XGameMap::ExpandBlockExternSize( xgc_size nExpandCount )
	{
		mBlockExtern.resize( mBlockExtern.size() + nExpandCount, xgc_nullptr );
		return mBlockExtern.size();
	}

	///
	/// 获取地砖扩展信息
	/// [6/25/2014] create by albert.xu
	/// @param ptCell 地砖的坐标
	/// @see iPoint
	///
	XGC_INLINE IBlockExtern* XGameMap::GetBlockExternInfo( iPoint ptCell )
	{
		if( ptCell.x < 0 || ptCell.y < 0 || ptCell.x > mMapConf.mCellConf.cx || ptCell.y > mMapConf.mCellConf.cy )
			return xgc_nullptr;

		return GetBlockExternInfo( GetBlockExternIdx( ptCell ) );
	}

	///
	/// 获取地砖扩展信息
	/// [6/25/2014] create by albert.xu
	/// @param nBlockExternIdx 扩展信息索引号
	///
	XGC_INLINE IBlockExtern* XGameMap::GetBlockExternInfo( xgc_uint32 nBlockExternIdx )
	{
		if( nBlockExternIdx >= mBlockExtern.size() )
			return xgc_nullptr;

		return mBlockExtern[nBlockExternIdx];
	}

	XGC_INLINE xgc_void XGameMap::SetCellBlock( xgc_int32 x, xgc_int32 y, xgc_bool bBlock /*= true*/ )
	{
		MapCell* pCell = GetCell( x, y );
		XGC_ASSERT_RETURN( pCell, xgc_void( 0 ) );

		pCell->block = bBlock;
	}

	XGC_INLINE xgc_bool XGameMap::IsCellBlock( xgc_int32 x, xgc_int32 y, xgc_bool bTestBarrier /*= true*/ )
	{
		MapCell* pCell = GetCell( x, y );
		XGC_ASSERT_RETURN( pCell, true );
		return ( bTestBarrier && pCell->barrier == 1 ) || pCell->block == 1;
	}

	///
	/// 判断是否格子是安全区
	/// [8/25/2014] create by albert.xu
	///
	XGC_INLINE xgc_bool XGameMap::IsCellSafety( xgc_int32 x, xgc_int32 y )
	{
		XGC_ASSERT_RETURN( x >= 0 && x < mMapConf.mCellConf.cx && y >= 0 && y < mMapConf.mCellConf.cy, true );
		return mpCells[y * mMapConf.mCellConf.cx + x].safe == 1;
	}

	XGC_INLINE xgc_bool XGameMap::GetCounter( xgc_lpcstr lpName, xgc_long &nValue )
	{
		FUNCTION_BEGIN;
		auto it = mMapCounter.find( lpName );
		if( it == mMapCounter.end() )
			return false;

		nValue = std::get< 2 >( it->second );
		return true;
		FUNCTION_END;
		return false;
	}

	///
	/// 删除计数器
	/// [7/28/2014] create by albert.xu
	///
	XGC_INLINE xgc_void XGameMap::DeleteCounter( xgc_lpcstr lpSourceName )
	{
		mMapCounter.erase( lpSourceName );
	}

	///
	/// 删除场景开关
	/// [7/28/2014] create by albert.xu
	///
	XGC_INLINE xgc_void XGameMap::DeleteSwitch( xgc_lpcstr lpSourceName )
	{
		mMapSwitch.erase( lpSourceName );
	}

	///
	/// 客户端场景事件触发
	/// [10/2/2014] create by albert.xu
	///
	XGC_INLINE xgc_bool XGameMap::TriggerClientEvent( xgc_uint16 nEvent, xgc_uint16 nState )
	{
		FUNCTION_BEGIN;
		auto it = mClientEvents.find( nEvent );
		
		auto title = getString( attrSceneTitle );

		XGC_ASSERT_RETURN( it != mClientEvents.end(), false, "Scene = %s, nEvent = %u", title, nEvent );

		it->second = nState;
		return true;
		FUNCTION_END;
		return false;
	}

	///
	/// 服务器场景事件触发
	/// [10/27/2014] create by albert.xu
	///
	XGC_INLINE xgc_bool XGameMap::TriggerServerEvent( xgc_uint16 nEvent, xgc_uint16 nState )
	{
		FUNCTION_BEGIN;
		auto it = mServerEvents.find( nEvent );
		auto title = getString( attrSceneTitle );

		XGC_ASSERT_RETURN( it != mServerEvents.end(), false, "Scene = %s, nEvent = %u", title, nEvent );

		it->second( this, nEvent, nState );
		return true;
		FUNCTION_END;
		return false;
	}

	///
	/// 获取视野内的对象
	/// [10/28/2014] create by albert.xu
	///
	XGC_INLINE xObjectSet XGameMap::CaptureObjectByEyeshot( const XVector3 &vCenter, const std::function< xgc_bool( xObject ) > &fnFilter /*= xgc_nullptr*/ )
	{
		iPoint ptCenter = WorldToArea( vCenter.x, vCenter.y );
		iRect rcRange( 
			ptCenter - iSize( mMapConf.mEyesight.cx, mMapConf.mEyesight.cy ), 
			ptCenter + iSize( mMapConf.mEyesight.cx, mMapConf.mEyesight.cy ) );
		return FetchObject( rcRange, fnFilter );
	}
}
#endif // _XGAME_MAP_H_