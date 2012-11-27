//
//  BinaryHeap.h
//  Empire
//
//  Created by Sean McBeth on 10/30/12.
//  Copyright (c) 2012 Sean McBeth. All rights reserved.
//

#ifndef Empire_BinaryHeap_h
#define Empire_BinaryHeap_h

#include "Platform.h"
#include "Vector.h"

template< class T >
class BinaryHeap
{
public:
	
	typedef bool (T::*ComparisonOperator)( const T& ) const;
    
	// A lesser than operator or method will result in ascending ordering.
	explicit BinaryHeap( ComparisonOperator comparisonOperator );
    
    // Use of this constructor requires both < and > operators to be defined.
    explicit BinaryHeap( bool isAscending );
	
	// Use grow rules.
	void	 EnsureCapacity( uint32 capacity );
    
	// Do not use grow rules.
	void	 SetCapacity( uint32 capacity );
	uint32	 GetCapacity( ) const;
    
	// 0 == double in size.
	void	 SetGrowAmount( uint32 growAmount );
    
	uint32	 GetCount( ) const;
    
	T*		 GetFront( ) const;
    
	T*		 PopFront( );
    
	void	 RemoveFront( );
	void	 RemoveAll( );
	void	 RemoveAndDeleteAll( );
    
	void	 Insert( T* src );
	void	 Insert( T** src, uint32 count );
    
	void	 ResortExistingElement( T* src );
    
private:
    
    // Can't copy this unless we explicitly do it!
	BinaryHeap( const BinaryHeap& heap ) { FAssert(); }
    
	void InsertionCompareAndSwap_( T* src, uint32 index );
	
	ComparisonOperator comparisonOperator_;
    
	Vector<T*>         heap_;
};

// ------------------------------ Public Methods -------------------------------

template< class T > inline
BinaryHeap< T >::BinaryHeap( ComparisonOperator comparisonOperator )
: comparisonOperator_( comparisonOperator )
{
	
}

//------------------------------------------------------------------------------

template< class T > inline
BinaryHeap< T >::BinaryHeap( bool isAscending )
{
	if( isAscending )
        comparisonOperator_ = &T::operator<;
    else
        comparisonOperator_ = &T::operator>;
}

//------------------------------------------------------------------------------


template< class T > inline void
BinaryHeap< T >::EnsureCapacity( uint32 capacity )
{
	heap_.EnsureCapacity( capacity );
}

//------------------------------------------------------------------------------

template< class T > inline void
BinaryHeap< T >::SetCapacity( uint32 capacity )
{
	heap_.SetCapacity( capacity );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
BinaryHeap< T >::GetCapacity( ) const
{
	return heap_.GetCapacity();
}

//------------------------------------------------------------------------------

template< class T > inline void
BinaryHeap< T >::SetGrowAmount( uint32 growAmount )
{
	heap_.SetGrowAmount( growAmount );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
BinaryHeap< T >::GetCount( ) const
{
	return heap_.GetCount();
}

//------------------------------------------------------------------------------

template< class T > inline T*
BinaryHeap< T >::GetFront( ) const
{
	return heap_.GetFront();
}

//------------------------------------------------------------------------------

template< class T > inline T*
BinaryHeap< T >::PopFront( )
{
	T* retPtr = heap_.GetFront();
    
	RemoveFront();
    
	return retPtr;
}

//------------------------------------------------------------------------------

template< class T > void
BinaryHeap< T >::RemoveFront( )
{
	// Replace front element with end element.
	heap_[0] = heap_.GetBack();
	heap_.RemoveBack();
    
	uint32 index = 1;
    
	while( index * 2 <= heap_.GetCount() )
	{
		T* element = heap_[index - 1];
        
		index *= 2;
        
		bool firstChildResult = (heap_[index - 1]->*comparisonOperator_)( *element );
        
		// Compare element to one child case.
		if( index == heap_.GetCount() )
		{
			if( firstChildResult )
			{
				heap_[index / 2 - 1] = heap_[index - 1];
				heap_[index - 1] = element;
			}
		}
		// Compare element to its two children.
		else if( (firstChildResult || (heap_[index]->*comparisonOperator_)( *element ) ) )
		{
			// Swap element with lower of its two children.
			if( (heap_[index - 1]->*comparisonOperator_)( *heap_[index] ) )
			{
				heap_[index / 2 - 1] = heap_[index - 1];
				heap_[index - 1] = element;
			}
			else
			{
				heap_[index / 2 - 1] = heap_[index];
				heap_[index] = element;
				++index;
			}
		}
		else
		{
			break;
		}
	}
}

//------------------------------------------------------------------------------

template< class T > inline void
BinaryHeap< T >::RemoveAll( )
{
	heap_.RemoveAll();
}

//------------------------------------------------------------------------------

template< class T > inline void
BinaryHeap< T >::RemoveAndDeleteAll( )
{
	heap_.RemoveAndDeleteAll();
}

//------------------------------------------------------------------------------

template< class T > inline void
BinaryHeap< T >::Insert( T* src )
{
	heap_.InsertBack( src );
    
	uint32 index = heap_.GetCount();
    
	InsertionCompareAndSwap_(src, index);
}

//------------------------------------------------------------------------------

template< class T > inline void
BinaryHeap< T >::Insert( T** src, uint32 count )
{
	for(uint32 i = 0; i < count; ++i)
	{
		Insert( src[i] );
	}
}

//------------------------------------------------------------------------------

template< class T > void
BinaryHeap< T >::ResortExistingElement( T* src )
{
	for(uint32 index = 0; index < heap_.GetCount(); ++index)
	{
		if(src == heap_[index])
		{
            // Offset index by 1 to get proper multiplied values.
			index += 1;
            
			InsertionCompareAndSwap_(src, index);
		}
	}
}

// ------------------------------ Private Methods ------------------------------

template< class T > void
BinaryHeap< T >::InsertionCompareAndSwap_( T* src, uint32 index )
{
	while( index > 1 )
	{
		uint32 parentIndex = index / 2;
        
		if( (src->*comparisonOperator_)( *heap_[parentIndex - 1] ) )
		{
			// Swap
			heap_[index - 1] = heap_[parentIndex - 1];
			heap_[parentIndex - 1] = src;
		}
		else
		{
			break;
		}
        
		index = parentIndex;
	}
}

//------------------------------------------------------------------------------

#endif
