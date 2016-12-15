#ifndef _SINGLETON_H_
#define _SINGLETON_H_
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <cstdlib>
#include <new>
#include <vector>
#include <list>
#include <memory>
#include <mutex>

#include "defines.h"

namespace xgc
{
	namespace common
	{
		typedef void (__cdecl *atexit_pfn_t)();

		// added by lcj 2013-6-07 start
		typedef void (*destoryFunc)();

		struct SingletonInfo {
			void*	    ptr;
			destoryFunc func;	
		};
		typedef std::list<SingletonInfo> SingletPtrList;
		typedef SingletPtrList::iterator SingletPtrListItr;
		typedef SingletPtrList::reverse_iterator SingletPtrListRItr;

		static SingletPtrList& get_singleton_list()
		{
			static SingletPtrList gSingletonList;
			return gSingletonList;
		}

		static void register_singleton_ptr(void* p, destoryFunc pfunc)
		{				
			SingletonInfo st;
			st.ptr  = p;
			st.func = pfunc;
			get_singleton_list().push_back(st);
		}

		static void release_singleton_all()
		{
			SingletPtrList& gList = get_singleton_list();
			SingletPtrListRItr gListItr = gList.rbegin();
			for (; gListItr != gList.rend(); gListItr++)
			{
				gListItr->func();
			}
		}
		// added by lcj 2013-6-07 end

		namespace Private
		{

		#ifndef EXPORT_API
			void __cdecl AtExitFn(); // declaration needed below   
		#else
			void __cdecl AtExitFn();
		#endif

			class LifetimeTracker;

		#define ENABLE_NEW_SETLONGLIVITY        
		#ifdef ENABLE_NEW_SETLONGLIVITY

			// Helper data
			// std::list because of the inserts
			typedef std::list<LifetimeTracker*> TrackerArray;
			extern TrackerArray* pTrackerArray;
		#else
			// Helper data
			typedef LifetimeTracker** TrackerArray;
			extern TrackerArray pTrackerArray;
			extern unsigned int elements;
		#endif

			////////////////////////////////////////////////////////////////////////////////
			// class LifetimeTracker
			// Helper class for SetLongevity
			////////////////////////////////////////////////////////////////////////////////

			class LifetimeTracker
			{
			public:
				LifetimeTracker(unsigned int x) : longevity_(x) 
				{}

				virtual ~LifetimeTracker() = 0;

				static bool Compare(const LifetimeTracker* lhs,
					const LifetimeTracker* rhs)
				{
					return lhs->longevity_ > rhs->longevity_;
				}

			private:
				unsigned int longevity_;
			};

			// Definition required
			inline LifetimeTracker::~LifetimeTracker() {} 

			// Helper destroyer function
			template <typename T>
			struct Deleter
			{
				typedef void (*Type)(T*);
				static void Delete(T* pObj)
				{ delete pObj; }
			};

			// Concrete lifetime tracker for objects of type T
			template <typename T, typename Destroyer>
			class ConcreteLifetimeTracker : public LifetimeTracker
			{
			public:
				ConcreteLifetimeTracker(T* p,unsigned int longevity, Destroyer d)
					: LifetimeTracker(longevity)
					, pTracked_(p)
					, destroyer_(d)
				{}

				~ConcreteLifetimeTracker()
				{ destroyer_(pTracked_); }

			private:
				T* pTracked_;
				Destroyer destroyer_;
			};

		} // namespace Private

		////////////////////////////////////////////////////////////////////////////////
		///  \ingroup LifetimeGroup
		///  
		///  Assigns an object a longevity; ensures ordered destructions of objects 
		///  registered thusly during the exit sequence of the application
		////////////////////////////////////////////////////////////////////////////////

		#ifdef ENABLE_NEW_SETLONGLIVITY

