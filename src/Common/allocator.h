////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// This code accompanies the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design 
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author or Addison-Wesley Longman make no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////
#ifndef SMALLOBJ_INC_
#define SMALLOBJ_INC_
#include <cstddef>
#include <new> // needed for std::nothrow_t parameter.

#include "exports.h"
#include "singleton.h"

#ifndef DEFAULT_CHUNK_SIZE
#define DEFAULT_CHUNK_SIZE 4096
#endif

#ifndef MAX_SMALL_OBJECT_SIZE
#define MAX_SMALL_OBJECT_SIZE 256
#endif

#ifndef DEFAULT_OBJECT_ALIGNMENT
#define DEFAULT_OBJECT_ALIGNMENT 4
#endif

#ifndef DEFAULT_SMALLOBJ_LIFETIME
#define DEFAULT_SMALLOBJ_LIFETIME LongevityLifetime::DieAsSmallObjectParent
#endif

///  \defgroup  SmallObjectGroup Small objects
///
///  \defgroup  SmallObjectGroupInternal Internals
///  \ingroup   SmallObjectGroup

namespace xgc
{
	namespace common
	{
		namespace LongevityLifetime
		{
			/** @struct DieAsSmallObjectParent
				@ingroup SmallObjectGroup
				Lifetime policy to manage lifetime dependencies of 
				SmallObject base and child classes.
				The Base class should have this lifetime
			*/
			template <class T>
			struct DieAsSmallObjectParent  : DieLast<T> {};

			/** @struct DieAsSmallObjectChild
				@ingroup SmallObjectGroup
				Lifetime policy to manage lifetime dependencies of 
				SmallObject base and child classes.
				The Child class should have this lifetime
			*/
			template <class T>
			struct DieAsSmallObjectChild  : DieDirectlyBeforeLast<T> {};

		} 

		class FixedAllocator;

		/** @class SmallObjAllocator
			@ingroup SmallObjectGroupInternal
		 Manages pool of fixed-size allocators.
		 Designed to be a non-templated base class of AllocatorSingleton so that
		 implementation details can be safely hidden in the source code file.
		 */
		class COMMON_API SmallObjAllocator
		{
		protected:
			/** The only available constructor needs certain parameters in order to
			 initialize all the FixedAllocator's.  This throws only if
			 @param pageSize # of bytes in a page of memory.
			 @param maxObjectSize Max # of bytes which this may allocate.
			 @param objectAlignSize # of bytes between alignment boundaries.
			 */
			SmallObjAllocator( std::size_t pageSize, std::size_t maxObjectSize, std::size_t objectAlignSize );

			/** Destructor releases all blocks, all Chunks, and FixedAllocator's.
			 Any outstanding blocks are unavailable, and should not be used after
			 this destructor is called.  The destructor is deliberately non-virtual
			 because it is protected, not public.
			 */
			~SmallObjAllocator( void );

		public:
			/** Allocates a block of memory of requested size.  Complexity is often
			 constant-time, but might be O(C) where C is the number of Chunks in a
			 FixedAllocator. 

			 @par Exception Safety Level
			 Provides either strong-exception safety, or no-throw exception-safety
			 level depending upon doThrow parameter.  The reason it provides two
			 levels of exception safety is because it is used by both the nothrow
			 and throwing new operators.  The underlying implementation will never
			 throw of its own accord, but this can decide to throw if it does not
			 allocate.  The only exception it should emit is std::bad_alloc.

			 @par Allocation Failure
			 If it does not allocate, it will call TrimExcessMemory and attempt to
			 allocate again, before it decides to throw or return NULL.  Many
			 allocators loop through several new_handler functions, and terminate
			 if they can not allocate, but not this one.  It only makes one attempt
			 using its own implementation of the new_handler, and then returns NULL
			 or throws so that the program can decide what to do at a higher level.
			 (Side note: Even though the C++ Standard allows allocators and
			 new_handlers to terminate if they fail, the Loki allocator does not do
			 that since that policy is not polite to a host program.)

			 @param size # of bytes needed for allocation.
			 @param doThrow True if this should throw if unable to allocate, false
			  if it should provide no-throw exception safety level.
			 @return NULL if nothing allocated and doThrow is false.  Else the
			  pointer to an available block of memory.
			 */
			void * Allocate( std::size_t size, bool doThrow );

