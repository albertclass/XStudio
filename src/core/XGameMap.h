#pragma once
#ifndef _XGAME_MAP_H_
#define _XGAME_MAP_H_

#define INVALID_BLOCK_INDEX 0x0000ffffU

//////////////////////////////////////////////////////////////////////////
// MapBlock ���ӹ���
namespace xgc
{
	/// @var ��������
	extern CORE_API xAttrIndex attrSceneIndex;
	/// @var ������ʾ��
	extern CORE_API xAttrIndex attrSceneTitle;
	/// @var ����������(ΨһID)
	extern CORE_API xAttrIndex attrSceneStrName;
	/// @var ������ͼ��
	extern CORE_API xAttrIndex attrSceneMapName;
	/// @var �������
	extern CORE_API xAttrIndex attrSceneFlags;

	class XAction;
	class XGameObject;
	///
	/// ���Ӷ���
	/// [6/25/2014] create by albert.xu
	///
	struct IBlockExtern
	{
		///
		/// ����ʱ����
		/// [6/25/2014] create by albert.xu
		/// @param pObject ���õĶ���
		///
		virtual xgc_void OnEnterBlock( XGameObject* pObject ) = 0;

		///
		/// Խ��ʱ����
		/// [6/25/2014] create by albert.xu
		/// @param pObject ���õĶ���
		///
		virtual xgc_void OnCrossBlock( XGameObject* pObject ) = 0;

		///
		/// �뿪ʱ����
		/// [6/25/2014] create by albert.xu
		/// @param pObject ���õĶ���
		///
		virtual xgc_void OnLeaveBlock( XGameObject* pObject ) = 0;

		///
		/// �ͷŶ��󣬸ú���Ӧ����ÿ���̳�������
		/// [6/25/2014] create by albert.xu
		///
		virtual xgc_void Release() = 0;
	};

	#define COLLISION_NONE	      0	  ///< ʲôҲ�����
	#define COLLISION_PATH	      1	  ///< ���·����ײ
	#define COLLISION_DEST	      2	  ///< ����յ���ײ
	#define COLLISION_NOMOVEFLAG  4   ///< ������ƶ���ʶ

	#define EYESHOTAREA_FORCEfLUSH  256   ///< ��Ұǿ��ˢ��,9-16

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

	/// ��ͼ�¼�
	struct CORE_API XGameMapEvent
	{
		/// @var �¼�
		XObjectEvent cast;
		/// @var Դ������
		xgc_lpcstr alias;

		union
		{
			struct
			{
				/// @var ������ֵ
				xgc_long val;
				/// @var ����������
				xgc_long inc;
			}counter;

			struct
			{
				/// @var ������ֵ
				xgc_long val;
			}switches;
		};
	};

	/// ��ͼ��ʼ������
	struct CORE_API MapConf
	{
		/// @var ��Ԫ���С��һ����Ԫ��ĳ��Ϳ��������굥λ��
		fSize mCellSize;

		/// @var ��Ԫ�����ã�һ�ŵ�ͼ������������������
		iSize mCellConf;

		/// @var ��Ұ�����С��һ��������ĵ�Ԫ����������
		iSize mAreaCell;

		/// @var ��Ұ���򣨳���������ӵ�е���Ұ��
		iSize mEyesight;

		/// ��Щ�ͻ�����Ϊ���������ĵ�Ϊ��0��0���㣬��Щ��Ϊ���Ͻ�Ϊ��0��0���㣬�ñ������ڱ任����ϵ
		/// @var ����任���������ĵ�������������ĵ�Ĳ�ֵ��
		fPoint mTransfrom;
	};

	///
	/// ������ͼ����
	/// ������Ҫʵ���˻�������ײ�������Ұ��������ģ��
	/// [6/25/2014] create by albert.xu
	///
	class CORE_API XGameMap : public XObjectNode
	{
		DECLARE_XCLASS();
	private:
		struct MapConf mMapConf;

		/// @var �����С�������������굥λ��
		fSize mAreaSize;

		/// @var �������ã�һ�ŵ�ͼ������������������
		iSize mAreaConf;

		/// @var ��Ұ����������
		TwiceEnterGuard	mEyeshotChecker;

		/// @var �ƶ�����������
		TwiceEnterGuard	mDynamicChecker;

		xObjectSet *mpAreas; ///< �����б�

