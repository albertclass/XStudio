#include "XHeader.h"
#include "XActor.h"
#include "XBag.h"

namespace xgc
{
	IMPLEMENT_XCLASS_BEGIN( XBag, XObject )

	IMPLEMENT_XCLASS_END();

	///
	/// \brief �������� 
	/// \date 12/27/2017
	/// \author albert.xu
	///
	XBag::XBag( xgc_uint32 nType, xgc_uint32 nCapacity )
		: mType( nType )
		, mCapacity( nCapacity )
		, mSlots( xgc_nullptr )
	{

	}

	///
	/// \brief �������� 
	/// \date 12/27/2017
	/// \author albert.xu
	///
	XBag::~XBag()
	{
		for( xSlot slot = 0; slot < mCapacity; slot++ )
			mSlots[slot].setEmpty();

		SAFE_DELETE_ARRAY( mSlots );
		mCapacity = 0;
	}

	xgc_bool XBag::setCapacity( xgc_uint32 nCapacity )
	{
		if( nCapacity <= mCapacity )
		{
			// ����Ҫ�ı�
			USR_ERR( "Can not modify, capacity is small than old. mCapacity:[%d], nCapacity:[%d]", mCapacity, nCapacity );
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
			SYS_ERR( "new XSlot failed, nCapacity:[%d]",
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

		return true;
	}

	///
	/// \brief �ҵ���һ��Ϊ�յĸ���
	/// \prarm nSlot: ��nSlot��ʼ����
	/// \author albert.xu
	/// \date 2017/10/19
	///
	xSlot XBag::findEmpty( xSlot nStartSlot /*= 0*/ ) const
	{
		for( xgc_uint32 i = nStartSlot; i < mCapacity; i++ )
		{
			if( mSlots[i].isEmpty() )
				return i;
		}

		return INVALID_SLOT_ID;
	}

	xSlot XBag::findSlot( xObject hGoods, xSlot nStartSlot ) const
	{
		for( xgc_uint32 i = nStartSlot; i < mCapacity; i++ )
		{
			if( mSlots[i].isThisGoods( hGoods ) || mSlots[i].isEmpty() )
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

	///
	/// \brief ��ȡ��������Ʒ������ 
	/// \author wuhailin.jerry
	/// \date [8/27/2014]
	///
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
			if( mSlots[i].isSameGoods( nGoodsIndex ) )
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

	xgc_void XBag::Sort( const std::function< xgc_bool( const XSlot&, const XSlot& ) > &fnCompair, xgc_bool bOverlap )
	{
		FUNCTION_BEGIN;

		// ������
		std::sort( mSlots, mSlots + mCapacity, fnCompair );

		// �ٹ鲢
		xgc_uint32 i = 0, j = 1;

		while( j < mCapacity )
		{
			auto pSlot1 = getSlot( i );
			auto pSlot2 = getSlot( j );

			if( pSlot1->isFull() )
			{
				++i;
				continue;
			}

			if( pSlot2->isEmpty() )
			{
				++j;
				continue;
			}

			if( pSlot1 == pSlot2 )
			{
				++j;
				continue;
			}

			if( pSlot1->isEmpty() )
			{
				// ��slot2�е���Ʒ��slot1���������Ϊֹ��
				pSlot2->delGoods( pSlot1->putGoods( pSlot2->getGoodsId(), pSlot2->getCount() ) );
				if( pSlot2->isEmpty() )
					++j;
			}
			else if( pSlot1->getGoodsId() == pSlot2->getGoodsId() )
			{
				// ��slot2�е���Ʒ��slot1���������Ϊֹ��
				pSlot2->delGoods( pSlot1->putGoods( pSlot2->getGoodsId(), pSlot2->getCount() ) );
				if( pSlot2->isEmpty() )
					++j;
			}
			else
			{
				++i;
			}
		}
		FUNCTION_END;
	}

	///
	/// \brief ���������� 
	/// \date 11/6/2017
	/// \author xufeng04
	/// \param ����Ŀ��
	///
	XBagTrans::XBagTrans( XBag &Bag ) 
		: mBag( Bag )
		, mSequence( 0x80000000 )
	{

	}

	///
	/// \brief ������������ 
	/// \date 11/6/2017
	/// \author xufeng04
	///

	XBagTrans::~XBagTrans()
	{
		Cancel();
	}

	///
	/// \brief ȷ������������� 
	/// \authro albert.xu
	/// \date 2017/11/06
	///
	xgc_void XBagTrans::Commit()
	{
		// ���ݱ�����λ������һ�������б����б��¼�˴�λ��ÿһ���ı仯
		xgc::map< xgc::tuple< xObject, xSlot >, xgc::list< stSlotCommand* > > mapUpdate;

		// ������Ʒ����һ�������б�
		xgc::map< xgc::tuple< xObject, xObject >, xgc_long > mapOperator;

		// ���������еĲ������й鲢
		for( auto &cmd : mSlotCommands )
		{
			// ����ͬλ�õĲ������кϲ�����
			mapUpdate[{cmd.hBag, cmd.nSlot}].push_back( &cmd );

			// ����ÿһ������������Ʒ�������������
			auto it = mapOperator.find( {cmd.hBag, cmd.After.hGoods} );
			if( it == mapOperator.end() )
			{
				auto ib = mapOperator.insert( { { cmd.hBag, cmd.After.hGoods }, 0 } );
				if( false == ib.second )
					break;

				it = ib.first;
			}
					
			it->second += cmd.After.nCount - cmd.Befor.nCount;
		}

		XBag::SlotEvent evt1;

		// �ύ�����¼�
		for( auto &upd : mapUpdate )
		{
			auto &lst = upd.second;

			auto pBag = ObjectCast< XBag >( std::get< 0 >( upd.first ) );
			evt1.nSlot = std::get< 1 >( upd.first );

			auto top = lst.front();
			evt1.Befor.hGoods = top->Befor.hGoods;
			evt1.Befor.nCount = top->Befor.nCount;

			auto end = lst.back();
			evt1.After.hGoods = top->After.hGoods;
			evt1.After.nCount = top->After.nCount;

			pBag->EmmitEvent( pBag->InitEvent( evt1.cast, evt_bag_update) );
		}

		// �ύ�����¼�
		XBag::TransEvent evt2;
		for( auto &chg : mapOperator )
		{
			evt2.hBag = std::get< 0 >( chg.first );
			evt2.hGoods = std::get< 1 >( chg.first );
			evt2.nCount = chg.second;

			auto pBag = ObjectCast< XBag >( evt2.hBag );
			if( pBag )
				pBag->EmmitEvent( pBag->InitEvent( evt2.cast, evt_bag_change ) );
		}

		mSlotCommands.clear();
	}

	///
	/// \brief ����ȡ�� 
	/// \authro albert.xu
	/// \date 2017/11/06
	///
	xgc_void XBagTrans::Cancel()
	{
		while( !mSlotCommands.empty() )
			Undo();
	}

	xgc_bool XBagTrans::Put( xObject hGoods, xgc_uint32 nCount, xSlot nStartSlot )
	{
		FUNCTION_BEGIN;

		if( 0 == nCount )
		{
			USR_TIP( "Try putting GoodsNum is 0." );
			return true;
		}

		stSlotCommand stCmd;
		// �������
		stCmd.nSequence = mSequence++;
		// ��������
		stCmd.eOperator = slot_op_put;
		// Ŀ�����
		stCmd.hBag = mBag.GetObjectID();

		// ��ʼ������Ʒ������
		for( xSlot nSlot = nStartSlot; nCount; ++nSlot )
		{
			nSlot = mBag.findSlot( hGoods, nSlot );
			if( nSlot == INVALID_SLOT_ID )
				return false;

			auto pSlot = mBag.getSlot( nSlot );
			if( pSlot == xgc_nullptr )
				return false;

			if( pSlot->isFull() )
				continue;
			
			stCmd.nSlot = nSlot;

			stCmd.Befor.hGoods = pSlot->getGoodsId();
			stCmd.Befor.nCount = pSlot->getCount();

			nCount -= pSlot->putGoods( hGoods, nCount );

			stCmd.After.hGoods = pSlot->getGoodsId();
			stCmd.After.nCount = pSlot->getCount();

			mSlotCommands.push_back( stCmd );
		}

		return true;
		FUNCTION_END;
		return false;
	}

	xgc_bool XBagTrans::Del( xObject hGoods, xgc_uint32 nCount, xSlot nStartSlot )
	{
		FUNCTION_BEGIN;

		if( 0 == nCount )
		{
			USR_TIP( "Try putting GoodsNum is 0." );
			return true;
		}

		stSlotCommand stCmd;
		// �������
		stCmd.nSequence = mSequence++;
		// ��������
		stCmd.eOperator = slot_op_del;
		// Ŀ�����
		stCmd.hBag = mBag.GetObjectID();

		// ��ʼ�Ӱ���ɾ����Ʒ
		for( xSlot nSlot = nStartSlot; nCount; ++nSlot )
		{
			nSlot = mBag.findSlot( hGoods, nSlot );
			if( nSlot == INVALID_SLOT_ID )
				return false;

			auto pSlot = mBag.getSlot( nSlot );
			if( pSlot == xgc_nullptr )
				return false;

			if( pSlot->isEmpty() )
				continue;

			stCmd.nSlot = nSlot;

			stCmd.Befor.hGoods = pSlot->getGoodsId();
			stCmd.Befor.nCount = pSlot->getCount();

			nCount -= pSlot->delGoods( nCount );

			stCmd.After.hGoods = pSlot->getGoodsId();
			stCmd.After.nCount = pSlot->getCount();

			mSlotCommands.push_back( stCmd );
		}

		return true;
		FUNCTION_END
		return false;
	}

	xgc_bool XBagTrans::Swap( xSlot nFrom, xSlot nTo, xObject hTarget )
	{
		FUNCTION_BEGIN;

		XGC_ASSERT_RETURN( nFrom != nTo, false );

		XGC_ASSERT_RETURN( nFrom < mBag.getCapacity() && nTo < mBag.getCapacity(), false );

		auto pSlot1 = mBag.getSlot( nFrom );
		auto pSlot2 = mBag.getSlot( nTo );

		stSlotCommand stCmd;
		stCmd.nSequence = mSequence++;
		stCmd.eOperator = slot_op_swap;

		stCmd.hBag = mBag.GetObjectID();

		stCmd.Befor.hGoods = pSlot1->getGoodsId();
		stCmd.Befor.nCount = pSlot1->getCount();

		stCmd.After.hGoods = pSlot2->getGoodsId();
		stCmd.After.nCount = pSlot2->getCount();

		stCmd.nSlot = nFrom;
		mSlotCommands.push_back( stCmd );

		stCmd.hBag = hTarget;

		stCmd.Befor.hGoods = pSlot2->getGoodsId();
		stCmd.Befor.nCount = pSlot2->getCount();

		stCmd.After.hGoods = pSlot1->getGoodsId();
		stCmd.After.nCount = pSlot1->getCount();

		stCmd.nSlot = nTo;
		mSlotCommands.push_back( stCmd );

		mBag.swap( nFrom, nTo );

		return true;
		FUNCTION_END;
		return false;
	}

	///
	/// \brief �ƶ������е���Ʒ 
	/// \date 11/6/2017
	/// \author xufeng04
	/// \param Slot1 ��ǰ�����ĸ��ӣ�Ҫ�ƶ��ĸ���
	/// \param hBag �ƶ����ĸ�����
	/// \param Slot2 �ƶ����������ĸ�������
	/// \param nCount �ƶ�����Ʒ����
	/// \return �ƶ��˶��ٸ���Ʒ
	///
	xgc_uint32 XBagTrans::MoveTo( xObject hTarget, xSlot nTo, xSlot nFrom, xgc_uint32 nCount )
	{
		FUNCTION_BEGIN;
		auto pTarget = hTarget == INVALID_OBJECT_ID ? &mBag : ObjectCast< XBag >( hTarget );
		XGC_ASSERT_RETURN( pTarget, 0 );

		auto pSlot1 = mBag.getSlot( nFrom );
		auto pSlot2 = pTarget->getSlot( nTo );

		// �ж��Ƿ�����ƶ�
		if( pSlot1->getGoodsId() != pSlot2->getGoodsId() && false == pSlot2->isEmpty() )
			return 0;

		stSlotCommand stCmd;
		stCmd.nSequence = mSequence++;
		stCmd.eOperator = slot_op_move;

		//////////////////////////////////////////////////////////////////////////
		stCmd.hBag  = mBag.GetObjectID();
		stCmd.nSlot = nFrom;

		stCmd.Befor.hGoods = pSlot1->getGoodsId();
		stCmd.Befor.nCount = pSlot1->getCount();

		auto nTake = pSlot1->delGoods( nCount );

		stCmd.After.hGoods = pSlot1->getGoodsId();
		stCmd.After.nCount = pSlot1->getCount();

		mSlotCommands.push_back( stCmd );

		//////////////////////////////////////////////////////////////////////////
		stCmd.hBag  = hTarget;
		stCmd.nSlot = nTo;

		stCmd.Befor.hGoods = pSlot2->getGoodsId();
		stCmd.Befor.nCount = pSlot2->getCount();

		if( nTake == pSlot2->putGoods( pSlot1->getGoods(), nTake ) )
		{
			stCmd.After.hGoods = pSlot2->getGoodsId();
			stCmd.After.nCount = pSlot2->getCount();

			mSlotCommands.push_back( stCmd );
		}
		else
		{
			Undo();
		}

		return nTake;
		FUNCTION_END;
		return 0;
	}

	///
	/// \brief ���ز��� 
	/// \date 11/6/2017
	/// \author xufeng04
	/// \return �����Ƿ�ɹ�
	///

	xgc_bool XBagTrans::Undo()
	{
		stSlotCommand &stCmd = mSlotCommands.back();

		auto nSeq = stCmd.nSequence;

		while( stCmd.nSequence == nSeq )
		{
			auto pBag = ObjectCast< XBag >( stCmd.hBag );
			XGC_ASSERT_RETURN( pBag, false );

			auto pSlot = pBag->getSlot( stCmd.nSlot );
			XGC_ASSERT_RETURN( pSlot, false );

			if( pSlot->getGoodsId() != stCmd.After.hGoods )
				return false;

			if( pSlot->getCount() != stCmd.After.nCount )
				return false;

			pSlot->set( stCmd.Befor.hGoods, stCmd.Befor.nCount );
			mSlotCommands.pop_back();
		}

		return true;
	}
}