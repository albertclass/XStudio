#ifndef _LOOKASIDE_H_
#define _LOOKASIDE_H_
#include "logger.h"
#include "synchronous.h"

#define IMPLEMENT_LOOKASIDE( TYPE ) Lookaside< TYPE > LookasideImpl< TYPE >::Inst;

namespace XGC
{
	namespace memory
	{
		using XGC::common::sync_section;
		const unsigned int alignment = 4;
		const unsigned char sign = 0xeb;

		const unsigned int pow_inc_time = 1000;
		const unsigned int pow_dec_time = 10000;

		COMMON_API void PrintLookasideInfomation();
		struct COMMON_API Lookaside_BasicStage
		{
			volatile
			struct Lookaside_BasicStage* mNextLookaside;
		protected:
			Lookaside_BasicStage( bool bCheck );
			~Lookaside_BasicStage();

		protected:
			xgc_bool		mCheck;
			sync_section	mSection;

			HANDLE			mHeap;

			UINT			mPrevNewChunkTime;

			volatile
			UINT		mChunkPow;

			volatile
			UINT		mTotalAllocCount;

			volatile
			UINT		mFreeListCount;

			volatile
			UINT		mAllocListCount;

			volatile
			UINT		mChunkCount;

			UINT		mPrevTime;
		protected:
			bool Check( void *pObject, size_t nSize, const char* file, int line );

			void Fill( void *pObject, size_t nSize );

			sync_section* GetLock(){ return &mSection; }

			virtual
			const char* GetTypeName()const volatile = 0;
		public:
			void Logger() volatile;
		};

		template< typename T, size_t ChunkSize = 128, size_t PowMax = 1, typename Stage = Lookaside_BasicStage >
		struct Lookaside : public Stage
		{
			struct EntryList
			{
				unsigned int mGuard;
				struct EntryList* pPrev;
				struct EntryList* pNext;
			};

			struct ChunkFreeList
			{
				struct EntryList List;
				unsigned short nFreeListSize;		// 当前容量
				unsigned short nFreeCount;			// 队列中对象个数
				unsigned short nNextAlloc;			// 当前分配位置
				unsigned short nNextFree;			// 当前释放位置
			};

			struct ChunkEntry
			{
				struct EntryList List;
				size_t mObjectCount;
			};

			Lookaside( bool bCheck = true )
				: Stage( bCheck )
				, mHead( NULL )
				, mAllocList( NULL )
				, mFreeList( NULL )
			{
				DBG_INFO( "{HEAP:%p(%s)} Heap create.", mHeap, typeid(T).name() );
			}

			~Lookaside()
			{
				FreeAll();
				DBG_INFO( "{HEAP:%p(%s)} Heap destroy.", mHeap, typeid(T).name() );
			}

		private:
			EntryList	*mAllocList, *mFreeList;
			EntryList	*mHead;

			virtual const char* GetTypeName()const volatile
			{ 
				return typeid(T).name();
			}

			void FreeAll()
			{
				do
				{
					EntryList* pEnd = mHead;
					mHead = Remove( pEnd );
					HeapFree( mHeap, 0, pEnd );
				}while( mHead );
			}

			// insert pNew after pNode
			// return the newer;
			EntryList *Insert( EntryList *pNew, EntryList *pNode )
			{
				if( pNew == NULL )
				{
					return pNew;
				}

				if( pNode )
				{
					pNew->pNext = pNode->pNext;
					pNode->pNext->pPrev = pNew;

					pNew->pPrev = pNode;
					pNode->pNext = pNew;
				}
				else
				{
					pNew->pNext = pNew;
					pNew->pPrev = pNew;
				}

				pNew->mGuard = 0xdedefdfd;
				return pNew;
			}

			// remove node return after
			EntryList *Remove( EntryList *pEntry )
			{
				if( pEntry == NULL )
					return NULL;

				if( pEntry->mGuard != 0xdedefdfd )
				{
					DBG_FILE_LOG( "{HEAP:%p(%s)} chunk data broke. %p, guard = 0x%08x", mHeap, GetTypeName(), pEntry, pEntry->mGuard );
					return NULL;
				}

				if( pEntry->pPrev == pEntry || pEntry->pNext == pEntry )
					return NULL;

				pEntry->pNext->pPrev = pEntry->pPrev;
				pEntry->pPrev->pNext = pEntry->pNext;

				return pEntry->pNext;
			}

