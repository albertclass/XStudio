#include "XHeader.h"
#include "XGameObject.h"
#include "XGameMap.h"

#include "XCore.h"

namespace xgc
{
	/// @var ��������
	CORE_API xAttrIndex attrSceneIndex;
	/// @var ������ʾ��
	CORE_API xAttrIndex attrSceneTitle;
	/// @var ����������(ΨһID)
	CORE_API xAttrIndex attrSceneStrName;
	/// @var ������ͼ��(��������)
	CORE_API xAttrIndex attrSceneMapName;
	/// @var �������
	CORE_API xAttrIndex attrSceneFlags;

	IMPLEMENT_XCLASS_BEGIN( XGameMap, XObjectNode )
		IMPLEMENT_ATTRIBUTE( SceneIndex, VT_U32, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SceneTitle, VT_STRING, ATTR_FLAG_NONE, "20150211" )
		IMPLEMENT_ATTRIBUTE( SceneStrName, VT_STRING, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SceneMapName, VT_STRING, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SceneFlags, VT_U32, ATTR_FLAG_NONE, "20140912" )
	IMPLEMENT_XCLASS_END()

	//////////////////////////////////////////////////////////////////////////
	// CGameMap
	// ��ʼ����ͼ
	XGameMap::XGameMap()
		: mpCells( xgc_nullptr )
		, mpAreas( xgc_nullptr )
	{
		// ��ͼ������Ϣ
	}

	XGameMap::~XGameMap()
	{
		DestroyContainer( mBlockExtern );
	}

	xgc_void XGameMap::InitializeMap( const struct MapConf &Conf )
	{
		mMapConf = Conf;
		xgc_size nCellCount = mMapConf.mCellConf.cx * mMapConf.mCellConf.cy;
		mpCells = XGC_NEW MapCell[nCellCount];
		for( xgc_size i = 0; i < nCellCount; ++i )
		{
			mpCells[i].barrier = false;
			mpCells[i].block = false;
			mpCells[i].data = -1;
		}

		mAreaSize.cx = mMapConf.mAreaCell.cx * mMapConf.mCellSize.cx;
		mAreaSize.cy = mMapConf.mAreaCell.cy * mMapConf.mCellSize.cy;

		mAreaConf.cx = ( mMapConf.mCellConf.cx + mMapConf.mAreaCell.cx - 1 ) / mMapConf.mAreaCell.cx;
		mAreaConf.cy = ( mMapConf.mCellConf.cy + mMapConf.mAreaCell.cy - 1 ) / mMapConf.mAreaCell.cy;

		mpAreas = XGC_NEW xObjectSet[mAreaConf.cx*mAreaConf.cy];
	}

	xgc_void XGameMap::OnDestroy()
	{
		mMapConf.mCellConf.cx = 0;
		mMapConf.mCellConf.cy = 0;
		mMapConf.mCellSize.cx = 0.0f;
		mMapConf.mCellSize.cy = 0.0f;
		mMapConf.mTransfrom.x = 0.0f;
		mMapConf.mTransfrom.y = 0.0f;

		mAreaSize.cx = 0.0f;
		mAreaSize.cy = 0.0f;
		mAreaConf.cx = 0;
		mAreaConf.cy = 0;

		SAFE_DELETE_ARRAY( mpCells );
		SAFE_DELETE_ARRAY( mpAreas );

		for( auto it : mMapClock ) getTimer().remove( it.second );
		mMapClock.clear();

		for( auto it : mMapTimer ) getTimer().remove( it.second );
		mMapTimer.clear();

		mMapEventConf.clear();
	}

	/////
	/// [1/5/2011 Albert]
	/// Description:	�ڵ�ͼ����ӳ������� 
	/////
	xgc_bool XGameMap::Insert( XGameObject* pObject, const XVector3& vPosition )
	{
		if( pObject->IsInheritFrom( XGameObject::GetThisClass() ) )
		{
			iPoint ptCell = WorldToCell( vPosition.x, vPosition.y );
			if( IsCellBlock( ptCell.x, ptCell.y, false ) )
			{
				USR_ERROR( "[%s]���볡��ʧ��[%s(%f,%f)]Ϊ��Ч��", pObject->getString( attrObjectName ),
					getString( attrSceneStrName ), vPosition.x, vPosition.y );
				return false;
			}

			if( false == pObject->PreEnterMap( this ) )
			{
				return false;
			}
		}

		return XObjectNode::Insert( pObject, (xgc_lpvoid)( &vPosition ) );
	}

	/////
	/// [1/5/2011 Albert]
	/// Description:	�ӳ������Ƴ����� 
	/////
	void XGameMap::Remove( XGameObject* pObject )
	{
		if( pObject->PreLeaveMap( this ) )
		{
			XObjectNode::Remove( pObject );
		}
	}