	public:
		typedef xgc::unordered_map< xgc::string, xgc::list< xgc::tuple< xgc_long, xNotify > > > MapEventConf;
		typedef xgc::unordered_map< xgc::string, xgc::tuple< xgc_long, xgc_long, xgc_long > > MapCounter;
		typedef xgc::unordered_map< xgc::string, xgc_long > MapSwitch;
		typedef xgc::unordered_map< xgc::string, timer_h > MapTimer;
		typedef xgc::unordered_map< xgc::string, timer_h > MapClock;

	protected:
		///
		/// ��ש���ݽṹ
		/// [6/28/2014] create by albert.xu
		///
		union MapCell
		{
			struct
			{
				xgc_uint32 block : 1;		///< �赲
				xgc_uint32 barrier : 1;		///< դ��
				xgc_uint32 safe : 1;		///< ��ȫ��
				xgc_uint32 attack : 1;		///< �Ƿ�ɹ������������PK��
				xgc_uint32 unused : 2;		///< δʹ�ñ���

				xgc_uint32 ext : 2;		///< ��¼IBlockExtern������ռ�����ֽ�
				xgc_uint32 data : 24;	///< ��չ����
			};
			xgc_uint32 mask; ///< ���ڳ�ʼ��
		};

		MapCell *mpCells;	///< ��ש����
		//struct IActorMapEventHandler *mpActorEventHandler; ///< ��ɫ�¼�

		/// @var ��ͼ�¼�
		MapEventConf mMapEventConf;
		/// @var ����������
		MapCounter	mMapCounter;
		/// @var �����ڵĿ��ض���
		MapSwitch	mMapSwitch;
		/// @var �����ڵļ�ʱ��
		MapTimer	mMapTimer;
		/// @var �����ڵĶ�ʱ��
		MapClock	mMapClock;

		/// @var ��ש��չ��Ϣ
		xgc::vector< IBlockExtern* > mBlockExtern; 

		/// @var �ͻ����¼��б� < event, state >
		xgc::map< xgc_uint16, xgc_uint16 > mClientEvents;

		/// @var �������¼��б� < event, eventHandle >
		xgc::map< xgc_uint16, function< xgc_void( XGameMap*, xgc_uint16, xgc_uint16 ) > > mServerEvents;

		/// @var ��������־
		function< xgc_void( xgc_lpcstr, xgc_uint32, xObject, xObject ) > mpTriggerLogger;

		///
		/// ��ȡһ����ͼ���ӵ�ָ��
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
		/// �������캯��
		/// [6/25/2014] create by albert.xu
		///
		XGameMap();

		///
		/// ��������
		/// [6/25/2014] create by albert.xu
		///
		~XGameMap();

		/////
		/// ��ʼ����ͼ 
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
		/// \brief �������껻��Ϊ��������
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
		/// \brief �������껻��Ϊ�������꣬ȡ���ĵ�λ��
		/// [6/25/2014] create by albert.xu
		///
		XVector2 CellToWorld( xgc_int32 x, xgc_int32 y )const
		{
			return XVector2(
				( x + 0.5f )*mMapConf.mCellSize.cx - mMapConf.mTransfrom.x,
				( y + 0.5f )*mMapConf.mCellSize.cy - mMapConf.mTransfrom.y );
		}

		///
		/// \brief �������껻����Ұ����Ԫ����
		/// [10/14/2014] create by albert.xu
		///
		iPoint WorldToArea( xgc_real32 x, xgc_real32 y )const
		{
			return iPoint(
				xgc_int32( ( x + mMapConf.mTransfrom.x ) / mAreaSize.cx ),
				xgc_int32( ( y + mMapConf.mTransfrom.y ) / mAreaSize.cy ) );
		}

		///
		/// \brief �������Ԫ���������������꣨ȡ���ĵ㣩
		/// [10/14/2014] create by albert.xu
		///
		XVector2 AreaToWorld( xgc_int32 x, xgc_int32 y )const
		{
			return XVector2(
				( x + 0.5f )*mAreaSize.cx - mMapConf.mTransfrom.x,
				( y + 0.5f )*mAreaSize.cy - mMapConf.mTransfrom.y );
		}

		///
		/// \brief ������չ��������
		/// [6/25/2014] create by albert.xu
		/// \param ptCell ��ש������
		/// \see iPoint
		///
		xgc_void SetBlockExternIdx( iPoint ptCell, xgc_uint32 nIdx );

