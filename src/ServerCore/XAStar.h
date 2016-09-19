#ifndef _XAStar_H_
#define _XAStar_H_

#ifdef CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif	//CORE_EXPORTS
#pragma warning( disable:4251 )

#include "stdafx.h"
#include "XGeometry.h"


/*********************************************************************/
/*
FileName:   XAStar.h
Author:     zhangyupeng
A startѰ·�㷨ʵ��
���㷨��Ŀ���� �����������б�������Ŀ���
���ұ����Ĺ����� �����Щ�ڵ㻭����������б�ߵȷ���ָ�򸸽ڵ�
Ȼ�����Ŀ��������Щ������ݵ�Դ�㡢
Ѱ·ʵ����ÿһ��������̰�ķ� ����ӽ���Ȩֵ��С��Ŀ����һ��

��׼����ϵ
height
|
|
|
|
0,0------------- width

���������� Ϊ�˽�ʡ�ռ� Ӧ���Ǵ��� ������һ������
��ά���黻��һά����


�ռ��Ż�  ����ȫ������W*H��BlockStepValue

Ӧ����
{
point
BlockStepValue
}
��std::lowberbond����������  ��������ʵ�����
1. ����С
2. �ж�ĳ��point��BlockStepValue��


open�б��close�б��Ǳ���� ���ܰ�close����ȫ������W*H��BlockStepValue��

������е�close��Ϊ���ҵ�·��ʱ ���з��ʹ��ýڵ�   ����������һ��·������

��С���㷨�ڿ����б�����F��С��
*/
/*********************************************************************/
namespace XGC
{
	struct BlockValue
	{

		//�Ź�����
		enum eDirType
		{
			eSelf = 0,
			eUp,
			eLeftUp,
			eLeft,
			eLeftDown,
			eDown,
			eRightDown,
			eRight,
			eRightUp
		};

		enum eDirValue
		{
			eHOrV = 10,//ˮƽ����������
			eSlash = 14//�Խ��� û���ø���2*10 ��Ϊ�˸�����ٶ�
		};

		enum eBlockProcessStatus //ע����������ǵݽ���ϵ  IsVisited�ݴ��ж�
		{
			eNotVisit = 0, //����ڵ㻹û�м��� F G ֵ
			eHasParent,//�Ѿ����������� F Gֵ Ҳ�����и��׵�
			eClose//����Ѿ���Ϊ̰���㷨�̶���
		};

		BlockValue()
		{
			Reset();
		}

		xgc_void Reset()
		{
			G = NULL;
			H = NULL;
			F = NULL;
			eDir2Parent = eSelf;
			m_eBlockStatus = eNotVisit;
		}

		xgc_uint32 G;//ԭλ�ÿ�ʼ�Ĳ���ֵ ���ۼ�
		xgc_uint32 H;//����ʽԤ��ֵ ˮƽ�ͺ����Ԥ��ֵ ȷ����
		xgc_uint32 F;//�ܺ�

		eDirType eDir2Parent;//ָ�򸸽ڵ�ķ���
		//C2DPoint m_pos;
		eBlockProcessStatus m_eBlockStatus;//��ǰ�ڵ�Ĵ������
		iPoint m_oPoint;
	};

	//AStar�õ������ͼ
	/*
	�Ż���:
	1. init��uinitʱBlockStepValue���Կ����ö���� ������ֱ��new��delete
	2. ��һά��width*height�����ά����
	*/
	class CORE_API IAStarMap
	{
	public:
		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// û���޲������캯�� �����ƶ�����
		// ------------------------------------------------ //
		IAStarMap( xgc_uint32 dwWidth, xgc_uint32 dwHeight );
		virtual ~IAStarMap();;
		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// Խ������赲��
		// ------------------------------------------------ //
		virtual xgc_bool IsBlock( xgc_uint32 dwX, xgc_uint32 dwY, xgc_bool bTestBarrier = true ) = 0;

		xgc_uint32 GetWidth() const;
		xgc_uint32 GetHeight() const;

		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// ��ȡȨֵ�ṹ���ⲿУ��Խ��
		// ------------------------------------------------ //
		BlockValue * GetBlockValue( xgc_uint32 x, xgc_uint32 y )const;

		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// ��ȡ����֮��Ŀ������ⲿ����û��Խ��
		// ------------------------------------------------ //
		static xgc_uint32 GetBetweenBlocks( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY );
		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// ��ȡ����֮��Ŀ������ڲ��ж�û��Խ��
		// ------------------------------------------------ //
		xgc_bool GetBetweenBlocksStrict( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY, xgc_uint32 & dwBlocks );

		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// ÿ��ִ����֮��Ҫ���������ǵ���Ϣ
		// ------------------------------------------------ //
		xgc_void Reset();
	private:
		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// ��ʼ��ÿһ�������Ȩֵ
		// ------------------------------------------------ //
		xgc_void Init( xgc_uint32 dwWidth, xgc_uint32 dwHeight );
		xgc_void UnInit();
		xgc_uint32 m_dwWidth;
		xgc_uint32 m_dwHeight;
		BlockValue* m_searchWorkBench;
	};

	// �㷨����ʱ m_searchWorkBench�е�ÿ��Ԫ��eDir2Parent��Ϊ����
	//���Ӧ����Ϊͼ��
	class CORE_API AStarIPathSearcherImpl
	{
	public:
		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// ���������ͼ����  ���client��ͼ���������� ����Ҫ��������
		// ------------------------------------------------ //
		xgc_void SetNetMap( IAStarMap* pNetMap );

		// ------------------------------------------------ //
		// [12/12/2014 zhangyupeng]
		// ����·��
		// bool bNeedConcis = false ��ʾ���·���� ֻ�����ߵ� ���� ��ÿһ����
		// ------------------------------------------------ //
		xgc_bool SearchWorkBench( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY, xgc_vector<iPoint> &path, xgc_bool bTestBarrier = true, xgc_bool bNeedConcis = false );

		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// ��ӡ��ǰ״̬������Ϣ
		// ------------------------------------------------ //
		xgc_void dump();
	private:

		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		//ֻ����SearchWorkBench����true��ʱ�����Ч
		// SearchWorkBench֮���ȡpath��������� �յ㱾��
		//bool bNeedConcis = false ��ʾ���·���� ֻ�����ߵ�
		// ------------------------------------------------ //
		xgc_void GetPath( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY, xgc_vector<iPoint> &path, xgc_bool bNeedConcis = false );

		//���㺢�ӵ�Ȩֵ �� ��ֵ���ڵ�
		xgc_void ProcessChildren( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY, xgc_bool bTestBarrier );

		//��̰���㷨�̶���
		//todo �Ľ���ÿ������BlockStepValue��������������־
		//����Խ�� ����IsBlock
		xgc_bool IsClose( xgc_uint32 dwX, xgc_uint32 dwY );

		//�Ƿ��д����
		xgc_bool IsVisited( xgc_uint32 dwX, xgc_uint32 dwY );

		xgc_bool IsVisited( BlockValue * pBlockStepValue );

		IAStarMap* m_pNetMap;

		xgc_vector<BlockValue*> m_vecOpenList; ///<�����б����ݽṹ��vector,�����㷨��heap
	};

	struct HeapCompare_f
	{
		bool operator() ( const BlockValue* pLeft, const BlockValue* pRight )const
		{
			if( !pLeft || !pRight )
			{
				return false;
			}
			return pLeft->F > pRight->F;
		}
	};

}
#endif