	// �����ײ
	xgc_bool XGameMap::TestCollision( xgc_int32 x0, xgc_int32 y0, xgc_int32 &x1, xgc_int32 &y1, xgc_bool bTestBarrier /*= true*/ )
	{
		xgc_int32 x = x0, y = y0;
		if( x0 != x1 || y0 != y1 )
		{
			xgc_int32 sx = ( ( x1 - x0 ) > 0 ) ? 1 : -1;
			xgc_int32 sy = ( ( y1 - y0 ) > 0 ) ? 1 : -1;

			xgc_int32 dx = abs( x1 - x0 );
			xgc_int32 dy = abs( y1 - y0 );

			bool steep = dy > dx;
			if( steep )
			{
				std::swap( x, y );
				std::swap( dx, dy );
				std::swap( sx, sy );

				xgc_int32 e = ( dy << 1 ) - dx;

				for( xgc_int32 i = 0; i < dx; ++i )
				{
					xgc_int32 tx = x, ty = y;
					while( e >= 0 )
					{
						if( IsCellBlock( y, x + sx, bTestBarrier ) && IsCellBlock( y + sy, x, bTestBarrier ) )
						{
							x1 = ty;
							y1 = tx;
							return true;
						}

						y += sy;
						e -= ( dx << 1 );
					}
					x += sx;
					e += ( dy << 1 );

					// collision test
					if( IsCellBlock( y, x, bTestBarrier ) )
					{
						// ��x,y��Ϊ���һ�����ߵ�Cell������
						x1 = ty;
						y1 = tx;
						return true;
					}
				}
			}
			else
			{
				xgc_int32 e = ( dy << 1 ) - dx;

				for( xgc_int32 i = 0; i < dx; ++i )
				{
					xgc_int32 tx = x, ty = y;
					while( e >= 0 )
					{
						if( IsCellBlock( x, y + sy, bTestBarrier ) && IsCellBlock( x + sx, y, bTestBarrier ) )
						{
							x1 = tx;
							y1 = ty;
							return true;
						}

						y += sy;
						e -= ( dx << 1 );
					}
					x += sx;
					e += ( dy << 1 );

					// collision test
					if( IsCellBlock( x, y, bTestBarrier ) )
					{
						// ��x,y��Ϊ���һ�����ߵ�Cell������
						x1 = tx;
						y1 = ty;
						return true;
					}
				}
			}
		}
		return false;
	}

	///	Obj		:	���ƶ��Ķ���
	///	enDir	:	�ƶ�����
	///	nStep	:	�ƶ�����
	///	return	:	�Ƿ��ƶ�
	xgc_bool XGameMap::DynamicMoveTo( XGameObject* pObject, XVector3 &vPositionNew, xgc_uint32 nCollistionMask, xgc_uintptr lpContext )
	{
		XGC_CHECK_REENTER_CALL( mDynamicChecker );
		XGC_ASSERT_RETURN( pObject->GetParent() == GetObjectID(), false, "�����ڸó�����ȴ�����ڸó����ƶ���" );
		xgc_bool ret = true;

		XVector3 vPositionOld = pObject->GetPosition();

		if( !( nCollistionMask & EYESHOTAREA_FORCEfLUSH )
			&& vPositionOld == vPositionNew )
		{
			return ret;
		}

		if( !( nCollistionMask & COLLISION_NOMOVEFLAG ) &&
			( false == pObject->GetMoveFlag() || false == GetMoveFlag() ) )
		{
			return false;
		}

		iPoint iOldCell = WorldToCell( vPositionOld.x, vPositionOld.y );
		iPoint iNewCell = WorldToCell( vPositionNew.x, vPositionNew.y );

		if( nCollistionMask & COLLISION_PATH )
		{
			if( TestCollision( iOldCell.x, iOldCell.y, iNewCell.x, iNewCell.y, pObject->GetBarrierFlag() ) )
			{
				// ����Ϊ����ײ���ӵ����ĵ�
				XVector2 fPosition = CellToWorld( iNewCell.x, iNewCell.y );
				vPositionNew.x = fPosition.x;
				vPositionNew.y = fPosition.y;
				vPositionNew.z = 0.0f;

				iNewCell = WorldToCell( vPositionNew.x, vPositionNew.y );
				ret = false;
			}
		}

		if( nCollistionMask & COLLISION_DEST )
		{
			if( IsCellBlock( iNewCell.x, iNewCell.y, pObject->GetBarrierFlag() ) )
			{
				return false;
			}
		}

		iPoint iOldArea = WorldToArea( vPositionOld.x, vPositionOld.y );
		iPoint iNewArea = WorldToArea( vPositionNew.x, vPositionNew.y );

		pObject->SetPosition( vPositionNew ); //������λ��
		ExchangeBlock( pObject, iOldCell, iNewCell );
		//XGC_DEBUG_CODE( CMapBlock *pArea = GetBlock(x,y); if( pArea ) XGC_ASSERT_MSG( pArea->CheckExist(pObj->GetObjID()), "���ַɻ���" ); )
		ExchangeArea( pObject, iOldArea, iNewArea, mMapConf.mEyesight, nCollistionMask );
		//����ExchangeEyeshotArea�󴥷�
		pObject->OnMove( vPositionOld, lpContext );

		return ret;
	}

