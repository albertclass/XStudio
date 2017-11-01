#pragma once
#ifndef __XBAG_H__
#define __XBAG_H__
#include "XGoods.h"

namespace xgc
{
	typedef xgc_uint32 xSlot;

	#define INVALID_SLOT_ID (xSlot)(-1)

	// 包裹每个位置的存储信息 有了 xGoodsPtr 就不可能是POD了
	struct CORE_API XSlot
	{
		/// @var 存储的物品
		xObject mGoods;   
		/// @var 该位置物品个数
		xgc_uint32 mCount;

		XSlot()
			: mGoods( INVALID_OBJECT_ID )
			, mCount( 0 )
		{
		}

		XSlot( xObject hGoods, xgc_uint32 nCount = 1 )
			: mGoods( hGoods )
			, mCount( nCount )
		{
			auto pGoods = ObjectCast< XGoods >( hGoods );
			XGC_ASSERT( nCount <= pGoods->GetOverlapNum() );
			if( nCount > pGoods->GetOverlapNum() )
			{
				USR_WARNING( "overlap overflow dbid = %u", pGoods->getValue< xgc_uint32 >( attrGoodsIndex ) );
				DumpStackFrame();
			}
		}

		~XSlot()
		{
			mGoods = INVALID_OBJECT_ID;
			mCount = 0;
		}

		xObject getGoodsId()const
		{
			return mGoods;
		}

		xgc_uint32 getCount()const
		{
			return mCount;
		}

		XGoods* getGoods()const
		{
			return ObjectCast< XGoods >( mGoods );
		}

		xgc_bool isEmpty() const
		{
			return ( 0 == mCount );
		}

		xgc_void setEmpty()
		{
			mGoods = INVALID_OBJECT_ID;
			mCount = 0;
		}

		///
		/// 格子是否满 
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool isFull() const
		{
			auto pGoods = ObjectCast< XGoods >( mGoods );
			if( xgc_nullptr == pGoods )
				return false;

			if( mCount < pGoods->GetOverlapNum() ) // 物品满了
				return false;

			return true;
		}

		///
		/// 是否是相同的物品配置
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool isThisGoodsIndex( xgc_uint32 nGoodsIndex ) const
		{
			if( isEmpty() ) // 没有放物品
				return false;

			auto pGoods = ObjectCast< XGoods >( mGoods );
			if( xgc_nullptr == pGoods )
				return false;

			if( pGoods->getValue< xgc_uint32 >( attrGoodsIndex ) != nGoodsIndex ) // 物品不一样
				return false;

			return true;
		}

		///
		/// 是否是相同物品 完全相同 注意 格子中只会判断是唯一物品
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool isThisGoods( xObject hGoods ) const
		{
			return mGoods == hGoods; // 物品不一样
		}

		///
		/// 是否是相同物品 完全相同 注意 格子中只会判断是唯一物品
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool isThisGoods( XGoods *pGoods ) const
		{
			return isThisGoods( pGoods->GetObjectID() );
		}

		///
		/// 放入物品,返回成功放入的个数 
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_uint32 putGoods( xObject hGoods, xgc_uint32 nCount )
		{
			auto pItem = ObjectCast< XGoods >( hGoods );
			if( xgc_nullptr == pItem )
				return 0;

			if( mCount && mGoods != hGoods )
				return 0;

			auto nEmpty = pItem->GetOverlapNum() - mCount;
			auto nNeeds = nEmpty >= nCount ? nCount : nEmpty;

			mCount += nNeeds;
			return nNeeds;
		}

		///
		/// 删除物品，返回成功删除的个数 
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_uint32 delGoods( xgc_uint32 nCount )
		{
			// 到这儿,说明到了要扣除的位置
			if( mCount >= nCount )
			{
				// 物品够扣除
				mCount -= nCount;
				return nCount;
			}
			else
			{
				// 这个地方不够扣除或者扣除之后为空
				nCount = mCount;
				setEmpty();
				return nCount;
			}
		}
	};

	struct CORE_API XBagTrans;

	// 背包类
	class CORE_API XBag : public XObject
	{
	public:
		DECLARE_XCLASS();

		// eType 必须配置
		XBag( xgc_uint32 nCapacity )
			: mCapacity( 0 )
			, mSlots( xgc_nullptr )
		{

		}

		~XBag()
		{
			for( xSlot slot = 0; slot < mCapacity; slot++ )
				mSlots[slot].setEmpty();

			SAFE_DELETE_ARRAY( mSlots );
			mCapacity = 0;
		}

		XBag( const XBag &) = delete;
		XBag( XBag &&) = delete;

		///
		/// 背包拥有者 
		/// [1/21/2015] create by wuhailin.jerry
		///
		xgc_bool setOwner( xObject hOwner ) 
		{
			auto pNewOwner = ObjectCast< XCharactor >( GetParent() );
			if( xgc_nullptr == pNewOwner )
				return false;

			return pNewOwner->Insert( this, xgc_nullptr );
		}

