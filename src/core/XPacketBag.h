#pragma once

#ifndef _XPACKETBAG_H_
#define _XPACKETBAG_H_


#ifdef CORE_EXPORTS
#	define CORE_API __declspec(dllexport)
#else
#	define CORE_API __declspec(dllimport)
#endif	//CORE_EXPORTS

#include "XObject.h"
#include "XGoods.h"
#include <memory>

using namespace std;

namespace XGC
{
	typedef xgc_uint16 xSlot;

	#define INVALID_SLOT (xSlot)(-1)

	// 背包类型
	enum ENPacketType : xgc_uint16
	{
		EN_PacketBegin  = 0,
		EN_PackageItems = 0,   ///< 角色包裹
		EN_StorageItems = 1,   ///< 角色仓库
		EN_BodyItems    = 2,   ///< 角色穿戴物品
		EN_PetItems     = 3,   ///< 角色宠物栏
		EN_ReBackItems  = 4,   ///< 回购栏
		EN_TaskPackage  = 5,   ///< 任务包裹
		EN_RidePackage  = 6,   ///< 坐骑包裹
		EN_StuffPackage = 7,   ///< 材料包裹
		EN_TitlePackage = 8,   ///< 称号包裹
		EN_PackageEnd,         ///< 最大包裹
	};

	enum ENPacketMode : xgc_uint8
	{
		EN_PacketNewSlot = 0, ///< 新的格子,需要通知客户端ADD
		EN_PacketOldSlot = 1, ///< 旧的格子,需要通知客户端Update
		EN_PacketGet = 2,
	};

	struct CORE_API stSlotOperatorInfo ///< 格子操作信息
	{
		ENPacketMode  ENMode;       ///< 操作类型
		xgc_uint32    nAfterNum;    ///< 此次操作后格子内物品个数
		xgc_uint32    nOperatorNum; ///< 此次操作的数量
		xGoodsPtr     pxGoods;      ///< 物品
		xSlot         nSlot;        ///< 格子
		ENPacketType  eType;        ///< 背包类型
		xgc_bool      bIsAdd;       ///< Put还是Remove
		xgc_uint32    nEvtType;     ///< 物品操作类型

		xGoodsPtr     pxOldGoods;   ///< 原来的物品（如果有合并操作，这个字段就不是空的了）
		xgc_uint32    nOperatorNumTemp; ///< 此次操作的数量
	};

	typedef xgc_map<xgc_uint32, stSlotOperatorInfo> xSlotOperatorMap; ///< key:高16 背包类型 低16 背包格子,value: 

	// 包裹每个位置的存储信息 有了 xGoodsPtr 就不可能是POD了
	struct CORE_API stPackageSlot
	{
		xGoodsPtr pShGoods;   // 存储的物品
		xgc_uint32 nGoodsNum; // 该位置物品个数

		stPackageSlot();

		explicit stPackageSlot( xGoodsPtr pGoods, xgc_uint32 nCount = 1 );

		~stPackageSlot()
		{
			pShGoods = xgc_nullptr;
			nGoodsNum = 0;
		}

		xgc_bool IsEmpty() const
		{
			return ( 0 == nGoodsNum );
		}

		xgc_void SetEmpty()
		{
			pShGoods = xgc_nullptr;
			nGoodsNum = 0;
		}

		///
		/// 格子是否满 
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool IsFull() const
		{
			if( pShGoods && pShGoods->GetOverlapNum() == nGoodsNum ) // 物品满了
			{
				return true;
			}

			return false;
		}

		///
		/// 是否是相同的DbId 
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool IsSameDbId( xgc_uint32 nDbId ) const
		{
			if( IsEmpty() ) // 没有放物品
			{
				return false;
			}

			if( pShGoods->GetAttribute( attrDbIndex ).toUnsigned() != nDbId ) // 物品不一样
			{
				return false;
			}

			return true;
		}

		///
		/// 是否是相同物品 完全相同 注意 格子中只会判断是唯一物品
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool IsSameGoods(xObject hGoods) const
		{
			if( IsEmpty() ) // 没有放物品
			{
				return false;
			}

			if( pShGoods->GetObjectID() != hGoods ) // 物品不一样
			{
				return false;
			}

			return true;
		}