	/////
	/// ���Ͷ���
	/// [8/7/2009 Albert]
	/////
	xgc_bool XGameMap::TeleportTo( XGameObject* pObject, XVector3 &vPositionNew, xgc_uintptr lpContext )
	{
		XGC_CHECK_REENTER_CALL( mDynamicChecker );
		XGC_ASSERT_RETURN( pObject->GetParent() == GetObjectID(), false, "�����ڸó�����ȴ�����ڸó����ƶ���" );
		xgc_bool ret = true;

		XVector3 vPositionOld = pObject->GetPosition();

		iPoint iOldCell = WorldToCell( vPositionOld.x, vPositionOld.y );
		iPoint iNewCell = WorldToCell( vPositionNew.x, vPositionNew.y );


		//XGC_DEBUG_CODE( CMapBlock *pArea = GetBlock(x,y); if( pArea ) XGC_ASSERT_MSG( pArea->CheckExist(pObj->GetObjID()), "���ַɻ���" ); )
		iPoint iOldArea = WorldToArea( vPositionOld.x, vPositionOld.y );
		iPoint iNewArea = WorldToArea( vPositionNew.x, vPositionNew.y );

		pObject->OnTeleport( 0, vPositionNew, lpContext );

		ExchangeBlock( pObject, iOldCell, iPoint( -1, -1 ) );
		ExchangeArea( pObject, iOldArea, iSize( -1, -1 ), mMapConf.mEyesight );

		pObject->OnTeleport( 1, vPositionNew, lpContext );

		pObject->SetPosition( vPositionNew ); //������λ��
		ExchangeBlock( pObject, iPoint( -1, -1 ), iNewCell );
		ExchangeArea( pObject, iSize( -1, -1 ), iNewArea, mMapConf.mEyesight );

		pObject->OnTeleport( 2, vPositionOld, lpContext );

		return ret;
	}

	xgc_void XGameMap::ExchangeArea( XGameObject* pObject, const iPoint& iOldArea, const iPoint& iNewArea, const iSize& iEyeshot, xgc_uint32 nCollistionMask )
	{
		XGC_CHECK_REENTER_CALL( mEyeshotChecker );
		if( !( nCollistionMask & EYESHOTAREA_FORCEfLUSH ) && iOldArea == iNewArea )
			return;

		xObjectList leave_eyeshot_list;
		xObjectList enter_eyeshot_list;
		// �滻�ɸ��ӣ�����¸���
		for( xgc_int32 x = -iEyeshot.cx; x <= iEyeshot.cx; ++x )
		{
			for( xgc_int32 y = -iEyeshot.cy; y <= iEyeshot.cy; ++y )
			{
				if( !( nCollistionMask & EYESHOTAREA_FORCEfLUSH ) &&
					abs( iOldArea.x + x - iNewArea.x ) <= iEyeshot.cx &&
					abs( iOldArea.y + y - iNewArea.y ) <= iEyeshot.cy )
				{
					continue;
				}

				xObjectSet* pOldArea = GetArea( iOldArea.x + x, iOldArea.y + y );
				xObjectSet* pNewArea = GetArea( iNewArea.x - x, iNewArea.y - y );//��ôȷ������ǿ������µ����򣿾���Գ�

				if( pOldArea )
				{
					// ֪ͨ�뿪��Ұ
					copy( pOldArea->begin(), pOldArea->end(), std::back_inserter( leave_eyeshot_list ) );
				}

				if( pNewArea )
				{
					// ֪ͨ������Ұ
					copy( pNewArea->begin(), pNewArea->end(), std::back_inserter( enter_eyeshot_list ) );
				}
			}
		}

		// �任��������
		// ȡ���ɵĸ�������
		xObjectSet* pOldArea = GetArea( iOldArea.x, iOldArea.y );
		xObjectSet* pNewArea = GetArea( iNewArea.x, iNewArea.y );

		if( pOldArea )
		{
			xgc_size nClear = pOldArea->erase( pObject->GetObjectID() );
			XGC_ASSERT_MESSAGE( 1 == nClear, "ɾ��ʱδ�ҵ���˵�����껻�������⡣" );
		}

		if( pNewArea )
		{
			xObjectSet::_Pairib ib = pNewArea->insert( pObject->GetObjectID() );
			XGC_ASSERT_MESSAGE( ib.second, "���������������ʧ��" );
		}

		// �뿪��Ұ
		for( auto it : leave_eyeshot_list )
		{
			NotifyLeaveEyeshot( pObject, it, VisualMode::eLeave );
		}

		// ������Ұ
		for( auto it : enter_eyeshot_list )
		{
			NotifyEnterEyeshot( pObject, it, VisualMode::eEnter );
		}
		return;
	}

	//---------------------------------------------------//
	// [8/3/2009 Albert]
	// Description:	֪ͨ�����ڵ����ж���,���˽�������Ұ
	// nObjID:		��֪ͨ�Ķ���ID
	// pObj:		�����ƶ��Ķ���
	//---------------------------------------------------//
	xgc_void XGameMap::NotifyEnterEyeshot( XGameObject* pObject, xObject nObjID, VisualMode eMode )
	{
		XGameObject* pTarget = ObjectCast< XGameObject >( nObjID );
		if( !pTarget || !pObject || pTarget == pObject )
			return;

		//if( pObject->GetClassType() == 0x14611100 )
		//{
		//	DBG_INFO( "[%s:%08x]�ƶ�����[%s:%08x]����Ұ��",
		//		pObject->GetStrAttr( attrObjectName ), pObject->GetObjectID(),
		//		pTarget->GetStrAttr( attrObjectName ), pTarget->GetObjectID() );
		//}

		if( pTarget->GetBit( attrObjectFlags, XGameObject::Flag_NotifyEyeshot, true ) )
			pTarget->OnEnterEyeshot( pObject, eMode );

		if( pObject->GetBit( attrObjectFlags, XGameObject::Flag_NotifyEyeshot, true ) )
			pObject->OnEnterEyeshot( pTarget, eMode );
	}

