#include "stdafx.h"
#include "XAStar.h"

namespace XGC
{
#define MaxFindChildCount 512

	//循环顺序 Y: height->0  X:0->width
	xgc_void AStarIPathSearcherImpl::dump()
	{
		FUNCTION_BEGIN;
		BlockValue * pCurBlockValue = NULL;
		//j为外层！
		//for (xgc_uint32 j = m_pNetMap->GetHeight()-1 ; j >= 0 ; --j)//这种写法 j==0时进入了 然后--了  将会得到一个很大的正数 并且还会循环！
		for( xgc_uint32 j = m_pNetMap->GetHeight(); j > 0; --j )
		{
			printf( " " );
			for( xgc_uint32 i = 0; i < m_pNetMap->GetWidth(); ++i )
			{
				//貌似不必要
				//m_searchWorkBench[i][j].eDir2Parent = eSelf;
				//m_searchWorkBench[i][j].H = m_pNetMap->GetBetweenBlocks(i, j, dwTargetX, dwTargetY) * BlockStepValue::eHOrV;


				//switch (m_searchWorkBench[i][j]->eDir2Parent)
				pCurBlockValue = m_pNetMap->GetBlockValue( i, j - 1 );
				XGC_ASSERT( pCurBlockValue );
				switch( pCurBlockValue->eDir2Parent )
				{
					case BlockValue::eSelf:
					{
						printf( "%3s", "O" );
						break;
					}
					case BlockValue::eUp:
					{
						printf( "%3s", "↑" );
						break;
					}
					case BlockValue::eLeftUp:
					{
						printf( "%3s", "↖" );
						break;
					}
					case BlockValue::eLeft:
					{
						printf( "%3s", "←" );
						break;
					}
					case BlockValue::eLeftDown:
					{
						printf( "%3s", "↙" );
						break;
					}
					case BlockValue::eDown:
					{
						printf( "%3s", "↓" );
						break;
					}
					case BlockValue::eRightDown:
					{
						printf( "%3s", "↘" );
						break;
					}
					case BlockValue::eRight:
					{
						printf( "%3s", "→" );
						break;
					}
					case BlockValue::eRightUp:
					{
						printf( "%3s", "↗" );
						break;
					}
					default:
					break;
				}
				printf( " " );
			}
			printf( "\r\n" );
		}

		printf( "\r\n" );
		FUNCTION_END;
	}