		template <typename T, typename Destroyer>
		void SetLongevity(T* pDynObject, unsigned int longevity,
			Destroyer d)
		{
			using namespace Private;

			// manage lifetime of stack manually
			if(pTrackerArray==0)
				pTrackerArray = XGC_NEW TrackerArray;

			// automatically delete the ConcreteLifetimeTracker object when a exception is thrown
			std::auto_ptr<LifetimeTracker> 
				p( XGC_NEW ConcreteLifetimeTracker<T, Destroyer>( pDynObject, longevity, d ) );

			// Find correct position
			TrackerArray::iterator pos = std::upper_bound(
				pTrackerArray->begin(), 
				pTrackerArray->end(), 
				p.get(), 
				LifetimeTracker::Compare);

			// Insert the pointer to the ConcreteLifetimeTracker object into the queue
			pTrackerArray->insert(pos, p.get());

			// nothing has thrown: don't delete the ConcreteLifetimeTracker object
			p.release();

			// Register a call to AtExitFn
			std::atexit(Private::AtExitFn);
		}

		#else

		template <typename T, typename Destroyer>
		void SetLongevity(T* pDynObject, unsigned int longevity,
			Destroyer d)
		{
			using namespace Private;

			TrackerArray pNewArray = static_cast<TrackerArray>(
				std::realloc(pTrackerArray, 
				sizeof(*pTrackerArray) * (elements + 1)));
			if (!pNewArray) throw std::bad_alloc();

			// Delayed assignment for exception safety
			pTrackerArray = pNewArray;

			LifetimeTracker* p = XGC_NEW ConcreteLifetimeTracker<T, Destroyer>(
				pDynObject, longevity, d);

			// Insert a pointer to the object into the queue
			TrackerArray pos = std::upper_bound(
				pTrackerArray, 
				pTrackerArray + elements, 
				p, 
				LifetimeTracker::Compare);
			std::copy_backward(
				pos, 
				pTrackerArray + elements,
				pTrackerArray + elements + 1);
			*pos = p;
			++elements;

			// Register a call to AtExitFn
			std::atexit(Private::AtExitFn);
		}

		#endif

		template <typename T>
		void SetLongevity(T* pDynObject, unsigned int longevity,
			typename Private::Deleter<T>::Type d = Private::Deleter<T>::Delete)
		{
			SetLongevity<T, typename Private::Deleter<T>::Type>(pDynObject, longevity, d);
		}

		////////////////////////////////////////////////////////////////////////////////
		///  \struct CreateUsingNew 
		///
		///  \ingroup CreationGroup
		///  Implementation of the CreationPolicy used by SingletonHolder
		///  Creates objects using a straight call to the new operator 
		////////////////////////////////////////////////////////////////////////////////
		template <class T> struct CreateUsingNew
		{
			static T* Create()
			{ 
				return XGC_NEW T;
			}

			static void Destroy(T* p)
			{ 
				delete p; 
			}
		};

		////////////////////////////////////////////////////////////////////////////////
		///  \struct CreateUsing
		///
		///  \ingroup CreationGroup
		///  Implementation of the CreationPolicy used by SingletonHolder
		///  Creates objects using a custom allocater.
		///  Usage: e.g. CreateUsing<std::allocator>::Allocator
		////////////////////////////////////////////////////////////////////////////////
		template<template<class> class Alloc>
		struct CreateUsing
		{
			template <class T>
			struct Allocator
			{
				static Alloc<T> allocator;

				static T* Create()
				{
					return XGC_NEW( allocator.allocate( 1 ) ) T;
				}

				static void Destroy(T* p)
				{
					//allocator.destroy(p);
					p->~T();
					allocator.deallocate(p,1);
				}
			};
		};

		////////////////////////////////////////////////////////////////////////////////
		///  \struct CreateUsingMalloc
		///
		///  \ingroup CreationGroup
		///  Implementation of the CreationPolicy used by SingletonHolder
		///  Creates objects using a call to std::malloc, followed by a call to the 
		///  placement new operator
		////////////////////////////////////////////////////////////////////////////////
		template <class T> struct CreateUsingMalloc
		{
			static T* Create()
			{
				void* p = std::malloc(sizeof(T));
				if (!p) return 0;
				return new(p) T;
			}

			static void Destroy(T* p)
			{
				p->~T();
				std::free(p);
			}
		};