	//---------------------------------------------------//
	// [8/3/2009 Albert]
	// Description:	֪ͨ�����ڵ����ж���,�����뿪����Ұ
	// nObjID:		��֪ͨ�Ķ���ID
	// pObj:		�����ƶ��Ķ���
	//---------------------------------------------------//
	xgc_void XGameMap::NotifyLeaveEyeshot( XGameObject* pObject, xObject nObjID, VisualMode eMode )
	{
		XGameObject* pTarget = ObjectCast< XGameObject >( nObjID );
		if( !pTarget || !pObject || pTarget == pObject )
			return;

		//if( pObject->GetClassType() == 0x14611100 )
		//{
		//	DBG_INFO( "[%s:%08x]�ƶ��뿪[%s:%08x]����Ұ��",
		//		pObject->GetStrAttr( attrObjectName ), pObject->GetObjectID(),
		//		pTarget->GetStrAttr( attrObjectName ), pTarget->GetObjectID() );
		//}

		if( pTarget->GetBit( attrObjectFlags, XGameObject::Flag_NotifyEyeshot, true ) )
			pTarget->OnLeaveEyeshot( pObject, eMode );
		//else if( pObject->IsInheritFrom( 0x14611100 ) )
		//	DBG_INFO( "[%s:%08x]��[Flag_NotifyEyeshot]=false��", pObject->GetStrAttr( attrObjectName ), pObject->GetObjectID() );

		if( pObject->GetBit( attrObjectFlags, XGameObject::Flag_NotifyEyeshot, true ) )
			pObject->OnLeaveEyeshot( pTarget, eMode );
		//else if( pTarget->IsInheritFrom( 0x14611100 ) )
		//	DBG_INFO( "[%s:%08x]��[Flag_NotifyEyeshot]=false��", pTarget->GetStrAttr( attrObjectName ), pTarget->GetObjectID() );
	}

	//---------------------------------------------------//
	// [12/29/2010 Albert]
	// Description:	 �������������ȡ����
	//---------------------------------------------------//
	inline xObjectSet* XGameMap::GetArea( float x, float y )const
	{
		xgc_int32 x0 = (xgc_int32) ( ( x - mMapConf.mTransfrom.x ) / mAreaSize.cx );
		xgc_int32 y0 = (xgc_int32) ( ( y - mMapConf.mTransfrom.y ) / mAreaSize.cy );

		return GetArea( x0, y0 );
	}


	///
	/// ����������ֵ�ͼ
	/// [8/6/2014] create by albert.xu
	///

	xgc_void XGameMap::OutputStringMap( xgc_lpcstr lpFileName )
	{
		XGC_ASSERT_RETURN( lpFileName, XGC_NONE );

		FILE* fp = xgc_nullptr;
		fp = fopen( lpFileName, "w+" );
		
		XGC_ASSERT_RETURN( fp, XGC_NONE );

		if( fp == xgc_nullptr )
			return;

		for( xgc_int32 y = 0; y < mMapConf.mCellConf.cy; ++y )
		{
			for( xgc_int32 x = 0; x < mMapConf.mCellConf.cx; ++x )
			{
				if( mpCells[y * mMapConf.mCellConf.cx + x].block )
				{
					fputc( '#', fp );
				}
				else if( mpCells[y * mMapConf.mCellConf.cx + x].barrier )
				{
					fputc( '@', fp );
				}
				else
				{
					fputc( ' ', fp );
				}
			}
			
			fputc( '\n', fp );
		}

		fclose( fp );
	}

	inline xObjectSet* XGameMap::GetArea( xgc_int32 x, xgc_int32 y )const
	{
		if( x < 0 || x >= mAreaConf.cx || y < 0 || y >= mAreaConf.cy )
			return xgc_nullptr;

		return mpAreas ? mpAreas + ( y * mAreaConf.cx + x ) : xgc_nullptr;
	}

	xgc_void XGameMap::CreateClock( xgc_lpcstr lpName, xgc_lpcstr lpStart, xgc_lpcstr lpParam, xgc_lpcstr lpDuration )
	{
		auto it = mMapClock.find( lpName );
		if( it != mMapClock.end() )
		{
			USR_WARNING( "����[%s]������ʱ��[%s]ʱ���ֶ�ʱ���Ѵ��ڡ�", getString( attrSceneStrName ), lpName );
			return;
		}

		datetime dt_start, dt_close;
		datetime now = datetime::now();

		if( _strnicmp( "relative", lpStart, 8 ) == 0 )
		{
			dt_start = datetime::relative_time( timespan::convert( lpStart + 8 ) );
		}
		else
		{
			dt_start = datetime::convert( lpStart );
		}

		timespan ts_duration = lpDuration ? timespan::convert( lpDuration ) : timespan( 0 );

		xgc_char szDateTime[64] = { 0 };
		DBG_INFO( "����%s������ʱ��%s ����ʱ��Ϊ%s", getString( attrSceneMapName ), lpName, dt_start.to_string( szDateTime, sizeof( szDateTime ) ) );

		XGameMapEvent evt;
		evt.cast.id = evt_map_clock;
		evt.cast.sender = GetObjectID();
		evt.alias = it->first.c_str();

		mMapClock[lpName] = getClock().insert(
			std::bind( (XEventBind1)&XObject::EmmitEvent, this, evt.cast ),
			dt_start, ts_duration, lpParam );
	}

