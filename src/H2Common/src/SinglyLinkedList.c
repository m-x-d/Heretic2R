//
// SinglyLinkedList.c
//
// Copyright 1998 Raven Software
//

#include "SinglyLinkedList.h"
#include "ResourceManager.h"

typedef struct SinglyLinkedListNode_s
{
	union GenericUnion4_u data;
	struct SinglyLinkedListNode_s* next;
} SinglyLinkedListNode_t;

ResourceManager_t sllist_nodes_mgr;

H2COMMON_API void SLList_DefaultCon(SinglyLinkedList_t* this_ptr)
{
	SinglyLinkedListNode_t* node = ResMngr_AllocateResource(&sllist_nodes_mgr, SLL_NODE_SIZE);
	node->next = NULL;

	this_ptr->rearSentinel = node;
	this_ptr->front = node;
	this_ptr->current = node;
}

// List destructor.
H2COMMON_API void SLList_Des(SinglyLinkedList_t* this_ptr)
{
	if (this_ptr->rearSentinel == NULL) //mxd. List was already destroyed...
		return;

	SinglyLinkedListNode_t* node = this_ptr->front;
	while (node != this_ptr->rearSentinel)
	{
		SinglyLinkedListNode_t* next = node->next;
		ResMngr_DeallocateResource(&sllist_nodes_mgr, node, SLL_NODE_SIZE);
		node = next;
	}

	this_ptr->current = this_ptr->rearSentinel;
	ResMngr_DeallocateResource(&sllist_nodes_mgr, this_ptr->rearSentinel, SLL_NODE_SIZE);
}

H2COMMON_API qboolean SLList_AtEnd(const SinglyLinkedList_t* this_ptr)
{
	return this_ptr->current == this_ptr->rearSentinel;
}

H2COMMON_API qboolean SLList_AtLast(const SinglyLinkedList_t* this_ptr)
{
	return this_ptr->current->next == this_ptr->rearSentinel;
}

H2COMMON_API qboolean SLList_IsEmpty(const SinglyLinkedList_t* this_ptr)
{
	return this_ptr->front == this_ptr->rearSentinel;
}

H2COMMON_API GenericUnion4_t SLList_Increment(SinglyLinkedList_t* this_ptr)
{
	this_ptr->current = this_ptr->current->next;
	return this_ptr->current->data;
}

H2COMMON_API GenericUnion4_t SLList_PostIncrement(SinglyLinkedList_t* this_ptr)
{
	const GenericUnion4_t val = this_ptr->current->data;
	this_ptr->current = this_ptr->current->next;

	return val;
}

H2COMMON_API GenericUnion4_t SLList_Front(SinglyLinkedList_t* this_ptr)
{
	this_ptr->current = this_ptr->front;
	return this_ptr->current->data;
}

H2COMMON_API GenericUnion4_t SLList_ReplaceCurrent(const SinglyLinkedList_t* this_ptr, const GenericUnion4_t to_replace)
{
	const GenericUnion4_t val = this_ptr->current->data;
	this_ptr->current->data = to_replace;

	return val;
}

H2COMMON_API void SLList_PushEmpty(SinglyLinkedList_t* this_ptr)
{
	SinglyLinkedListNode_t* node = ResMngr_AllocateResource(&sllist_nodes_mgr, SLL_NODE_SIZE);
	node->next = this_ptr->front;
	this_ptr->front = node;
}

H2COMMON_API void SLList_Push(SinglyLinkedList_t* this_ptr, const GenericUnion4_t to_insert)
{
	SinglyLinkedListNode_t* node = ResMngr_AllocateResource(&sllist_nodes_mgr, SLL_NODE_SIZE);
	node->data = to_insert;
	node->next = this_ptr->front;
	this_ptr->front = node;
}

H2COMMON_API GenericUnion4_t SLList_Pop(SinglyLinkedList_t* this_ptr)
{
	SinglyLinkedListNode_t* front = this_ptr->front;
	SinglyLinkedListNode_t* next = front->next;

	this_ptr->front = next;
	if (this_ptr->current == front)
		this_ptr->current = next;

	const GenericUnion4_t val = front->data;
	ResMngr_DeallocateResource(&sllist_nodes_mgr, front, SLL_NODE_SIZE);

	return val;
}

H2COMMON_API void SLList_Chop(SinglyLinkedList_t* this_ptr)
{
	SinglyLinkedListNode_t* next = this_ptr->current->next;
	while (next != this_ptr->rearSentinel)
	{
		SinglyLinkedListNode_t* next_next = next->next;
		ResMngr_DeallocateResource(&sllist_nodes_mgr, next, SLL_NODE_SIZE);
		next = next_next;
	}
 
	this_ptr->current = this_ptr->rearSentinel;
}

H2COMMON_API void SLList_InsertAfter(const SinglyLinkedList_t* this_ptr, const GenericUnion4_t to_insert)
{
	SinglyLinkedListNode_t* node = ResMngr_AllocateResource(&sllist_nodes_mgr, SLL_NODE_SIZE);
	node->data = to_insert;
	node->next = this_ptr->current->next;
	this_ptr->current->next = node;
}