			/** Deallocates a block of memory at a given place and of a specific
			size.  Complexity is almost always constant-time, and is O(C) only if
			it has to search for which Chunk deallocates.  This never throws.
			 */
			void Deallocate( void * p, std::size_t size );

			/** Deallocates a block of memory at a given place but of unknown size
			size.  Complexity is O(F + C) where F is the count of FixedAllocator's
			in the pool, and C is the number of Chunks in all FixedAllocator's.  This
			does not throw exceptions.  This overloaded version of Deallocate is
			called by the nothow delete operator - which is called when the nothrow
			new operator is used, but a constructor throws an exception.
			 */
			void Deallocate( void * p );

			/// Returns max # of bytes which this can allocate.
			inline std::size_t GetMaxObjectSize() const	{ return maxSmallObjectSize_; }

			/// Returns # of bytes between allocation boundaries.
			inline std::size_t GetAlignment() const { return objectAlignSize_; }

			/** Releases empty Chunks from memory.  Complexity is O(F + C) where F
			is the count of FixedAllocator's in the pool, and C is the number of
			Chunks in all FixedAllocator's.  This will never throw.  This is called
			by AllocatorSingleto::ClearExtraMemory, the new_handler function for
			Loki's allocator, and is called internally when an allocation fails.
			@return True if any memory released, or false if none released.
			 */
			bool TrimExcessMemory( void );

			/** Returns true if anything in implementation is corrupt.  Complexity
			 is O(F + C + B) where F is the count of FixedAllocator's in the pool,
			 C is the number of Chunks in all FixedAllocator's, and B is the number
			 of blocks in all Chunks.  If it determines any data is corrupted, this
			 will return true in release version, but assert in debug version at
			 the line where it detects the corrupted data.  If it does not detect
			 any corrupted data, it returns false.
			 */
			bool IsCorrupt( void ) const;

		private:
			/// Default-constructor is not implemented.
			SmallObjAllocator( void );
			/// Copy-constructor is not implemented.
			SmallObjAllocator( const SmallObjAllocator & );
			/// Copy-assignment operator is not implemented.
			SmallObjAllocator & operator = ( const SmallObjAllocator & );

			/// Pointer to array of fixed-size allocators.
			FixedAllocator * pool_;

			/// Largest object size supported by allocators.
			const std::size_t maxSmallObjectSize_;

			/// Size of alignment boundaries.
			const std::size_t objectAlignSize_;
		};


		/** @class AllocatorSingleton
			@ingroup SmallObjectGroupInternal
		 This template class is derived from
		 SmallObjAllocator in order to pass template arguments into it, and still
		 have a default constructor for the singleton.  Each instance is a unique
		 combination of all the template parameters, and hence is singleton only 
		 with respect to those parameters.  The template parameters have default
		 values and the class has typedefs identical to both SmallObject and
		 SmallValueObject so that this class can be used directly instead of going
		 through SmallObject or SmallValueObject.  That design feature allows
		 clients to use the new_handler without having the name of the new_handler
		 function show up in classes derived from SmallObject or SmallValueObject.
		 Thus, the only functions in the allocator which show up in SmallObject or
		 SmallValueObject inheritance hierarchies are the new and delete
		 operators.
		*/
		template
		<
			template <class> class ThreadingModel = MultiThread,
			std::size_t chunkSize = DEFAULT_CHUNK_SIZE,
			std::size_t maxSmallObjectSize = MAX_SMALL_OBJECT_SIZE,
			std::size_t objectAlignSize = DEFAULT_OBJECT_ALIGNMENT,
			template <class> class LifetimePolicy = DEFAULT_SMALLOBJ_LIFETIME
		>
		class AllocatorSingleton : public SmallObjAllocator
		{
		public:

			/// Defines type of allocator.
			typedef AllocatorSingleton< ThreadingModel, chunkSize, maxSmallObjectSize, objectAlignSize, LifetimePolicy > MyAllocator;

			/// Defines type for thread-safety locking mechanism.
			typedef ThreadingModel< MyAllocator > MyThreadingModel;

			/// Defines singleton made from allocator.
			typedef SingletonHolder< MyAllocator, CreateStatic, LifetimePolicy, ThreadingModel > MyAllocatorSingleton;

