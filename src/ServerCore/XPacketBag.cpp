#include "stdafx.h"
#include "XPacketBag.h"

namespace XGC
{
	BEGIN_IMPLEMENT_XCLASS( XPacketBag, XObject, TypeXPacketBag )

	END_IMPLEMENT_XCLASS();


	XPacketBag::~XPacketBag()
	{
		for( xSlot slot = 0; slot < m_nCapacity; slot++ )
		{
			m_pSlotArray[slot].pShGoods = xgc_nullptr;
		}

		SAFE_DELETE_ARRAY( m_pSlotArray );

		m_nCapacity = 0;
		m_pSlotArray = xgc_nullptr;
	}

	xgc_void XPacketBag::SetCapacity( xgc_uint32 nCapacity )
	{
		FUNCTION_BEGIN;

		if( 0 == nCapacity || nCapacity <= m_nCapacity )
		{
			USR_ERROR( "Can not modify Capacity smaller. m_nCapacity:[%d], nCapacity:[%d]",
				m_nCapacity, nCapacity );
			return; // 不需要改变
		}

		stPackageSlot *pTmpSlot = XGC_NEW stPackageSlot[nCapacity];
		if( xgc_nullptr == pTmpSlot )
		{
			SYS_ERROR( "new stPackageSlot failed, nCapacity:[%d]",
				nCapacity );
			return;
		}

		if( xgc_nullptr != m_pSlotArray ) // 有背包的数据
		{
			for( xgc_uint32 n = 0; n < m_nCapacity; n++ )
			{
				pTmpSlot[n] = m_pSlotArray[n];
			}

			SAFE_DELETE_ARRAY( m_pSlotArray );
		}
		else // 新的
		{
			
		}

		m_pSlotArray = pTmpSlot;
		m_nCapacity = nCapacity;

		FUNCTION_END;
	}

	xSlot XPacketBag::FindSlot( xGoodsPtr pShGoods, xSlot nSlot, xgc_bool bOverLap )
	{
		FUNCTION_BEGIN;

		for( xgc_uint32 i = nSlot; i < m_nCapacity; i++ )
		{
			if (m_pSlotArray[i].CanInsert(pShGoods, bOverLap))
			{
				return i;
			}
		}

		return -1;
		FUNCTION_END;
		return -1;
	}

	xGoodsPtr XPacketBag::GetGoodsPtrByDbId( xgc_uint32 nGoodsId )
	{
		FUNCTION_BEGIN;

		XPacketBag &usedbag = getCurBag( nGoodsId );

		for( xgc_uint32 i = 0; i < usedbag.GetCapacity(); i++ )
		{
			if( usedbag.m_pSlotArray[i].IsSameDbId( nGoodsId ) )
			{
				return usedbag.m_pSlotArray[i].pShGoods;
			}
		}

		return xgc_nullptr;
		FUNCTION_END;
		return xgc_nullptr;
	}

	xGoodsPtr XPacketBag::TakeBySlot( xSlot Slot ) const
	{
		FUNCTION_BEGIN;

		xgc_uint32 nNum = 0;

		return TakeBySlot( Slot, nNum );

		FUNCTION_END;
		return xgc_nullptr;
	}

	xGoodsPtr XPacketBag::TakeBySlot( xSlot Slot, xgc_uint32 &nGoodsNum ) const
	{
		FUNCTION_BEGIN;

		nGoodsNum = 0;

		if( Slot >= m_nCapacity )
		{
			return xgc_nullptr;
		}

		stPackageSlot &stSlot = m_pSlotArray[Slot];

		nGoodsNum = stSlot.nGoodsNum; // 物品个数

		return stSlot.pShGoods;
		FUNCTION_END;
		return xgc_nullptr;
	}

	xgc_uint32 XPacketBag::GetByGoodsId( xgc_uint32 nGoodsId )
	{
		FUNCTION_BEGIN;

		xgc_int32 nCount = 0;

		XPacketBag &usedbag = getCurBag( nGoodsId );

		for( xgc_uint32 i = 0; i < usedbag.GetCapacity(); i++ )
		{
			if( usedbag.m_pSlotArray[i].IsSameDbId( nGoodsId ) )
			{
				nCount += usedbag.m_pSlotArray[i].nGoodsNum; // 加上物品个数
			}
		}

		return nCount;
		FUNCTION_END;
		return 0;
	}