		///
		/// \brief ��ȡ��չ��������
		/// [6/25/2014] create by albert.xu
		/// \param ptCell ��ש������
		/// \see iPoint
		///
		xgc_uint32 GetBlockExternIdx( iPoint ptCell );

		///
		/// \brief ���õ�ש��չ��Ϣ
		/// [6/25/2014] create by albert.xu
		/// \param pExtern ��չ�ӿڵ�ָ��
		/// \param ptCell ��ש������
		/// \see iPoint
		/// \see IBlockExtern
		///
		xgc_void SetBlockExternInfo( xgc_uint32 nBlockExternIdx, IBlockExtern* pExtern );

		///
		/// \brief ������չ��Ϣ�۸���
		/// [6/25/2014] create by albert.xu
		/// \param pExtern ��չ�ӿڵ�ָ��
		/// \param ptCells ��ש�������б�
		/// \see iPoint
		/// \see IBlockExtern
		///
		xgc_void SetBlockExternSize( xgc_size nBlockExternSize );

		///
		/// \brief ��ȡ��չ��Ϣ�۸���
		/// [7/4/2014] create by albert.xu
		/// \return ��չ��Ϣ�۸���
		///
		xgc_size GetBlockExternSize()const;

		///
		/// \brief ��չ��չ��Ϣ�۸���
		/// [7/4/2014] create by albert.xu
		/// \return ��չ��Ϣ�۸���
		///
		xgc_size ExpandBlockExternSize( xgc_size nExpandCount );

		///
		/// \brief ��ȡ��ש��չ��Ϣ
		/// [6/25/2014] create by albert.xu
		/// \param ptCell ��ש������
		/// \see iPoint
		///
		IBlockExtern* GetBlockExternInfo( iPoint ptCell );

		///
		/// \brief ��ȡ��ש��չ��Ϣ
		/// [6/25/2014] create by albert.xu
		/// \param nBlockExternIdx ��չ��Ϣ������
		///
		IBlockExtern* GetBlockExternInfo( xgc_uint32 nBlockExternIdx );

		///
		/// \brief ���ø�������
		/// [10/14/2014] create by albert.xu
		///
		xgc_void SetCellBlock( xgc_int32 x, xgc_int32 y, xgc_bool bBlock = true );

		///
		/// [1/4/2011 Albert]
		/// \brief �����Ƿ���ײ 
		///
		xgc_bool IsCellBlock( xgc_int32 x, xgc_int32 y, xgc_bool bTestBarrier = true );

		///
		/// \brief �����Ƿ��ǰ�ȫ��
		/// [6/16/2014] create by jianglei.kinly
		///
		xgc_bool IsCellSafety( xgc_int32 x, xgc_int32 y );

		///
		/// \brief ��ͼ�Ƿ������ƶ�
		/// [9/12/2014] create by albert.xu
		///
		xgc_bool GetMoveFlag()const
		{
			return GetBit( attrSceneFlags, (xgc_size)MapFlags::eCanMove, true );
		}

		///
		/// \brief ���õ�ͼ�Ƿ������ƶ�
		/// [9/12/2014] create by albert.xu
		///
		xgc_void SetMoveFlag( xgc_bool bCanMove = true )
		{
			if( bCanMove )
				SetBit( attrSceneFlags, (xgc_size)MapFlags::eCanMove );
			else
				ClrBit( attrSceneFlags, (xgc_size)MapFlags::eCanMove );
		}

		///
		/// \brief ���������� 
		/// \author albert.xu
		/// \date 1/5/2011
		///
		xgc_void CreateCounter( xgc_lpcstr lpCounterName, xgc_long nInit, xgc_long nMax, xgc_long nMin = 0 )
		{
			auto ib = mMapCounter.insert( std::make_pair( lpCounterName, std::make_tuple( nMin, nMax, nInit ) ) );
			XGC_ASSERT( ib.second );
		}

		///
		/// \brief ���������� 
		/// \author albert.xu
		/// \date 1/5/2011
		///
		xgc_void CreateSwitch( xgc_lpcstr lpSwitchName, xgc_long nInit )
		{
			auto ib = mMapSwitch.insert( std::make_pair( lpSwitchName, nInit ) );
			XGC_ASSERT( ib.second );
		}

		///
		/// \brief ��ȡ�����������б�
		/// \author albert.xu
		/// \date 8/12/2014
		///
		const MapCounter& GetMapCounter()const
		{
			return mMapCounter;
		}