			/// Returns reference to the singleton.
			inline static AllocatorSingleton & Instance( void )
			{
				return MyAllocatorSingleton::Instance();
			}

			/// The default constructor is not meant to be called directly.
			inline AllocatorSingleton() :
				SmallObjAllocator( chunkSize, maxSmallObjectSize, objectAlignSize )
				{}

			/// The destructor is not meant to be called directly.
			inline ~AllocatorSingleton( void ) {}

			void * LockAllocate( std::size_t size, bool doThrow );

			/** Deallocates a block of memory at a given place and of a specific
			size.  Complexity is almost always constant-time, and is O(C) only if
			it has to search for which Chunk deallocates.  This never throws.
			 */
			void LockDeallocate( void * p, std::size_t size );


			/** Clears any excess memory used by the allocator.  Complexity is
			 O(F + C) where F is the count of FixedAllocator's in the pool, and C
			 is the number of Chunks in all FixedAllocator's.  This never throws.
			 @note This function can be used as a new_handler when Loki and other
			 memory allocators can no longer allocate.  Although the C++ Standard
			 allows new_handler functions to terminate the program when they can
			 not release any memory, this will not do so.
			 */
			static void ClearExtraMemory( void );

			/** Returns true if anything in implementation is corrupt.  Complexity
			 is O(F + C + B) where F is the count of FixedAllocator's in the pool,
			 C is the number of Chunks in all FixedAllocator's, and B is the number
			 of blocks in all Chunks.  If it determines any data is corrupted, this
			 will return true in release version, but assert in debug version at
			 the line where it detects the corrupted data.  If it does not detect
			 any corrupted data, it returns false.
			 */
			static bool IsCorrupted( void );

		private:
			typename MyThreadingModel::Mutex mutex_;
			/// Copy-constructor is not implemented.
			AllocatorSingleton( const AllocatorSingleton & );
			/// Copy-assignment operator is not implemented.
			AllocatorSingleton & operator = ( const AllocatorSingleton & );
		};

		template
		<
			template <class> class T,
			std::size_t C,
			std::size_t M,
			std::size_t O,
			template <class> class L
		>
		void * AllocatorSingleton< T, C, M, O, L >::LockAllocate( std::size_t size, bool doThrow )
		{
			typename MyThreadingModel::Lock lock(mutex_);
			return Allocate( size, doThrow );
		}

		template
		<
			template <class> class T,
			std::size_t C,
			std::size_t M,
			std::size_t O,
			template <class> class L
		>
		void AllocatorSingleton< T, C, M, O, L >::LockDeallocate( void * p, std::size_t size )
		{
			typename MyThreadingModel::Lock lock(mutex_);
			Deallocate( p, size );
		}

		template
		<
			template <class> class T,
			std::size_t C,
			std::size_t M,
			std::size_t O,
			template <class> class L
		>
		void AllocatorSingleton< T, C, M, O, L >::ClearExtraMemory( void )
		{
			typename MyThreadingModel::Lock lock(AllocatorSingleton< T, C, M, O, L >::mutex_);
			Instance().TrimExcessMemory();
		}

		template
		<
			template <class> class T,
			std::size_t C,
			std::size_t M,
			std::size_t O,
			template <class> class L
		>
		bool AllocatorSingleton< T, C, M, O, L >::IsCorrupted( void )
		{
			typename MyThreadingModel::Lock lock(AllocatorSingleton< T, C, M, O, L >::mutex_);
			return Instance().IsCorrupt();
		}

		/** This standalone function provides the longevity level for Small-Object
		 Allocators which use the Loki::SingletonWithLongevity policy.  The
		 SingletonWithLongevity class can find this function through argument-
		 dependent lookup.

		 @par Longevity Levels
		 No Small-Object Allocator depends on any other Small-Object allocator, so
		 this does not need to calculate dependency levels among allocators, and
		 it returns just a constant.  All allocators must live longer than the
		 objects which use the allocators, it must return a longevity level higher
		 than any such object.
		 */
		template
		<
			template <class> class T,
			std::size_t C,
			std::size_t M,
			std::size_t O,
			template <class> class L
		>
		COMMON_API inline unsigned int GetLongevity(
			AllocatorSingleton< T, C, M, O, L > * )
		{
			// Returns highest possible value.
			return 0xFFFFFFFF;
		}


