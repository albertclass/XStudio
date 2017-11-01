#include "XHeader.h"
#include "XCharactor.h"
#include "XBag.h"

namespace xgc
{
	IMPLEMENT_XCLASS_BEGIN( XBag, XObject )

	IMPLEMENT_XCLASS_END();


	xgc_bool XBag::setCapacity( xgc_uint32 nCapacity )
	{
		if( nCapacity <= mCapacity )
		{
			// ����Ҫ�ı�
			USR_ERROR( "Can not modify, capacity is small than old. mCapacity:[%d], nCapacity:[%d]", mCapacity, nCapacity );
			return false;
		}

		// �ж��µ������Ƿ���װ���ϱ����е���Ʒ��
		xgc_uint32 nCount = 0;
		for( xgc_uint32 i = 0; i < mCapacity; ++i )
		{
			if( false == mSlots[i].isEmpty() )
				++nCount;
		}

		// װ����������ʧ��
		if( nCapacity < nCount )
			return false;

		XSlot *pSlots = XGC_NEW XSlot[nCapacity];
		if( xgc_nullptr == pSlots )
		{
			SYS_ERROR( "new XSlot failed, nCapacity:[%d]",
				nCapacity );
			return false;
		}

		if( xgc_nullptr != mSlots ) // �б���������
		{
			for( xgc_uint32 n = 0; n < mCapacity && nCount ; n++ )
				pSlots[n] = mSlots[n];

			SAFE_DELETE_ARRAY( mSlots );
		}

		mSlots = pSlots;
		mCapacity = nCapacity;
	}

	///
	/// \brief �ҵ���һ��Ϊ�յĸ���
	/// \prarm nSlot: ��nSlot��ʼ����
	/// \author albert.xu
	/// \date 2017/10/19
	///
	xSlot XBag::findEmpty( xSlot nStartSlot = 0 ) const
	{
		for( xgc_uint32 i = nStartSlot; i < mCapacity; i++ )
		{
			if( mSlots[i].isEmpty() )
				return i;
		}
	}

	xSlot XBag::findSlot( xObject hGoods, xSlot nStartSlot ) const
	{
		for( xgc_uint32 i = nStartSlot; i < mCapacity; i++ )
		{
			if( mSlots[i].isThisGoods( hGoods ) )
				return i;
		}

		return INVALID_SLOT_ID;
	}

	///
	/// ���������ҳ������õĸ���
	/// @ nSlot: ��nSlot��ʼ����
	/// [8/27/2014] create by wuhailin.jerry
	///
	xSlot XBag::findSlot( std::function< xgc_bool( const XSlot&, xSlot ) > &&filter, xSlot nStartSlot /*= 0*/ ) const
	{
		for( xSlot i = nStartSlot; i < mCapacity; ++i )
		{
			if( filter( mSlots[i], i ) )
				return i;
		}

		return INVALID_SLOT_ID;
	}

	xgc_uint32 XBag::getGoodsTotalByID( xObject hGoods ) const
	{
		xgc_uint32 nCount = 0;

		for( xgc_uint32 i = 0; i < getCapacity(); ++i )
		{
			if( mSlots[i].isThisGoods( hGoods ) )
			{
				nCount += mSlots[i].getCount(); // ������Ʒ����
			}
		}

		return nCount;
	}

	xgc_uint32 XBag::getGoodsTotalByIndex( xgc_uint32 nGoodsIndex ) const
	{
		xgc_uint32 nCount = 0;

		for( xgc_uint32 i = 0; i < getCapacity(); ++i )
		{
			if( mSlots[i].isThisGoodsIndex( nGoodsIndex ) )
			{
				nCount += mSlots[i].getCount(); // ������Ʒ����
			}
		}

		return nCount;
	}

	xgc_uint32 XBag::getEmptyCount() const
	{
		xgc_uint32 nCount = 0;
		for( xSlot i = 0; i < getCapacity(); ++i )
		{
			if( mSlots[i].isEmpty() ) // û�з���Ʒ
				nCount++;
		}

		return nCount;
	}

