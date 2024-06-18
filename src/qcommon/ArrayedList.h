//
// ArrayedList.h
//
// Copyright 1998 Raven Software
//

#pragma once

typedef struct ArrayedListNode_s
{
	int data;
	int next;
	int inUse;
} ArrayedListNode_t;

#define ARRAYEDLISTNODE_NULL -1

//TODO: un-inline?
_inline int GetFreeNode(ArrayedListNode_t* nodeArray, int max)
{
	for (int i = 0; i < max; ++i)
	{
		if (!nodeArray[i].inUse)
		{
			nodeArray[i].inUse = 1;
			return i;
		}
	}

	return -1;
}

//TODO: un-inline?
_inline void FreeNode(ArrayedListNode_t* nodeArray, int index)
{
	nodeArray[index].inUse = 0;
}