	xgc_uint32 XPacketBag::GetByGoodsId( xgc_uint32 nGoodsId, xSlotOperatorMap &xSlotMap )
	{
		FUNCTION_BEGIN;

		XPacketBag &usedbag = getCurBag( nGoodsId );

		xSlotMap.clear();
		xgc_int32 nCount = 0;
		for( xgc_uint32 i = 0; i < usedbag.GetCapacity(); i++ )
		{
			if( usedbag.m_pSlotArray[i].IsSameDbId( nGoodsId ) )
			{
				nCount += usedbag.m_pSlotArray[i].nGoodsNum; // 加上物品个数

				stSlotOperatorInfo stInfo;
				stInfo.ENMode = EN_PacketGet;
				stInfo.nAfterNum = usedbag.m_pSlotArray[i].nGoodsNum;
				stInfo.nOperatorNum = 0;
				stInfo.eType = usedbag.m_PacketType;
				stInfo.nSlot = i;
				stInfo.pxGoods = usedbag.m_pSlotArray[i].pShGoods;

				xSlotMap[MakeChangedSlotMapKey(stInfo.eType, i)] = std::move( stInfo );
			}
		}

		return nCount;
		FUNCTION_END;
		return 0;
	}

	xgc_uint32 XPacketBag::GetByGoodsPtr( xGoodsPtr pShGoods, xSlot &nSlot ) const
	{
		FUNCTION_BEGIN;

		nSlot = INVALID_SLOT;
		XGC_ASSERT_RETURN( pShGoods, 0 );

		for( xgc_uint32 i = 0; i < m_nCapacity; i++ )
		{
			if (m_pSlotArray[i].IsSameGoods(pShGoods->GetObjectID()))
			{
				nSlot = i;
				return m_pSlotArray[i].nGoodsNum;
			}
		}

		return 0;
		FUNCTION_END;
		return 0;
	}

	// ------------------------------------------------ //
	// [2/4/2014 wuhailin.jerry]
	// 根据ObjectId获取物品所在格子以及物品个数
	// ------------------------------------------------ //
	xgc_uint32 XPacketBag::GetByObjectId( xObject hObject, xSlot &nSlot ) const
	{
		FUNCTION_BEGIN;

		nSlot = INVALID_SLOT;

		for( xgc_uint32 i = 0; i < m_nCapacity; i++ )
		{
			if( m_pSlotArray[i].IsSameGoods( hObject ) )
			{
				nSlot = i;
				return m_pSlotArray[i].nGoodsNum;
			}
		}

		return 0;
		FUNCTION_END;
		return 0;
	}

	xgc_uint32 XPacketBag::GetEmptySlotNum() const
	{
		xgc_uint32 nEmpyNum = 0;
		for( xSlot i = 0; i < m_nCapacity; i++ )
		{
			if( m_pSlotArray[i].IsEmpty() ) // 没有放物品
			{
				nEmpyNum++;
			}
		}

		return nEmpyNum;
	}

	xSlot XPacketBag::GetEmptySlot() const
	{
		FUNCTION_BEGIN;

		for( xSlot i = 0; i < m_nCapacity; i++ )
		{
			if( m_pSlotArray[i].IsEmpty() ) // 没有放物品
			{
				return i;
			}
		}

		return INVALID_SLOT;
		FUNCTION_END;
		return INVALID_SLOT;
	}

	xgc_void XPacketBagOperator::xTransactionCancel()
	{
		FUNCTION_BEGIN;

		for( auto &iter : m_ChangedSlotMap )
		{
			// TODO
			ENPacketType eType;
			xSlot nSlot = INVALID_SLOT;
			GetChangedSlotMapKey( iter.first, eType, nSlot );

			XPacketBag &usedBag = m_Bag.getBagByType( eType );

			usedBag.SetSlotInfo( nSlot, iter.second );
		}

		m_SlotOperatorList.clear();

		FUNCTION_END;
	}