			ChunkFreeList* NewChunk( size_t nCount )
			{
				//static clock_t tt = 0;
				//static unsigned int cnt = 0;

				clock_t t1 = clock();

				// alloc chunk
				if( mChunkPow < PowMax && t1 - mPrevNewChunkTime < pow_inc_time )
					++mChunkPow;
				if( mChunkPow > 1 && t1 - mPrevNewChunkTime > pow_dec_time )
					--mChunkPow;

				nCount = (nCount < ChunkSize ? ChunkSize : nCount) * mChunkPow;

				mPrevNewChunkTime = clock();

				size_t nChunkSize = sizeof(ChunkEntry);
				size_t nObjectTotalSize = nCount * sizeof(T);

				ChunkEntry* pEntry = (ChunkEntry*)HeapAlloc( mHeap, 0, nChunkSize + nObjectTotalSize );
				if( pEntry == NULL )
					return NULL;

				Fill( (void*)(pEntry+1), nObjectTotalSize );

				pEntry->mObjectCount = nCount;

				// alloc freelist
				ChunkFreeList* pFreeEntry = NULL;
				size_t nFreeChunkSize = sizeof(ChunkFreeList);
				size_t nFreeListAllocSize = sizeof(T**) * nCount;
				pFreeEntry = (ChunkFreeList*)HeapAlloc( mHeap, 0,  nFreeChunkSize + nFreeListAllocSize );

				if( pFreeEntry == NULL )
				{
					HeapFree( mHeap, 0, pEntry );
					return NULL;
				}

				memset( pFreeEntry, 0, nFreeChunkSize + nFreeListAllocSize );

				pFreeEntry->nFreeListSize	= nCount;
				pFreeEntry->nNextAlloc		= 0;
				pFreeEntry->nNextFree		= 0;
				pFreeEntry->nFreeCount		= nCount;

				for( size_t i = 0; i < nCount; ++i )
				{
					((T**)(pFreeEntry+1))[i] = (T*)(pEntry+1) + i;
				}

				mHead		= Insert( (EntryList*)pEntry, mHead );
				mAllocList	= Insert( (EntryList*)pFreeEntry, mAllocList );

				++mChunkCount;
				++mAllocListCount;
				//tt += clock() - t1;
				//if( ++cnt % 1024 == 0 )
				//	printf( "new chunk cnt = %u, time = %u, avg = %f\n", cnt, tt, tt / 1000.0f / cnt );
				clock_t nNowTime = clock();
				if( nNowTime - mPrevTime > 10.0f )
					Logger();

				return pFreeEntry;
			}

			T* Inner_Alloc( ChunkFreeList* pFreeEntry, const char* file, int line )
			{
				//static clock_t tt = 0;
				//static unsigned int cnt = 0;

				clock_t t1 = clock();

				if( pFreeEntry == NULL )
					return NULL;

				T* pObject = ((T**)(pFreeEntry+1))[pFreeEntry->nNextAlloc];
				((T**)(pFreeEntry+1))[pFreeEntry->nNextAlloc] = NULL;
				pFreeEntry->nNextAlloc++;
				pFreeEntry->nFreeCount--;

				if( pFreeEntry->nNextAlloc == pFreeEntry->nFreeListSize )
					pFreeEntry->nNextAlloc = 0;

				Check( pObject, sizeof(T), file, line );

				++mTotalAllocCount;
				//tt += clock() - t1;
				//if( ++cnt % ( 1024 * ChunkSize / 2 ) == 0 )
				//	printf( "alloc cnt = %u, time = %u, avg = %f\n", cnt, tt, tt / 1000.0f / cnt );

				return pObject;
			}

		public:
			T* Alloc(const char* file, int line)
			{
				Stage::AutoLock lock( GetLock() );

				while( mAllocList ) 
				{
					EntryList* pEnd = mAllocList;

					if( mAllocList->mGuard != 0xdedefdfd )
					{
						DBG_FILE_LOG( "chunk data broke. %p, guard = 0x%08x", mHead, mHead->mGuard );
						return NULL;
					}

					ChunkFreeList* pAllocEntry = (ChunkFreeList*)mAllocList;
					if( pAllocEntry->nFreeCount > 0 )
					{
						return Inner_Alloc( pAllocEntry, file, line );
					}
					else
					{
						EntryList *pNext = Remove( mAllocList );

						Insert( mAllocList, mFreeList );
						if( mFreeList == NULL )
						{
							mFreeList = mAllocList;
						}

						mAllocList = pNext;

						++mFreeListCount;
						--mAllocListCount;
					}
				}

				ChunkFreeList *pAllocEntry = NewChunk( ChunkSize );
				return Inner_Alloc( pAllocEntry, file, line );
			}