	xgc_void XBagTrans::Cancel()
	{
		FUNCTION_BEGIN;

		for( auto &iter : m_ChangedSlotMap )
		{
			// TODO
			ENPacketType eType;
			xSlot nSlot = INVALID_SLOT_ID;
			GetChangedSlotMapKey( iter.first, eType, nSlot );

			XBag &usedBag = mBag.getBagByType( eType );

			usedBag.putSlot( nSlot, iter.second );
		}

		m_SlotOperatorList.clear();

		FUNCTION_END;
	}

	xgc_void XBagTrans::RecordOldSlot( ENPacketType eType, xSlot slot )
	{
		FUNCTION_BEGIN;

		XBag &UsedBag = mBag.getBagByType( eType );

		auto pSlot = UsedBag.GetSlotInfo( slot );

		XGC_ASSERT_RETURN( pSlot, xgc_void( 0 ) );

		auto nKey = MakeChangedSlotMapKey( eType, slot );
		auto iter = m_ChangedSlotMap.find( nKey );
		if( iter == m_ChangedSlotMap.end() ) // û�оͱ���
		{
			m_ChangedSlotMap[nKey] = *pSlot; // �мǲ���ʹ��std::move
		}

		FUNCTION_END;
	}

	xgc_bool XBagTrans::Put( xObject hGoods, xgc_uint32 mCount, xgc_uint32 nEvtType, xgc_bool bOverlap, xSlot nSlot )
	{
		FUNCTION_BEGIN;

		if( 0 == mCount )
		{
			USR_INFO( "Try putting GoodsNum is 0." );
			return true;
		}

		xgc_uint32 nRealPutNum = 0;

		auto pGoods = ObjectCast< XGoods >( hGoods );
		if( xgc_nullptr == pGoods )
			return false;

		auto findNext = [this, pGoods]( xObject hObject, xgc_bool bOverlap, xSlot nSlot )->xSlot{
			if( false == bOverlap ) 
			{
				// ���ѵ����ʹӵ�һ���ո��ӿ�ʼ
				return mBag.findEmpty( nSlot );
			}
			else
			{
				// ���ҿɶѵ��ĸ���
				return mBag.findSlot( [pGoods]( const XSlot& slot, xSlot pos ){
					if( slot.isFull() )
						return false;

					if( !slot.isThisGoods( pGoods ) )
						return false;

					return true;
				}, nSlot );
			}
		};

		xgc_uint32 nUseSlot = 0; // ʹ�õĸ���
		while( 0 != mCount )
		{
			nSlot = UsedBag.findSlot( pShGoods, nSlot, bOverLap );

			auto pstSlot = UsedBag.GetSlotInfo( nSlot );

			if( xgc_nullptr == pstSlot )
				return nRealPutNum;

			// ��Ҫ�Ա������в����ˣ�������
			RecordOldSlot( UsedBag.GetPacketType(), nSlot );

			ENPacketMode enMode;

			if( pstSlot->isEmpty() ) // �յ�
			{
				if( 0 == nUseSlot ) // ���α�����һ�η���
				{
					UsedBag.putSlot( nSlot, pShGoods );
				}
				else
				{
					UsedBag.putSlot( nSlot, pShGoods->Copy( ) ); // ����һ��
				}
				nUseSlot++;

				enMode = slot_op_new;
			}
			else
			{
				enMode = slot_op_old;
			}
			
			xgc_uint32 nFillNum = UsedBag.AddSlotGoods( nSlot, mCount );
			if( nFillNum > 0 ) // ��Ʒ����ɹ���
			{
				// ������Ʒ��Parent����Ϊ������Object
				pstSlot->pShGoods->SetParent( UsedBag.GetObjectID() );

				pstSlot->pShGoods->AddOperatorCount();

				stSlotOperatorInfo stInfo;
				stInfo.ENMode       = enMode;
				stInfo.nAfterNum    = pstSlot->mCount;
				stInfo.nOperatorNum = nFillNum;
				stInfo.eType        = UsedBag.GetPacketType();
				stInfo.nSlot        = nSlot;
				stInfo.pxGoods      = pstSlot->pShGoods;
				stInfo.bIsAdd       = true;
				stInfo.nEvtType     = nEvtType;

				if( pstSlot->mCount != nFillNum )
					stInfo.pxOldGoods = pShGoods;

				m_SlotOperatorList.push_back( std::move( stInfo ) );
			}

			pstSlot->pShGoods->AfterPutIntoBag();


			nRealPutNum += nFillNum;
			mCount -= nFillNum;
		}

		return nRealPutNum;
		FUNCTION_END;
		return 0;
	}