	xgc_void XGameMap::CreateTimer( xgc_lpcstr lpName, xgc_lpcstr lpStart, xgc_lpcstr lpParam, xgc_lpcstr lpDuration )
	{
		auto it = mMapTimer.find( lpName );
		if( it != mMapTimer.end() )
		{
			USR_WARNING( "����[%s]������ʱ��[%s]ʱ���ֶ�ʱ���Ѵ��ڡ�", getString( attrSceneStrName ), lpName );
			return;
		}

		datetime dt_start, dt_close;
		datetime now = datetime::now();

		if( _strnicmp( "relative", lpStart, 8 ) == 0 )
		{
			dt_start = datetime::relative_time( timespan::convert( lpStart + 8 ) );
		}
		else
		{
			dt_start = datetime::convert( lpStart );
		}

		timespan ts_duration = lpDuration ? timespan::convert( lpDuration ) : timespan( 0 );

		xgc_char szDateTime[64] = { 0 };
		DBG_INFO( "����%s������ʱ��%s ����ʱ��Ϊ%s", getString( attrSceneMapName ), lpName, dt_start.to_string( szDateTime, sizeof( szDateTime ) ) );

		XGameMapEvent evt;
		evt.cast.id = evt_map_timer;
		evt.cast.sender = GetObjectID();
		evt.alias = it->first.c_str();

		mMapTimer[lpName] = getTimer().insert(
			std::bind( (XEventBind1)&XObject::EmmitEvent, this, evt.cast ),
			dt_start,
			ts_duration,
			lpParam );
	}

	///
	/// ɾ����ʱ��
	/// [7/28/2014] create by albert.xu
	///
	xgc_void XGameMap::DeleteClock( xgc_lpcstr lpSourceName )
	{
		auto iter = mMapClock.find( lpSourceName );
		if( iter != mMapClock.end() )
		{
			getTimer().remove( iter->second );
			mMapClock.erase( iter );
		}
	}

	///
	/// ɾ����ʱ��
	/// [7/28/2014] create by albert.xu
	///
	xgc_void XGameMap::DeleteTimer( xgc_lpcstr lpSourceName )
	{
		auto iter = mMapTimer.find( lpSourceName );
		if( iter != mMapTimer.end() )
		{
			getTimer().remove( iter->second );
			mMapTimer.erase( iter );
		}
	}

	xgc_void XGameMap::IncCounter( xgc_lpcstr lpCounterName, xgc_long nInc )
	{
		auto iter = mMapCounter.find( lpCounterName );
		XGC_ASSERT_RETURN( iter != mMapCounter.end(), xgc_void( 0 ), "Scene = %s, Counter = %s", getString( attrSceneTitle ), lpCounterName );

		// ExecuteTrigger��Counter���ܻᱻɾ���������ԣ�ExecuteTrigger��Ҫ�ټ���ʹ�ø�����ֵ��
		xgc_long nVal = (std::get< 2 >(iter->second) += nInc);

		XGameMapEvent evt;
		evt.alias = lpCounterName;
		evt.counter.val = nVal;
		evt.counter.inc = nInc;
	
		if( nInc != 0 )
		{
			// �����ı�
			EmmitEvent( evt_map_counter_change, evt.cast );
		}

		if( nVal <= std::get< 0 >( iter->second ) )
		{
			// ����
			EmmitEvent( evt_map_counter_underflow, evt.cast );
			USR_WARNING( "Scene = %s, Counter = %s nVal[%ld]:nInc[%ld]", getString( attrSceneTitle ), lpCounterName, nVal, nInc );
		}
		else if( nVal >= std::get< 1 >( iter->second ) )
		{
			// ����
			EmmitEvent( evt_map_counter_overflow, evt.cast );
			USR_WARNING( "Scene = %s, Counter = %s nVal[%ld]:nInc[%ld]", getString( attrSceneTitle ), lpCounterName, nVal, nInc );
		}
	}

	xgc_void XGameMap::SetCounter( xgc_lpcstr lpCounterName, xgc_long nVal )
	{
		auto iter = mMapCounter.find( lpCounterName );
		XGC_ASSERT_RETURN( iter != mMapCounter.end(), xgc_void( 0 ) );

		IncCounter( lpCounterName, nVal - std::get< 2 >( iter->second ) );
	}