		///
		/// 物品是否可以塞入格子中 
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool CanInsert(xGoodsPtr pGoods, xgc_bool bOverLap) const
		{
			if( IsEmpty() )
			{
				return true;
			}

			if (!bOverLap)
			{
				return false;
			}

			if( !pShGoods->IsSameGoods( pGoods ) ) // 物品不一样
			{
				return false;
			}

			if( pShGoods->GetOverlapNum() == nGoodsNum ) // 物品满了
			{
				return false;
			}

			return true;
		}

		///
		/// 放入物品,返回成功放入的个数 
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_uint32 AddGoods( xgc_uint32 nNum )
		{
			if( xgc_nullptr == pShGoods )
			{
				USR_ERROR( "share_ptr for Goods is NULL" );
				return 0;
			}

			xgc_uint32 nCanFillNum = pShGoods->GetOverlapNum() - nGoodsNum;
			if( nCanFillNum >= nNum ) // 没有超过可堆叠总个数
			{
				nGoodsNum += nNum;
				return nNum;
			}
			else // 放不下
			{
				nGoodsNum += nCanFillNum;
				return nCanFillNum;
			}
		}

		///
		/// 删除物品，返回成功删除的个数 
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_uint32 DelGoods( xgc_uint32 nNum )
		{
			// 到这儿,说明到了要扣除的位置
			if( nGoodsNum > nNum ) // 物品够扣除
			{
				nGoodsNum -= nNum;
				return nNum;
			}
			else // 这个地方不够扣除或者扣除之后为空
			{
				xgc_int32 nSubNum = nGoodsNum;

				SetEmpty();
				return nSubNum;
			}
		}
	};

	struct CORE_API XPacketBagOperator;

	// 背包类
	class CORE_API XPacketBag : public XObject
	{
	public:
		DECLARE_XCLASS();

		// eType 必须配置
		explicit XPacketBag( ENPacketType eType ) : m_nCapacity( 0 ), m_pSlotArray( xgc_nullptr ), m_PacketType( eType )
		{

		}

		virtual ~XPacketBag();

		XPacketBag( const XPacketBag &) = delete;
		XPacketBag( XPacketBag &&) = delete;

		///
		/// 根据条件分别返回相应的真正需要操作的背包 
		/// [1/21/2015] create by wuhailin.jerry
		///
		virtual XPacketBag& getCurBag(xgc_uint32 nDbId) { return *this; }

		virtual XPacketBag& getBagByType(ENPacketType eType) = 0;

		///
		/// 背包拥有者 
		/// [1/21/2015] create by wuhailin.jerry
		///
		xgc_void SetPacketOwner( xObject hRole ) { m_hRole = hRole; }

		///
		/// 获取背包的主人 角色 
		/// [9/12/2014] create by wuhailin.jerry
		///
		xObject GetPacketOwner() const { return m_hRole; }

		///
		/// 获取背包类型 
		/// [9/12/2014] create by wuhailin.jerry
		///
		ENPacketType GetPacketType() const { return m_PacketType; }

		///
		/// 设置背包容量，如果有数据,就先复制，然后再删除之 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void SetCapacity( xgc_uint32 nCapacity );

		///
		/// 获取背包容量 
		/// [7/30/2014] create by wuhailin.jerry
		///
		xgc_uint32 GetCapacity() const { return m_nCapacity; }

		///
		/// 设置背包最大容量 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 GetMaxCapacity() const { return m_nMaxCapacity; }

		///
		/// 设置背包容量 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void SetMaxCapactiy( xgc_uint32 nMaxCapacity ) { m_nMaxCapacity = nMaxCapacity; }

		///
		/// 获取格子的信息 
		/// [10/2/2014] create by wuhailin.jerry
		///
		const stPackageSlot* GetSlotInfo( xSlot nSlot ) const
		{
			if (nSlot >= m_nCapacity)
			{
				return xgc_nullptr;
			}

			return &m_pSlotArray[nSlot];
		}