	xgc_uint32 XBagTrans::Remove( xGoodsPtr pShGoods, xgc_uint32 mCount, xgc_uint32 nEvtType, xSlot &nSlot )
	{
		FUNCTION_BEGIN;

		XGC_ASSERT_RETURN(pShGoods, 0);

		if( 0 == mCount )
		{
			USR_INFO( "Try Remove GoodsNum is 0." );
			return 0;
		}

		xgc_uint32 nRealRemoveNum = 0;

		XBag &UsedBag = GetPacket( pShGoods->GetUnsignedAttr( attrDbIndex ) );

		for( xgc_uint32 i = 0; i < UsedBag.GetCapacity(); i++ )
		{
			auto pstSlot = UsedBag.GetSlotInfo( i );
			
			XGC_ASSERT_RETURN( pstSlot, nRealRemoveNum );

			if( !pstSlot->IsSameGoods( pShGoods->GetObjectID() ) )
			{
				continue;
			}

			RecordOldSlot( UsedBag.GetPacketType(), i );

			xgc_int32 nSubNum = UsedBag.DelSlotGoods(i, mCount);

			if( pstSlot->isEmpty() ) // ����
			{
				UsedBag.putSlot( i, xgc_nullptr );
			}

			mCount -= nSubNum;
			nRealRemoveNum += nSubNum;

			nSlot = i;

			stSlotOperatorInfo stInfo;
			stInfo.bIsAdd       = false;
			stInfo.ENMode       = slot_op_old;
			stInfo.eType        = UsedBag.GetPacketType();
			stInfo.pxGoods      = pShGoods;
			stInfo.nSlot        = nSlot;
			stInfo.nEvtType     = nEvtType;
			stInfo.nAfterNum    = pstSlot->mCount;
			stInfo.nOperatorNum = nRealRemoveNum;

			m_SlotOperatorList.push_back( std::move( stInfo ) );

			// ֻ�۳�һ��
			break;
		}

		return nRealRemoveNum;
		FUNCTION_END
		return 0;
	}

	xgc_uint32 XBagTrans::RemoveBySlot( xSlot Slot, ENPacketType eType, xgc_uint32 mCount, xgc_uint32 nEvtType )
	{
		FUNCTION_BEGIN;

		if( 0 == mCount )
		{
			USR_INFO( "Try Remove GoodsNum is 0." );
			return 0;
		}

		XBag &UsedBag = mBag.getBagByType( eType );

		if( Slot >= UsedBag.GetCapacity() )
		{
			USR_INFO( "Slot Value is invailed." );
			return 0;
		}

		auto pstSlot = UsedBag.GetSlotInfo( Slot );

		XGC_ASSERT_RETURN( pstSlot, 0 );

		if( pstSlot->isEmpty() ) // û�з���Ʒ
		{
			return 0;
		}

		RecordOldSlot( UsedBag.GetPacketType() ,Slot );

		stSlotOperatorInfo stInfo;
		stInfo.bIsAdd   = false;
		stInfo.eType    = UsedBag.GetPacketType();
		stInfo.ENMode   = slot_op_old;
		stInfo.pxGoods  = pstSlot->pShGoods;
		stInfo.nSlot    = Slot;
		stInfo.nEvtType = nEvtType;
		
		xgc_uint32 nSubNum = UsedBag.DelSlotGoods(Slot, mCount);

		if( pstSlot->isEmpty() ) // ����
		{			
			UsedBag.putSlot( Slot, xgc_nullptr );
		}

		stInfo.nAfterNum = pstSlot->mCount;
		stInfo.nOperatorNum = nSubNum;

		m_SlotOperatorList.push_back( std::move( stInfo ) );

		return nSubNum;
		FUNCTION_END;
		return 0;
	}