	///
	/// ת������
	/// [1/5/2011 Albert]
	///
	xgc_void XGameMap::TurnSwitch( xgc_lpcstr lpSwitchName, xgc_long nSwitch )
	{
		auto it = mMapSwitch.find( lpSwitchName );
		if( it == mMapSwitch.end() )
			return;

	Switch:

		switch( nSwitch )
		{
			case -1:
			{
				// ��
				XGameMapEvent evt = { XObjectEvent(), lpSwitchName, -1 };
				EmmitEvent( evt_map_turn_off, evt.cast );
			}
			break;
			case 0:
			nSwitch = ~it->second;
			goto Switch;
			break;
			case 1:
			// ��
			{
				XGameMapEvent evt = { XObjectEvent(), lpSwitchName, +1 };
				EmmitEvent( evt_map_turn_on, evt.cast );
			}
			break;
		}

		// �����Ѿ���ɾ���ˣ�������������һ�α�֤����ҵ�
		it = mMapSwitch.find( lpSwitchName );
		if( it == mMapSwitch.end() )
			return;

		it->second = nSwitch;
	}

	///
	/// [1/5/2011 Albert]
	/// ���������� 
	///
	xgc_void XGameMap::BuildTrigger( XGameObject* pGameObject )
	{
		auto iter = mMapEventConf.find( pGameObject->getString( attrObjectAlias ) );
		if( iter == mMapEventConf.end() )
			return;

		for( auto &item : iter->second )
		{
			RegistEvent( std::get< 0 >( item ), std::bind( std::get< 1 >( item ), _1 ) );
		}
	}

	///
	/// �������ѡȡ
	/// ע�⣺����ָһ����Ұ���򣬶���һ��Block����Ұ�����Ƕ������Block�ļ���
	/// [3/3/2014 baomin]
	///
	xObjectSet XGameMap::FetchObject( iRect rc, const std::function< xgc_bool( xObject ) > &fnFilter )
	{
		xObjectSet objects;

		rc.NormalizeRect();

		if( rc.top < 0 )    rc.top = 0;
		if( rc.bottom < 0 ) rc.bottom = 0;
		if( rc.left < 0 )   rc.left = 0;
		if( rc.right < 0 )  rc.right = 0;
		if( rc.bottom >= mAreaConf.cy ) rc.bottom = mAreaConf.cy - 1;
		if( rc.right >= mAreaConf.cx ) rc.right = mAreaConf.cx - 1;

		for( xgc_int32 y = rc.top; y <= rc.bottom; ++y )
		{
			for( xgc_int32 x = rc.left; x <= rc.right; ++x )
			{
				auto pAreaSet = GetArea( x, y );

				if( xgc_nullptr == pAreaSet )
					continue;

				for( auto it = pAreaSet->begin(); it != pAreaSet->end(); )
				{
					XGameObject* pObj = ObjectCast<XGameObject>( *it );
					XGC_ASSERT_MESSAGE( pObj, "����ɾ������δ������(%p,%08X)", pObj, *it );

					if( pObj )
					{
						//������Χ�� Ҳ������˺���ʱ
						if( xgc_nullptr == fnFilter || ( fnFilter && fnFilter( *it ) ) )
						{
							objects.insert( *it );
						}
					}
					++it;
				}
			}
		}
		return std::move( objects );
	}

	/// ------------------------------------------------ //
	/// ������
	/// [4/18/2014 jianglei.kinly]
	/// ------------------------------------------------ //
	xObjectSet XGameMap::CaptureObjectByRectangle( const XVector3& vCenter, xgc_uint32 dwXRadius, xgc_uint32 dwYRadius, const std::function< xgc_bool( xObject ) > &fnFilter )
	{
		XVector3 leftTop( vCenter.x - dwXRadius, vCenter.y - dwYRadius, 0 );
		XVector3 rightDown( vCenter.x + dwXRadius, vCenter.y + dwYRadius, 0 );

		auto fn = [&]( xObject hObject )->xgc_bool {
			if( xgc_nullptr == fnFilter || fnFilter( hObject ) )
			{
				XGameObject* pObj = ObjectCast<XGameObject>( hObject );
				XGC_ASSERT_RETURN( pObj, false );

				return
					pObj->GetPosX() >= leftTop.x &&
					pObj->GetPosY() >= leftTop.y &&
					pObj->GetPosX() <= rightDown.x &&
					pObj->GetPosY() <= rightDown.y;
			}

			return false;
		};

		iRect rc(
			WorldToArea( vCenter.x - dwXRadius, vCenter.y - dwYRadius ),
			WorldToArea( vCenter.x + dwXRadius, vCenter.y + dwYRadius ) );

		return FetchObject( rc, fn );
	}