		///
		/// 设置格子内的信息 
		/// [10/2/2014] create by wuhailin.jerry
		///
		xgc_void SetSlotInfo(xSlot nSlot, const stPackageSlot &stSlot)
		{
			if (nSlot < m_nCapacity)
			{
				m_pSlotArray[nSlot] = stSlot;
			}
		}

		xgc_void SetSlotInfo( xSlot nSlot, const xGoodsPtr pGoods )
		{
			if( nSlot < m_nCapacity )
			{
				m_pSlotArray[nSlot].pShGoods = pGoods;
			}
		}
		xgc_void SetSlotInfo( xSlot nSlot, xgc_uint32 nNum )
		{
			if( nSlot < m_nCapacity )
			{
				m_pSlotArray[nSlot].nGoodsNum = nNum;
			}
		}

		///
		/// 添加或者删除格子内物品 
		/// [10/2/2014] create by wuhailin.jerry
		///
		xgc_uint32 AddSlotGoods(xSlot nSlot, xgc_uint32 nNum)
		{
			if( nSlot < m_nCapacity )
			{
				return m_pSlotArray[nSlot].AddGoods( nNum );
			}

			return 0;
		}

		xgc_uint32 DelSlotGoods( xSlot nSlot, xgc_uint32 nNum ) 
		{
			if( nSlot < m_nCapacity )
			{
				return m_pSlotArray[nSlot].DelGoods( nNum );
			}

			return 0;
		}

		///
		/// 交换格子 
		/// [10/2/2014] create by wuhailin.jerry
		///
		xgc_void SwapSlot(xSlot nSlot1, xSlot nSlot2)
		{
			XGC_ASSERT_RETURN( nSlot1 < m_nCapacity && nSlot2 < m_nCapacity, xgc_void( 0 ) );
			std::swap(m_pSlotArray[nSlot1], m_pSlotArray[nSlot2]);
		}

		///
		///  根据条件找出可以用的格子
		/// @ nSlot: 0:从0开始找,否则从nSlot开始
		/// @ bOverLap:false 不堆叠,true堆叠
		/// [8/27/2014] create by wuhailin.jerry
		///
		xSlot FindSlot( xGoodsPtr pShGoods, xSlot nSlot = 0, xgc_bool bOverLap = true );

		///
		/// 根据物品ID获取物品信息,只拿到第一个 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xGoodsPtr GetGoodsPtrByDbId( xgc_uint32 nGoodsId );

		///
		///  根据格子获取物品信息
		/// [9/1/2014] create by wuhailin.jerry
		///
		xGoodsPtr TakeBySlot( xSlot Slot ) const;

		///
		/// 根据格子获取物品信息以及物品个数 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xGoodsPtr TakeBySlot( xSlot Slot, xgc_uint32 &nGoodsNum ) const;

		///
		/// 获取背包中dwGoodsId的个数 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 GetByGoodsId( xgc_uint32 nGoodsId );


		xgc_uint32 GetByGoodsId( xgc_uint32 nGoodsId, xSlotOperatorMap &xSlotMap );
		
		///
		/// 获取物品所在格子以及物品个数 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 GetByGoodsPtr( xGoodsPtr pShGoods, xSlot &nSlot ) const;

		///
		/// 根据ObjectId获取物品所在格子以及物品个数 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 GetByObjectId( xObject hObject, xSlot &nSlot ) const;

