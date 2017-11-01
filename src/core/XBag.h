#pragma once
#ifndef __XBAG_H__
#define __XBAG_H__
#include "XGoods.h"

namespace xgc
{
	typedef xgc_uint32 xSlot;

	#define INVALID_SLOT_ID (xSlot)(-1)

	// ����ÿ��λ�õĴ洢��Ϣ ���� xGoodsPtr �Ͳ�������POD��
	struct CORE_API XSlot
	{
		/// @var �洢����Ʒ
		xObject mGoods;   
		/// @var ��λ����Ʒ����
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
		/// �����Ƿ��� 
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool isFull() const
		{
			auto pGoods = ObjectCast< XGoods >( mGoods );
			if( xgc_nullptr == pGoods )
				return false;

			if( mCount < pGoods->GetOverlapNum() ) // ��Ʒ����
				return false;

			return true;
		}

		///
		/// �Ƿ�����ͬ����Ʒ����
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool isThisGoodsIndex( xgc_uint32 nGoodsIndex ) const
		{
			if( isEmpty() ) // û�з���Ʒ
				return false;

			auto pGoods = ObjectCast< XGoods >( mGoods );
			if( xgc_nullptr == pGoods )
				return false;

			if( pGoods->getValue< xgc_uint32 >( attrGoodsIndex ) != nGoodsIndex ) // ��Ʒ��һ��
				return false;

			return true;
		}

		///
		/// �Ƿ�����ͬ��Ʒ ��ȫ��ͬ ע�� ������ֻ���ж���Ψһ��Ʒ
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool isThisGoods( xObject hGoods ) const
		{
			return mGoods == hGoods; // ��Ʒ��һ��
		}

		///
		/// �Ƿ�����ͬ��Ʒ ��ȫ��ͬ ע�� ������ֻ���ж���Ψһ��Ʒ
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool isThisGoods( XGoods *pGoods ) const
		{
			return isThisGoods( pGoods->GetObjectID() );
		}

		///
		/// ������Ʒ,���سɹ�����ĸ��� 
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
		/// ɾ����Ʒ�����سɹ�ɾ���ĸ��� 
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_uint32 delGoods( xgc_uint32 nCount )
		{
			// �����,˵������Ҫ�۳���λ��
			if( mCount >= nCount )
			{
				// ��Ʒ���۳�
				mCount -= nCount;
				return nCount;
			}
			else
			{
				// ����ط������۳����߿۳�֮��Ϊ��
				nCount = mCount;
				setEmpty();
				return nCount;
			}
		}
	};

	struct CORE_API XBagTrans;

	// ������
	class CORE_API XBag : public XObject
	{
	public:
		DECLARE_XCLASS();

		// eType ��������
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
		/// ����ӵ���� 
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
		/// ��ȡ���������� ��ɫ 
		/// [9/12/2014] create by wuhailin.jerry
		///
		xObject getOwner() const 
		{ 
			return GetParent(); 
		}

		///
		/// ���ñ������������������,���ȸ��ƣ�Ȼ����ɾ��֮ 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_bool setCapacity( xgc_uint32 nCapacity );

		///
		/// ��ȡ�������� 
		/// [7/30/2014] create by wuhailin.jerry
		///
		xgc_uint32 getCapacity() const { return mCapacity; }

		///
		/// ��ȡ���ӵ���Ϣ 
		/// [10/2/2014] create by wuhailin.jerry
		///
		XSlot* getSlot( xSlot nSlot )
		{
			if (nSlot >= mCapacity)
				return xgc_nullptr;

			return &mSlots[nSlot];
		}

		///
		/// \brief ������Ʒ��ָ��������
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
		/// �������� 
		/// [10/2/2014] create by wuhailin.jerry
		///
		xgc_void swap( xSlot nSlot1, xSlot nSlot2 )
		{
			XGC_ASSERT_RETURN( nSlot1 < mCapacity && nSlot2 < mCapacity, xgc_void( 0 ) );
			std::swap(mSlots[nSlot1], mSlots[nSlot2]);
		}

		///
		/// \brief �ҵ���һ��Ϊ�յĸ���
		/// \prarm nStartSlot: ��nSlot��ʼ����
		/// \author albert.xu
		/// \date 2017/10/19
		///
		xSlot findEmpty( xSlot nStartSlot = 0 ) const;

		///
		/// ���������ҳ������õĸ���
		/// @ nStartSlot: ��nSlot��ʼ����
		/// [8/27/2014] create by wuhailin.jerry
		///
		xSlot findSlot( xObject hGoods, xSlot nStartSlot = 0 ) const;

		///
		/// ���������ҳ������õĸ���
		/// @ nStartSlot: ��nSlot��ʼ����
		/// [8/27/2014] create by wuhailin.jerry
		///
		xSlot findSlot( std::function< xgc_bool( const XSlot&, xSlot ) > &&filter, xSlot nStartSlot = 0 ) const;

		///
		/// \brief ��ȡ��������Ʒ������ 
		/// \author wuhailin.jerry
		/// \date [8/27/2014]
		///
		xgc_uint32 getGoodsTotalByID( xObject hGoods ) const;

		///
		/// \brief ��ȡ��������Ʒ������ 
		/// \author wuhailin.jerry
		/// \date [8/27/2014]
		///
		xgc_uint32 getGoodsTotalByIndex( xgc_uint32 nGoodsIndex ) const;

		///
		/// \brief ��ȡ�����пյĸ�����
		/// \author wuhailin.jerry
		/// \date [8/27/2014]
		///
		xgc_uint32 getEmptyCount() const;