	xgc_void XBagTrans::EmptySlot( xSlot nSlot )
	{
		FUNCTION_BEGIN;

		if( nSlot >= mBag.GetCapacity() )
		{
			USR_INFO( "Slot Value is invailed. nSlot:[%u], m_nCapacity:[%u]", nSlot, mBag.GetCapacity() );
			return ;
		}

		auto pstSlot = mBag.GetSlotInfo( nSlot );

		XGC_ASSERT_RETURN( pstSlot, xgc_void( 0 ) );

		if( pstSlot->isEmpty() ) // û�з���Ʒ
		{
			return ;
		}

		RecordOldSlot( mBag.GetPacketType(), nSlot );

		mBag.putSlot( nSlot, xgc_nullptr );
		mBag.putSlot( nSlot, 0 );

		return;
		FUNCTION_END;
		return;
	}

	xgc_void XBagTrans::AutoPackPacket( xgc_uint32 nEvtType, std::function< xgc_bool( const xObject ) > fnCheckGoods )
	{
		FUNCTION_BEGIN;

		auto nCapacity = mBag.GetCapacity();

		// ������
		for( xgc_uint32 i = 0; i < nCapacity; i++ )
		{
			auto pstSlot1 = mBag.GetSlotInfo( i );

			XGC_ASSERT_RETURN( pstSlot1, xgc_void( 0 ) );

			// ����Ϊ�ջ����Ѿ�����
			if( pstSlot1->isEmpty() || pstSlot1->isFull() )
			{
				continue;
			}

			for( xgc_uint32 j = i + 1; j < nCapacity; j++ )
			{
				auto pstSlot2 = mBag.GetSlotInfo( j );

				XGC_ASSERT_RETURN( pstSlot2, xgc_void( 0 ) );

				if( pstSlot2->isEmpty() || pstSlot2->isFull())
				{
					continue;
				}

				if( !pstSlot1->pShGoods->IsSameGoods( pstSlot2->pShGoods ) ) // ����ȼ�����, ��������ø����е��ж�
				{
					continue;
				}

				// ������Ժϲ�һЩ��
				Swap( i, j, nEvtType, 0, fnCheckGoods );

				// ����֮�������һ���������ˣ�������
				if( pstSlot1->isFull() )
				{
					break;
				}
			}
		}

		// ð������ --��Ʒָ��Ϊxgc_nullptr�ķŵ����ȥ
		for( xgc_uint32 i = 0; i < nCapacity; i++ )
		{
			for( xgc_uint32 j = 0; j < nCapacity - i - 1; j++ )
			{
				auto pstSlot1 = mBag.GetSlotInfo( j );
				auto pstSlot2 = mBag.GetSlotInfo(j + 1);

				XGC_ASSERT_RETURN( pstSlot1 && pstSlot2, xgc_void( 0 ) );

				xgc_bool bIsSwap = true;

				// ֻҪ�����Ϊ�ջ���û������,��ֱ������
				if( pstSlot2->isEmpty() )
				{
					continue;
				}

				// ����Ϊ��,��ǰ���GoodsIDС�ں����GoodsId,������
				if( !pstSlot1->isEmpty() && !pstSlot2->isEmpty())
				{
					if (-1 != pstSlot1->pShGoods->CompareGoods(pstSlot2->pShGoods))
					{
						continue;
					}
				}

				// ��˵��Ҫ������,��Ʒ�϶���һ��
				Swap( j, j + 1, nEvtType, 0, fnCheckGoods );
			}
		}

		FUNCTION_END;
	}

