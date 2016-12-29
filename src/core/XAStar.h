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
A start寻路算法实现
该算法的目的是 遍历到开放列表中有了目标点
并且遍历的过程中 会给这些节点画上上下左右斜线等方向指向父节点
然后最后目标点根据这些方向回溯到源点、
寻路实际是每一步都是用贪心法 走最接近（权值最小）目标点的一步

标准坐标系
height
|
|
|
|
0,0------------- width

最后的输出结果 为了节省空间 应该是存线 而不是一个个点
二维数组换成一维数组


空间优化  不能全部申请W*H个BlockStepValue

应该是
{
point
BlockStepValue
}
用std::lowberbond和排序数组  可随机访问的链表
1. 找最小
2. 判断某个point有BlockStepValue否


open列表和close列表还是必须得 不能把close存在全部申请W*H个BlockStepValue的

最后所有的close即为到找到路径时 所有访问过得节点   这个里面会有一条路径存在

最小堆算法在开放列表中找F最小点
*/
/*********************************************************************/
namespace XGC
{
	struct BlockValue
	{

		//九宫格方向
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
			eHOrV = 10,//水平或者纵向上
			eSlash = 14//对角线 没有用根号2*10 是为了更快的速度
		};

		enum eBlockProcessStatus //注意这里必须是递进关系  IsVisited据此判断
		{
			eNotVisit = 0, //这个节点还没有计算 F G 值
			eHasParent,//已经处理过计算过 F G值 也就是有父亲的
			eClose//如果已经作为贪心算法固定过
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

		xgc_uint32 G;//原位置开始的步进值 会累加
		xgc_uint32 H;//启发式预估值 水平和横向的预估值 确定了
		xgc_uint32 F;//总和

		eDirType eDir2Parent;//指向父节点的方向
		//C2DPoint m_pos;
		eBlockProcessStatus m_eBlockStatus;//当前节点的处理情况
		iPoint m_oPoint;
	};

	//AStar用的网格地图
	/*
	优化点:
	1. init和uinit时BlockStepValue可以考虑用对象池 而不是直接new和delete
	2. 用一维的width*height代替二维数组
	*/
	class CORE_API IAStarMap
	{
	public:
		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// 没有无参数构造函数 必须制定长宽
		// ------------------------------------------------ //
		IAStarMap( xgc_uint32 dwWidth, xgc_uint32 dwHeight );
		virtual ~IAStarMap();;
		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// 越界或者阻挡点
		// ------------------------------------------------ //
		virtual xgc_bool IsBlock( xgc_uint32 dwX, xgc_uint32 dwY, xgc_bool bTestBarrier = true ) = 0;

		xgc_uint32 GetWidth() const;
		xgc_uint32 GetHeight() const;

		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// 获取权值结构，外部校验越界
		// ------------------------------------------------ //
		BlockValue * GetBlockValue( xgc_uint32 x, xgc_uint32 y )const;

		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// 获取两块之间的块数，外部控制没有越界
		// ------------------------------------------------ //
		static xgc_uint32 GetBetweenBlocks( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY );
		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// 获取两块之间的块数，内部判断没有越界
		// ------------------------------------------------ //
		xgc_bool GetBetweenBlocksStrict( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY, xgc_uint32 & dwBlocks );

		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// 每次执行完之后要重新清理标记点信息
		// ------------------------------------------------ //
		xgc_void Reset();
	private:
		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// 初始化每一个网格的权值
		// ------------------------------------------------ //
		xgc_void Init( xgc_uint32 dwWidth, xgc_uint32 dwHeight );
		xgc_void UnInit();
		xgc_uint32 m_dwWidth;
		xgc_uint32 m_dwHeight;
		BlockValue* m_searchWorkBench;
	};

	// 算法结束时 m_searchWorkBench中的每个元素eDir2Parent即为所求
	//这个应该作为图像
	class CORE_API AStarIPathSearcherImpl
	{
	public:
		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// 设置网格地图对象  如果client地图不符合条件 这里要作适配器
		// ------------------------------------------------ //
		xgc_void SetNetMap( IAStarMap* pNetMap );

		// ------------------------------------------------ //
		// [12/12/2014 zhangyupeng]
		// 搜索路径
		// bool bNeedConcis = false 表示简洁路径， 只存折线点 否则 存每一个点
		// ------------------------------------------------ //
		xgc_bool SearchWorkBench( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY, xgc_vector<iPoint> &path, xgc_bool bTestBarrier = true, xgc_bool bNeedConcis = false );

		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		// 打印当前状态调试信息
		// ------------------------------------------------ //
		xgc_void dump();
	private:

		// ------------------------------------------------ //
		// [2/24/2014 baomin]
		//只有在SearchWorkBench返回true的时候才有效
		// SearchWorkBench之后获取path，包括起点 终点本身
		//bool bNeedConcis = false 表示简洁路径， 只存折线点
		// ------------------------------------------------ //
		xgc_void GetPath( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY, xgc_vector<iPoint> &path, xgc_bool bNeedConcis = false );

		//计算孩子的权值 和 赋值父节点
		xgc_void ProcessChildren( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY, xgc_bool bTestBarrier );

		//被贪心算法固定过
		//todo 改进到每个上面BlockStepValue上面用作结束标志
		//可能越界 交给IsBlock
		xgc_bool IsClose( xgc_uint32 dwX, xgc_uint32 dwY );

		//是否有处理过
		xgc_bool IsVisited( xgc_uint32 dwX, xgc_uint32 dwY );

		xgc_bool IsVisited( BlockValue * pBlockStepValue );

		IAStarMap* m_pNetMap;

		xgc_vector<BlockValue*> m_vecOpenList; ///<开放列表数据结构用vector,操作算法用heap
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