	/// ------------------------------------------------ //
	/// [4/25/2014 jianglei.kinly]
	/// ������Σ�subCenter��
	/// ------------------------------------------------ //
	xObjectSet XGameMap::CaptureObjectByRectangle( const XVector3& vCenter, const XVector3& vDir, xgc_uint32 dwRadiusMax, xgc_uint32 dwRadiusMin, const std::function< xgc_bool( xObject ) > &fnFilter/* = xgc_nullptr*/ )
	{
		/*
		0,0
		-------------------------------x
		|
		|        ------
		|       /    /
		|      /    /
		|     ---|--
		|       subCenter
		|    б����dir�ķ���dwRadiusMax
		|    ������dwRadiusMin
		y
		*/
		// ȡ���ĵ�����
		XVector3 newDirect = vDir * ( dwRadiusMax * 1.0f );
		XVector3 newCenter = vCenter + newDirect;

		// ȡ�Խ���ֱ��
		xgc_uint32 dwDiameter = xgc_uint32( XMath::Sqrt( XMath::Pow( 1.0f * dwRadiusMax, 2.0f ) + XMath::Pow( 1.0f * dwRadiusMin, 2.0f ) ) );

		auto fn = [&]( xObject hObject )->xgc_bool {
			if( xgc_nullptr == fnFilter || fnFilter( hObject ) )
			{
				XGameObject* pObj = ObjectCast<XGameObject>( hObject );
				XGC_ASSERT_RETURN( pObj, false );

				// ȡ�Һ�Ŀ�����ߵ�����
				XVector3 subVec = pObj->GetPosition() - vCenter;
				// �жϼнǣ��ҵĳ������Һ�Ŀ�����ߵ����� > 0 ����н���0-90��Χ�ڣ�
				auto fTemp = vDir.DotProduct( subVec );
				if( fTemp < 0 )
					return false;
				// ������ͶӰ���ж��Ƿ��ڷ��������
				XVector3 vecy = vDir * fTemp;
				XVector3 vecx = subVec - vecy;
				return !( vecy.Length() > dwRadiusMax || vecx.Length() > dwRadiusMin / 2 );
			}

			return false;
		};

		iRect rc(
			WorldToArea( vCenter.x - dwDiameter / 2, vCenter.y - dwDiameter / 2 ),
			WorldToArea( vCenter.x + dwDiameter / 2, vCenter.y + dwDiameter / 2 ) );
		return FetchObject( rc, fn );
	}

	/// ------------------------------------------------ //
	/// [4/18/2014 jianglei.kinly]
	/// Բ��
	/// ------------------------------------------------ //
	xObjectSet XGameMap::CaptureObjectByCircle( const XVector3& vCenter, xgc_uint32 dwRadius, const std::function< xgc_bool( xObject ) > &fnFilter/* = xgc_nullptr*/ )
	{
		auto fn = [&]( xObject hObject )->xgc_bool {
			if( xgc_nullptr == fnFilter || fnFilter( hObject ) )
			{
				XGameObject* pObj = ObjectCast<XGameObject>( hObject );
				XGC_ASSERT_RETURN( pObj, false );

				return dwRadius * dwRadius * 1.0f > ( pObj->GetPosition() - vCenter ).SqurLength();
			}

			return false;
		};

		iRect rc(
			WorldToArea( vCenter.x - dwRadius, vCenter.y - dwRadius ),
			WorldToArea( vCenter.x + dwRadius, vCenter.y + dwRadius ) );

		// ȡ����������
		return FetchObject( rc, fn );
	}

	/// ------------------------------------------------ //
	/// [4/18/2014 jianglei.kinly]
	/// ����
	/// ------------------------------------------------ //
	xObjectSet XGameMap::CaptureObjectBySector( const XVector3& vCenter, const XVector3& vDir, xgc_uint32 dwRadius, xgc_uint16 wAngle, const std::function< xgc_bool( xObject ) > &fnFilter/* = xgc_nullptr*/ )
	{
		// ������ת����
		XVector3 vRotate = vDir.RotateZ( DEG2RAD( wAngle / 2 ) );
		xgc_real32 fDotValue = vDir.DotProduct( vRotate );
		auto fn = [&]( xObject hObject )->xgc_bool {
			if( xgc_nullptr == fnFilter || fnFilter( hObject ) )
			{
				XGameObject* pObj = ObjectCast<XGameObject>( hObject );
				XGC_ASSERT_RETURN( pObj, false );

				XVector3 vDistance = pObj->GetPosition() - vCenter;
				if( vDistance.SqurLength() > dwRadius * dwRadius * 1.0f )
					return false;

				if( vDir.DotProduct( vDistance ) < fDotValue )
					return false;

				return true;
			}

			return false;
		};

		iRect rc(
			WorldToArea( vCenter.x - dwRadius, vCenter.y - dwRadius ),
			WorldToArea( vCenter.x + dwRadius, vCenter.y + dwRadius ) );
		return FetchObject( rc, fn );
	}

	/// ------------------------------------------------ //
	/// [4/23/2014 jianglei.kinly]
	/// Բ��
	/// ------------------------------------------------ //
	xObjectSet XGameMap::CaptureObjectByAnnular( const XVector3& vCenter, xgc_uint32 dwRadiusMax, xgc_uint32 dwRadiusMin, const std::function< xgc_bool( xObject ) > &fnFilter/* = xgc_nullptr*/ )
	{
		auto fn = [&]( xObject hObject )->xgc_bool {
			if( xgc_nullptr == fnFilter || fnFilter( hObject ) )
			{
				XGameObject* pObj = ObjectCast<XGameObject>( hObject );
				XGC_ASSERT_RETURN( pObj, false );

				// �������
				xgc_real32 fSqurDistance = ( pObj->GetPosition() - vCenter ).SqurLength();

				if( fSqurDistance > dwRadiusMax * dwRadiusMax * 1.0f )
					return false;

				if( fSqurDistance < dwRadiusMin * dwRadiusMin * 1.0f )
					return false;

				return true;
			}
			return false;
		};

		// ȡ����������
		iRect rc(
			WorldToArea( vCenter.x - dwRadiusMax, vCenter.y - dwRadiusMax ),
			WorldToArea( vCenter.x + dwRadiusMax, vCenter.y + dwRadiusMax ) );
		return FetchObject( rc, fn );
	}

