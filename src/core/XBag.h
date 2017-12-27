#pragma once
#ifndef __XBAG_H__
#define __XBAG_H__
#include "XGoods.h"

namespace xgc
{
	typedef xgc_uint32 xSlot;

	#define INVALID_SLOT_ID (xSlot)(-1)

	/// 
	/// \brief ����ÿ��λ�õĴ洢��Ϣ
	/// \date 11/6/2017
	/// \author xufeng04
	///
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
		/// \brief �Ƿ�����ͬ����Ʒ���� 
		/// \date 11/6/2017
		/// \author xufeng04
		/// \param ��Ʒ����
		/// \return 
		///
		xgc_bool isSameGoods( xgc_uint32 nGoodsIndex ) const
		{
			if( isEmpty() ) // û�з���Ʒ
				return false;

			auto pGoods = ObjectCast< XGoods >( mGoods );
			if( xgc_nullptr == pGoods )
				return false;

			if( pGoods->getValue< xgc_uint32 >( XGoods::Index ) != nGoodsIndex ) // ��Ʒ��һ��
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
		/// \brief ������Ʒ�����سɹ�����ĸ��� 
		/// \date 11/6/2017
		/// \author xufeng04
		/// \param hGoods ��Ʒ���
		/// \param nCount ��Ʒ����
		/// \return �������Ʒ������ 0 - δ�����κ���Ʒ
		///
		xgc_uint32 putGoods( xObject hGoods, xgc_uint32 nCount )
		{
			return putGoods( ObjectCast< XGoods >( hGoods ), nCount );
		}

		///
		/// \brief ������Ʒ�����سɹ�����ĸ��� 
		/// \date 11/6/2017
		/// \author xufeng04
		/// \param pGoods ��Ʒָ��
		/// \param nCount ��Ʒ����
		/// \return �������Ʒ������ 0 - δ�����κ���Ʒ
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
		/// \brief ɾ����Ʒ�����سɹ�ɾ���ĸ���
		/// \date 11/6/2017
		/// \author xufeng04
		/// \param nCount ɾ���ĸ���
		/// \return ʵ��ɾ������Ʒ����
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

	///
	/// \brief �����¼�ID
	/// \date 11/6/2017
	/// \author xufeng04
	///
	enum enBagEvent
	{
		evt_bag_change,
		evt_bag_update,
	};

	///
	/// \brief  ������ 
	/// \date 11/6/2017
	/// \author xufeng04
	///
	class CORE_API XBag : public XObject
	{
		DECLARE_XCLASS();
	public:
		///
		/// \brief �������Ӹ���
		/// \date 11/6/2017
		/// \author xufeng04
		///
		struct SlotEvent
		{
			/// ����ת��
			XObjectEvent cast;
			/// �����ĸ��ӱ��
			xSlot nSlot;
			/// ���ӱ��
			struct
			{
				/// ��������������е���ƷID
				xObject	hGoods;
				/// �˴β���������
				xgc_uint32 nCount;
			} Befor, After;
		};

		///
		/// \brief ���������¼� 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		struct TransEvent
		{
			/// ����ת��
			XObjectEvent cast;
			/// �����ı���
			xObject hBag;
			/// ��������Ʒ
			xObject hGoods;
			/// ����������
			xgc_long nCount;
		};

	public:
		///
		/// \brief �������� 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		XBag( xgc_uint32 nType, xgc_uint32 nCapacity );

		///
		/// \brief �������� 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		~XBag();

		///
		/// \brief ������Ĺ��� 
		/// \date 12/27/2017
		/// \author albert.xu
		///
		XBag( const XBag &) = delete;
		XBag( XBag &&) = delete;

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

		///
		/// \brief ���������ȸ���Compare���㷨����,Ȼ���һ������Ʒ���ӣ�  
		/// \date 11/6/2017
		/// \author xufeng04
		/// \param fnCompair �Ƚ��㷨
		/// \param bOverlap �Ƿ�ѵ�
		///
		xgc_void Sort( const std::function< xgc_bool( const XSlot&, const XSlot& ) > &fnCompair, xgc_bool bOverlap = true );

	protected:
		///
		/// \brief ���ٶ���ʱ���� 
		/// \author albert.xu
		/// \date 6/9/2014
		///
		virtual xgc_void OnDestroy() override
		{
			for( xSlot i = 0; i < mCapacity; ++i )
				mSlots[i].delGoods( -1 );
		}

	protected:
		/// @var ��������
		xgc_uint32 mType;
		/// @var �����������
		xgc_uint32 mCapacity;
		/// @var ָ�����λ�ô洢�ṹָ���ָ��
		XSlot *mSlots; 
	};

	// �����������
	struct CORE_API XBagTrans : public noncopyable
	{
	protected:
		enum enOperator
		{
			/// �����Ʒ
			slot_op_put = 0,
			/// ɾ����Ʒ
			slot_op_del = 1,
			/// ������Ʒ
			slot_op_swap = 2,
			/// ת����Ʒ
			slot_op_move = 3,
		};

		struct CORE_API stSlotCommand ///< ���Ӳ�����Ϣ
		{
			/// �������
			xgc_uint32	nSequence;
			/// �����İ���
			xObject		hBag;
			/// �����ĸ��ӱ��
			xSlot       nSlot;
			/// ��������
			enOperator	eOperator;
			/// ���ӱ��
			struct
			{
				/// ��������������е���ƷID
				xObject		hGoods;
				/// �˴β���������
				xgc_uint32  nCount;
			} Befor, After;
		};

		/// �������
		xgc_uint32 mSequence;
		/// �����ı���
		XBag &mBag;
		/// �����ļ�¼
		xgc::vector< stSlotCommand > mSlotCommands;

	public:
		///
		/// \brief ���������� 
		/// \date 11/6/2017
		/// \author xufeng04
		/// \param ����Ŀ��
		///
		explicit XBagTrans(XBag &Bag);

		///
		/// \brief ������������ 
		/// \date 11/6/2017
		/// \author xufeng04
		///
		~XBagTrans();

		///
		/// \brief �ύ�������� 
		/// \authro albert.xu
		/// \date 2017/11/06
		///
		xgc_void Commit();

		///
		/// \brief ����ȡ�� 
		/// \authro albert.xu
		/// \date 2017/11/06
		///
		xgc_void Cancel();

		///
		/// \brief ����Ʒ���뱳��
		/// \authro albert.xu
		/// \date 2017/11/06
		/// \return ȫ�����뷵�� true�����򷵻� false
		///
		xgc_bool Put( xObject hGoods, xgc_uint32 nCount, xSlot nStartSlot = 0 );

		///
		/// \brief �۳�ָ���ĵ���
		/// \authro albert.xu
		/// \date 2017/11/06
		/// \return ȫ��ɾ������ true�����򷵻� false
		///
		xgc_bool Del( xObject hGoods, xgc_uint32 nCount, xSlot nStartSlot = 0 );

		///
		/// \brief ��������
		/// \date 2017/11/06
		/// \author albert.xu
		///
		xgc_bool Swap( xSlot nFrom, xSlot nTo, xObject hTarget = INVALID_OBJECT_ID );

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
		xgc_uint32 Move( xSlot nFrom, xSlot nTo, xgc_uint32 nCount )
		{
			return MoveTo( INVALID_OBJECT_ID, nTo, nFrom, nCount );
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
		xgc_uint32 MoveTo( xObject hTarget, xSlot nTo, xSlot nFrom, xgc_uint32 nCount );

		///
		/// \brief ���ز��� 
		/// \date 11/6/2017
		/// \author xufeng04
		/// \return �����Ƿ�ɹ�
		///
		xgc_bool Undo();
	};
}


#endif