		/** @class SmallObjectBase
			@ingroup SmallObjectGroup
		 Base class for small object allocation classes.
		 The shared implementation of the new and delete operators are here instead
		 of being duplicated in both SmallObject or SmallValueObject, later just 
		 called Small-Objects.  This class is not meant to be used directly by clients, 
		 or derived from by clients. Class has no data members so compilers can 
		 use Empty-Base-Optimization.

		 @par ThreadingModel
		 This class doesn't support ObjectLevelLockable policy for ThreadingModel.
		 The allocator is a singleton, so a per-instance mutex is not necessary.
		 Nor is using ObjectLevelLockable recommended with SingletonHolder since
		 the SingletonHolder::MakeInstance function requires a mutex that exists
		 prior to when the object is created - which is not possible if the mutex
		 is inside the object, such as required for ObjectLevelLockable.  If you
		 attempt to use ObjectLevelLockable, the compiler will emit errors because
		 it can't use the default constructor in ObjectLevelLockable.  If you need
		 a thread-safe allocator, use the ClassLevelLockable policy.

		 @par Lifetime Policy
     
		 The SmallObjectBase template needs a lifetime policy because it owns
		 a singleton of SmallObjAllocator which does all the low level functions. 
		 When using a Small-Object in combination with the SingletonHolder template
		 you have to choose two lifetimes, that of the Small-Object and that of
		 the singleton. The rule is: The Small-Object lifetime must be greater than
		 the lifetime of the singleton hosting the Small-Object. Violating this rule
		 results in a crash on exit, because the hosting singleton tries to delete
		 the Small-Object which is then already destroyed. 
     
		 The lifetime policies recommended for use with Small-Objects hosted 
		 by a SingletonHolder template are 
			 - LongevityLifetime::DieAsSmallObjectParent / LongevityLifetime::DieAsSmallObjectChild
			 - SingletonWithLongevity
			 - FollowIntoDeath (not supported by MSVC 7.1)
			 - NoDestroy
     
		 The default lifetime of Small-Objects is 
		 LongevityLifetime::DieAsSmallObjectParent to
		 insure that memory is not released before a object with the lifetime
		 LongevityLifetime::DieAsSmallObjectChild using that
		 memory is destroyed. The LongevityLifetime::DieAsSmallObjectParent
		 lifetime has the highest possible value of a SetLongevity lifetime, so
		 you can use it in combination with your own lifetime not having also
		 the highest possible value.
     
		 The DefaultLifetime and PhoenixSingleton policies are *not* recommended 
		 since they can cause the allocator to be destroyed and release memory 
		 for singletons hosting a object which inherit from either SmallObject
		 or SmallValueObject.  
     
		 @par Lifetime usage
    
			- LongevityLifetime: The Small-Object has 
			  LongevityLifetime::DieAsSmallObjectParent policy and the Singleton
			  hosting the Small-Object has LongevityLifetime::DieAsSmallObjectChild. 
			  The child lifetime has a hard coded SetLongevity lifetime which is 
			  shorter than the lifetime of the parent, thus the child dies 
			  before the parent.
         
			- Both Small-Object and Singleton use SingletonWithLongevity policy.
			  The longevity level for the singleton must be lower than that for the
			  Small-Object. This is why the AllocatorSingleton's GetLongevity function 
			  returns the highest value.
         
			- FollowIntoDeath lifetime: The Small-Object has 
			  FollowIntoDeath::With<LIFETIME>::AsMasterLiftime
			  policy and the Singleton has 
			  FollowIntoDeath::AfterMaster<MASTERSINGLETON>::IsDestroyed policy,
			  where you could choose the LIFETIME. 
        
			- Both Small-Object and Singleton use NoDestroy policy. 
			  Since neither is ever destroyed, the destruction order does not matter.
			  Note: you will get memory leaks!
         
			- The Small-Object has NoDestroy policy but the Singleton has
			  SingletonWithLongevity policy. Note: you will get memory leaks!
         
     
		 You should *not* use NoDestroy for the singleton, and then use
		 SingletonWithLongevity for the Small-Object. 
     
		 @par Examples:
     
		 - test/SmallObj/SmallSingleton.cpp
		 - test/Singleton/Dependencies.cpp
		 */
		template
		<
			template <class> class ThreadingModel,
			std::size_t chunkSize,
			std::size_t maxSmallObjectSize,
			std::size_t objectAlignSize,
			template <class> class LifetimePolicy
		>
		class COMMON_API SmallObjectBase
		{