	xgc_void AStarIPathSearcherImpl::ProcessChildren( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY, xgc_bool bTestBarrier )
	{
		FUNCTION_BEGIN;
		//BlockValue * pParent = m_searchWorkBench[dwSrcX][dwSrcY];
		BlockValue * pParent = m_pNetMap->GetBlockValue( dwSrcX, dwSrcY );//[dwSrcX][dwSrcY]
		XGC_ASSERT( pParent );
		BlockValue * pCur = NULL;
		xgc_uint32 dwCurX = NULL;
		xgc_uint32 dwCurY = NULL;



		//上 没有越界，阻挡点并且没有固定
		dwCurX = dwSrcX;
		dwCurY = dwSrcY + 1;
		if( !m_pNetMap->IsBlock( dwCurX, dwCurY, bTestBarrier ) && !IsClose( dwCurX, dwCurY ) )
		{
			//pCur = m_searchWorkBench[dwCurX][dwCurY];
			pCur = m_pNetMap->GetBlockValue( dwCurX, dwCurY );
			XGC_ASSERT( pCur );
			if( !IsVisited( pCur ) )
			{
				pCur->m_eBlockStatus = BlockValue::eHasParent;
				pCur->eDir2Parent = BlockValue::eDown;//指向父亲
				pCur->H = BlockValue::eHOrV * m_pNetMap->GetBetweenBlocks( dwCurX, dwCurY, dwTargetX, dwTargetY );
				pCur->G = pParent->G + BlockValue::eHOrV;//直接一格
				pCur->F = pCur->H + pCur->G;
				pCur->m_oPoint.SetPoint( dwCurX, dwCurY );
				m_vecOpenList.push_back( pCur );
				std::push_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
			}
			else
			{
				//更新G和F
				//如果父节点G+位移值 比子节点 G小    节点的H是不会改变的
				if( pParent->G + BlockValue::eHOrV < pCur->G )
				{
					pCur->G = pParent->G + BlockValue::eHOrV;
					pCur->eDir2Parent = BlockValue::eDown;//指向新父亲
					pCur->F = pCur->H + pCur->G;

					std::make_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
				}
			}

		}

		//左上
		dwCurX = dwSrcX - 1;
		dwCurY = dwSrcY + 1;
		if( !m_pNetMap->IsBlock( dwCurX, dwCurY, bTestBarrier ) && !IsClose( dwCurX, dwCurY ) )
		{
			//pCur = m_searchWorkBench[dwCurX][dwCurY];
			pCur = m_pNetMap->GetBlockValue( dwCurX, dwCurY );
			XGC_ASSERT( pCur );
			if( !IsVisited( pCur ) )
			{
				pCur->m_eBlockStatus = BlockValue::eHasParent;
				pCur->eDir2Parent = BlockValue::eRightDown;//指向父亲
				pCur->H = BlockValue::eHOrV * m_pNetMap->GetBetweenBlocks( dwCurX, dwCurY, dwTargetX, dwTargetY );
				pCur->G = pParent->G + BlockValue::eSlash;//斜着一格
				pCur->F = pCur->H + pCur->G;
				pCur->m_oPoint.SetPoint( dwCurX, dwCurY );
				m_vecOpenList.push_back( pCur );
				std::push_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
			}
			else
			{
				//更新G和F
				//如果父节点G+位移值 比子节点 G小    节点的H是不会改变的
				if( pParent->G + BlockValue::eSlash < pCur->G )
				{
					pCur->G = pParent->G + BlockValue::eSlash;
					pCur->eDir2Parent = BlockValue::eRightDown;//指向新父亲
					pCur->F = pCur->H + pCur->G;
					std::make_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
				}
			}
		}

		//左
		dwCurX = dwSrcX - 1;
		dwCurY = dwSrcY;
		if( !m_pNetMap->IsBlock( dwCurX, dwCurY, bTestBarrier ) && !IsClose( dwCurX, dwCurY ) )
		{
			//pCur = m_searchWorkBench[dwCurX][dwCurY];
			pCur = m_pNetMap->GetBlockValue( dwCurX, dwCurY );
			XGC_ASSERT( pCur );
			if( !IsVisited( pCur ) )
			{
				pCur->m_eBlockStatus = BlockValue::eHasParent;
				pCur->eDir2Parent = BlockValue::eRight;//指向父亲
				pCur->H = BlockValue::eHOrV * m_pNetMap->GetBetweenBlocks( dwCurX, dwCurY, dwTargetX, dwTargetY );
				pCur->G = pParent->G + BlockValue::eHOrV;//
				pCur->F = pCur->H + pCur->G;
				pCur->m_oPoint.SetPoint( dwCurX, dwCurY );
				m_vecOpenList.push_back( pCur );
				std::push_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
			}
			else
			{
				//更新G和F
				//如果父节点G+位移值 比子节点 G小    节点的H是不会改变的
				if( pParent->G + BlockValue::eHOrV < pCur->G )
				{
					pCur->G = pParent->G + BlockValue::eHOrV;
					pCur->eDir2Parent = BlockValue::eRight;//指向新父亲
					pCur->F = pCur->H + pCur->G;
					std::make_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
				}
			}
		}

		//左下
		dwCurX = dwSrcX - 1;
		dwCurY = dwSrcY - 1;
		if( !m_pNetMap->IsBlock( dwCurX, dwCurY, bTestBarrier ) && !IsClose( dwCurX, dwCurY ) )
		{
			//pCur = m_searchWorkBench[dwCurX][dwCurY];
			pCur = m_pNetMap->GetBlockValue( dwCurX, dwCurY );
			XGC_ASSERT( pCur );
			if( !IsVisited( pCur ) )
			{
				pCur->m_eBlockStatus = BlockValue::eHasParent;
				pCur->eDir2Parent = BlockValue::eRightUp;//指向父亲
				pCur->H = BlockValue::eHOrV * m_pNetMap->GetBetweenBlocks( dwCurX, dwCurY, dwTargetX, dwTargetY );
				pCur->G = pParent->G + BlockValue::eSlash;//斜着一格
				pCur->F = pCur->H + pCur->G;
				pCur->m_oPoint.SetPoint( dwCurX, dwCurY );
				m_vecOpenList.push_back( pCur );
				std::push_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );

			}
			else
			{
				//更新G和F
				//如果父节点G+位移值 比子节点 G小    节点的H是不会改变的
				if( pParent->G + BlockValue::eSlash < pCur->G )
				{
					pCur->G = pParent->G + BlockValue::eSlash;
					pCur->eDir2Parent = BlockValue::eRightUp;//指向新父亲
					pCur->F = pCur->H + pCur->G;
					std::make_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );

				}
			}
		}

		//下
		dwCurX = dwSrcX;
		dwCurY = dwSrcY - 1;
		if( !m_pNetMap->IsBlock( dwCurX, dwCurY, bTestBarrier ) && !IsClose( dwCurX, dwCurY ) )
		{
			//pCur = m_searchWorkBench[dwCurX][dwCurY];
			pCur = m_pNetMap->GetBlockValue( dwCurX, dwCurY );
			XGC_ASSERT( pCur );
			if( !IsVisited( pCur ) )
			{
				pCur->m_eBlockStatus = BlockValue::eHasParent;
				pCur->eDir2Parent = BlockValue::eUp;//指向父亲
				pCur->H = BlockValue::eHOrV * m_pNetMap->GetBetweenBlocks( dwCurX, dwCurY, dwTargetX, dwTargetY );
				pCur->G = pParent->G + BlockValue::eHOrV;//
				pCur->F = pCur->H + pCur->G;
				pCur->m_oPoint.SetPoint( dwCurX, dwCurY );
				m_vecOpenList.push_back( pCur );
				std::push_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
			}
			else
			{
				//更新G和F
				//如果父节点G+位移值 比子节点 G小    节点的H是不会改变的
				if( pParent->G + BlockValue::eHOrV < pCur->G )
				{
					pCur->G = pParent->G + BlockValue::eHOrV;
					pCur->eDir2Parent = BlockValue::eUp;//指向新父亲
					pCur->F = pCur->H + pCur->G;

					std::make_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
				}
			}
		}

		//右下
		dwCurX = dwSrcX + 1;
		dwCurY = dwSrcY - 1;
		if( !m_pNetMap->IsBlock( dwCurX, dwCurY, bTestBarrier ) && !IsClose( dwCurX, dwCurY ) )
		{
			//pCur = m_searchWorkBench[dwCurX][dwCurY];
			pCur = m_pNetMap->GetBlockValue( dwCurX, dwCurY );
			XGC_ASSERT( pCur );
			if( !IsVisited( pCur ) )
			{
				pCur->m_eBlockStatus = BlockValue::eHasParent;
				pCur->eDir2Parent = BlockValue::eLeftUp;//指向父亲
				pCur->H = BlockValue::eHOrV * m_pNetMap->GetBetweenBlocks( dwCurX, dwCurY, dwTargetX, dwTargetY );
				pCur->G = pParent->G + BlockValue::eSlash;//
				pCur->F = pCur->H + pCur->G;
				pCur->m_oPoint.SetPoint( dwCurX, dwCurY );
				m_vecOpenList.push_back( pCur );
				std::push_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
			}
			else
			{
				//更新G和F
				//如果父节点G+位移值 比子节点 G小    节点的H是不会改变的
				if( pParent->G + BlockValue::eSlash < pCur->G )
				{
					pCur->G = pParent->G + BlockValue::eSlash;
					pCur->eDir2Parent = BlockValue::eLeftUp;//指向新父亲
					pCur->F = pCur->H + pCur->G;
					std::make_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
				}
			}
		}

		//右边
		dwCurX = dwSrcX + 1;
		dwCurY = dwSrcY;
		if( !m_pNetMap->IsBlock( dwCurX, dwCurY, bTestBarrier ) && !IsClose( dwCurX, dwCurY ) )
		{
			//pCur = m_searchWorkBench[dwCurX][dwCurY];
			pCur = m_pNetMap->GetBlockValue( dwCurX, dwCurY );
			XGC_ASSERT( pCur );
			if( !IsVisited( pCur ) )
			{
				pCur->m_eBlockStatus = BlockValue::eHasParent;
				pCur->eDir2Parent = BlockValue::eLeft;//指向父亲
				pCur->H = BlockValue::eHOrV * m_pNetMap->GetBetweenBlocks( dwCurX, dwCurY, dwTargetX, dwTargetY );
				pCur->G = pParent->G + BlockValue::eHOrV;//
				pCur->F = pCur->H + pCur->G;
				pCur->m_oPoint.SetPoint( dwCurX, dwCurY );
				m_vecOpenList.push_back( pCur );
				std::push_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
			}
			else
			{
				//更新G和F
				//如果父节点G+位移值 比子节点 G小    节点的H是不会改变的
				if( pParent->G + BlockValue::eHOrV < pCur->G )
				{
					pCur->G = pParent->G + BlockValue::eHOrV;
					pCur->eDir2Parent = BlockValue::eLeft;//指向新父亲
					pCur->F = pCur->H + pCur->G;
					std::make_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
				}
			}
		}

		//右上
		dwCurX = dwSrcX + 1;
		dwCurY = dwSrcY + 1;
		if( !m_pNetMap->IsBlock( dwCurX, dwCurY, bTestBarrier ) && !IsClose( dwCurX, dwCurY ) )
		{
			//pCur = m_searchWorkBench[dwCurX][dwCurY];
			pCur = m_pNetMap->GetBlockValue( dwCurX, dwCurY );
			XGC_ASSERT( pCur );
			if( !IsVisited( pCur ) )
			{
				pCur->m_eBlockStatus = BlockValue::eHasParent;
				pCur->eDir2Parent = BlockValue::eLeftDown;//指向父亲
				pCur->H = BlockValue::eHOrV * m_pNetMap->GetBetweenBlocks( dwCurX, dwCurY, dwTargetX, dwTargetY );
				pCur->G = pParent->G + BlockValue::eSlash;//
				pCur->F = pCur->H + pCur->G;
				pCur->m_oPoint.SetPoint( dwCurX, dwCurY );
				m_vecOpenList.push_back( pCur );
				std::push_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
			}
			else
			{
				//更新G和F
				//如果父节点G+位移值 比子节点 G小    节点的H是不会改变的
				if( pParent->G + BlockValue::eSlash < pCur->G )
				{
					pCur->G = pParent->G + BlockValue::eSlash;
					pCur->eDir2Parent = BlockValue::eLeftDown;//指向新父亲
					pCur->F = pCur->H + pCur->G;

					std::make_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
				}
			}
		}
		FUNCTION_END;
	}

	xgc_void AStarIPathSearcherImpl::GetPath( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY, xgc_vector<iPoint> &path, xgc_bool bNeedConcis/* = false*/ )
	{
		FUNCTION_BEGIN;
		xgc_uint32 dwCurX = dwTargetX;
		xgc_uint32 dwCurY = dwTargetY;
		BlockValue::eDirType eLastDir = BlockValue::eSelf;
		BlockValue::eDirType eCurDir;

		path.push_back( iPoint( dwCurX, dwCurY ) );//先存入终点

		//或者用dwCurX ==dwSrcX && dwCurY == dwSrcY作为判断条件 
		//while (m_searchWorkBench[dwCurX][dwCurY]->eDir2Parent != eSelf)
		while( ( eCurDir = m_pNetMap->GetBlockValue( dwCurX, dwCurY )->eDir2Parent ) != BlockValue::eSelf )
		{
			//eCurDir = m_pNetMap->GetBlockValue(dwCurX, dwCurY)->eDir2Parent;
			switch( eCurDir )
			{
				case BlockValue::eUp:
				{
					++dwCurY;
					break;
				}
				case BlockValue::eLeftUp:
				{
					--dwCurX;
					++dwCurY;
					break;
				}
				case BlockValue::eLeft:
				{
					--dwCurX;
					break;
				}
				case BlockValue::eLeftDown:
				{
					--dwCurX;
					--dwCurY;
					break;
				}
				case BlockValue::eDown:
				{
					--dwCurY;
					break;
				}
				case BlockValue::eRightDown:
				{
					++dwCurX;
					--dwCurY;
					break;
				}
				case BlockValue::eRight:
				{
					++dwCurX;
					break;
				}
				case BlockValue::eRightUp:
				{
					++dwCurX;
					++dwCurY;
					break;
				}
			}

			if( bNeedConcis )
			{
				if( eLastDir == eCurDir )
				{
					//直接存最后这一个点
					//path.back().SetIX(dwCurX);// = dwCurX;
					//path.back().SetIY(dwCurY);
					path.back().SetPoint( dwCurX, dwCurY );
				}
				else
				{
					path.push_back( iPoint( dwCurX, dwCurY ) );
					eLastDir = eCurDir;
				}
			}
			else
			{
				path.push_back( iPoint( dwCurX, dwCurY ) );
			}
		}
		FUNCTION_END;
	}

	xgc_void AStarIPathSearcherImpl::SetNetMap( IAStarMap* pNetMap )
	{
		FUNCTION_BEGIN;
		//pNetMap非空时处理
		//m_searchWorkBench非空处理

		m_pNetMap = pNetMap;
		FUNCTION_END;
	}


	xgc_bool AStarIPathSearcherImpl::SearchWorkBench( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY, xgc_vector<iPoint> &path, xgc_bool bTestBarrier, xgc_bool bNeedConcis/* = false*/ )
	{
		FUNCTION_BEGIN;
		//1. 将当前节点设置为
		//m_qCloseList.push(C2DPoint(dwSrcX, dwSrcY));

		//BlockValue * pSrc = m_searchWorkBench[dwSrcX][dwSrcY];
		BlockValue * pSrc = m_pNetMap->GetBlockValue( dwSrcX, dwSrcY );
		XGC_ASSERT_RETURN( pSrc, false );
		//清理之前寻路的信息
		m_pNetMap->Reset();
		m_vecOpenList.clear();
		pSrc->m_eBlockStatus = BlockValue::eHasParent;
		pSrc->eDir2Parent = BlockValue::eSelf;//指向自己
		pSrc->m_oPoint.SetPoint( dwSrcX, dwSrcY );
		m_vecOpenList.push_back( pSrc );

		//目标点实际上在开放列表中就可以了，也可以放在关闭列表里，但是这样更快
		xgc_uint32 nCount = 0;
		while( !IsVisited( dwTargetX, dwTargetY ) && !m_vecOpenList.empty() && nCount++ < MaxFindChildCount )
		{
			//取出最小f值的点，在堆中删除,在容器中删除
			BlockValue* pBlock = m_vecOpenList.front();
			std::pop_heap( m_vecOpenList.begin(), m_vecOpenList.end(), HeapCompare_f() );
			m_vecOpenList.pop_back();
			XGC_ASSERT_RETURN( pBlock, false );
			pBlock->m_eBlockStatus = BlockValue::eClose;

			ProcessChildren( pBlock->m_oPoint.GetX(), pBlock->m_oPoint.GetY(), dwTargetX, dwTargetY, bTestBarrier );
		}
		//USR_ERROR( "zyp nCount[%u] m_vecOpenList[%u]", nCount, (xgc_uint32) m_vecOpenList.size() );
		if( IsVisited( dwTargetX, dwTargetY ) )
		{
			GetPath( dwSrcX, dwSrcY, dwTargetX, dwTargetY, path, bNeedConcis );
			return true;
		}
		return false;
		FUNCTION_END;
		return false;
	}

	xgc_bool AStarIPathSearcherImpl::IsClose( xgc_uint32 dwX, xgc_uint32 dwY )
	{
		FUNCTION_BEGIN;
		//return m_qCloseList.find
		//return m_searchWorkBench[dwX][dwY]->m_eBlockStatus == BlockValue::eClose;
		return m_pNetMap->GetBlockValue( dwX, dwY )->m_eBlockStatus == BlockValue::eClose;
		FUNCTION_END;
		return true;
	}

	xgc_bool AStarIPathSearcherImpl::IsVisited( xgc_uint32 dwX, xgc_uint32 dwY )
	{
		FUNCTION_BEGIN;
		//return m_qCloseList.find
		//return m_searchWorkBench[dwX][dwY]->m_eBlockStatus >= BlockValue::eHasParent;
		return m_pNetMap->GetBlockValue( dwX, dwY )->m_eBlockStatus >= BlockValue::eHasParent;
		FUNCTION_END;
		return true;
	}

	xgc_bool AStarIPathSearcherImpl::IsVisited( BlockValue * pBlockStepValue )
	{
		FUNCTION_BEGIN;
		return pBlockStepValue->m_eBlockStatus >= BlockValue::eHasParent;
		FUNCTION_END;
		return true;
	}


	IAStarMap::IAStarMap( xgc_uint32 dwWidth, xgc_uint32 dwHeight )
	{
		Init( dwWidth, dwHeight );
	}

	IAStarMap::~IAStarMap()
	{
		UnInit();
	}

	xgc_uint32 IAStarMap::GetWidth() const
	{
		return m_dwWidth;
	}

	xgc_uint32 IAStarMap::GetHeight() const
	{
		return m_dwHeight;
	}

	BlockValue * IAStarMap::GetBlockValue( xgc_uint32 x, xgc_uint32 y ) const
	{
		XGC_ASSERT_RETURN( x < m_dwWidth && y < m_dwHeight, false );
		return m_searchWorkBench + y * m_dwWidth + x;
	}

	xgc_uint32 IAStarMap::GetBetweenBlocks( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY )
	{
		xgc_uint32  dwBlocks = ( dwSrcX > dwTargetX ) ? ( dwSrcX - dwTargetX ) : ( dwTargetX - dwSrcX );
		dwBlocks += ( dwSrcY > dwTargetY ) ? ( dwSrcY - dwTargetY ) : ( dwTargetY - dwSrcY );
		return dwBlocks;
	}

	xgc_bool IAStarMap::GetBetweenBlocksStrict( xgc_uint32 dwSrcX, xgc_uint32 dwSrcY, xgc_uint32 dwTargetX, xgc_uint32 dwTargetY, xgc_uint32 & dwBlocks )
	{
		if( dwSrcX >= m_dwWidth || dwSrcY >= m_dwHeight || dwTargetX >= m_dwWidth || dwTargetY >= m_dwHeight )
		{
			return false;
		}
		dwBlocks = ( dwSrcX > dwTargetX ) ? ( dwSrcX - dwTargetX ) : ( dwTargetX - dwSrcX );
		dwBlocks += ( dwSrcY > dwTargetY ) ? ( dwSrcY - dwTargetY ) : ( dwTargetY - dwSrcY );
		return true;
	}

	xgc_void IAStarMap::Init( xgc_uint32 dwWidth, xgc_uint32 dwHeight )
	{
		m_dwWidth = dwWidth;
		m_dwHeight = dwHeight;
		m_searchWorkBench = XGC_NEW BlockValue[m_dwWidth * m_dwHeight];
	}

	xgc_void IAStarMap::Reset()
	{
		FUNCTION_BEGIN;

		memset( m_searchWorkBench, 0, m_dwWidth * m_dwHeight * sizeof( BlockValue ) );
		FUNCTION_END;
	}

	xgc_void IAStarMap::UnInit()
	{
		SAFE_DELETE_ARRAY( m_searchWorkBench );
		m_dwWidth = m_dwHeight = 0;
	}

}