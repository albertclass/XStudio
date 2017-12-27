#pragma once
#ifndef __XBAG_H__
#define __XBAG_H__
#include "XGoods.h"

namespace xgc
{
	typedef xgc_uint32 xSlot;

	#define INVALID_SLOT_ID (xSlot)(-1)

	/// 
	/// \brief 包裹每个位置的存储信息
	/// \date 11/6/2017
	/// \author xufeng04
	///
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
				USR_WARNING( "overlap overflow dbid = %u", pGoods->getValue< xgc_uint32 >( XGoods::Index ) );
				DumpStackFrame();
			}
		}

		~XSlot()
		{
			mGoods = INVALID_OBJECT_ID;
			mCount = 0;
		}

		xgc_void set( xObject hGoods, xgc_uint32 nCount )
		{
			mGoods = hGoods;
			mCount = nCount;
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
			return INVALID_OBJECT_ID == mGoods && 0 == mCount;
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
		/// \brief 是否是相同的物品配置 
		/// \date 11/6/2017
		/// \author xufeng04
		/// \param 物品索引
		/// \return 
		///
		xgc_bool isSameGoods( xgc_uint32 nGoodsIndex ) const
		{
			if( isEmpty() ) // 没有放物品
				return false;

			auto pGoods = ObjectCast< XGoods >( mGoods );
			if( xgc_nullptr == pGoods )
				return false;

			if( pGoods->getValue< xgc_uint32 >( XGoods::Index ) != nGoodsIndex ) // 物品不一样
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
		/// \brief 放入物品，返回成功放入的个数 
		/// \date 11/6/2017
		/// \author xufeng04
		/// \param hGoods 物品句柄
		/// \param nCount 物品数量
		/// \return 放入的物品数量， 0 - 未放入任何物品
		///
		xgc_uint32 putGoods( xObject hGoods, xgc_uint32 nCount )
		{
			return putGoods( ObjectCast< XGoods >( hGoods ), nCount );
		}

		///
		/// \brief 放入物品，返回成功放入的个数 
		/// \date 11/6/2017
		/// \author xufeng04
		/// \param pGoods 物品指针
		/// \param nCount 物品数量
		/// \return 放入的物品数量， 0 - 未放入任何物品
		///
		///
		xgc_uint32 putGoods( XGoods *pGoods, xgc_uint32 nCount )
		{
			XGC_ASSERT_RETURN( pGoods, 0 );

			if( mCount && !isThisGoods( pGoods ) )
				return 0;

			auto nEmpty = pGoods->GetOverlapNum() - mCount;
			auto nNeeds = nEmpty >= nCount ? nCount : nEmpty;

			mCount += nNeeds;
			return nNeeds;
		}

		///
		/// \brief 删除物品，返回成功删除的个数
		/// \date 11/6/2017
		/// \author xufeng04
		/// \param nCount 删除的个数
		/// \return 实际删除的物品个数
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

	///
	/// \brief 背包事件ID
	/// \date 11/6/2017
	/// \author xufeng04
	///
	enum enBagEvent
	{
		evt_bag_change,
		evt_bag_update,
	};

	///
	/// \brief  背包类 
	/// \date 11/6/2017
	/// \author xufeng04
	///
	class CORE_API XBag : public XObject
	{
		DECLARE_XCLASS();
	public:
		///
		/// \brief 背包格子更新
		/// \date 11/6/2017
		/// \author xufeng04
		///
		struct SlotEvent
		{
			/// 参数转换
			XObjectEvent cast;
			/// 操作的格子编号
			xSlot nSlot;
			/// 格子变更
			struct
			{
				/// 操作结束后格子中的物品ID
				xObject	hGoods;
				/// 此次操作的数量
				xgc_uint32 nCount;
			} Befor, After;
		};

		///
		/// \brief 背包操作事件 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		struct TransEvent
		{
			/// 参数转换
			XObjectEvent cast;
			/// 操作的背包
			xObject hBag;
			/// 操作的物品
			xObject hGoods;
			/// 操作的数量
			xgc_long nCount;
		};

	public:
		///
		/// \brief 背包构造 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		XBag( xgc_uint32 nType, xgc_uint32 nCapacity );

		///
		/// \brief 背包析构 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		~XBag();

		///
		/// \brief 不允许的构造 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		XBag( const XBag &) = delete;
		XBag( XBag &&) = delete;

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

		///
		/// \brief 整理背包（先根据Compare的算法排序,然后把一样的物品叠加）  
		/// \date 11/6/2017
		/// \author xufeng04
		/// \param fnCompair 比较算法
		/// \param bOverlap 是否堆叠
		///
		xgc_void Sort( const std::function< xgc_bool( const XSlot&, const XSlot& ) > &fnCompair, xgc_bool bOverlap = true );

	protected:
		///
		/// \brief 销毁对象时调用 
		/// \author albert.xu
		/// \date 6/9/2014
		///
		virtual xgc_void OnDestroy() override
		{
			for( xSlot i = 0; i < mCapacity; ++i )
				mSlots[i].delGoods( -1 );
		}

	protected:
		/// @var 背包类型
		xgc_uint32 mType;
		/// @var 包裹最大容量
		xgc_uint32 mCapacity;
		/// @var 指向包裹位置存储结构指针的指针
		XSlot *mSlots; 
	};

	// 背包事务操作
	struct CORE_API XBagTrans : public noncopyable
	{
	protected:
		enum enOperator
		{
			/// 添加物品
			slot_op_put = 0,
			/// 删除物品
			slot_op_del = 1,
			/// 交换物品
			slot_op_swap = 2,
			/// 转移物品
			slot_op_move = 3,
		};

		struct CORE_API stSlotCommand ///< 格子操作信息
		{
			/// 操作序号
			xgc_uint32	nSequence;
			/// 操作的包裹
			xObject		hBag;
			/// 操作的格子编号
			xSlot       nSlot;
			/// 操作类型
			enOperator	eOperator;
			/// 格子变更
			struct
			{
				/// 操作结束后格子中的物品ID
				xObject		hGoods;
				/// 此次操作的数量
				xgc_uint32  nCount;
			} Befor, After;
		};

		/// 操作序号
		xgc_uint32 mSequence;
		/// 操作的背包
		XBag &mBag;
		/// 操作的记录
		xgc::vector< stSlotCommand > mSlotCommands;

	public:
		///
		/// \brief 背包事务构造 
		/// \date 11/6/2017
		/// \author xufeng04
		/// \param 操作目标
		///
		explicit XBagTrans(XBag &Bag);

		///
		/// \brief 背包事务析构 
		/// \date 11/6/2017
		/// \author xufeng04
		///
		~XBagTrans();

		///
		/// \brief 提交包裹操作 
		/// \authro albert.xu
		/// \date 2017/11/06
		///
		xgc_void Commit();

		///
		/// \brief 事务取消 
		/// \authro albert.xu
		/// \date 2017/11/06
		///
		xgc_void Cancel();

		///
		/// \brief 将物品放入背包
		/// \authro albert.xu
		/// \date 2017/11/06
		/// \return 全部放入返回 true，否则返回 false
		///
		xgc_bool Put( xObject hGoods, xgc_uint32 nCount, xSlot nStartSlot = 0 );

		///
		/// \brief 扣除指定的道具
		/// \authro albert.xu
		/// \date 2017/11/06
		/// \return 全部删除返回 true，否则返回 false
		///
		xgc_bool Del( xObject hGoods, xgc_uint32 nCount, xSlot nStartSlot = 0 );

		///
		/// \brief 交换格子
		/// \date 2017/11/06
		/// \author albert.xu
		///
		xgc_bool Swap( xSlot nFrom, xSlot nTo, xObject hTarget = INVALID_OBJECT_ID );

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
		xgc_uint32 Move( xSlot nFrom, xSlot nTo, xgc_uint32 nCount )
		{
			return MoveTo( INVALID_OBJECT_ID, nTo, nFrom, nCount );
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
		xgc_uint32 MoveTo( xObject hTarget, xSlot nTo, xSlot nFrom, xgc_uint32 nCount );

		///
		/// \brief 撤回操作 
		/// \date 11/6/2017
		/// \author xufeng04
		/// \return 撤回是否成功
		///
		xgc_bool Undo();
	};
}


#endif