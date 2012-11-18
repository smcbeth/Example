//---------- Copyright � 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: This is a collection of quicksort variations on arrays
//
//-----------------------------------------------------------------------------
#pragma once

#include "platform.h"
#include "_math.h"

template<class T> FORCE_INLINE void
Sort (T data[], uint32 length, bool isAscending)
{
	if (length <= 1)
		return;

	Sort (data, 0, length - 1, isAscending);
}

template<class T> FORCE_INLINE void
Sort (T data[], uint32 left, uint32 right, bool isAscending)
{
	if (isAscending)
		QuicksortAscending (data, left, right);
	else
		QuicksortDescending (data, left, right);
}

template<class T> void
QuicksortAscending (T data[], uint32 left, uint32 right)
{
	if (left < right)
	{
		uint32 pivot = (left + right) / 2;
		pivot = PartitionAscending (data, left, right, pivot);
		if (pivot > 0)
			QuicksortAscending (data, left, pivot - 1);
        // Tail call on largest section for O(log n) space in worst case
		return QuicksortAscending (data, pivot + 1, right); 
	}
}

template<class T> void
QuicksortDescending (T data[], uint32 left, uint32 right)
{
	if (left > right)
	{
		uint32 pivot = (left + right) / 2;
		pivot = PartitionDescending (data, left, right, pivot);
		if (pivot > 0)
			QuicksortDescending (data, left, pivot - 1);
        // Tail call on largest section for O(log n) space in worst case
		return QuicksortDescending (data, pivot + 1, right); 
	}
}

template<class T> uint32
PartitionAscending (T data[], uint32 left, uint32 right, uint32 pivot)
{
	T pivotValue = data[pivot];
	Swap (data[pivot], data[right]); // move pivot to end
	uint32 storeIndex = left;
	for (uint32 i = left; i < right; ++i)
	{
		if (data[i] < pivotValue)
			Swap (data[i], data[storeIndex++]);
	}
	Swap (data[storeIndex], data[right]);
	return storeIndex;
}

template<class T> uint32
PartitionAscending (T* data[], uint32 left, uint32 right, uint32 pivot)
{
	T pivotValue = *(data[pivot]);
	Swap (data[pivot], data[right]); // move pivot to end
	uint32 storeIndex = left;
	for (uint32 i = left; i < right; ++i)
	{
		if (*(data[i]) < pivotValue)
			Swap (data[i], data[storeIndex++]);
	}
	Swap (data[storeIndex], data[right]);
	return storeIndex;
}

template<class T> uint32
PartitionDescending (T data[], uint32 left, uint32 right, uint32 pivot)
{
	T pivotValue = data[pivot];
	Swap (data[pivot], data[right]); // move pivot to end
	uint32 storeIndex = left;
	for (uint32 i = left; i < right; ++i)
	{
		if (data[i] > pivotValue)
			Swap (data[i], data[storeIndex++]);
	}
	Swap (data[storeIndex], data[right]);
	return storeIndex;
}

template<class T> uint32
PartitionDescending (T* data[], uint32 left, uint32 right, uint32 pivot)
{
	T pivotValue = *(data[pivot]);
	Swap (data[pivot], data[right]); // move pivot to end
	uint32 storeIndex = left;
	for (uint32 i = left; i < right; ++i)
	{
		if (*(data[i]) > pivotValue)
			Swap (data[i], data[storeIndex++]);
	}
	Swap (data[storeIndex], data[right]);
	return storeIndex;
}