	/// ------------------------------------------------ //
	/// [4/23/2014 jianglei.kinly]
	/// ���λ�
	/// ------------------------------------------------ //
	xObjectSet XGameMap::CaptureObjectBySectorRing( const XVector3& vCenter, const XVector3& vDir, xgc_uint32 dwRadiusMax, xgc_uint32 dwRadiusMin, xgc_uint16 wAngle, const std::function< xgc_bool( xObject ) > &fnFilter/* = xgc_nullptr*/ )
	{
		xgc_real32 fDotValue = vDir.RotateZ( DEG2RAD( wAngle / 2 ) ).DotProduct( vDir );

		auto fn = [&]( xObject hObject )->xgc_bool {
			if( xgc_nullptr == fnFilter || fnFilter( hObject ) )
			{
				XGameObject* pObj = ObjectCast<XGameObject>( hObject );
				XGC_ASSERT_RETURN( pObj, false );

				// �������
				XVector3 vDistance = pObj->GetPosition() - vCenter;
				xgc_real32 fSqurDistance = vDistance.SqurLength();

				if( fSqurDistance > dwRadiusMax * dwRadiusMax * 1.0f )
					return false;

				if( fSqurDistance < dwRadiusMin * dwRadiusMin * 1.0f )
					return false;

				// �жϼн�(dir���ǵ�λ����)
				if( vDir.DotProduct( vDistance ) < fDotValue )
					return false;

				return true;
			}

			return false;
		};

		iRect rc(
			WorldToArea( vCenter.x - dwRadiusMax, vCenter.y - dwRadiusMax ),
			WorldToArea( vCenter.x + dwRadiusMax, vCenter.y + dwRadiusMax ) );

		return FetchObject( rc, fn );
	}

	xgc_void XGameMap::ExchangeBlock( XGameObject* pObject, iPoint& iOldCell, iPoint& iNewCell )
	{
		FUNCTION_BEGIN;
		if( pObject->GetBarrierFlag() )
		{
			if( iOldCell.x >= 0 && iOldCell.x < mMapConf.mCellConf.cx &&
				iOldCell.y >= 0 && iOldCell.y < mMapConf.mCellConf.cy )
			{
				mpCells[iOldCell.y * mMapConf.mCellConf.cx + iOldCell.x].barrier = 0;
			}

			if( iNewCell.x >= 0 && iNewCell.x < mMapConf.mCellConf.cx &&
				iNewCell.y >= 0 && iNewCell.y < mMapConf.mCellConf.cy )
			{
				mpCells[iNewCell.y * mMapConf.mCellConf.cx + iNewCell.x].barrier = 1;
			}
		}
		OnExchangeBlock( pObject, iOldCell, iNewCell );
		FUNCTION_END;
	}

	void XGameMap::OnExchangeBlock( XGameObject* pObject, iPoint& ptOldCell, iPoint& ptNewCell )
	{
		if( mBlockExtern.size() )
		{
			IBlockExtern *pOld, *pNew;
			if( ( pOld = GetBlockExternInfo( ptOldCell ) ) != ( pNew = GetBlockExternInfo( ptNewCell ) ) )
			{
				if( pOld ) pOld->OnLeaveBlock( pObject );
				if( pNew ) pNew->OnEnterBlock( pObject );
			}
		}
	}

	xgc_void XGameMap::OnInsertChild( XObject* pChild, xgc_lpvoid lpContext )
	{
		if( pChild->IsInheritFrom( &XGameObject::GetThisClass() ) )
		{
			XGameObject* pObject = static_cast<XGameObject*>( pChild );
			BuildTrigger( pObject );

			pObject->SetPosition( *(XVector3*) lpContext );
			pObject->OnBornInMap( this );

			const XVector3& vPosition = pObject->GetPosition();
			// ���뵽�����б���
			ExchangeBlock( pObject, iPoint( -1, -1 ), WorldToCell( vPosition.x, vPosition.y ) );
			ExchangeArea( pObject, iPoint( -1, -1 ), WorldToArea( vPosition.x, vPosition.y ), mMapConf.mEyesight );

			pObject->OnEnterMap( this );
		}
	}

	xgc_void XGameMap::OnRemoveChild( XObject* pChild )
	{
		if( pChild->IsInheritFrom( &XGameObject::GetThisClass() ) )
		{
			XGameObject* pObject = static_cast<XGameObject*>( pChild );
			XVector3 vPosition = pObject->GetPosition();
			ExchangeBlock( pObject, WorldToCell( vPosition.x, vPosition.y ), iPoint( -1, -1 ) );
			ExchangeArea( pObject, WorldToArea( vPosition.x, vPosition.y ), iPoint( -1, -1 ), mMapConf.mEyesight );

			pObject->OnLeaveMap( this );
			pObject->RemoveEvent( GetObjectID() );
		}
	}
}