		///
		/// \brief ��ȡ�����������б�
		/// \author albert.xu
		/// \date 8/12/2014
		///
		const MapClock& GetMapClock()const
		{
			return mMapClock;
		}

		///
		/// \brief ��ȡ�����������б�
		/// [8/12/2014] create by albert.xu
		///
		const MapTimer& GetMapTimer()const
		{
			return mMapTimer;
		}

		///
		/// \brief ��������
		/// \author albert.xu
		/// \date 12/7/2012
		///
		xgc_void CreateClock( xgc_lpcstr lpName, xgc_lpcstr lpStart, xgc_lpcstr lpParam, xgc_lpcstr lpDuration );

		///
		/// \brief ������ʱ��
		/// \author albert.xu
		/// \date 12/22/2010
		///
		xgc_void CreateTimer( xgc_lpcstr lpName, xgc_lpcstr lpStart, xgc_lpcstr lpParam, xgc_lpcstr lpDuration );

		///
		/// \brief ���봥���� 
		/// \author albert.xu
		/// \date 12/22/2010
		/// \example AddEventConf( "test", evt_actor_dead, std::bind( &XGameMap::IncCounter, "counter", 1 ) );
		xgc_void AddEventConf( xgc_lpcstr lpAlias, xgc_long nEventId, const xNotify &fn )
		{
			XGC_ASSERT_RETURN( lpAlias && lpAlias[0], xgc_void( 0 ) );
			mMapEventConf[lpAlias].emplace_back( std::make_tuple( nEventId, fn ) );
		}

		///
		/// \brief ɾ��������
		/// [7/28/2014] create by albert.xu
		///
		xgc_void DeleteCounter( xgc_lpcstr lpSourceName );

		///
		/// \brief ɾ����������
		/// [7/28/2014] create by albert.xu
		///
		xgc_void DeleteSwitch( xgc_lpcstr lpSourceName );

		///
		/// \brief ɾ����ʱ��
		/// [7/28/2014] create by albert.xu
		///
		xgc_void DeleteClock( xgc_lpcstr lpSourceName );

		///
		/// \brief ɾ����ʱ��
		/// [7/28/2014] create by albert.xu
		///
		xgc_void DeleteTimer( xgc_lpcstr lpSourceName );

		///
		/// \brief ���Ӽ����� 
		/// [1/5/2011 Albert]
		///
		xgc_void IncCounter( xgc_lpcstr lpCounterName, xgc_long nInc );

		///
		/// \brief ���ü����� 
		/// [1/5/2011 Albert]
		///
		xgc_void SetCounter( xgc_lpcstr lpCounterName, xgc_long nVal );

		///
		/// \brief ��ȡ������ֵ
		/// [11/5/2014] create by albert.xu
		///
		xgc_bool GetCounter( xgc_lpcstr lpCounterName, xgc_long &nVal );

		///
		/// \brief ת������
		/// [1/5/2011 Albert]
		///
		xgc_void TurnSwitch( xgc_lpcstr lpSwitchName, xgc_long nSwitch );

		///
		/// \brief �ͻ��˳����¼�����
		/// [10/2/2014] create by albert.xu
		///
		xgc_bool TriggerClientEvent( xgc_uint16 nEvent, xgc_uint16 nState );

		///
		/// \brief �����������¼�����
		/// [10/27/2014] create by albert.xu
		///
		xgc_bool TriggerServerEvent( xgc_uint16 nEvent, xgc_uint16 nState );

		///
		/// \brief ��ȡ�ͻ����¼��б�
		/// [10/28/2014] create by albert.xu
		///
		const decltype( mClientEvents )& GetClientEvents()const
		{
			return mClientEvents;
		}

		///
		/// \brief ��ȡָ�������¼���ֵ
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
		/// \brief ����������ֵ�ͼ
		/// [8/6/2014] create by albert.xu
		///
		xgc_void OutputStringMap( xgc_lpcstr lpFileName );

		///
		/// [12/29/2010 Albert]
		/// \brief �������������ȡ����
		///
		xObjectSet* GetArea( xgc_real32 x, xgc_real32 y )const;

		///
		/// [12/29/2010 Albert]
		/// \brief ��ȡ���� 
		///
		xObjectSet* GetArea( xgc_int32 x, xgc_int32 y )const;

		///
		/// \brief �ƶ�����
		/// [8/7/2009 Albert]
		///
		xgc_bool DynamicMoveTo( XGameObject* pObject, XVector3 &vPosition, xgc_uint32 nCollistionMask = COLLISION_PATH, xgc_uintptr lpContext = 0 );

