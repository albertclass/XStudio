#include "lookaside.h"
#include "xsystem.h"
namespace XGC
{
	namespace memory
	{
		volatile struct Lookaside_BasicStage* gLookasideHead = NULL;

		void PrintLookasideInfomation()
		{
			volatile struct Lookaside_BasicStage* pHead = gLookasideHead;

			while( pHead )
			{
				pHead->Logger();
				pHead = pHead->mNextLookaside;
			}
		}

		Lookaside_BasicStage::Lookaside_BasicStage( bool bCheck )
			: mCheck( bCheck )
			, mNextLookaside( xgc_nullptr )
			, mHeap( INVALID_HANDLE_VALUE )
			, mChunkPow( 1 )
			, mPrevNewChunkTime( 0 )
			, mTotalAllocCount( 0 )
			, mFreeListCount( 0 )
			, mAllocListCount( 0 )
			, mPrevTime( 0 )
		{
			mHeap = HeapCreate( 0, 0, 0 );
			if( mHeap )
			{
#pragma warning(suppress: 28112)
				InterlockedExchangePointer( (volatile PVOID*)&mNextLookaside, (PVOID)gLookasideHead );
				InterlockedExchangePointer( (volatile PVOID*)&gLookasideHead, (PVOID)this );
			}
		}

		Lookaside_BasicStage::~Lookaside_BasicStage()
		{
			if( mHeap != INVALID_HANDLE_VALUE )
			{
				HeapDestroy( mHeap );
				mHeap = INVALID_HANDLE_VALUE;
			}
		}

		bool Lookaside_BasicStage::Check( void *pObject, size_t nSize, const char* file, int line )
		{
			unsigned char *pCheck = (unsigned char*)pObject;
			for( size_t i = 0; i < nSize; ++i )
			{
				if( pCheck[i] != sign )
				{
					char szHex[1024 * 4];
					XGC::Binary2Hex( (xgc_byte*)pObject, nSize, szHex, sizeof(szHex), 1 );
					DBG_INFO( "{HEAP:%p(%s)} error write after free. sign = 0x%08x, offset = %d, Data:%s From File (%s) Line (%d)"
						, mHeap
						, GetTypeName()
						, *(unsigned int*)(pCheck + i)
						, i
						, szHex 
						, file
						, line);
					return false;
				}
			}

			return true;
		}

		void Lookaside_BasicStage::Fill( void *pObject, size_t nSize )
		{
			memset( pObject, sign, nSize );
		}

		void Lookaside_BasicStage::Logger() volatile
		{
			DBG_INFO( "{HEAP:%p(%s)} NewChunk Info ( ChunkCount = %u, AllocChunkCount = %u, FreeChunkCount = %u, TotalAllocCount = %u )"
				, mHeap
				, GetTypeName()
				, mChunkCount
				, mAllocListCount
				, mFreeListCount
				, mTotalAllocCount );
		}
	}
}