	xgc_bool XBagTrans::Swap( xSlot Slot1, xSlot Slot2, xgc_uint32 nEvtType, xgc_uint32 nNum, std::function< xgc_bool( const xObject ) > fnCheckGoods )
	{
		FUNCTION_BEGIN;

		if( Slot1 == Slot2 || Slot1 >= mBag.GetCapacity() || Slot2 >= mBag.GetCapacity() )
		{
			return false;
		}

		auto pstSlot1 = mBag.GetSlotInfo( Slot1 );
		auto pstSlot2 = mBag.GetSlotInfo( Slot2 );

		auto eType = mBag.GetPacketType();

		if (xgc_nullptr == pstSlot1 || xgc_nullptr == pstSlot2)
		{
			return false;
		}

		xgc_uint32 nNumInSlot2 = pstSlot2->mCount;

		if( nNum > nNumInSlot2 ) ///< ��Ʒ��������
		{
			return false;
		}

		if( 0 == nNum ) ///< Ϊ0����ȫ����
		{
			nNum = nNumInSlot2;
		}

		RecordOldSlot( eType, Slot1 );
		RecordOldSlot( eType, Slot2 );

		///< ��һ��Ϊ��
		if( pstSlot1->isEmpty() )
		{
			mBag.putSlot( Slot1, pstSlot2->pShGoods );
			mBag.putSlot( Slot1, pstSlot2->mCount );

			mBag.putSlot( Slot2, xgc_nullptr );
			mBag.putSlot( Slot2, 0 );

			return true;
		}

		///< ����Ʒ�ǲ���һ��
		if (pstSlot2->pShGoods)
		{
			if( pstSlot1->pShGoods->IsSameGoods( pstSlot2->pShGoods ) ) ///< ���һ���Ϳ����ܲ��ܺϲ�
			{
				if( fnCheckGoods )
				{
					if( fnCheckGoods )
					{
						if( false == fnCheckGoods( pstSlot1->pShGoods->GetObjectID() ) || fnCheckGoods( pstSlot2->pShGoods->GetObjectID() ) == false ) // �����Կ۳�
						{
							return false;
						}
					}
				}
				
				///< ���Է�
				if( pstSlot1->pShGoods->GetOverlapNum() > pstSlot1->mCount)
				{
					xgc_uint32 nLeftNum = pstSlot1->pShGoods->GetOverlapNum() - pstSlot1->mCount;
					xgc_uint32 nAddNum = Put( pstSlot2->pShGoods, nNum, nEvtType, Slot1 );
					xgc_uint32 nSubNum = RemoveBySlot( Slot2, mBag.GetPacketType(), nAddNum, nEvtType );

					if (nAddNum != nSubNum)
					{
						XGC_ASSERT_MESSAGE( false, "������Ʒʧ����." );
						return false;
					}

					return true;
				}
			}
			else
			{
				if( nNumInSlot2 != nNum || pstSlot1->pShGoods->GetUnsignedAttr(attrDbIndex) == pstSlot2->pShGoods->GetUnsignedAttr(attrDbIndex)) ///< ������Ʒ��һ�����Ͳ����Բ�ֹ�ȥ
				{
					return false;
				}
			}
		}

		///< ��������ָ��
		/*xGoodsPtr xPtrBak = pstSlot1->pShGoods->shared_from_this();
		pstSlot1->pShGoods = pstSlot2->pShGoods;
		pstSlot2->pShGoods = xPtrBak;

		///< ��������
		xgc_uint32 nNumBak = pstSlot1->nGoodsNum;
		pstSlot1->nGoodsNum = pstSlot2->nGoodsNum;
		pstSlot2->nGoodsNum = nNumBak;*/

		mBag.SwapSlot( Slot1, Slot2 );

		return true;
		FUNCTION_END;
		return false;
	}

	xgc_bool XBagTrans::Swap( xGoodsPtr pGoods1, xGoodsPtr pGoods2, xSlot &Slot1, xSlot &Slot2, xgc_uint32 nEvtType, xgc_uint32 nNum, std::function< xgc_bool( const xObject ) > fnCheckGoods )
	{
		FUNCTION_BEGIN;

		mBag.GetByGoodsPtr( pGoods1, Slot1 );
		xgc_uint32 nNumInSlot2 = mBag.GetByGoodsPtr( pGoods2, Slot2 );

		if (0 == nNum)
		{
			nNum = nNumInSlot2;
		}

		xgc_bool bRet = false;
		if( INVALID_SLOT_ID != Slot1 && INVALID_SLOT_ID != Slot2 )
		{
			bRet = Swap( Slot1, Slot2, nEvtType, nNum, fnCheckGoods );
		}

		std::swap( Slot1, Slot2 );

		return bRet;
		FUNCTION_END;
		return false;
	}
}