	#if (MAX_SMALL_OBJECT_SIZE != 0) && (DEFAULT_CHUNK_SIZE != 0) && (DEFAULT_OBJECT_ALIGNMENT != 0)

		public:        
			/// Defines type of allocator singleton, must be public 
			/// to handle singleton lifetime dependencies.
			typedef AllocatorSingleton< ThreadingModel, chunkSize, maxSmallObjectSize, objectAlignSize, LifetimePolicy > ObjAllocatorSingleton;
    
			/// Defines type _Myt;
			typedef SmallObjectBase< ThreadingModel, chunkSize, maxSmallObjectSize, objectAlignSize, LifetimePolicy > _Myt;
		private:

			/// Defines type for thread-safety locking mechanism.
			typedef ThreadingModel< ObjAllocatorSingleton > MyThreadingModel;

			/// Use singleton defined in AllocatorSingleton.
			typedef typename ObjAllocatorSingleton::MyAllocatorSingleton MyAllocatorSingleton;
        
		public:

			/// Throwing single-object new throws bad_alloc when allocation fails.
	#ifdef _MSC_VER
			/// @note MSVC complains about non-empty exception specification lists.
			static void * operator new ( std::size_t size )
	#else
			static void * operator new ( std::size_t size ) throw ( std::bad_alloc )
	#endif
			{
				typename MyThreadingModel::Lock lock( _Myt::mutex_ );
				return MyAllocatorSingleton::Instance().Allocate( size, true );
			}

			/// Non-throwing single-object new returns NULL if allocation fails.
			static void * operator new ( std::size_t size, const std::nothrow_t & ) throw ()
			{
				typename MyThreadingModel::Lock lock( _Myt::mutex_ );
				return MyAllocatorSingleton::Instance().Allocate( size, false );
			}

			/// Placement single-object new merely calls global placement new.
			inline static void * operator new ( std::size_t size, void * place )
			{
				return ::operator new( size, place );
			}

			/// Single-object delete.
			static void operator delete ( void * p, std::size_t size ) throw ()
			{
				typename MyThreadingModel::Lock lock( _Myt::mutex_ );
				MyAllocatorSingleton::Instance().Deallocate( p, size );
			}

			/** Non-throwing single-object delete is only called when nothrow
			 new operator is used, and the constructor throws an exception.
			 */
			static void operator delete ( void * p, const std::nothrow_t & ) throw()
			{
				typename MyThreadingModel::Lock lock( _Myt::mutex_ );
				MyAllocatorSingleton::Instance().Deallocate( p );
			}

			/// Placement single-object delete merely calls global placement delete.
			inline static void operator delete ( void * p, void * place )
			{
				::operator delete ( p, place );
			}

			/// Throwing array-object new throws bad_alloc when allocation fails.
	#ifdef _MSC_VER
			/// @note MSVC complains about non-empty exception specification lists.
			static void * operator new [] ( std::size_t size )
	#else
			static void * operator new [] ( std::size_t size )
				throw ( std::bad_alloc )
	#endif
			{
				typename MyThreadingModel::Lock lock( _Myt::mutex_ );
				return MyAllocatorSingleton::Instance().Allocate( size, true );
			}

			/// Non-throwing array-object new returns NULL if allocation fails.
			static void * operator new [] ( std::size_t size, const std::nothrow_t & ) throw ()
			{
				typename MyThreadingModel::Lock lock( _Myt::mutex_ );
				return MyAllocatorSingleton::Instance().Allocate( size, false );
			}

			/// Placement array-object new merely calls global placement new.
			inline static void * operator new [] ( std::size_t size, void * place )
			{
				return ::operator new( size, place );
			}

			/// Array-object delete.
			static void operator delete [] ( void * p, std::size_t size ) throw ()
			{
				typename MyThreadingModel::Lock lock( _Myt::mutex_ );
				MyAllocatorSingleton::Instance().Deallocate( p, size );
			}