		////////////////////////////////////////////////////////////////////////////////
		///  \struct CreateStatic
		///
		///  \ingroup CreationGroup
		///  Implementation of the CreationPolicy used by SingletonHolder
		///  Creates an object in static memory
		///  Implementation is slightly nonportable because it uses the MaxAlign trick 
		///  (an union of all types to ensure proper memory alignment). This trick is 
		///  nonportable in theory but highly portable in practice.
		////////////////////////////////////////////////////////////////////////////////
		template <class T> struct CreateStatic
		{

		#ifdef _MSC_VER
		#pragma warning( push ) 
		#pragma warning( disable : 4121 )
			// alignment of a member was sensitive to packing
		#endif // _MSC_VER

			union MaxAlign
			{
				char t_[sizeof(T)];
				short int shortInt_;
				int int_;
				long int longInt_;
				float float_;
				double double_;
				long double longDouble_;
				struct Test;
				int Test::* pMember_;
				int (Test::*pMemberFn_)(int);
			};

		#ifdef _MSC_VER
		#pragma warning( pop )
		#endif // _MSC_VER

			static T* Create()
			{
				static MaxAlign staticMemory_;
				return new(&staticMemory_) T;
			}

			static void Destroy(T* p)
			{
				p->~T();
			}
		};

		////////////////////////////////////////////////////////////////////////////////
		///  \struct DefaultLifetime
		///
		///  \ingroup LifetimeGroup
		///  Implementation of the LifetimePolicy used by SingletonHolder
		///  Schedules an object's destruction as per C++ rules
		///  Forwards to std::atexit
		////////////////////////////////////////////////////////////////////////////////
		template <class T>
		struct DefaultLifetime
		{
			static void ScheduleDestruction(T*, atexit_pfn_t pFun)
			{ std::atexit(pFun); }

			static void OnDeadReference()
			{ throw std::logic_error("Dead Reference Detected"); }
		};

		////////////////////////////////////////////////////////////////////////////////
		///  \struct  PhoenixSingleton
		///
		///  \ingroup LifetimeGroup
		///  Implementation of the LifetimePolicy used by SingletonHolder
		///  Schedules an object's destruction as per C++ rules, and it allows object 
		///  recreation by not throwing an exception from OnDeadReference
		////////////////////////////////////////////////////////////////////////////////
		template <class T>
		class PhoenixSingleton
		{
		public:
			static void ScheduleDestruction(T*, atexit_pfn_t pFun)
			{
		#ifndef ATEXIT_FIXED
				if (!destroyedOnce_)
		#endif
					std::atexit(pFun);
			}

			static void OnDeadReference()
			{
		#ifndef ATEXIT_FIXED
				destroyedOnce_ = true;
		#endif
			}

		private:
		#ifndef ATEXIT_FIXED
			static bool destroyedOnce_;
		#endif
		};

		#ifndef ATEXIT_FIXED
		template <class T> bool PhoenixSingleton<T>::destroyedOnce_ = false;
		#endif

		////////////////////////////////////////////////////////////////////////////////
		// Copyright (c) 2004 by Curtis Krauskopf - curtis@decompile.com
		///
		///  \struct  DeletableSingleton
		///
		///  \ingroup LifetimeGroup
		///
		///  A DeletableSingleton allows the instantiated singleton to be 
		///  destroyed at any time. The singleton can be reinstantiated at 
		///  any time, even during program termination.
		///  If the singleton exists when the program terminates, it will 
		///  be automatically deleted.
		///
		///  \par Usage:  
		///  The singleton can be deleted manually:
		///
		///  DeletableSingleton<MyClass>::GracefulDelete();
		////////////////////////////////////////////////////////////////////////////////
		template <class T>
		class DeletableSingleton
		{
		public:

			static void ScheduleDestruction(T*, atexit_pfn_t pFun)
			{
				static bool firstPass = true;
				isDead = false;
				deleter = pFun;
				if (firstPass || needCallback)
				{
					std::atexit(atexitCallback);
					firstPass = false;
					needCallback = false;
				}
			}

			static void OnDeadReference()
			{ 
			}
			///  delete singleton object manually
			static void GracefulDelete()
			{
				if (isDead)
					return;
				isDead = true;
				deleter();
			}

		protected:
			static atexit_pfn_t deleter;
			static bool isDead;
			static bool needCallback;

			static void atexitCallback()
			{
		#ifdef ATEXIT_FIXED
				needCallback = true;
		#else
				needCallback = false;
		#endif
				GracefulDelete();
			}

		};

		template <class T>
		atexit_pfn_t DeletableSingleton<T>::deleter = 0;

		template <class T>
		bool DeletableSingleton<T>::isDead = true;

		template <class T>
		bool DeletableSingleton<T>::needCallback = true;

		////////////////////////////////////////////////////////////////////////////////
		// class template Adapter
		// Helper for SingletonWithLongevity below
		////////////////////////////////////////////////////////////////////////////////

		namespace Private
		{
			template <class T>
			struct Adapter
			{
				void operator()(T*) { return pFun_(); }
				atexit_pfn_t pFun_;
			};
		}

		////////////////////////////////////////////////////////////////////////////////
		///  \struct SingletonWithLongevity
		///
		///  \ingroup LifetimeGroup
		///  Implementation of the LifetimePolicy used by SingletonHolder
		///  Schedules an object's destruction in order of their longevities
		///  Assumes a visible function GetLongevity(T*) that returns the longevity of the
		///  object.
		////////////////////////////////////////////////////////////////////////////////
		template <class T>
		class SingletonWithLongevity
		{
		public:
			static void ScheduleDestruction(T* pObj, atexit_pfn_t pFun)
			{
				Private::Adapter<T> adapter = { pFun };
				SetLongevity(pObj, GetLongevity(pObj), adapter);
			}

			static void OnDeadReference()
			{ throw std::logic_error("Dead Reference Detected"); }
		};

		////////////////////////////////////////////////////////////////////////////////
		///  \struct NoDestroy
		///
		///  \ingroup LifetimeGroup
		///  Implementation of the LifetimePolicy used by SingletonHolder
		///  Never destroys the object
		////////////////////////////////////////////////////////////////////////////////
		template <class T>
		struct NoDestroy
		{
			static void ScheduleDestruction(T*, atexit_pfn_t)
			{}

			static void OnDeadReference()
			{}
		};


		////////////////////////////////////////////////////////////////////////////////
		///  \defgroup LongevityLifetimeGroup LongevityLifetime
		///  \ingroup LifetimeGroup
		///
		///  \namespace LongevityLifetime
		///
		///  \ingroup LongevityLifetimeGroup
		///  \brief  In this namespace are special lifetime policies to manage lifetime
		///  dependencies.
		////////////////////////////////////////////////////////////////////////////////
		namespace LongevityLifetime
		{
			////////////////////////////////////////////////////////////////////////////////
			///  \struct  SingletonFixedLongevity 
			///
			///  \ingroup LongevityLifetimeGroup
			///  Add your own lifetimes into the namespace 'LongevityLifetime'
			///  with your prefered lifetime by adding a struct like this:
			///
			///  template<class T>
			///  struct MyLifetime  : SingletonFixedLongevity< MyLifetimeNumber ,T> {}
			////////////////////////////////////////////////////////////////////////////////
			template <unsigned int Longevity, class T>
			class SingletonFixedLongevity
			{
			public:
				virtual ~SingletonFixedLongevity() {}

				static void ScheduleDestruction(T* pObj, atexit_pfn_t pFun)
				{
					Private::Adapter<T> adapter = { pFun };
					SetLongevity(pObj, Longevity , adapter);
				}