		///
		/// \brief ���Ͷ���
		/// [8/7/2009 Albert]
		///
		xgc_bool TeleportTo( XGameObject* pObject, XVector3 &vPosition, xgc_uintptr lpContext = 0 );

		///
		/// [1/5/2011 Albert]
		/// \brief ���������� 
		///
		xgc_void BuildTrigger( XGameObject* pServerObject );

		/////
		/// �ڵ�ͼ����ӳ������� 
		/// @param pObject ���뵽�����еĶ���
		/// @param vPosition �����ڳ����е�λ��
		/// [1/5/2011 Albert]
		/////
		xgc_bool Insert( XGameObject* pObject, const XVector3& vPosition );

		/////
		/// �ӳ������Ƴ����� 
		/// @param pObject �ӳ������Ƴ��Ķ���
		/// [1/5/2011 Albert]
		/////
		xgc_void Remove( XGameObject* pObject );

		/////
		/// ��Ұ�ڶ���ѡȡ
		/// ע�⣺����ָһ����Ұ���򣬶���һ��Block����Ұ�����Ƕ������Block�ļ���
		/// [3/3/2014 baomin]
		/////
		xObjectSet CaptureObjectByEyeshot( const XVector3 &vCenter, const std::function< xgc_bool( xObject ) > &fnFilter = xgc_nullptr );

		/// ------------------------------------------------ //
		/// ��ȡ�����Ķ���Բ��
		/// [4/18/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		xObjectSet CaptureObjectByCircle( const XVector3 &vCenter, xgc_uint32 dwRadius, const std::function< xgc_bool( xObject ) > &fnFilter = xgc_nullptr );

		/// ------------------------------------------------ //
		/// ��ȡ�����Ķ�������
		/// [4/18/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		xObjectSet CaptureObjectBySector( const XVector3 &vCenter, const XVector3& dir, xgc_uint32 dwRadius, xgc_uint16 wAngle, const std::function< xgc_bool( xObject ) > &fnFilter = xgc_nullptr );

		/// ------------------------------------------------ //
		/// Բ��
		/// [4/23/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		xObjectSet CaptureObjectByAnnular( const XVector3 &vCenter, xgc_uint32 dwRadiusMax, xgc_uint32 dwRadiusMin, const std::function< xgc_bool( xObject ) > &fnFilter = xgc_nullptr );

		/// ------------------------------------------------ //
		/// ���λ����нǶ����Ƶ�Բ����
		/// [4/23/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		xObjectSet CaptureObjectBySectorRing( const XVector3 &vCenter, const XVector3& dir, xgc_uint32 dwRadiusMax, xgc_uint32 dwRadiusMin, xgc_uint16 wAngle, const std::function< xgc_bool( xObject ) > &fnFilter = xgc_nullptr );

		/// ------------------------------------------------ //
		/// ��ȡ�����Ķ��󣬳�����
		/// [4/18/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		xObjectSet CaptureObjectByRectangle( const XVector3 &vCenter, xgc_uint32 dwXRadius, xgc_uint32 dwYRadius, const std::function< xgc_bool( xObject ) > &fnFilter = xgc_nullptr );

		/// ------------------------------------------------ //
		/// ��ȡ�������
		/// [4/25/2014 jianglei.kinly]
		/// ------------------------------------------------ //
		xObjectSet CaptureObjectByRectangle( const XVector3 &vCenter, const XVector3& vDir, xgc_uint32 dwRadiusMax, xgc_uint32 dwRadiusMin, const std::function< xgc_bool( xObject ) > &fnFilter = xgc_nullptr );

	protected:
		///
		/// ѡȡ�����ڳ�������
		/// [9/29/2014] create by albert.xu
		///
		xObjectSet FetchObject( iRect rc, const std::function< xgc_bool( xObject ) > &fnFilter );

		/////
		/// ��ײ���
		/// [8/3/2009 Albert]
		/// @param x0,y0 ��ʼ����
		/// @param x1,y1 ��ֹ����
		/// @param bTestBarrier �Ƿ���դ������
		/// @return �Ƿ�����ײ
		/////
		xgc_bool TestCollision( xgc_int32 x0, xgc_int32 y0, xgc_int32 &x1, xgc_int32 &y1, xgc_bool bTestBarrier = true );