			/** Non-throwing array-object delete is only called when nothrow
			 new operator is used, and the constructor throws an exception.
			 */
			static void operator delete [] ( void * p, const std::nothrow_t & ) throw()
			{
				typename MyThreadingModel::Lock lock( _Myt::mutex_ );
				MyAllocatorSingleton::Instance().Deallocate( p );
			}

			/// Placement array-object delete merely calls global placement delete.
			inline static void operator delete [] ( void * p, void * place )
			{
				::operator delete ( p, place );
			}

	#endif  // #if default template parameters are not zero

		protected:
			inline SmallObjectBase( void ) {}
			inline SmallObjectBase( const SmallObjectBase & ) {}
			inline SmallObjectBase & operator = ( const SmallObjectBase & )
			{ return *this; }
			inline ~SmallObjectBase() {}
		private:
			typename MyThreadingModel::Mutex mutex_;
		}; // end class SmallObjectBase


		/** @class SmallObject
			@ingroup SmallObjectGroup
		 SmallObject Base class for polymorphic small objects, offers fast
		 allocations & deallocations.  Destructor is virtual and public.  Default
		 constructor is trivial.   Copy-constructor and copy-assignment operator are
		 not implemented since polymorphic classes almost always disable those
		 operations.  Class has no data members so compilers can use
		 Empty-Base-Optimization.
		 */
		template
		<
			template <class> class ThreadingModel = MutilThread,
			std::size_t chunkSize = DEFAULT_CHUNK_SIZE,
			std::size_t maxSmallObjectSize = MAX_SMALL_OBJECT_SIZE,
			std::size_t objectAlignSize = DEFAULT_OBJECT_ALIGNMENT,
			template <class> class LifetimePolicy = DEFAULT_SMALLOBJ_LIFETIME
		>
		class COMMON_API SmallObject : public SmallObjectBase< ThreadingModel, chunkSize, maxSmallObjectSize, objectAlignSize, LifetimePolicy >
		{

		public:
			virtual ~SmallObject() {}
		protected:
			inline SmallObject( void ) {}

		private:
			/// Copy-constructor is not implemented.
			SmallObject( const SmallObject & );
			/// Copy-assignment operator is not implemented.
			SmallObject & operator = ( const SmallObject & );
		}; // end class SmallObject


		/** @class SmallValueObject
			@ingroup SmallObjectGroup
		 SmallValueObject Base class for small objects with value-type
		 semantics - offers fast allocations & deallocations.  Destructor is
		 non-virtual, inline, and protected to prevent unintentional destruction
		 through base class.  Default constructor is trivial.   Copy-constructor
		 and copy-assignment operator are trivial since value-types almost always
		 need those operations.  Class has no data members so compilers can use
		 Empty-Base-Optimization.
		 */
		template
		<
			template <class> class ThreadingModel = MutilThread,
			std::size_t chunkSize = DEFAULT_CHUNK_SIZE,
			std::size_t maxSmallObjectSize = MAX_SMALL_OBJECT_SIZE,
			std::size_t objectAlignSize = DEFAULT_OBJECT_ALIGNMENT,
			template <class> class LifetimePolicy = DEFAULT_SMALLOBJ_LIFETIME
		>
		class COMMON_API SmallValueObject : public SmallObjectBase< ThreadingModel, chunkSize, maxSmallObjectSize, objectAlignSize, LifetimePolicy >
		{
		protected:
			inline SmallValueObject( void ) {}
			inline SmallValueObject( const SmallValueObject & ) {}
			inline SmallValueObject & operator = ( const SmallValueObject & )
			{ return *this; }
			inline ~SmallValueObject() {}
		}; // end class SmallValueObject

		template
		<
			typename Type,
			template <class> class ThreadingModel = MultiThread,
			std::size_t chunkSize = DEFAULT_CHUNK_SIZE,
			std::size_t maxSmallObjectSize = MAX_SMALL_OBJECT_SIZE,
			std::size_t objectAlignSize = DEFAULT_OBJECT_ALIGNMENT,
			template <class> class LifetimePolicy = DEFAULT_SMALLOBJ_LIFETIME,
			typename AllocT = AllocatorSingleton< ThreadingModel, chunkSize, maxSmallObjectSize, objectAlignSize, LifetimePolicy >
		>
		class COMMON_API allocator
		{
		public:

