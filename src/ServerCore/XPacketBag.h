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

	// ��������
	enum ENPacketType : xgc_uint16
	{
		EN_PacketBegin  = 0,
		EN_PackageItems = 0,   ///< ��ɫ����
		EN_StorageItems = 1,   ///< ��ɫ�ֿ�
		EN_BodyItems    = 2,   ///< ��ɫ������Ʒ
		EN_PetItems     = 3,   ///< ��ɫ������
		EN_ReBackItems  = 4,   ///< �ع���
		EN_TaskPackage  = 5,   ///< �������
		EN_RidePackage  = 6,   ///< �������
		EN_StuffPackage = 7,   ///< ���ϰ���
		EN_TitlePackage = 8,   ///< �ƺŰ���
		EN_PackageEnd,         ///< ������
	};

	enum ENPacketMode : xgc_uint8
	{
		EN_PacketNewSlot = 0, ///< �µĸ���,��Ҫ֪ͨ�ͻ���ADD
		EN_PacketOldSlot = 1, ///< �ɵĸ���,��Ҫ֪ͨ�ͻ���Update
		EN_PacketGet = 2,
	};

	struct CORE_API stSlotOperatorInfo ///< ���Ӳ�����Ϣ
	{
		ENPacketMode  ENMode;       ///< ��������
		xgc_uint32    nAfterNum;    ///< �˴β������������Ʒ����
		xgc_uint32    nOperatorNum; ///< �˴β���������
		xGoodsPtr     pxGoods;      ///< ��Ʒ
		xSlot         nSlot;        ///< ����
		ENPacketType  eType;        ///< ��������
		xgc_bool      bIsAdd;       ///< Put����Remove
		xgc_uint32    nEvtType;     ///< ��Ʒ��������

		xGoodsPtr     pxOldGoods;   ///< ԭ������Ʒ������кϲ�����������ֶξͲ��ǿյ��ˣ�
		xgc_uint32    nOperatorNumTemp; ///< �˴β���������
	};

	typedef xgc_map<xgc_uint32, stSlotOperatorInfo> xSlotOperatorMap; ///< key:��16 �������� ��16 ��������,value: 

	// ����ÿ��λ�õĴ洢��Ϣ ���� xGoodsPtr �Ͳ�������POD��
	struct CORE_API stPackageSlot
	{
		xGoodsPtr pShGoods;   // �洢����Ʒ
		xgc_uint32 nGoodsNum; // ��λ����Ʒ����

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
		/// �����Ƿ��� 
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool IsFull() const
		{
			if( pShGoods && pShGoods->GetOverlapNum() == nGoodsNum ) // ��Ʒ����
			{
				return true;
			}

			return false;
		}

		///
		/// �Ƿ�����ͬ��DbId 
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool IsSameDbId( xgc_uint32 nDbId ) const
		{
			if( IsEmpty() ) // û�з���Ʒ
			{
				return false;
			}

			if( pShGoods->GetAttribute( attrDbIndex ).toUnsigned() != nDbId ) // ��Ʒ��һ��
			{
				return false;
			}

			return true;
		}

		///
		/// �Ƿ�����ͬ��Ʒ ��ȫ��ͬ ע�� ������ֻ���ж���Ψһ��Ʒ
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_bool IsSameGoods(xObject hGoods) const
		{
			if( IsEmpty() ) // û�з���Ʒ
			{
				return false;
			}

			if( pShGoods->GetObjectID() != hGoods ) // ��Ʒ��һ��
			{
				return false;
			}

			return true;
		}

		///
		/// ��Ʒ�Ƿ������������� 
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

			if( !pShGoods->IsSameGoods( pGoods ) ) // ��Ʒ��һ��
			{
				return false;
			}

			if( pShGoods->GetOverlapNum() == nGoodsNum ) // ��Ʒ����
			{
				return false;
			}

			return true;
		}

		///
		/// ������Ʒ,���سɹ�����ĸ��� 
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
			if( nCanFillNum >= nNum ) // û�г����ɶѵ��ܸ���
			{
				nGoodsNum += nNum;
				return nNum;
			}
			else // �Ų���
			{
				nGoodsNum += nCanFillNum;
				return nCanFillNum;
			}
		}

		///
		/// ɾ����Ʒ�����سɹ�ɾ���ĸ��� 
		/// [9/26/2014] create by wuhailin.jerry
		///
		xgc_uint32 DelGoods( xgc_uint32 nNum )
		{
			// �����,˵������Ҫ�۳���λ��
			if( nGoodsNum > nNum ) // ��Ʒ���۳�
			{
				nGoodsNum -= nNum;
				return nNum;
			}
			else // ����ط������۳����߿۳�֮��Ϊ��
			{
				xgc_int32 nSubNum = nGoodsNum;

				SetEmpty();
				return nSubNum;
			}
		}
	};

	struct CORE_API XPacketBagOperator;

	// ������
	class CORE_API XPacketBag : public XObject
	{
	public:
		DECLARE_XCLASS();

		// eType ��������
		explicit XPacketBag( ENPacketType eType ) : m_nCapacity( 0 ), m_pSlotArray( xgc_nullptr ), m_PacketType( eType )
		{

		}

		virtual ~XPacketBag();

		XPacketBag( const XPacketBag &) = delete;
		XPacketBag( XPacketBag &&) = delete;

		///
		/// ���������ֱ𷵻���Ӧ��������Ҫ�����ı��� 
		/// [1/21/2015] create by wuhailin.jerry
		///
		virtual XPacketBag& getCurBag(xgc_uint32 nDbId) { return *this; }

		virtual XPacketBag& getBagByType(ENPacketType eType) = 0;

		///
		/// ����ӵ���� 
		/// [1/21/2015] create by wuhailin.jerry
		///
		xgc_void SetPacketOwner( xObject hRole ) { m_hRole = hRole; }

		///
		/// ��ȡ���������� ��ɫ 
		/// [9/12/2014] create by wuhailin.jerry
		///
		xObject GetPacketOwner() const { return m_hRole; }

		///
		/// ��ȡ�������� 
		/// [9/12/2014] create by wuhailin.jerry
		///
		ENPacketType GetPacketType() const { return m_PacketType; }

		///
		/// ���ñ������������������,���ȸ��ƣ�Ȼ����ɾ��֮ 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void SetCapacity( xgc_uint32 nCapacity );

		///
		/// ��ȡ�������� 
		/// [7/30/2014] create by wuhailin.jerry
		///
		xgc_uint32 GetCapacity() const { return m_nCapacity; }

		///
		/// ���ñ���������� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 GetMaxCapacity() const { return m_nMaxCapacity; }

		///
		/// ���ñ������� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void SetMaxCapactiy( xgc_uint32 nMaxCapacity ) { m_nMaxCapacity = nMaxCapacity; }

		///
		/// ��ȡ���ӵ���Ϣ 
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
		/// ���ø����ڵ���Ϣ 
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
		/// ��ӻ���ɾ����������Ʒ 
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
		/// �������� 
		/// [10/2/2014] create by wuhailin.jerry
		///
		xgc_void SwapSlot(xSlot nSlot1, xSlot nSlot2)
		{
			XGC_ASSERT_RETURN( nSlot1 < m_nCapacity && nSlot2 < m_nCapacity, xgc_void( 0 ) );
			std::swap(m_pSlotArray[nSlot1], m_pSlotArray[nSlot2]);
		}

		///
		///  ���������ҳ������õĸ���
		/// @ nSlot: 0:��0��ʼ��,�����nSlot��ʼ
		/// @ bOverLap:false ���ѵ�,true�ѵ�
		/// [8/27/2014] create by wuhailin.jerry
		///
		xSlot FindSlot( xGoodsPtr pShGoods, xSlot nSlot = 0, xgc_bool bOverLap = true );

		///
		/// ������ƷID��ȡ��Ʒ��Ϣ,ֻ�õ���һ�� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xGoodsPtr GetGoodsPtrByDbId( xgc_uint32 nGoodsId );

		///
		///  ���ݸ��ӻ�ȡ��Ʒ��Ϣ
		/// [9/1/2014] create by wuhailin.jerry
		///
		xGoodsPtr TakeBySlot( xSlot Slot ) const;

		///
		/// ���ݸ��ӻ�ȡ��Ʒ��Ϣ�Լ���Ʒ���� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xGoodsPtr TakeBySlot( xSlot Slot, xgc_uint32 &nGoodsNum ) const;

		///
		/// ��ȡ������dwGoodsId�ĸ��� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 GetByGoodsId( xgc_uint32 nGoodsId );


		xgc_uint32 GetByGoodsId( xgc_uint32 nGoodsId, xSlotOperatorMap &xSlotMap );
		
		///
		/// ��ȡ��Ʒ���ڸ����Լ���Ʒ���� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 GetByGoodsPtr( xGoodsPtr pShGoods, xSlot &nSlot ) const;

		///
		/// ����ObjectId��ȡ��Ʒ���ڸ����Լ���Ʒ���� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 GetByObjectId( xObject hObject, xSlot &nSlot ) const;

		///
		/// ��ȡ���и����ܸ��� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 GetEmptySlotNum() const;
		/// ��ȡ��һ�����и��ӵ�λ�� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xSlot GetEmptySlot() const;
		///
		/// ����������������ش��� 
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
	/// ������������ 
	/// [7/30/2014] create by wuhailin.jerry
	///
	xgc_bool AddCapacity( xgc_uint32 nAddNum )
	{
		if (0 == nAddNum)
		{
			return false;
		}

		if( m_nCapacity + nAddNum > m_nMaxCapacity ) // �����������
		{
			return false;
		}

		SetCapacity( m_nCapacity + nAddNum );

		return true;
	}

protected:
		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	�����ӽڵ�ǰ����
		// return :		true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		//---------------------------------------------------//
		virtual xgc_bool PreAddChild( XObject* pChild, xgc_lpcvoid lpContext ) { return true; }

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	�����ӽڵ�����
		//---------------------------------------------------//
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext ) {}

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	�����ӽڵ�ǰ����
		// return :		true - ȷ���Ƴ��ӽڵ�, false - �ӽڵ㱻���,�Ƴ��ӽڵ�ʧ��.
		//---------------------------------------------------//
		virtual xgc_bool PreRemoveChild( XObject* pChild, xgc_bool bRelease ) { return true; }

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	ɾ���ӽڵ�����,��ʱ������δ��ɾ��
		//---------------------------------------------------//
		virtual xgc_void OnRemoveChild( XObject* pChild, xgc_bool bRelease ) {}

		///
		/// ���ٶ���ʱ���� 
		/// [6/9/2014] create by albert.xu
		/// \virtual XObject���麯��
		///
		virtual xgc_void OnDestroy() {}

		///
		/// ��ȡ����ʣ��ո�������
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
		stPackageSlot *m_pSlotArray; // ָ�����λ�ô洢�ṹָ���ָ��
		xgc_uint32 m_nCapacity;        // �����������

		xgc_uint32 m_nMaxCapacity; ///< �������

