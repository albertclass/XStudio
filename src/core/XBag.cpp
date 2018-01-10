#include "XHeader.h"
#include "XActor.h"
#include "XBag.h"

namespace xgc
{
	IMPLEMENT_XCLASS_BEGIN( XBag, XObject )

	IMPLEMENT_XCLASS_END();

	///
	/// \brief 背包构造 
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
	/// \brief 背包析构 
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
			// 不需要改变
			USR_ERR( "Can not modify, capacity is small than old. mCapacity:[%d], nCapacity:[%d]", mCapacity, nCapacity );
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
			SYS_ERR( "new XSlot failed, nCapacity:[%d]",
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

		return true;
	}

	///
	/// \brief 找到第一个为空的格子
	/// \prarm nSlot: 从nSlot开始查找
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

	///
	/// \brief 获取背包中物品的总数 
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
			if( mSlots[i].isSameGoods( nGoodsIndex ) )
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

	xgc_void XBag::Sort( const std::function< xgc_bool( const XSlot&, const XSlot& ) > &fnCompair, xgc_bool bOverlap )
	{
		FUNCTION_BEGIN;

		// 先整理
		std::sort( mSlots, mSlots + mCapacity, fnCompair );

		// 再归并
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
				// 将slot2中的物品往slot1里面填，填满为止。
				pSlot2->delGoods( pSlot1->putGoods( pSlot2->getGoodsId(), pSlot2->getCount() ) );
				if( pSlot2->isEmpty() )
					++j;
			}
			else if( pSlot1->getGoodsId() == pSlot2->getGoodsId() )
			{
				// 将slot2中的物品往slot1里面填，填满为止。
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
	/// \brief 背包事务构造 
	/// \date 11/6/2017
	/// \author xufeng04
	/// \param 操作目标
	///
	XBagTrans::XBagTrans( XBag &Bag ) 
		: mBag( Bag )
		, mSequence( 0x80000000 )
	{

	}

	///
	/// \brief 背包事务析构 
	/// \date 11/6/2017
	/// \author xufeng04
	///

	XBagTrans::~XBagTrans()
	{
		Cancel();
	}

	///
	/// \brief 确定包裹操作完成 
	/// \authro albert.xu
	/// \date 2017/11/06
	///
	xgc_void XBagTrans::Commit()
	{
		// 根据背包和位置生成一个操作列表，该列表记录了此位置每一步的变化
		xgc::map< xgc::tuple< xObject, xSlot >, xgc::list< stSlotCommand* > > mapUpdate;

		// 根据物品生成一个增减列表
		xgc::map< xgc::tuple< xObject, xObject >, xgc_long > mapOperator;

		// 对事务所有的操作进行归并
		for( auto &cmd : mSlotCommands )
		{
			// 将相同位置的操作进行合并处理
			mapUpdate[{cmd.hBag, cmd.nSlot}].push_back( &cmd );

			// 处理每一步操作，将物品的增减量提出来
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

		// 提交更新事件
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

		// 提交事务事件
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
	/// \brief 事务取消 
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
		// 操作序号
		stCmd.nSequence = mSequence++;
		// 操作类型
		stCmd.eOperator = slot_op_put;
		// 目标包裹
		stCmd.hBag = mBag.GetObjectID();

		// 开始放置物品到包裹
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
		// 操作序号
		stCmd.nSequence = mSequence++;
		// 操作类型
		stCmd.eOperator = slot_op_del;
		// 目标包裹
		stCmd.hBag = mBag.GetObjectID();

		// 开始从包裹删除物品
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
	/// \brief 移动格子中的物品 
	/// \date 11/6/2017
	/// \author xufeng04
	/// \param Slot1 当前包裹的格子，要移动的格子
	/// \param hBag 移动到哪个背包
	/// \param Slot2 移动到背包的哪个格子中
	/// \param nCount 移动的物品数量
	/// \return 移动了多少个物品
	///
	xgc_uint32 XBagTrans::MoveTo( xObject hTarget, xSlot nTo, xSlot nFrom, xgc_uint32 nCount )
	{
		FUNCTION_BEGIN;
		auto pTarget = hTarget == INVALID_OBJECT_ID ? &mBag : ObjectCast< XBag >( hTarget );
		XGC_ASSERT_RETURN( pTarget, 0 );

		auto pSlot1 = mBag.getSlot( nFrom );
		auto pSlot2 = pTarget->getSlot( nTo );

		// 判断是否可以移动
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
	/// \brief 撤回操作 
	/// \date 11/6/2017
	/// \author xufeng04
	/// \return 撤回是否成功
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