				static void OnDeadReference()
				{ throw std::logic_error("Dead Reference Detected"); }
			};

			///  \struct DieLast
			///  \ingroup LongevityLifetimeGroup
			///  \brief  Longest possible SingletonWithLongevity lifetime: 0xFFFFFFFF
			template <class T>
			struct DieLast  : SingletonFixedLongevity<0xFFFFFFFF ,T>
			{};

			///  \struct DieDirectlyBeforeLast
			///  \ingroup LongevityLifetimeGroup
			///  \brief  Lifetime is a one less than DieLast: 0xFFFFFFFF-1
			template <class T>
			struct DieDirectlyBeforeLast  : SingletonFixedLongevity<0xFFFFFFFF-1 ,T>
			{};

			///  \struct DieFirst
			///  \ingroup LongevityLifetimeGroup
			///  \brief  Shortest possible SingletonWithLongevity lifetime: 0 
			template <class T>
			struct DieFirst : SingletonFixedLongevity<0,T>
			{};

		}//namespace LongevityLifetime

		////////////////////////////////////////////////////////////////////////////////
		///  \class FollowIntoDeath
		///
		///  \ingroup LifetimeGroup
		///
		///  Lifetime policyfor the SingletonHolder tempalte. 
		///  Followers will die after the master dies Followers will not die, if
		///    - master never dies (NoDestroy policy)
		///    - master never created
		///    - master dies not in the function registered with atexit
		///    - master dies not by a call of a the atexit registerd function (DeletableSingleton::GracefulDelete)         
		///
		///  \par Usage:
		///
		///  Lifetimes of the master and the follower singletons,  e.g. with a M and a F class:
		///   \code SingletonHolder< M , FollowIntoDeath::With<DefaultLifetime>::AsMasterLifetime > MasterSingleton; \endcode
		///   \code SingletonHolder< F , CreateUsingNew, FollowIntoDeath::AfterMaster< MasterSingleton >::IsDestroyed > FollowerSingleton \endcode
		////////////////////////////////////////////////////////////////////////////////
		class FollowIntoDeath
		{
			template<class T>
			class Followers
			{
				typedef std::vector<atexit_pfn_t> Container;
				typedef typename Container::iterator iterator;
				static Container* followers_;

			public:
				static void Init()
				{
					static bool done = false;
					if(!done)
					{
						followers_ = XGC_NEW Container;
						done = true;
					}
				}

				static void AddFollower(atexit_pfn_t ae)
				{
					Init();
					followers_->push_back(ae);
				}

				static void DestroyFollowers()
				{
					Init();
					for(iterator it = followers_->begin();it != followers_->end();++it)
						(*it)();    
					delete followers_;
				}
			};

		public:

			///  \struct With
			///  Template for the master 
			///  \param Lifetime Lifetime policy for the master
			template<template <class> class Lifetime>
			struct With
			{
				///  \struct AsMasterLifetime
				///  Policy for master
				template<class Master>
				struct AsMasterLifetime
				{
					static void ScheduleDestruction(Master* pObj, atexit_pfn_t pFun)
					{
						Followers<Master>::Init();
						Lifetime<Master>::ScheduleDestruction(pObj, pFun);

						// use same policy for the followers and force a new 
						// template instantiation,  this adds a additional atexit entry
						// does not work with SetLonlevity, but there you can control
						// the lifetime with the GetLongevity function.
						Lifetime<Followers<Master> >::ScheduleDestruction(0,Followers<Master>::DestroyFollowers);
					}

					static void OnDeadReference()
					{ 
						throw std::logic_error("Dead Reference Detected"); 
					}
				};
			};

			///  \struct AfterMaster
			///  Template for the follower
			///  \param Master Master to follow into death
			template<class Master>
			struct AfterMaster
			{
				///  \struct IsDestroyed
				///  Policy for followers 
				template<class F>
				struct IsDestroyed
				{
					static void ScheduleDestruction(F*, atexit_pfn_t pFun)
					{
						Followers<Master>::AddFollower(pFun);
					}