		/////
		/// ������ռ���ӷ����ı�ʱ����,����ͬһ��Ұ����ʱ���ֱ�ӷ���
		/// [8/3/2009 Albert]
		/////
		xgc_void ExchangeArea( XGameObject *pObject, const iPoint &vOldPosition, const iPoint &vNewPosition, const iSize &iEyeshot, xgc_uint32 nCollistionMask = 0 );

		///
		/// ���ø����赲��Ϣ
		/// [6/28/2014] create by albert.xu
		///
		xgc_void ExchangeBlock( XGameObject *pObject, iPoint &iOldCell, iPoint &iNewCell );

		/////
		/// �����ӽڵ�����
		/// [8/3/2009 Albert]
		/////
		xgc_void OnInsertChild( XObject* pChild, xgc_lpvoid lpContext ) override;

		/////
		/// ɾ���ӽڵ�����,��ʱ������δ��ɾ��
		/// [8/3/2009 Albert]
		/////
		xgc_void OnRemoveChild( XObject* pChild ) override;

		/////
		/// ���ٵ�ͼ 
		/// [1/4/2011 Albert]
		/////
		xgc_void OnDestroy() override;

		/////
		/// ֪ͨ�����ڵ����ж���,���˽�������Ұ
		/// @param nObjID	:	��֪ͨ�Ķ���ID
		/// @param pObj		:	�����ƶ��Ķ���
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void NotifyEnterEyeshot( XGameObject* pObj, xObject nObjID, VisualMode eMode );

		/////
		/// ֪ͨ�����ڵ����ж���,�����뿪����Ұ
		/// @param nObjID	:	��֪ͨ�Ķ���ID
		/// @param pObj		:	�����ƶ��Ķ���
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void NotifyLeaveEyeshot( XGameObject* pObj, xObject nObjID, VisualMode eMode );

		///
		/// �����л�����ʱ����
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
	/// ���õ�ש��չ��Ϣ
	/// [6/25/2014] create by albert.xu
	/// @param pExtern ��չ�ӿڵ�ָ��
	/// @param ptCell ��ש������
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
	/// ������չ��Ϣ�۸���
	/// [6/25/2014] create by albert.xu
	/// @param pExtern ��չ�ӿڵ�ָ��
	/// @param ptCells ��ש�������б�
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
	/// ��ȡ��չ��Ϣ�۸���
	/// [7/4/2014] create by albert.xu
	/// @return ��չ��Ϣ�۸���
	///
	XGC_INLINE xgc_size XGameMap::GetBlockExternSize()const
	{
		return mBlockExtern.size();
	}

	///
	/// ��չ��չ��Ϣ�۸���
	/// [7/4/2014] create by albert.xu
	/// @return ��չ��Ϣ�۸���
	///
	XGC_INLINE xgc_size XGameMap::ExpandBlockExternSize( xgc_size nExpandCount )
	{
		mBlockExtern.resize( mBlockExtern.size() + nExpandCount, xgc_nullptr );
		return mBlockExtern.size();
	}

	///
	/// ��ȡ��ש��չ��Ϣ
	/// [6/25/2014] create by albert.xu
	/// @param ptCell ��ש������
	/// @see iPoint
	///
	XGC_INLINE IBlockExtern* XGameMap::GetBlockExternInfo( iPoint ptCell )
	{
		if( ptCell.x < 0 || ptCell.y < 0 || ptCell.x > mMapConf.mCellConf.cx || ptCell.y > mMapConf.mCellConf.cy )
			return xgc_nullptr;

		return GetBlockExternInfo( GetBlockExternIdx( ptCell ) );
	}

	///
	/// ��ȡ��ש��չ��Ϣ
	/// [6/25/2014] create by albert.xu
	/// @param nBlockExternIdx ��չ��Ϣ������
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
	/// �ж��Ƿ�����ǰ�ȫ��
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
	/// ɾ��������
	/// [7/28/2014] create by albert.xu
	///
	XGC_INLINE xgc_void XGameMap::DeleteCounter( xgc_lpcstr lpSourceName )
	{
		mMapCounter.erase( lpSourceName );
	}

	///
	/// ɾ����������
	/// [7/28/2014] create by albert.xu
	///
	XGC_INLINE xgc_void XGameMap::DeleteSwitch( xgc_lpcstr lpSourceName )
	{
		mMapSwitch.erase( lpSourceName );
	}

	///
	/// �ͻ��˳����¼�����
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
	/// �����������¼�����
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
	/// ��ȡ��Ұ�ڵĶ���
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