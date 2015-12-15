#pragma once
#include <exception>
#include "defines.h"
#include "xutility.h"
namespace XGC
{
	namespace common
	{
		class COMMON_API sync_object : public noncopyable
		{
		public:
			explicit sync_object()
				: m_object_h( NULL )
			{

			}

			~sync_object()
			{
				if (m_object_h != NULL)
				{
					::CloseHandle(m_object_h);
					m_object_h = NULL;
				}
			}

		protected:
			HANDLE  m_object_h;

			// Attributes
		public:
			operator HANDLE() const{ return m_object_h; }
		};

		/////////////////////////////////////////////////////////////////////////////
		// CSemaphore

		class COMMON_API semaphore : public sync_object
		{
		public:
			semaphore( long init = 1, long max = 1, xgc_lpctstr name = NULL, LPSECURITY_ATTRIBUTES lpsaAttributes = NULL )
			{
				m_object_h = ::CreateSemaphore( lpsaAttributes, init, max, name );
				if( m_object_h == NULL )
					throw( std::exception( "create semaphore failed!" ) );
			}

			// Implementation
		public:
			inline void lock( unsigned int dwMilliseconds = INFINITE )
			{
				::WaitForSingleObject( m_object_h, dwMilliseconds );
			}

			inline int unlock(long count, long* prevcount_ptr = NULL)
			{
				return ::ReleaseSemaphore( m_object_h, count, prevcount_ptr );
			}

			inline int unlock()
			{
				return unlock( 1, NULL );
			}
		};

		/////////////////////////////////////////////////////////////////////////////
		// CMutex

		class COMMON_API mutex : public sync_object
		{
		public:
			mutex(int owner = false, xgc_lpctstr name = NULL, LPSECURITY_ATTRIBUTES lpsaAttribute = NULL)
			{
				m_object_h = ::CreateMutex( lpsaAttribute, owner, name );
				if( m_object_h == NULL )
					throw( std::exception( "create mutex failed!" ) );
			}

			// Implementation
		public:
			int unlock()
			{
				return ::ReleaseMutex( m_object_h );
			}
		};

		/////////////////////////////////////////////////////////////////////////////
		// CEvent

		class COMMON_API event : public sync_object
		{
		public:
			event( int init = false, int manual_reset = false, xgc_lpctstr name = NULL, LPSECURITY_ATTRIBUTES lpsaAttribute = NULL)
			{
				m_object_h = ::CreateEvent( lpsaAttribute, manual_reset, init, name );
				if( m_object_h == NULL )
					throw( std::exception( "create event failed!" ) );
			}

			// Operations
		public:
			inline void wait( unsigned int dwMilliseconds = INFINITE ){ ::WaitForSingleObject( m_object_h, dwMilliseconds ); }

			int set(){ return ::SetEvent(m_object_h); }
			int pulse(){ return ::PulseEvent(m_object_h); }
			int reset(){ return ::ResetEvent(m_object_h); }
		};

		class COMMON_API sync_section : public noncopyable
		{
		public:
			sync_section()
			{
				InitializeCriticalSection( &cri_ );
			}

			~sync_section()
			{
				DeleteCriticalSection( &cri_ );
			}

			void lock()
			{
				EnterCriticalSection( &cri_ );
			}

			void free_lock()
			{
				LeaveCriticalSection( &cri_ );
			}

			operator CRITICAL_SECTION&(){ return cri_; }

		private:
			CRITICAL_SECTION cri_;
		};

		class sync_nolock : public noncopyable
		{
		public:
			void lock() {}
			void free_lock() {}
		};

		class COMMON_API autolock
		{
		public:
			explicit autolock( CRITICAL_SECTION& cri )
				: ref_( cri )
				, locked_( false )
			{
				lock();
			}

			virtual ~autolock(void)throw()
			{
				free_lock();
			}

			void lock()
			{ 
				if( !locked_ )
				{ 
					EnterCriticalSection( &ref_ ); 
					locked_ = true; 
				} 
			}

			void free_lock()
			{ 
				if( locked_ )
				{ 
					locked_ = false; 
					LeaveCriticalSection( &ref_ ); 
				}
			}

		private:
			autolock& operator=( const autolock &rhs );
		protected:
			CRITICAL_SECTION& ref_;
			bool locked_;
		};

#if _WIN32_WINNT >= 0x0400
		class COMMON_API trylock : public noncopyable
		{
		public:
			explicit trylock( CRITICAL_SECTION& cri )
				: ref_( cri )
				, locked_(false)
			{
				locked_ = (TryEnterCriticalSection( &cri ) == TRUE);
			}

			~trylock(void)
			{
				if( locked_ )
					LeaveCriticalSection( &ref_ );
			}

			bool judge()const 
			{ 
				return locked_; 
			}

			void lock()
			{ 
				if( !locked_ )
				{ 
					EnterCriticalSection( &ref_ ); 
					locked_ = true; 
				} 
			}

			void free_lock( )
			{ 
				if( locked_ )
				{ 
					locked_ = false; 
					LeaveCriticalSection( &ref_ ); 
				}
			}

		protected:
			CRITICAL_SECTION& ref_;
			bool locked_;
		};
#endif
	}
}