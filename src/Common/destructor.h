#pragma once
#ifndef _DESTRUCTOR_H_
#define _DESTRUCTOR_H_
#include "debuger.h"

template< class T >
struct AutoDeletePoint
{
	XGC_INLINE static void SafeDelete( const T & )
	{

	}
};

template< class T >
struct AutoDeletePoint< T* >
{
	XGC_INLINE static void SafeDelete( T* P )
	{
		SAFE_DELETE( P );
	}
};

template< class T >
struct AutoDeletePoint< const T* >
{
	XGC_INLINE static void SafeDelete( const T* P )
	{
		delete P;
	}
};

namespace xgc
{
	namespace common
	{
		template< class T >
		struct ContainerDestroyer
		{
			XGC_INLINE static void Destroy( const T& )
			{
			}
		};

		template< class T >
		struct ContainerDestroyer< T* >
		{
			XGC_INLINE static void Destroy( T* Point )
			{
				FUNCTION_BEGIN;
				AutoDeletePoint< T* >::SafeDelete( Point );
				FUNCTION_END;
			}
		};

		template< class T >
		struct ContainerDestroyer< const T* >
		{
			XGC_INLINE static void Destroy( const T* Point )
			{
				FUNCTION_BEGIN;
				AutoDeletePoint< const T* >::SafeDelete( Point );
				FUNCTION_END;
			}
		};


		template< class K, class V, class P, class A >
		struct ContainerDestroyer< typename std::map< K, V, P, A > >
		{
			XGC_INLINE static void Destroy( const std::map< K, V, P, A >& C )
			{
				for( auto iter = C.begin(); iter != C.end(); ++iter )
				{
					ContainerDestroyer< K >::Destroy( iter->first );
					ContainerDestroyer< V >::Destroy( iter->second );
				}
			}
		};

		template< class K, class V, class P, class A >
		struct ContainerDestroyer< typename std::map< K, V, P, A >* >
		{
			XGC_INLINE static void Destroy( const std::map< K, V, P, A >* C )
			{
				for( auto iter = C->begin(); iter != C->end(); ++iter )
				{
					ContainerDestroyer< K >::Destroy( iter->first );
					ContainerDestroyer< V >::Destroy( iter->second );
				}

				delete C;
			}
		};

		template< class K, class V, class H, class P, class A >
		struct ContainerDestroyer< typename std::unordered_map< K, V, H, P, A > >
		{
			XGC_INLINE static void Destroy( const std::unordered_map< K, V, H, P, A >& C )
			{
				for( auto iter = C.begin(); iter != C.end(); ++iter )
				{
					ContainerDestroyer< K >::Destroy( iter->first );
					ContainerDestroyer< V >::Destroy( iter->second );
				}
			}
		};

		template< class K, class V, class H, class P, class A >
		struct ContainerDestroyer< typename std::unordered_map< K, V, H, P, A >* >
		{
			XGC_INLINE static void Destroy( const std::unordered_map< K, V, H, P, A >* C )
			{
				for( auto iter = C->begin(); iter != C->end(); ++iter )
				{
					ContainerDestroyer< K >::Destroy( iter->first );
					ContainerDestroyer< V >::Destroy( iter->second );
				}

				delete C;
			}
		};

		template< class V, class P, class A >
		struct ContainerDestroyer< typename std::set< V, P, A > >
		{
			XGC_INLINE static void Destroy( const std::set< V, P, A >& C )
			{
				for( auto = iter, C.begin(); iter != C.end(); ++iter )
				{
					ContainerDestroyer< V >::Destroy( *iter );
				}
			}
		};

		template< class V, class P, class A >
		struct ContainerDestroyer< typename std::set< V, P, A >* >
		{
			XGC_INLINE static void Destroy( const std::set< V, P, A >* C )
			{
				for( auto iter = C->begin(); iter != C->end(); ++iter )
				{
					ContainerDestroyer< V >::Destroy( *iter );
				}

				delete C;
			}
		};

		template< class K, class H, class P, class A >
		struct ContainerDestroyer< typename std::unordered_set< K, H, P, A > >
		{
			XGC_INLINE static void Destroy( const std::unordered_set< K, H, P, A >& C )
			{
				for( auto iter = C.begin(); iter != C.end(); ++iter )
				{
					ContainerDestroyer< K >::Destroy( *iter );
				}
			}
		};

		template< class K, class H, class P, class A >
		struct ContainerDestroyer< typename std::unordered_set< K, H, P, A >* >
		{
			XGC_INLINE static void Destroy( const std::unordered_set< K, H, P, A >* C )
			{
				for( auto iter = C->begin(); iter != C->end(); ++iter )
				{
					ContainerDestroyer< K >::Destroy( *iter );
				}

				delete C;
			}
		};

		template< class V, class A >
		struct ContainerDestroyer< typename std::list< V, A > >
		{
			XGC_INLINE static void Destroy( const std::list< V, A >& C )
			{
				for( auto iter = C.begin(); iter != C.end(); ++iter )
				{
					ContainerDestroyer< V >::Destroy( *iter );
				}
			}
		};

		template< class V, class A >
		struct ContainerDestroyer< typename std::list< V, A >* >
		{
			XGC_INLINE static void Destroy( const std::list< V, A >* C )
			{
				for( auto iter = C->begin(); iter != C->end(); ++iter )
				{
					ContainerDestroyer< V >::Destroy( *iter );
				}

				delete C;
			}
		};

		template< class V, class A >
		struct ContainerDestroyer< typename std::vector< V, A > >
		{
			XGC_INLINE static void Destroy( const std::vector< V, A >& C )
			{
				for( auto iter = C.begin(); iter != C.end(); ++iter )
				{
					ContainerDestroyer< V >::Destroy( *iter );
				}
			}
		};

		template< class V, class A >
		struct ContainerDestroyer< typename std::vector< V, A >* >
		{
			XGC_INLINE static void Destroy( const std::vector< V, A >* C )
			{
				for( auto iter = C->begin(); iter != C->end(); ++iter )
				{
					ContainerDestroyer< V >::Destroy( *iter );
				}

				delete C;
			}
		};

		template< class K, class V, class P, class A >
		struct ContainerDestroyer < typename std::multimap< K, V, P, A > >
		{
			XGC_INLINE static void Destroy( const std::multimap< K, V, P, A >& C )
			{
				for( auto iter = C.begin(); iter != C.end(); ++iter )
				{
					ContainerDestroyer< K >::Destroy( iter->first );
					ContainerDestroyer< V >::Destroy( iter->second );
				}
			}
		};

		template< class K, class V, class P, class A >
		struct ContainerDestroyer < typename std::multimap< K, V, P, A >* >
		{
			XGC_INLINE static void Destroy( const std::multimap< K, V, P, A >* C )
			{
				for( auto iter = C->begin(); iter != C->end(); ++iter )
				{
					ContainerDestroyer< K >::Destroy( iter->first );
					ContainerDestroyer< V >::Destroy( iter->second );
				}

				delete C;
			}
		};

		template< class T >
		void DestroyContainer( const T& v )
		{
			ContainerDestroyer< T >::Destroy( v );
		}

		template< class T, template < class > class D >
		void DestroyContainer( const T& v )
		{
			D< T >::Destroy( v );
		}

		template< class T, class D >
		void DestroyContainer( const T& v, D pFn )
		{
			for each( auto it in v ) pFn( it );
		}
	}
}

#endif // _DESTRUCTOR_H_