	xgc_void XPacketBagOperator::RecordOldSlot( ENPacketType eType, xSlot slot )
	{
		FUNCTION_BEGIN;

		XPacketBag &UsedBag = m_Bag.getBagByType( eType );

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

	xgc_uint32 XPacketBagOperator::Put( xGoodsPtr pShGoods, xgc_uint32 nGoodsNum, xgc_uint32 nEvtType,xSlot nSlot, xgc_bool bOverLap )
	{
		FUNCTION_BEGIN;

		XGC_ASSERT_RETURN(pShGoods, 0);

		if( 0 == nGoodsNum )
		{
			USR_INFO( "Try putting GoodsNum is 0." );
			return 0;
		}

		xgc_uint32 nRealPutNum = 0;

		XPacketBag &UsedBag = GetPacket( pShGoods->GetUnsignedAttr( attrDbIndex ) );

		if( false == bOverLap ) // 不堆叠，就从第一个空格子开始
		{
			if( INVALID_SLOT == nSlot )
			{
				nSlot = UsedBag.GetEmptySlot();
			}
		}
		else
		{
			if( INVALID_SLOT == nSlot )
			{
				nSlot = 0;
			}
		}

		xgc_uint32 nUseSlot = 0; // 使用的格子
		while( 0 != nGoodsNum )
		{
			nSlot = UsedBag.FindSlot( pShGoods, nSlot, bOverLap );

			auto pstSlot = UsedBag.GetSlotInfo( nSlot );

			if( xgc_nullptr == pstSlot )
				return nRealPutNum;

			// 需要对背包进行操作了，备份下
			RecordOldSlot( UsedBag.GetPacketType(), nSlot );

			ENPacketMode enMode;

			if( pstSlot->IsEmpty() ) // 空的
			{
				if( 0 == nUseSlot ) // 本次背包第一次放入
				{
					UsedBag.SetSlotInfo( nSlot, pShGoods );
				}
				else
				{
					UsedBag.SetSlotInfo( nSlot, pShGoods->Copy( ) ); // 拷贝一份
				}
				nUseSlot++;

				enMode = EN_PacketNewSlot;
			}
			else
			{
				enMode = EN_PacketOldSlot;
			}
			
			xgc_uint32 nFillNum = UsedBag.AddSlotGoods( nSlot, nGoodsNum );
			if( nFillNum > 0 ) // 物品放入成功了
			{
				// 将该物品的Parent设置为背包的Object
				pstSlot->pShGoods->SetParent( UsedBag.GetObjectID() );

				pstSlot->pShGoods->AddOperatorCount();

				stSlotOperatorInfo stInfo;
				stInfo.ENMode       = enMode;
				stInfo.nAfterNum    = pstSlot->nGoodsNum;
				stInfo.nOperatorNum = nFillNum;
				stInfo.eType        = UsedBag.GetPacketType();
				stInfo.nSlot        = nSlot;
				stInfo.pxGoods      = pstSlot->pShGoods;
				stInfo.bIsAdd       = true;
				stInfo.nEvtType     = nEvtType;

				if( pstSlot->nGoodsNum != nFillNum )
					stInfo.pxOldGoods = pShGoods;

				m_SlotOperatorList.push_back( std::move( stInfo ) );
			}

			pstSlot->pShGoods->AfterPutIntoBag();


			nRealPutNum += nFillNum;
			nGoodsNum -= nFillNum;
		}

		return nRealPutNum;
		FUNCTION_END;
		return 0;
	}

	xgc_uint32 XPacketBagOperator::Remove( xGoodsPtr pShGoods, xgc_uint32 nGoodsNum, xgc_uint32 nEvtType, xSlot &nSlot )
	{
		FUNCTION_BEGIN;

		XGC_ASSERT_RETURN(pShGoods, 0);

		if( 0 == nGoodsNum )
		{
			USR_INFO( "Try Remove GoodsNum is 0." );
			return 0;
		}

		xgc_uint32 nRealRemoveNum = 0;

		XPacketBag &UsedBag = GetPacket( pShGoods->GetUnsignedAttr( attrDbIndex ) );

		for( xgc_uint32 i = 0; i < UsedBag.GetCapacity(); i++ )
		{
			auto pstSlot = UsedBag.GetSlotInfo( i );
			
			XGC_ASSERT_RETURN( pstSlot, nRealRemoveNum );

			if( !pstSlot->IsSameGoods( pShGoods->GetObjectID() ) )
			{
				continue;
			}

			RecordOldSlot( UsedBag.GetPacketType(), i );

			xgc_int32 nSubNum = UsedBag.DelSlotGoods(i, nGoodsNum);

			if( pstSlot->IsEmpty() ) // 空了
			{
				UsedBag.SetSlotInfo( i, xgc_nullptr );
			}

			nGoodsNum -= nSubNum;
			nRealRemoveNum += nSubNum;

			nSlot = i;

			stSlotOperatorInfo stInfo;
			stInfo.bIsAdd       = false;
			stInfo.ENMode       = EN_PacketOldSlot;
			stInfo.eType        = UsedBag.GetPacketType();
			stInfo.pxGoods      = pShGoods;
			stInfo.nSlot        = nSlot;
			stInfo.nEvtType     = nEvtType;
			stInfo.nAfterNum    = pstSlot->nGoodsNum;
			stInfo.nOperatorNum = nRealRemoveNum;

			m_SlotOperatorList.push_back( std::move( stInfo ) );

			// 只扣除一次
			break;
		}

		return nRealRemoveNum;
		FUNCTION_END
		return 0;
	}

	xgc_uint32 XPacketBagOperator::RemoveBySlot( xSlot Slot, ENPacketType eType, xgc_uint32 nGoodsNum, xgc_uint32 nEvtType )
	{
		FUNCTION_BEGIN;

		if( 0 == nGoodsNum )
		{
			USR_INFO( "Try Remove GoodsNum is 0." );
			return 0;
		}

		XPacketBag &UsedBag = m_Bag.getBagByType( eType );

		if( Slot >= UsedBag.GetCapacity() )
		{
			USR_INFO( "Slot Value is invailed." );
			return 0;
		}

		auto pstSlot = UsedBag.GetSlotInfo( Slot );

		XGC_ASSERT_RETURN( pstSlot, 0 );

		if( pstSlot->IsEmpty() ) // 没有放物品
		{
			return 0;
		}

		RecordOldSlot( UsedBag.GetPacketType() ,Slot );

		stSlotOperatorInfo stInfo;
		stInfo.bIsAdd   = false;
		stInfo.eType    = UsedBag.GetPacketType();
		stInfo.ENMode   = EN_PacketOldSlot;
		stInfo.pxGoods  = pstSlot->pShGoods;
		stInfo.nSlot    = Slot;
		stInfo.nEvtType = nEvtType;
		
		xgc_uint32 nSubNum = UsedBag.DelSlotGoods(Slot, nGoodsNum);

		if( pstSlot->IsEmpty() ) // 空了
		{			
			UsedBag.SetSlotInfo( Slot, xgc_nullptr );
		}

		stInfo.nAfterNum = pstSlot->nGoodsNum;
		stInfo.nOperatorNum = nSubNum;

		m_SlotOperatorList.push_back( std::move( stInfo ) );

		return nSubNum;
		FUNCTION_END;
		return 0;
	}

	xgc_void XPacketBagOperator::EmptySlot( xSlot nSlot )
	{
		FUNCTION_BEGIN;

		if( nSlot >= m_Bag.GetCapacity() )
		{
			USR_INFO( "Slot Value is invailed. nSlot:[%u], m_nCapacity:[%u]", nSlot, m_Bag.GetCapacity() );
			return ;
		}

		auto pstSlot = m_Bag.GetSlotInfo( nSlot );

		XGC_ASSERT_RETURN( pstSlot, xgc_void( 0 ) );

		if( pstSlot->IsEmpty() ) // 没有放物品
		{
			return ;
		}

		RecordOldSlot( m_Bag.GetPacketType(), nSlot );

		m_Bag.SetSlotInfo( nSlot, xgc_nullptr );
		m_Bag.SetSlotInfo( nSlot, 0 );

		return;
		FUNCTION_END;
		return;
	}

	xgc_void XPacketBagOperator::AutoPackPacket( xgc_uint32 nEvtType, std::function< xgc_bool( const xObject ) > fnCheckGoods )
	{
		FUNCTION_BEGIN;

		auto nCapacity = m_Bag.GetCapacity();

		// 先整理
		for( xgc_uint32 i = 0; i < nCapacity; i++ )
		{
			auto pstSlot1 = m_Bag.GetSlotInfo( i );

			XGC_ASSERT_RETURN( pstSlot1, xgc_void( 0 ) );

			// 格子为空或者已经满了
			if( pstSlot1->IsEmpty() || pstSlot1->IsFull() )
			{
				continue;
			}

			for( xgc_uint32 j = i + 1; j < nCapacity; j++ )
			{
				auto pstSlot2 = m_Bag.GetSlotInfo( j );

				XGC_ASSERT_RETURN( pstSlot2, xgc_void( 0 ) );

				if( pstSlot2->IsEmpty() || pstSlot2->IsFull())
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
				if( pstSlot1->IsFull() )
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
				auto pstSlot1 = m_Bag.GetSlotInfo( j );
				auto pstSlot2 = m_Bag.GetSlotInfo(j + 1);

				XGC_ASSERT_RETURN( pstSlot1 && pstSlot2, xgc_void( 0 ) );

				xgc_bool bIsSwap = true;

				// 只要后面的为空或者没有数据,就直接跳过
				if( pstSlot2->IsEmpty() )
				{
					continue;
				}

				// 都不为空,且前面的GoodsID小于后面的GoodsId,就跳过
				if( !pstSlot1->IsEmpty() && !pstSlot2->IsEmpty())
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

	xgc_bool XPacketBagOperator::Swap( xSlot Slot1, xSlot Slot2, xgc_uint32 nEvtType, xgc_uint32 nNum, std::function< xgc_bool( const xObject ) > fnCheckGoods )
	{
		FUNCTION_BEGIN;

		if( Slot1 == Slot2 || Slot1 >= m_Bag.GetCapacity() || Slot2 >= m_Bag.GetCapacity() )
		{
			return false;
		}

		auto pstSlot1 = m_Bag.GetSlotInfo( Slot1 );
		auto pstSlot2 = m_Bag.GetSlotInfo( Slot2 );

		auto eType = m_Bag.GetPacketType();

		if (xgc_nullptr == pstSlot1 || xgc_nullptr == pstSlot2)
		{
			return false;
		}

		xgc_uint32 nNumInSlot2 = pstSlot2->nGoodsNum;

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
		if( pstSlot1->IsEmpty() )
		{
			m_Bag.SetSlotInfo( Slot1, pstSlot2->pShGoods );
			m_Bag.SetSlotInfo( Slot1, pstSlot2->nGoodsNum );

			m_Bag.SetSlotInfo( Slot2, xgc_nullptr );
			m_Bag.SetSlotInfo( Slot2, 0 );

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
				if( pstSlot1->pShGoods->GetOverlapNum() > pstSlot1->nGoodsNum)
				{
					xgc_uint32 nLeftNum = pstSlot1->pShGoods->GetOverlapNum() - pstSlot1->nGoodsNum;
					xgc_uint32 nAddNum = Put( pstSlot2->pShGoods, nNum, nEvtType, Slot1 );
					xgc_uint32 nSubNum = RemoveBySlot( Slot2, m_Bag.GetPacketType(), nAddNum, nEvtType );

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

		m_Bag.SwapSlot( Slot1, Slot2 );

		return true;
		FUNCTION_END;
		return false;
	}

	xgc_bool XPacketBagOperator::Swap( xGoodsPtr pGoods1, xGoodsPtr pGoods2, xSlot &Slot1, xSlot &Slot2, xgc_uint32 nEvtType, xgc_uint32 nNum, std::function< xgc_bool( const xObject ) > fnCheckGoods )
	{
		FUNCTION_BEGIN;

		m_Bag.GetByGoodsPtr( pGoods1, Slot1 );
		xgc_uint32 nNumInSlot2 = m_Bag.GetByGoodsPtr( pGoods2, Slot2 );

		if (0 == nNum)
		{
			nNum = nNumInSlot2;
		}

		xgc_bool bRet = false;
		if( INVALID_SLOT != Slot1 && INVALID_SLOT != Slot2 )
		{
			bRet = Swap( Slot1, Slot2, nEvtType, nNum, fnCheckGoods );
		}

		std::swap( Slot1, Slot2 );

		return bRet;
		FUNCTION_END;
		return false;
	}


	stPackageSlot::stPackageSlot() 
		: pShGoods( xgc_nullptr )
		, nGoodsNum( 0 )
	{
		FUNCTION_BEGIN;

		FUNCTION_END;
	}

	stPackageSlot::stPackageSlot( xGoodsPtr pGoods, xgc_uint32 nCount /*= 1 */ ) 
		: pShGoods( pGoods )
		, nGoodsNum( nCount )
	{
		FUNCTION_BEGIN;
		XGC_ASSERT( nCount <= pGoods->GetOverlapNum() );
		if( nCount > pGoods->GetOverlapNum() )
		{
			USR_WARNING( "overlap overflow dbid = %u", pGoods->GetAttrValue< xgc_uint32 >( attrDbIndex ) );
			DumpStackFrame();
		}
		FUNCTION_END;
	}

}