					static void OnDeadReference()
					{ 
						throw std::logic_error("Dead Reference Detected"); 
					}
				};
			};
		};

		template<class T>
		typename FollowIntoDeath::Followers<T>::Container* 
			FollowIntoDeath::Followers<T>::followers_ = 0;

		template<class H>
		class SingleThread
		{
		public:
			typedef H VolatileType;
			typedef int Mutex;

			class Lock
			{
			public:
				Lock( int t ){ t; }
				~Lock(){}
			};
		};

		template<class H>
		class MultiThread
		{
		public:
			typedef volatile H VolatileType;
			typedef std::mutex Mutex;

			class Lock
			{
			public:
				explicit Lock( Mutex& cri )
					: ref_( cri )
					, locked_( false )
				{
					lock();
				}

				virtual ~Lock(void)throw()
				{
					free_lock( );
				}

				void lock()
				{ 
					if( !locked_ )
					{ 
						ref_.lock(); 
						locked_ = true; 
					} 
				}

				void free_lock( )
				{ 
					if( locked_ )
					{ 
						locked_ = false; 
						ref_.unlock(); 
					}
				}

			protected:
				Mutex& ref_;
				bool locked_;
			};
		};

		////////////////////////////////////////////////////////////////////////////////
		///  \class  SingletonHolder
		///
		///  \ingroup SingletonGroup
		///
		///  Provides Singleton amenities for a type T
		///  To protect that type from spurious instantiations, 
		///  you have to protect it yourself.
		///  
		///  \param CreationPolicy Creation policy, default: CreateUsingNew
		///  \param LifetimePolicy Lifetime policy, default: DefaultLifetime,
		///  \param ThreadingModel Threading policy, 
		///                         default: LOKI_DEFAULT_THREADING_NO_OBJ_LEVEL
		////////////////////////////////////////////////////////////////////////////////
		template
			<
			typename T,
			template <class> class CreationPolicy = CreateUsingNew,
			template <class> class LifetimePolicy = DefaultLifetime,
			template <class> class ThreadingModel = MultiThread
			>
		class SingletonHolder
		{
		public:

			///  Type of the singleton object
			typedef T ObjectType;
			typedef ThreadingModel<T*> ThreadType;

			///  Returns a reference to singleton object
			static T& Instance();

		private:
			// Helpers
			static void MakeInstance();
			static void __cdecl DestroySingleton();

			// Protection
			SingletonHolder();

			// Data
			typedef typename ThreadType::VolatileType PtrInstanceType;
			typedef typename ThreadType::Mutex Mutex;

			//static Mutex mutex_;
			static PtrInstanceType pInstance_;
			static bool destroyed_;
		};

		////////////////////////////////////////////////////////////////////////////////
		// SingletonHolder's data
		////////////////////////////////////////////////////////////////////////////////

		template
		<
			class T,
			template <class> class C,
			template <class> class L,
			template <class> class M
		>
		typename SingletonHolder<T, C, L, M>::PtrInstanceType
		SingletonHolder<T, C, L, M>::pInstance_ = 0;

		//template
		//<
		//	class T,
		//	template <class> class C,
		//	template <class> class L,
		//	template <class> class M
		//>
		//typename SingletonHolder<T, C, L, M>::Mutex 
		//SingletonHolder<T, C, L, M>::mutex_;

		template
		<
			class T,
			template <class> class C,
			template <class> class L,
			template <class> class M
		>
		bool SingletonHolder<T, C, L, M>::destroyed_ = false;

		////////////////////////////////////////////////////////////////////////////////
		// SingletonHolder::Instance
		////////////////////////////////////////////////////////////////////////////////