		///
		/// 获取空闲格子总个数 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 GetEmptySlotNum() const;
		/// 获取第一个空闲格子的位置 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xSlot GetEmptySlot() const;
		///
		/// 遍历背包并进行相关处理 
		/// [9/2/2014] create by wuhailin.jerry
		///
		xgc_void ForEachBag(const std::function<xgc_bool(const stPackageSlot &, const xSlot nSlot)> &fFun)
		{
			for( xgc_uint32 nPos = 0; nPos < m_nCapacity; nPos++ )
			{
				if (false == fFun( m_pSlotArray[nPos], nPos ))
				{
					break;
				}
			}
		}

protected:
	///
	/// 新增包裹容量 
	/// [7/30/2014] create by wuhailin.jerry
	///
	xgc_bool AddCapacity( xgc_uint32 nAddNum )
	{
		if (0 == nAddNum)
		{
			return false;
		}

		if( m_nCapacity + nAddNum > m_nMaxCapacity ) // 超过最大容量
		{
			return false;
		}

		SetCapacity( m_nCapacity + nAddNum );

		return true;
	}

protected:
		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	增加子节点前调用
		// return :		true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		//---------------------------------------------------//
		virtual xgc_bool PreAddChild( XObject* pChild, xgc_lpcvoid lpContext ) { return true; }

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	加入子节点后调用
		//---------------------------------------------------//
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext ) {}

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	增加子节点前调用
		// return :		true - 确认移除子节点, false - 子节点被否决,移除子节点失败.
		//---------------------------------------------------//
		virtual xgc_bool PreRemoveChild( XObject* pChild, xgc_bool bRelease ) { return true; }

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	删除子节点后调用,此时对象尚未被删除
		//---------------------------------------------------//
		virtual xgc_void OnRemoveChild( XObject* pChild, xgc_bool bRelease ) {}

		///
		/// 销毁对象时调用 
		/// [6/9/2014] create by albert.xu
		/// \virtual XObject的虚函数
		///
		virtual xgc_void OnDestroy() {}

		///
		/// 获取背包剩余空格子数量
		/// [6/26/2015] create by jianglei.kinly
		///
		xgc_uint32 PackageEmptySize()
		{
			xgc_uint32 nSize = 0;
			for( xgc_uint32 i = 0; i < m_nCapacity; ++i )
			{
				if( m_pSlotArray[i].IsEmpty() )
				{
					nSize += 1;
				}
			}
			return nSize;
		}

private:
		stPackageSlot *m_pSlotArray; // 指向包裹位置存储结构指针的指针
		xgc_uint32 m_nCapacity;        // 包裹最大容量

		xgc_uint32 m_nMaxCapacity; ///< 最大容量

protected:
		xObject m_hRole;             ///< 背包的主人
		ENPacketType m_PacketType;   ///< 背包类型
	};

	// 背包事务操作
	struct CORE_API XPacketBagOperator : public noncopyable
	{
	public:

		explicit XPacketBagOperator(XPacketBag &Bag) : m_Bag(Bag)
		{

		}

		~XPacketBagOperator()
		{
			xTransactionCancel();
		}

		///
		/// 确定包裹操作完成 
		/// [8/27/2014] create by wuhailin.jerry
		///
		virtual xgc_void TransactionCommit(xgc_bool bNoticeClinet)
		{
			m_ChangedSlotMap.clear();
		}

		///
		/// 事务取消 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void xTransactionCancel();

		///
		/// 将dwGoodsNum 个 Goods放入背包中,如果 nSlot ！= INVALID_SLOT 那么从该
		/// 值得位置开始放,否则从0开始放， bOverLap控制为是否堆叠放
		/// 返回真正放入的数目
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 Put( xGoodsPtr pShGoods, xgc_uint32 nGoodsNum, xgc_uint32 nEvtType,
			xSlot nSlot = INVALID_SLOT, xgc_bool bOverLap = true );

		///
		/// 扣除指定的道具,不够扣除时也会扣除,会返回扣除个数,所以需要自己判断是否够扣除
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 Remove( xGoodsPtr pShGoods, xgc_uint32 nGoodsNum, xgc_uint32 nEvtType, xSlot &nSlot );

		///
		/// 删除格子N中的物品M个 ,不够扣除时也会扣除,会返回扣除个数,所以需要自己判断是否够扣除
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 RemoveBySlot( xSlot nSlot, ENPacketType eType, xgc_uint32 nGoodsNum, xgc_uint32 nEvtType );

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

		XPacketBag &GetPacket(xgc_uint32 nDbId = 0) const 
		{ 
			if (0 == nDbId)
			{
				return m_Bag;
			}

			return m_Bag.getCurBag(nDbId); 
		};

		const xgc_list<stSlotOperatorInfo> &GetSlotOperatorList() const { return m_SlotOperatorList; }

	private:
		///
		/// 保存该格子数据 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void RecordOldSlot( ENPacketType eType, xSlot nSlot );

protected:
		XPacketBag &m_Bag;

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