			void Free( T *pObject )
			{
				Stage::AutoLock lock( GetLock() );

				if( pObject == NULL )
					return;

				if( mAllocList == NULL )
					return;

				EntryList *pCur = (mFreeList == NULL ? mAllocList : mFreeList);
				EntryList *pEnd = pCur;

				Fill( pObject, sizeof(T) );

				do
				{
					if( pCur->mGuard != 0xdedefdfd )
					{
						DBG_FILE_LOG( "{HEAP:%p(%s)} chunk data broke. %p, guard = 0x%08x", mHeap, GetTypeName(), pCur, pCur->mGuard );
						return;
					}

					ChunkFreeList *pChunk = (ChunkFreeList*)pCur;

					if( pChunk->nFreeCount != pChunk->nFreeListSize )
					{
						((T**)(pChunk+1))[pChunk->nNextFree] = pObject;
						++pChunk->nNextFree;
						++pChunk->nFreeCount;

						--mTotalAllocCount;
						if( pChunk->nNextFree == pChunk->nFreeListSize )
							pChunk->nNextFree = 0;

						if( pChunk->nFreeCount == pChunk->nFreeListSize && pCur == mFreeList )
						{
							EntryList *pNext = Remove( pCur );
							Insert( pCur, mAllocList );

							mFreeList = pNext;

							--mFreeListCount;
							++mAllocListCount;
						}
						return;
					}
				}while( (pCur = pCur->pNext) && pCur != pEnd );

				DBG_FILE_LOG( "{HEAP:%p(%s)} error free object. no free space.", mHeap, GetTypeName() );
			}
		};

		template< typename T >
		struct LookasideImpl
		{
			static Lookaside< T > Inst;

			static T* XCB_Alloc(const char* file, int line)
			{
				T* Ptr = LookasideImpl< T >::Inst.Alloc(file, line);
				return Ptr ? new ( Ptr )( T ) : NULL;
			}

			template< class P1 >
			static T* XCB_Alloc(const char* file, int line, P1& p1 )
			{
				T* Ptr = LookasideImpl< T >::Inst.Alloc(file, line);
				return Ptr ? new ( Ptr )T( p1 ) : NULL;
			}

			
			template< class P1, class P2 >
			static T* XCB_Alloc(const char* file, int line, P1& p1, P2& p2 )
			{
				T* Ptr = LookasideImpl< T >::Inst.Alloc(file, line);
				return Ptr ? new ( Ptr ) T( p1, p2 ) : NULL;
			}

			template< class P1, class P2, class P3 >
			static T* XCB_Alloc(const char* file, int line, P1 &p1, P2 &p2, P3 &p3 )
			{
				T* Ptr = LookasideImpl< T >::Inst.Alloc(file, line);
				return Ptr ? new ( Ptr ) T( p1, p2, p3 )  : NULL;
			}

			template< class P1, class P2, class P3, class P4 >
			static T* XCB_Alloc(const char* file, int line, P1 &p1, P2 &p2, P3 &p3, P4 &p4 )
			{
				T* Ptr = LookasideImpl< T >::Inst.Alloc(file, line);
				return Ptr ? new ( Ptr ) T( p1, p2, p3, p4 )  : NULL;
			}
		};

		template< typename T >
		Lookaside< T > LookasideImpl< T >::Inst;

		template< typename T >
		void XCB_Free( T* Ptr )
		{
			Ptr->~T();
			LookasideImpl< T >::Inst.Free( Ptr );
		}
	}
}

#define XCB_NEW(T, ...)   XGC::memory::LookasideImpl< T >::XCB_Alloc(__FILE__, __LINE__, __VA_ARGS__ )
#define XCB_DELETE(p)     XGC::memory::XCB_Free(p); p = NULL;

#endif //_LOOKASIDE_H_