		template
		<
			class T,
			template <class> class CreationPolicy,
			template <class> class LifetimePolicy,
			template <class> class ThreadingModel
		>
		inline T& SingletonHolder<T, CreationPolicy, LifetimePolicy, ThreadingModel>::Instance()
		{
			if (!pInstance_)
			{
				MakeInstance();
			}
			return *pInstance_;
		}

		////////////////////////////////////////////////////////////////////////////////
		// SingletonHolder::MakeInstance (helper for Instance)
		////////////////////////////////////////////////////////////////////////////////

		template
		<
			class T,
			template <class> class CreationPolicy,
			template <class> class LifetimePolicy,
			template <class> class ThreadingModel
		>
		void SingletonHolder<T, CreationPolicy, LifetimePolicy, ThreadingModel>::MakeInstance()
		{
			static Mutex mutex_;

			typename ThreadingModel<SingletonHolder>::Lock guard( mutex_ );

			if (!pInstance_)
			{
				if (destroyed_)
				{
					destroyed_ = false;
					LifetimePolicy<T>::OnDeadReference();
				}
				pInstance_ = CreationPolicy<T>::Create();
				LifetimePolicy<T>::ScheduleDestruction(pInstance_, &DestroySingleton);

				//  added by lcj 2013-6-7 start
				register_singleton_ptr(pInstance_, &DestroySingleton);
				//  added by lcj 2013-6-7 end
			}
		}

		template
		<
			class T,
			template <class> class CreationPolicy,
			template <class> class L,
			template <class> class M
		>
		void __cdecl 
		SingletonHolder<T, CreationPolicy, L, M>::DestroySingleton()
		{
			// modified by lcj 2013-6-7 start
			//assert(!destroyed_);
			if (destroyed_)
			{
				return;
			}
			// modified by lcj 2013-6-7 end
			CreationPolicy<T>::Destroy(pInstance_);
			pInstance_ = 0;
			destroyed_ = true;
		}


		////////////////////////////////////////////////////////////////////////////////
		///  \class  Singleton
		///
		///  \ingroup SingletonGroup
		///
		///  Convenience template to implement a getter function for a singleton object.
		///  Often needed in a shared library which hosts singletons.
		///  
		///  \par Usage
		///
		///  see test/SingletonDll
		///
		////////////////////////////////////////////////////////////////////////////////

		template<class T>
		class Singleton
		{
		public:
			static T& Instance();
		};
	}
	
	//The first way to declare a singleton class
	/**
	how to use
	1. To avoid make and release the object freely, you should declare
	the construct and destruct function into private.
	2. Adding the DECLARE_SINGLETON(cls) in your class definition,
	the cls is the name of your class.
	3. Adding IMPLEMENT_SINGLETON(cls) to the implementation file of
	your class

	for example:
	//Sample.h
	class CSample
	{
	CSample();
	~CSample();
	DECLARE_SINGLETON(CSample)
	}

	//Sample.cpp
	IMPLEMENT_SINGLETON(CSample)
	*/

	//The second way to declare a singleton class
	#define  SINGLETON_FRIEND_CLASS_REGISTER(T)  friend  class  CSingleton<T> 
	/**
	@brief Singleton template class
	*
	*Remarks:
	*1.Your singleton class should be derived from this class publicly
	and its construct,destruct function should be declared in private.
	*2.Add the macro SINGLETON_FRIEND_CLASS_REGISTER(T) in the declaration
	of your class to register as friend class
	*/
	template<class  T>
	class   CSingleton
	{
	public:

		static bool CreateInstance()
		{
			if( NULL == m_poInstance )
			{
				m_poInstance = new T;
			}
			return m_poInstance != NULL;
		}
		static  T* Instance( void )
		{
			return m_poInstance;
		}

		static void DestroyInstance( void )
		{
			delete  m_poInstance;
			m_poInstance = NULL;
		}

	protected:
		CSingleton( void ) {}
		virtual ~CSingleton( void ) {}

	private:
		static T*   m_poInstance;
	};

	template<class T>
	T* CSingleton<T>::m_poInstance = NULL;
}
#endif// _SINGLETON_H_