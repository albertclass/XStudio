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
			// 不需要改变
			USR_ERROR( "Can not modify, capacity is small than old. mCapacity:[%d], nCapacity:[%d]", mCapacity, nCapacity );
			return false;
		}

		// 判断新的容量是否能装下老背包中的物品。
		xgc_uint32 nCount = 0;
		for( xgc_uint32 i = 0; i < mCapacity; ++i )
		{
			if( false == mSlots[i].isEmpty() )
				++nCount;
		}

		// 装不下则设置失败
		if( nCapacity < nCount )
			return false;

		XSlot *pSlots = XGC_NEW XSlot[nCapacity];
		if( xgc_nullptr == pSlots )
		{
			SYS_ERROR( "new XSlot failed, nCapacity:[%d]",
				nCapacity );
			return false;
		}

		if( xgc_nullptr != mSlots ) // 有背包的数据
		{
			for( xgc_uint32 n = 0; n < mCapacity && nCount ; n++ )
				pSlots[n] = mSlots[n];

			SAFE_DELETE_ARRAY( mSlots );
		}

		mSlots = pSlots;
		mCapacity = nCapacity;
	}

	///
	/// \brief 找到第一个为空的格子
	/// \prarm nSlot: 从nSlot开始查找
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
	/// 根据条件找出可以用的格子
	/// @ nSlot: 从nSlot开始查找
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
				nCount += mSlots[i].getCount(); // 加上物品个数
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
				nCount += mSlots[i].getCount(); // 加上物品个数
			}
		}

		return nCount;
	}

	xgc_uint32 XBag::getEmptyCount() const
	{
		xgc_uint32 nCount = 0;
		for( xSlot i = 0; i < getCapacity(); ++i )
		{
			if( mSlots[i].isEmpty() ) // 没有放物品
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
		if( iter == m_ChangedSlotMap.end() ) // 没有就保存
		{
			m_ChangedSlotMap[nKey] = *pSlot; // 切记不可使用std::move
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
				// 不堆叠，就从第一个空格子开始
				return mBag.findEmpty( nSlot );
			}
			else
			{
				// 查找可堆叠的格子
				return mBag.findSlot( [pGoods]( const XSlot& slot, xSlot pos ){
					if( slot.isFull() )
						return false;

					if( !slot.isThisGoods( pGoods ) )
						return false;

					return true;
				}, nSlot );
			}
		};

		xgc_uint32 nUseSlot = 0; // 使用的格子
		while( 0 != mCount )
		{
			nSlot = UsedBag.findSlot( pShGoods, nSlot, bOverLap );

			auto pstSlot = UsedBag.GetSlotInfo( nSlot );

			if( xgc_nullptr == pstSlot )
				return nRealPutNum;

			// 需要对背包进行操作了，备份下
			RecordOldSlot( UsedBag.GetPacketType(), nSlot );

			ENPacketMode enMode;

			if( pstSlot->isEmpty() ) // 空的
			{
				if( 0 == nUseSlot ) // 本次背包第一次放入
				{
					UsedBag.putSlot( nSlot, pShGoods );
				}
				else
				{
					UsedBag.putSlot( nSlot, pShGoods->Copy( ) ); // 拷贝一份
				}
				nUseSlot++;

				enMode = slot_op_new;
			}
			else
			{
				enMode = slot_op_old;
			}
			
			xgc_uint32 nFillNum = UsedBag.AddSlotGoods( nSlot, mCount );
			if( nFillNum > 0 ) // 物品放入成功了
			{
				// 将该物品的Parent设置为背包的Object
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

			if( pstSlot->isEmpty() ) // 空了
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

			// 只扣除一次
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

		if( pstSlot->isEmpty() ) // 没有放物品
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

		if( pstSlot->isEmpty() ) // 空了
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

		if( pstSlot->isEmpty() ) // 没有放物品
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

		// 先整理
		for( xgc_uint32 i = 0; i < nCapacity; i++ )
		{
			auto pstSlot1 = mBag.GetSlotInfo( i );

			XGC_ASSERT_RETURN( pstSlot1, xgc_void( 0 ) );

			// 格子为空或者已经满了
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

				if( !pstSlot1->pShGoods->IsSameGoods( pstSlot2->pShGoods ) ) // 不相等继续找, 这儿不能用格子中的判断
				{
					continue;
				}

				// 这儿可以合并一些了
				Swap( i, j, nEvtType, 0, fnCheckGoods );

				// 交换之后，如果上一个格子满了，就跳过
				if( pstSlot1->isFull() )
				{
					break;
				}
			}
		}

		// 冒泡排序 --物品指针为xgc_nullptr的放到最后去
		for( xgc_uint32 i = 0; i < nCapacity; i++ )
		{
			for( xgc_uint32 j = 0; j < nCapacity - i - 1; j++ )
			{
				auto pstSlot1 = mBag.GetSlotInfo( j );
				auto pstSlot2 = mBag.GetSlotInfo(j + 1);

				XGC_ASSERT_RETURN( pstSlot1 && pstSlot2, xgc_void( 0 ) );

				xgc_bool bIsSwap = true;

				// 只要后面的为空或者没有数据,就直接跳过
				if( pstSlot2->isEmpty() )
				{
					continue;
				}

				// 都不为空,且前面的GoodsID小于后面的GoodsId,就跳过
				if( !pstSlot1->isEmpty() && !pstSlot2->isEmpty())
				{
					if (-1 != pstSlot1->pShGoods->CompareGoods(pstSlot2->pShGoods))
					{
						continue;
					}
				}

				// 就说明要交换了,物品肯定不一样
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

		if( nNum > nNumInSlot2 ) ///< 物品个数不对
		{
			return false;
		}

		if( 0 == nNum ) ///< 为0就是全部动
		{
			nNum = nNumInSlot2;
		}

		RecordOldSlot( eType, Slot1 );
		RecordOldSlot( eType, Slot2 );

		///< 第一个为空
		if( pstSlot1->isEmpty() )
		{
			mBag.putSlot( Slot1, pstSlot2->pShGoods );
			mBag.putSlot( Slot1, pstSlot2->mCount );

			mBag.putSlot( Slot2, xgc_nullptr );
			mBag.putSlot( Slot2, 0 );

			return true;
		}

		///< 看物品是不是一样
		if (pstSlot2->pShGoods)
		{
			if( pstSlot1->pShGoods->IsSameGoods( pstSlot2->pShGoods ) ) ///< 如果一样就看看能不能合并
			{
				if( fnCheckGoods )
				{
					if( fnCheckGoods )
					{
						if( false == fnCheckGoods( pstSlot1->pShGoods->GetObjectID() ) || fnCheckGoods( pstSlot2->pShGoods->GetObjectID() ) == false ) // 不可以扣除
						{
							return false;
						}
					}
				}
				
				///< 可以放
				if( pstSlot1->pShGoods->GetOverlapNum() > pstSlot1->mCount)
				{
					xgc_uint32 nLeftNum = pstSlot1->pShGoods->GetOverlapNum() - pstSlot1->mCount;
					xgc_uint32 nAddNum = Put( pstSlot2->pShGoods, nNum, nEvtType, Slot1 );
					xgc_uint32 nSubNum = RemoveBySlot( Slot2, mBag.GetPacketType(), nAddNum, nEvtType );

					if (nAddNum != nSubNum)
					{
						XGC_ASSERT_MESSAGE( false, "交换物品失败了." );
						return false;
					}

					return true;
				}
			}
			else
			{
				if( nNumInSlot2 != nNum || pstSlot1->pShGoods->GetUnsignedAttr(attrDbIndex) == pstSlot2->pShGoods->GetUnsignedAttr(attrDbIndex)) ///< 两个物品不一样，就不可以拆分过去
				{
					return false;
				}
			}
		}

		///< 交换智能指针
		/*xGoodsPtr xPtrBak = pstSlot1->pShGoods->shared_from_this();
		pstSlot1->pShGoods = pstSlot2->pShGoods;
		pstSlot2->pShGoods = xPtrBak;

		///< 交换个数
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