	protected:
		///
		/// \brief ���ٶ���ʱ���� 
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
		/// \brief ��ȡ����ʣ��ո�������
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
		/// @var �����������
		xgc_uint32 mCapacity;
		/// @var ��������
		xgc_uint16 mType;
		/// @var ָ�����λ�ô洢�ṹָ���ָ��
		XSlot *mSlots; 

	};

	// �����������
	struct CORE_API XBagTrans : public noncopyable
	{
	private:
		enum enMode
		{
			///< �µĸ���,��Ҫ֪ͨ�ͻ���Add
			slot_op_new = 0,
			///< �ɵĸ���,��Ҫ֪ͨ�ͻ���Update
			slot_op_old = 1, 
		};

		struct CORE_API stSlotOperatorInfo ///< ���Ӳ�����Ϣ
		{
			xSlot       nSlot;        ///< ����
			enMode		eMode;        ///< ��������
			xgc_bool    bIsAdd;       ///< Put����Remove
			xgc_uint32  nAfterNum;    ///< �˴β������������Ʒ����
			xgc_uint32  nOperatorNum; ///< �˴β���������
			xgc_uint32  nEvtType;     ///< ��Ʒ��������

			xObject hNewGoods;   ///< ��Ʒ
			xObject	hOldGoods;   ///< ԭ������Ʒ������кϲ�����������ֶξͲ��ǿյ��ˣ�
			xgc_uint32 nOperatorNumTemp; ///< �˴β���������
		};

		typedef xgc_map<xgc_uint32, stSlotOperatorInfo> xSlotOperatorMap; ///< key:��16 �������� ��16 ��������,value: 

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
		/// ȷ������������� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void Commit( xgc_bool bNoticeClinet )
		{
			m_ChangedSlotMap.clear();
		}

		///
		/// ����ȡ�� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void Cancel();

		///
		/// ��dwGoodsNum �� Goods���뱳����,��� nSlot ��= INVALID_SLOT_ID ��ô�Ӹ�
		/// ֵ��λ�ÿ�ʼ��,�����0��ʼ�ţ� bOverLap����Ϊ�Ƿ�ѵ���
		/// ���������������Ŀ
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_bool Put( xObject hGoods, xgc_uint32 mCount, xgc_uint32 nEvtType, xgc_bool bOverlap = true, xSlot nSlot = 0 );

		///
		/// �۳�ָ���ĵ���,�����۳�ʱҲ��۳�,�᷵�ؿ۳�����,������Ҫ�Լ��ж��Ƿ񹻿۳�
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 Remove( xGoodsPtr pShGoods, xgc_uint32 mCount, xgc_uint32 nEvtType, xSlot &nSlot );

		///
		/// ɾ������N�е���ƷM�� ,�����۳�ʱҲ��۳�,�᷵�ؿ۳�����,������Ҫ�Լ��ж��Ƿ񹻿۳�
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 RemoveBySlot( xSlot nSlot, ENPacketType eType, xgc_uint32 mCount, xgc_uint32 nEvtType );

		///
		/// ɾ��ָ��������������Ʒ 
		/// [8/6/2014] create by wuhailin.jerry
		///
		xgc_void EmptySlot( xSlot nSlot );

		///
		/// �Զ����������ȸ���Compare���㷨����,Ȼ���һ������Ʒ���ӣ� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void AutoPackPacket( xgc_uint32 nEvtType, std::function< xgc_bool( const xObject ) > fnCheckGoods = xgc_nullptr );

		///
		/// �������ӻ��ߺϲ�,��dwSlot2��dwNum����Ʒ�ŵ�dwSlot1�У������0����ȡdwSlot2�е����ֵ
		/// dwNum �� = dwSlot2����Ʒ���� ʱ��������������Ʒ������ͬ 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_bool Swap( xSlot Slot1, xSlot Slot2, xgc_uint32 nEvtType, xgc_uint32 nNum = 0, std::function< xgc_bool( const xObject ) > fnCheckGoods = xgc_nullptr );

		///
		/// ������Ʒ���������ӻ��ߺϲ�,��dwSlot2��dwNum����Ʒ�ŵ�dwSlot1�У������0����ȡdwSlot2�е����ֵ
		/// dwNum �� = dwSlot2����Ʒ���� ʱ��������������Ʒ������ͬ 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_bool Swap( xGoodsPtr pGoods1, xGoodsPtr pGoods2, xSlot &Slot1, xSlot &Slot2, xgc_uint32 nEvtType, xgc_uint32 nNum = 0, std::function< xgc_bool( const xObject ) > fnCheckGoods = xgc_nullptr );

		const xgc_list<stSlotOperatorInfo> &GetSlotOperatorList() const { return m_SlotOperatorList; }

	private:
		///
		/// ����ø������� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void RecordOldSlot( ENPacketType eType, xSlot nSlot );

protected:
		XBag &mBag;

		xgc_list<stSlotOperatorInfo> m_SlotOperatorList; ///< ����Commitǰ�Ը��ӵĲ���,����˳�򱣴棬����Ҳ��Ҫ����˳��֪ͨ�ͻ���
		xgc_map<xgc_uint32, stPackageSlot> m_ChangedSlotMap;  // �������ĸ�������,ÿ������ֻ�����һ���������� ��16 �������� ��16 ��������
	};

	//  ��16 �������� ��16 ��������
	inline xgc_uint32 MakeChangedSlotMapKey( ENPacketType eType, xSlot slot )
	{
		return ( ( (xgc_uint16) eType ) << 16 ) + slot;
	}

	//  ��16 �������� ��16 ��������
	inline xgc_void GetChangedSlotMapKey( xgc_uint32 nKey, ENPacketType &eType, xSlot &slot )
	{
		slot = nKey & 0x0000FFFF;
		eType = (ENPacketType) ( nKey >> 16 );
	}
}


#endif