			typedef ::std::size_t size_type;
			typedef ::std::ptrdiff_t difference_type;
			typedef Type * pointer;
			typedef const Type * const_pointer;
			typedef Type & reference;
			typedef const Type & const_reference;
			typedef Type value_type;

			/// Default constructor does nothing.
			inline allocator( void ) throw() { }

			/// Copy constructor does nothing.
			inline allocator( const allocator & ) throw() { }

			/// Type converting allocator constructor does nothing.
			template < typename Type1 >
			inline allocator( const allocator< Type1 > & ) throw() { }

			/// Destructor does nothing.
			inline ~allocator() throw() { }

			/// Convert an allocator<Type> to an allocator <Type1>.
			template < typename Type1 >
			struct rebind
			{
				typedef allocator< Type1 > other;
			};

			/// Return address of reference to mutable element.
			pointer address( reference elem ) const { return &elem; }

			/// Return address of reference to const element.
			const_pointer address( const_reference elem ) const { return &elem; }

			/** Allocate an array of count elements.  Warning!  The true parameter in
			 the call to Allocate means this function can throw exceptions.  This is
			 better than not throwing, and returning a null pointer in case the caller
			 assumes the return value is not null.
			 @param count # of elements in array.
			 @param hint Place where caller thinks allocation should occur.
			 @return Pointer to block of memory.
			 */
			pointer allocate( size_type count, const void * hint = 0 )
			{
				(void)hint;  // Ignore the hint.

				void * p = AllocT::Instance().LockAllocate( count * sizeof( Type ), true );
				return reinterpret_cast< pointer >( p );
			}

			/// Ask allocator to release memory at pointer with size bytes.
			void deallocate( pointer p, size_type size )
			{
				AllocT::Instance().LockDeallocate( p, size * sizeof( Type ) );
			}

			/// Calculate max # of elements allocator can handle.
			size_type max_size( void ) const throw()
			{
				// A good optimizer will see these calculations always produce the same
				// value and optimize this function away completely.
				const size_type max_bytes = size_type( -1 );
				const size_type bytes = max_bytes / sizeof( Type );
				return bytes;
			}

			/// Construct an element at the pointer.
			void construct( pointer p, const Type & value )
			{
				// A call to global placement new forces a call to copy constructor.
				::new( p ) Type( value );
			}

			/// Destruct the object at pointer.
			void destroy( pointer p )
			{
				// If the Type has no destructor, then some compilers complain about
				// an unreferenced parameter, so use the void cast trick to prevent
				// spurious warnings.
				(void)p;
				p->~Type();
			}

		};

		//-----------------------------------------------------------------------------

		/** All equality operators return true since allocator is basically a
		 monostate design pattern, so all instances of it are identical.
		 */
		template < typename Type >
		COMMON_API inline bool operator == ( const allocator< Type > &, const allocator< Type > & )
		{
			return true;
		}

		/** All inequality operators return false since allocator is basically a
		 monostate design pattern, so all instances of it are identical.
		 */
		template < typename Type >
		COMMON_API inline bool operator != ( const allocator< Type > &, const allocator< Type > & )
		{
			return false;
		}

	} // namespace common
} // namespace xgc

//template< xgc_size S >	using xBuffer = xBufferImpl< xBufferStaticStorage< S > >;

#if defined(XGC_ALLOCATOR)
#	include "allocator.h"
#	define xgc_allocator xgc::common::allocator
#	define xgc_allocator_fast xgc::common::allocator
#elif defined(BOOST_ALLOCATOR)
#	include "boost/pool/pool_alloc.hpp"
#	include "boost/pool/singleton_pool.hpp"
#	include "boost/pool/detail/mutex.hpp"
#	define xgc_allocator boost::pool_allocator
#	define xgc_allocator_fast boost::fast_pool_allocator
#else
#	define xgc_allocator std::allocator
#	define xgc_allocator_fast std::allocator
#endif

#include <set>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <deque>
#include <vector>
#include <string>
#include <array>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#if (_MSC_VER >= 1800)
#pragma warning( disable:4251 )
using xgc_string = std::basic_string< char, std::char_traits<char>, xgc_allocator< char > >;

