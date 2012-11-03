//
//  DMemory.h
//  Empire
//
//  Created by Sean McBeth on 5/3/12.
//  Copyright (c) 2012 Sean McBeth. All rights reserved.
//

#ifndef _Empire_DMemory_h
#define _Empire_DMemory_h

#include "Platform.h"

// NOTE: C-style memory allocation is used here for efficient reallocation.

template<class T>
class DMemory
{
public:
	DMemory( uint32 capacity = 0, uint32 growAmount = 0 );
	~DMemory( );
    
	T&		operator[]( uint32 i ) const;
	T&		GetElement( uint32 i ) const;
	T*		GetMemory( ) const;
    
	// Use grow rules.
	void	EnsureCapacity( uint32 capacity );
	// Do not use grow rules.
	void	SetCapacity( uint32 capacity );
	uint32	GetCapacity( ) const;
    
	// 0 == double in size
	void	SetGrowAmount( uint32 growAmount );
	uint32	GetGrowAmount( );
    
	// Free all memory.
	void	Purge( );
	bool	IsValidIndex( uint32 idx ) const;
    
private:
	// Can't copy this unless we explicitly do it!
	DMemory( const DMemory& other ) { Assert( 0 ); }
    
	// Increase the capacity by 'growAmount_' until we have enough.
	void Grow_( uint32 extraCapacity = 1 );
    
    
	T*		memory_;
	uint32	capacity_;
	uint32	growAmount_;
};

// ------------------------------ Public Methods -------------------------------

template< class T > inline
DMemory< T >::DMemory( uint32 capacity, uint32 growAmount )
{
	memory_		= NULL;
	capacity_	= capacity;
	growAmount_	= growAmount;
    
	if ( capacity_ )
	{
		// Create our initial memory.
		memory_ = (T*)Malloc( capacity_ * sizeof( T ) );
        Assert( memory_ );
	}
}

//------------------------------------------------------------------------------

template< class T > inline
DMemory< T >::~DMemory( )
{
	Purge();
}

//------------------------------------------------------------------------------

template< class T > inline T&
DMemory< T >::operator[]( uint32 i ) const
{
	return GetElement( i );
}

//------------------------------------------------------------------------------

template< class T > inline T&
DMemory< T >::GetElement( uint32 i ) const
{
	Assert( IsValidIndex( i ) );
	return memory_[ i ];
}

//------------------------------------------------------------------------------

template< class T > inline T*
DMemory< T >::GetMemory( ) const
{
	return memory_;
}

//-----------------------------------------------------------------------------
// Use grow rules
//-----------------------------------------------------------------------------
template< class T > inline void
DMemory< T >::EnsureCapacity( uint32 capacity )
{
	if ( capacity_ >= capacity )
	{
		return;
	}
    
	Grow_( capacity - capacity_ );
}

//-----------------------------------------------------------------------------
// Do not use grow rules
//-----------------------------------------------------------------------------
template< class T > inline void
DMemory< T >::SetCapacity( uint32 capacity )
{
	Assert( capacity );
    
	if( memory_ )
	{
		memory_ = (T*)Realloc( memory_, capacity * sizeof( T ) );
        Assert( memory_ );
	}
	else
	{
		memory_ = (T*)Malloc( capacity * sizeof( T ) );
        Assert( memory_ );
	}
    
	capacity_ = capacity;
}

//------------------------------------------------------------------------------

template< class T > inline uint32
DMemory< T >::GetCapacity( ) const
{
	return capacity_;
}

//-----------------------------------------------------------------------------
// 0 == double in size
//-----------------------------------------------------------------------------
template< class T > inline void
DMemory< T >::SetGrowAmount( uint32 growAmount )
{
	growAmount_ = growAmount;
}

//------------------------------------------------------------------------------

template< class T > inline uint32
DMemory< T >::GetGrowAmount( )
{
	return growAmount_;
}

//-----------------------------------------------------------------------------
// Free all memory
//-----------------------------------------------------------------------------
template< class T > inline void
DMemory< T >::Purge( )
{
	SAFE_FREE( memory_ );
    
	memory_		= NULL;
	capacity_	= 0;
}

//------------------------------------------------------------------------------

template< class T > inline bool
DMemory< T >::IsValidIndex( uint32 i ) const
{
	return( i < capacity_ );
}

// ------------------------------ Private Methods ------------------------------

template< class T > inline void
DMemory< T >::Grow_( uint32 extraCapacity )
{
	uint32 newCapacity			= capacity_;
	uint32 requestedCapacity	= capacity_ + extraCapacity;
    
	// Compute 'newCapacity'
	while ( newCapacity <  requestedCapacity )
	{
		if ( newCapacity != 0 )
		{
			if ( growAmount_ )
			{
				newCapacity += growAmount_;
			}
			else
			{
				newCapacity += newCapacity;
			}
		}
		else
		{
			// Compute an allocation which is at least as big as a cache line.
			newCapacity = ( CACHELINE_SIZE - 1 + sizeof( T ) ) / sizeof( T );
			Assert( newCapacity != 0 );
		}
	}
    
	// Resize our memory.
	if ( memory_ )
	{
		memory_ = (T*)Realloc( memory_, newCapacity * sizeof( T ) );
		Assert( memory_ );
	}
	else
	{
		memory_ = (T*)Malloc( newCapacity * sizeof( T ) );
		Assert( memory_ );
	}
    
	capacity_ = newCapacity;
}

//------------------------------------------------------------------------------

#endif