		///
		/// 获取背包的主人 角色 
		/// [9/12/2014] create by wuhailin.jerry
		///
		xObject getOwner() const 
		{ 
			return GetParent(); 
		}

		///
		/// 设置背包容量，如果有数据,就先复制，然后再删除之 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_bool setCapacity( xgc_uint32 nCapacity );

		///
		/// 获取背包容量 
		/// [7/30/2014] create by wuhailin.jerry
		///
		xgc_uint32 getCapacity() const { return mCapacity; }

		///
		/// 获取格子的信息 
		/// [10/2/2014] create by wuhailin.jerry
		///
		XSlot* getSlot( xSlot nSlot )
		{
			if (nSlot >= mCapacity)
				return xgc_nullptr;

			return &mSlots[nSlot];
		}

		///
		/// \brief 放置物品到指定格子中
		/// \author albert.xu
		/// \date 2017/10/19
		///
		xgc_uint32 put( xSlot nSlot, xObject hGoods, xgc_uint32 nCount )
		{
			XGC_ASSERT_RETURN( nSlot < mCapacity, 0 );

			return mSlots[nSlot].putGoods( hGoods, nCount );
		}

		xgc_uint32 del( xSlot nSlot, xgc_uint32 nCount ) 
		{
			XGC_ASSERT_RETURN( nSlot < mCapacity, 0 );

			return mSlots[nSlot].delGoods( nCount );
		}

		///
		/// 交换格子 
		/// [10/2/2014] create by wuhailin.jerry
		///
		xgc_void swap( xSlot nSlot1, xSlot nSlot2 )
		{
			XGC_ASSERT_RETURN( nSlot1 < mCapacity && nSlot2 < mCapacity, xgc_void( 0 ) );
			std::swap(mSlots[nSlot1], mSlots[nSlot2]);
		}

		///
		/// \brief 找到第一个为空的格子
		/// \prarm nStartSlot: 从nSlot开始查找
		/// \author albert.xu
		/// \date 2017/10/19
		///
		xSlot findEmpty( xSlot nStartSlot = 0 ) const;

		///
		/// 根据条件找出可以用的格子
		/// @ nStartSlot: 从nSlot开始查找
		/// [8/27/2014] create by wuhailin.jerry
		///
		xSlot findSlot( xObject hGoods, xSlot nStartSlot = 0 ) const;

		///
		/// 根据条件找出可以用的格子
		/// @ nStartSlot: 从nSlot开始查找
		/// [8/27/2014] create by wuhailin.jerry
		///
		xSlot findSlot( std::function< xgc_bool( const XSlot&, xSlot ) > &&filter, xSlot nStartSlot = 0 ) const;

		///
		/// \brief 获取背包中物品的总数 
		/// \author wuhailin.jerry
		/// \date [8/27/2014]
		///
		xgc_uint32 getGoodsTotalByID( xObject hGoods ) const;

		///
		/// \brief 获取背包中物品的总数 
		/// \author wuhailin.jerry
		/// \date [8/27/2014]
		///
		xgc_uint32 getGoodsTotalByIndex( xgc_uint32 nGoodsIndex ) const;

		///
		/// \brief 获取背包中空的格子数
		/// \author wuhailin.jerry
		/// \date [8/27/2014]
		///
		xgc_uint32 getEmptyCount() const;

	protected:
		///
		/// \brief 销毁对象时调用 
		/// \author albert.xu
		/// \date 6/9/2014
		///
		virtual xgc_void Destroy() override
		{
			for( xSlot i = 0; i < mCapacity; ++i )
				mSlots[i].delGoods( -1 );

			XObject::Destroy();
		}

		///
		/// \brief 获取背包剩余空格子数量
		/// \author jianglei.kinly
		/// \date 6/26/2015
		///
		xgc_uint32 PackageEmptySize()
		{
			xgc_uint32 nCount = 0;

			for( xgc_uint32 i = 0; i < mCapacity; ++i )
			{
				if( mSlots[i].isEmpty() )
					nCount += 1;
			}

			return nCount;
		}

	protected:
		/// @var 包裹最大容量
		xgc_uint32 mCapacity;
		/// @var 背包类型
		xgc_uint16 mType;
		/// @var 指向包裹位置存储结构指针的指针
		XSlot *mSlots; 

	};

	// 背包事务操作
	struct CORE_API XBagTrans : public noncopyable
	{
	private:
		enum enMode
		{
			///< 新的格子,需要通知客户端Add
			slot_op_new = 0,
			///< 旧的格子,需要通知客户端Update
			slot_op_old = 1, 
		};