template<class _Kty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast<_Kty> >
using xgc_set = std::set< _Kty, _Pr, _Alloc >;

template<class _Ty1, class _Ty2 >
using xgc_pair = std::pair< _Ty1, _Ty2 >;

template<class _Kty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast<_Kty> >
using xgc_multiset = std::multiset< _Kty, _Pr, _Alloc >;

template<class _Kty, class _Ty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
using xgc_map = std::map< _Kty, _Ty, _Pr, _Alloc >;

template<class _Kty, class _Ty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
using xgc_multimap = std::multimap< _Kty, _Ty, _Pr, _Alloc >;

template<class _Kty, class _Ty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
using xgc_unordered_map = std::unordered_map< _Kty, _Ty, _Hasher, _Keyeq, _Alloc >;

template<class _Kty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = xgc_allocator_fast< _Kty > >
using xgc_unordered_set = std::unordered_set< _Kty, _Hasher, _Keyeq, _Alloc > ;

template<class _Kty, class _Ty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
using xgc_unordered_multimap = std::unordered_multimap < _Kty, _Ty, _Hasher, _Keyeq, _Alloc > ;

template<class _Kty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = xgc_allocator_fast< _Kty > >
using xgc_unordered_multiset = std::unordered_multiset < _Kty, _Hasher, _Keyeq, _Alloc > ;

template<class _Ty, class _Ax = xgc_allocator< _Ty > >
using xgc_list = std::list< _Ty, _Ax >;

template<class _Ty, class _Ax = xgc_allocator< _Ty > >
using xgc_deque = std::deque< _Ty, _Ax >;

template< class _Ty, class _Ax = xgc_allocator_fast<_Ty>, class _Container = std::deque<_Ty, _Ax> >
using xgc_queue = std::queue< _Ty, _Container >;

template< class _Ty, class _Ax = xgc_allocator_fast<_Ty>, class _Container = std::deque<_Ty, _Ax> >
using xgc_stack = std::stack< _Ty, _Container >;

template< class _Ty, class _Ax = xgc_allocator_fast<_Ty> >
using xgc_vector = std::vector< _Ty, _Ax >;

template < class _Ty, size_t _Size >
using xgc_array = std::array< _Ty, _Size >;

template < class ... _Types >
using xgc_tuple = std::tuple < _Types... >;

#else
template<class _Kty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast<_Kty> >
struct xgc_set { typedef std::set< _Kty, _Pr, _Alloc > type; };

template<class _Ty1, class _Ty2 >
struct xgc_pair { typedef std::pair< _Ty1, _Ty2 > type; };

template<class _Kty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast<_Kty> >
struct xgc_multiset { typedef std::multiset< _Kty, _Pr, _Alloc > type };

template<class _Kty, class _Ty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
struct xgc_map { typedef std::map< _Kty, _Ty, _Pr, _Alloc > type; };

template<class _Kty, class _Ty, class _Pr = std::less<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
struct xgc_multimap { typedef std::multimap< _Kty, _Ty, _Pr, _Alloc > type; };

template<class _Kty, class _Ty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>, class _Alloc = xgc_allocator_fast< std::pair<const _Kty, _Ty> > >
struct xgc_unordered_map {typedef  std::unordered_map< _Kty, _Ty, _Hasher, _Keyeq, _Alloc > type };

template<class _Ty, class _Ax = xgc_allocator< _Ty > >
struct xgc_deque { typedef std::deque< _Ty, _Ax > type; };

template<class _Ty, class _Ax = xgc_allocator_fast<_Ty>, class _Container = std::deque<_Ty, _Ax> >
struct xgc_queue { typedef std::queue< _Ty, _Container > type };

template<class _Ty, class _Ax = xgc_allocator_fast<_Ty>, class _Container = std::deque<_Ty, _Ax> >
struct xgc_stack { typedef std::stack< _Ty, _Container > type; };

template< class _Ty, class _Ax = xgc_allocator_fast<_Ty> >
struct xgc_vector { typedef std::vector< _Ty, _Ax > type; };

template < class _Ty, size_t _Size >
struct xgc_array { typedef std::array< _Ty, _Size > type; };

#endif

#endif // end file guardian