protected:
		xObject m_hRole;             ///< ����������
		ENPacketType m_PacketType;   ///< ��������
	};

	// �����������
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
		/// ȷ������������� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		virtual xgc_void TransactionCommit(xgc_bool bNoticeClinet)
		{
			m_ChangedSlotMap.clear();
		}

		///
		/// ����ȡ�� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void xTransactionCancel();

		///
		/// ��dwGoodsNum �� Goods���뱳����,��� nSlot ��= INVALID_SLOT ��ô�Ӹ�
		/// ֵ��λ�ÿ�ʼ��,�����0��ʼ�ţ� bOverLap����Ϊ�Ƿ�ѵ���
		/// ���������������Ŀ
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 Put( xGoodsPtr pShGoods, xgc_uint32 nGoodsNum, xgc_uint32 nEvtType,
			xSlot nSlot = INVALID_SLOT, xgc_bool bOverLap = true );

		///
		/// �۳�ָ���ĵ���,�����۳�ʱҲ��۳�,�᷵�ؿ۳�����,������Ҫ�Լ��ж��Ƿ񹻿۳�
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 Remove( xGoodsPtr pShGoods, xgc_uint32 nGoodsNum, xgc_uint32 nEvtType, xSlot &nSlot );

		///
		/// ɾ������N�е���ƷM�� ,�����۳�ʱҲ��۳�,�᷵�ؿ۳�����,������Ҫ�Լ��ж��Ƿ񹻿۳�
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_uint32 RemoveBySlot( xSlot nSlot, ENPacketType eType, xgc_uint32 nGoodsNum, xgc_uint32 nEvtType );

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
		/// ����ø������� 
		/// [8/27/2014] create by wuhailin.jerry
		///
		xgc_void RecordOldSlot( ENPacketType eType, xSlot nSlot );

protected:
		XPacketBag &m_Bag;

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