		struct CORE_API stSlotOperatorInfo ///< 格子操作信息
		{
			xSlot       nSlot;        ///< 格子
			enMode		eMode;        ///< 操作类型
			xgc_bool    bIsAdd;       ///< Put还是Remove
			xgc_uint32  nAfterNum;    ///< 此次操作后格子内物品个数
			xgc_uint32  nOperatorNum; ///< 此次操作的数量
			xgc_uint32  nEvtType;     ///< 物品操作类型

			xObject hNewGoods;   ///< 物品
			xObject	hOldGoods;   ///< 原来的物品（如果有合并操作，这个字段就不是空的了）
			xgc_uint32 nOperatorNumTemp; ///< 此次操作的数量
		};

		typedef xgc_map<xgc_uint32, stSlotOperatorInfo> xSlotOperatorMap; ///< key:高16 背包类型 低16 背包格子,value: 

	public:
		explicit XBagTrans(XBag &Bag) 
			: mBag( Bag )
		{

		}

		~XBagTrans()
		{
			Cancel();
		}

		///
		/// 确定包裹操作完成 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void Commit( xgc_bool bNoticeClinet )
		{
			m_ChangedSlotMap.clear();
		}

		///
		/// 事务取消 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void Cancel();

		///
		/// 将dwGoodsNum 个 Goods放入背包中,如果 nSlot ！= INVALID_SLOT_ID 那么从该
		/// 值得位置开始放,否则从0开始放， bOverLap控制为是否堆叠放
		/// 返回真正放入的数目
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_bool Put( xObject hGoods, xgc_uint32 mCount, xgc_uint32 nEvtType, xgc_bool bOverlap = true, xSlot nSlot = 0 );

		///
		/// 扣除指定的道具,不够扣除时也会扣除,会返回扣除个数,所以需要自己判断是否够扣除
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 Remove( xGoodsPtr pShGoods, xgc_uint32 mCount, xgc_uint32 nEvtType, xSlot &nSlot );

		///
		/// 删除格子N中的物品M个 ,不够扣除时也会扣除,会返回扣除个数,所以需要自己判断是否够扣除
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 RemoveBySlot( xSlot nSlot, ENPacketType eType, xgc_uint32 mCount, xgc_uint32 nEvtType );

		///
		/// 删除指定格子内所有物品 
		/// [8/6/2014] create by wuhailin.jerry
		///
		xgc_void EmptySlot( xSlot nSlot );

		///
		/// 自动整理背包（先根据Compare的算法排序,然后把一样的物品叠加） 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void AutoPackPacket( xgc_uint32 nEvtType, std::function< xgc_bool( const xObject ) > fnCheckGoods = xgc_nullptr );

		///
		/// 交换格子或者合并,将dwSlot2中dwNum个物品放到dwSlot1中，如果是0，就取dwSlot2中的最大值
		/// dwNum ！ = dwSlot2的物品个数 时，两个格子中物品必须相同 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_bool Swap( xSlot Slot1, xSlot Slot2, xgc_uint32 nEvtType, xgc_uint32 nNum = 0, std::function< xgc_bool( const xObject ) > fnCheckGoods = xgc_nullptr );

		///
		/// 根据物品，交换格子或者合并,将dwSlot2中dwNum个物品放到dwSlot1中，如果是0，就取dwSlot2中的最大值
		/// dwNum ！ = dwSlot2的物品个数 时，两个格子中物品必须相同 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_bool Swap( xGoodsPtr pGoods1, xGoodsPtr pGoods2, xSlot &Slot1, xSlot &Slot2, xgc_uint32 nEvtType, xgc_uint32 nNum = 0, std::function< xgc_bool( const xObject ) > fnCheckGoods = xgc_nullptr );

		const xgc_list<stSlotOperatorInfo> &GetSlotOperatorList() const { return m_SlotOperatorList; }

	private:
		///
		/// 保存该格子数据 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void RecordOldSlot( ENPacketType eType, xSlot nSlot );

protected:
		XBag &mBag;

		xgc_list<stSlotOperatorInfo> m_SlotOperatorList; ///< 本次Commit前对格子的操作,按照顺序保存，所以也需要按照顺序通知客户端
		xgc_map<xgc_uint32, stPackageSlot> m_ChangedSlotMap;  // 操作过的格子数据,每个格子只保存第一个操作过的 高16 背包类型 低16 背包格子
	};

	//  高16 背包类型 低16 背包格子
	inline xgc_uint32 MakeChangedSlotMapKey( ENPacketType eType, xSlot slot )
	{
		return ( ( (xgc_uint16) eType ) << 16 ) + slot;
	}

	//  高16 背包类型 低16 背包格子
	inline xgc_void GetChangedSlotMapKey( xgc_uint32 nKey, ENPacketType &eType, xSlot &slot )
	{
		slot = nKey & 0x0000FFFF;
		eType = (ENPacketType) ( nKey >> 16 );
	}
}


#endif