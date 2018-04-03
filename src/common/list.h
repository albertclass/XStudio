#ifndef __LIST_H__
#define __LIST_H__

#pragma once
#include "defines.h"

namespace xgc
{
	struct node_t
	{
		struct node_t* list;
		struct node_t* prev;
		struct node_t* next;
	};

	inline struct node_t* list_init()
	{
		struct node_t* n = ( struct node_t* )malloc( sizeof( node_t ) );
		if( n == nullptr )
			return nullptr;

		n->list = n;
		n->next = n;
		n->prev = n;

		return n;
	}

	inline void list_free( struct node_t * lst )
	{
		lst->list = nullptr;
		lst->next = nullptr;
		lst->prev = nullptr;

		free( lst );
	}

	inline bool list_is_empty( struct node_t* lst )
	{
		return lst->prev == lst && lst->next == lst;
	}

	inline struct node_t* list_head( struct node_t* lst )
	{
		return lst->next;
	}

	inline struct node_t* list_tail( struct node_t* lst )
	{
		return lst;
	}

	inline struct node_t* list_last( struct node_t* lst )
	{
		return lst->prev;
	}

	inline void list_append( struct node_t* _Where, struct node_t* n )
	{
		n->prev = _Where;
		n->next = _Where->next;
		n->list = _Where->list;

		n->prev->next = n;
		n->next->prev = n;
	}

	inline void list_insert( struct node_t* _Where, struct node_t* n )
	{
		XGC_ASSERT( n->list == nullptr );

		n->next = _Where;
		n->prev = _Where->prev;
		n->list = _Where->list;

		n->prev->next = n;
		n->next->prev = n;
	}

	inline void list_remove( struct node_t* n )
	{
		if( list_is_empty( n ) )
			return;

		if( n == list_tail( n->list ) )
			return;

		n->next->prev = n->prev;
		n->prev->next = n->next;

		n->list = nullptr;
	}

	inline node_t* next_node( struct node_t* n )
	{
		return n->next;
	}

	inline node_t* prev_node( struct node_t* n )
	{
		return n->prev;
	}

	template< class _Pred >
	inline node_t* list_find_if( struct node_t* head, struct node_t* tail, _Pred& _fn )
	{
		if( head->list != tail->list )
			return nullptr;

		for( auto iter = head; iter != tail; iter = node_next( iter ) )
		{
			if( _fn( head ) )
				return head;
		}

		return tail;
	}
}
#endif // __LIST_H__