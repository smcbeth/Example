//
//  Vector.h
//  Empire
//
//  Created by Sean McBeth on 5/3/12.
//  Copyright (c) 2012 Sean McBeth. All rights reserved.
//

#ifndef _Empire_Vector_h
#define _Empire_Vector_h

//
// Memory Structure:
//  (Front)              (Back)
//  [0][1][2][3][4][5][6][7]
//  ---------->
//  Growth/Users/daedalic/Desktop/Louie.S03E13.HDTV.x264-LOL.mp4
//
// NOTE: Will construct new memory.
//
// Warning- Never maintain pointers to elements of this vector.
// Warning- The elements are not zeroed on construction, to zero the 
//          coordinates use 'Init(~)'.
//

#include "Platform.h"
#include "DMemory.h"
#include <string.h> // memmove

template< class T >
class Vector
{
public:
	enum
	{
		NOT_FOUND = -1
	};
    
	Vector( uint32 capacity = 0, uint32 growAmount = 0 );
	~Vector( );
    
	// TODO: Make a const T& operator[]( uint32 i ) const and a T& operator[]( uint32 i ).
    // The same is needed for GetElement.
	T&				operator[]( uint32 i ) const;
	Vector< T >&	operator=( const Vector< T >& vector );
	bool			operator==( const Vector& vector ) const;
	bool			operator!=( const Vector& vector ) const;
    
	// Use grow rules.
	void			EnsureCapacity( uint32 capacity );
	// Do not use grow rules.
	void			SetCapacity( uint32 capacity );
	uint32			GetCapacity( ) const;
	// 0 == double in size.
	void			SetGrowAmount( uint32 growAmount );
	uint32			GetCount( ) const;
    
	T&				GetElement( uint32 i ) const;
	T&				GetFront( ) const;
	T&				GetBack( ) const;
    
	uint32			InsertFront( const T& src );
	uint32			InsertFront( const T* src, uint32 count );
	uint32			InsertBack( const T& src );
	uint32			InsertBack( const T* src, uint32 count );
	uint32			InsertBack( const Vector<T>& src ); // TODO: add more overloads
	uint32			InsertBefore( uint32 idx, const T& src );
	uint32			InsertBefore( uint32 idx, const T* src, uint32 count );
	uint32			InsertAfter( uint32 idx, const T& src );
	uint32			InsertAfter( uint32 idx, const T* src, uint32 count );
    
	uint32			InsertEmptyFront( );
	uint32			InsertEmptyFront( uint32 count );
	uint32			InsertEmptyBack( );
	uint32			InsertEmptyBack( uint32 count );
	uint32			InsertEmptyBefore( uint32 idx );
	uint32			InsertEmptyBefore( uint32 idx, uint32 count );
	uint32			InsertEmptyAfter( uint32 idx );
	uint32			InsertEmptyAfter( uint32 idx, uint32 count );
    
	void			Move( uint32 from, uint32 to );
	void			MoveToFront( uint32 from );
	void			MoveToBack( uint32 from );
    
	void			Copy( const T src );
	void			Copy( const T* src, uint32 count );
    
	void			Remove( uint32 idx );
	// Doesn't preserve order.
	void			FastRemove( uint32 idx );
	void			RemoveMultiple( uint32 startIdx, uint32 count );
	void			RemoveFront( );
	void			RemoveBack( );
	void			RemoveAll( );
    
	void			RemoveAndDelete( uint32 idx );
	void			RemoveAndDeleteFront( );
	void			RemoveAndDeleteBack( );
	void			RemoveAndDeleteAll( );
    
	T				Pop( uint32 idx );
	T				PopFront( );
	T				PopBack( );
    
	// Free all memory.
	void			Purge( );
	void			PurgeAndDeleteAll( );
    
	bool			IsEqual( const Vector& vector ) const;
    
	// Returns 'NOT_FOUND' on fail.
	int32			FindFirst( const T& src ) const;
	// Returns true on success.
	bool			FindFirstAndRemove( const T& src );
	int32			Find( const T& src, uint32 idx ) const;
	void			FindAll( Vector< uint32 >* idxVec, const T& src ) const;
    
	T*				GetMemory( ) const;
	// Use to tell the vector how many elements we have after a direct memory modify.
	void			OverrideCount( uint32 count );
    
	bool			IsValidIndex( uint32 idx ) const;
    
private:
	// Can't copy this unless we explicitly do it!
	Vector( const Vector& vector ) { FAssert(); }
    
	// NOTE: Make sure you change 'count_' AFTER you call these methods.
	void		ShiftForward_( uint32 idx );
	void		ShiftForward_( uint32 idx, uint32 amount );
	void		ShiftBackward_( uint32 idx );
	void		ShiftBackward_( uint32 idx, uint32 amount );
    
    
	DMemory< T >	dMemory_;
	uint32			count_;
};

// ------------------------------ Public Methods -------------------------------

template< class T > inline
Vector< T >::Vector( uint32 capacity, uint32 growAmount )
: dMemory_( capacity, growAmount )
{
	count_ = 0;
}

//------------------------------------------------------------------------------

template< class T > inline
Vector< T >::~Vector( )
{
	Purge();
}

//------------------------------------------------------------------------------

template< class T > inline T&
Vector< T >::operator[ ]( uint32 i ) const
{
	return dMemory_[ i ];
}

//------------------------------------------------------------------------------

template< class T > inline
Vector< T >& Vector< T >::operator=( Vector< T > const& vector )
{
	Copy( vector.GetMemory(), vector.GetCount() );
	return *this;
}

//------------------------------------------------------------------------------

template< class T > inline bool
Vector< T >::operator==( Vector const& vector )  const
{
	return IsEqual( vector );
}

//------------------------------------------------------------------------------

template< class T > inline bool
Vector< T >::operator!=( Vector const& vector )  const
{
	return !IsEqual( vector );
}

//-----------------------------------------------------------------------------
// Use grow rules.
//-----------------------------------------------------------------------------
template< class T > inline void
Vector< T >::EnsureCapacity( uint32 capacity )
{
	dMemory_.EnsureCapacity( capacity );
}

//-----------------------------------------------------------------------------
// Do not use grow rules.
//-----------------------------------------------------------------------------
template< class T > inline void
Vector< T >::SetCapacity( uint32 capacity )
{
	dMemory_.SetCapacity( capacity );
}

//-----------------------------------------------------------------------------
// 0 == double in size.
//-----------------------------------------------------------------------------
template< class T > inline void
Vector< T >::SetGrowAmount( uint32 growAmount )
{
	dMemory_.SetGrowAmount( growAmount );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::GetCount( ) const
{
	return count_;
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::GetCapacity( ) const
{
	return dMemory_.GetCapacity();
}

//------------------------------------------------------------------------------

template< class T > inline T&
Vector< T >::GetElement( uint32 i ) const
{
	return dMemory_.GetElement( i );
}

//------------------------------------------------------------------------------

template< class T > inline T&
Vector< T >::GetFront( ) const
{
	Assert( count_ );
	return dMemory_[ 0 ];
}

//------------------------------------------------------------------------------

template< class T > inline T&
Vector< T >::GetBack( ) const
{
	Assert( count_ );
	return dMemory_[ count_ - 1 ];
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertFront( const T& src )
{
	return InsertBefore( 0, src );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertFront( const T* src, uint32 count )
{
	return InsertBefore( 0, src, count );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertBack( const T& src )
{
	return InsertBefore( count_, src );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertBack( const T* src, uint32 count )
{
	return InsertBefore( count_, src, count );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertBack( const Vector<T>& src )
{
	return InsertBack( src.GetMemory(), src.GetCount() );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertBefore( uint32 idx, const T& src )
{
	// Can insert at the end?
	Assert( idx == count_ || IsValidIndex( idx ) );
    
	EnsureCapacity( count_ + 1 );
    
	if( idx != count_ )
	{
		ShiftBackward_( idx );
	}
    
	// Copy construct 'new' memory.
	CopyConstruct< T >( &GetElement( idx ), src );
    
	++count_;
	return idx;
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertBefore( uint32 idx, const T* src, uint32 count )
{
	// Can insert at the end?
	Assert( idx == count_ || IsValidIndex( idx ) );
    
	EnsureCapacity( count_ + count );
    
	if( idx != count_ )
	{
		ShiftBackward_( idx, count );
	}
    
	// Copy construct 'new' memory.
	for( uint32 i = 0; i < count; ++i )
	{
		CopyConstruct< T >( &GetElement( idx + i ) , src[ i ] );
	}
    
	count_ += count;
	return idx;
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertAfter( uint32 idx, const T& src )
{
	return InsertBefore( idx + 1, src );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertAfter( uint32 idx, const T* src, uint32 count )
{
	return InsertBefore( idx + 1, src, count );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertEmptyFront( )
{
	return InsertEmptyBefore( 0 );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertEmptyFront( uint32 count )
{
	return InsertEmptyBefore( 0, count );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertEmptyBack( )
{
	return InsertEmptyBefore( count_ );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertEmptyBack( uint32 count )
{
	return InsertEmptyBefore( count_, count );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertEmptyBefore( uint32 idx )
{
	// Can insert at the end?
	Assert( idx == count_ || IsValidIndex( idx ) );
    
	EnsureCapacity( count_ + 1 );
    
	if( idx != count_ )
	{
		ShiftBackward_( idx );
	}
    
	// Construct 'new' memory.
	Construct< T >(  &GetElement( idx ) );
    
	++count_;
	return idx;
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertEmptyBefore( uint32 idx, uint32 count )
{
	// Can insert at the end?
	Assert( idx == count_ || IsValidIndex( idx ) );
    
	EnsureCapacity( count_ + count );
    
	if( idx != count_ )
	{
		ShiftBackward_( idx, count );
	}
    
	// Construct 'new' memory.
	for( uint32 i = 0; i < count; ++i )
	{
		Construct< T >(  &GetElement( idx + i ) );
	}
    
	count_ += count;
	return idx;
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertEmptyAfter( uint32 idx )
{
	return InsertEmptyBefore( idx + 1 );
}

//------------------------------------------------------------------------------

template< class T > inline uint32
Vector< T >::InsertEmptyAfter( uint32 idx, uint32 count )
{
	return InsertEmptyBefore( idx + 1, count );
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::Move( uint32 from, uint32 to )
{
	InsertBefore( to, GetElement( from ) );
	Remove( from );
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::MoveToFront( uint32 from )
{
	InsertFront( GetElement( from ) );
	Remove( from );
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::MoveToBack( uint32 from )
{
	InsertBack( GetElement( from ) );
	Remove( from );
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::Copy( const T src )
{
	RemoveAll();
	InsertBack( src );
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::Copy( const T* src, uint32 count )
{
	RemoveAll();
	EnsureCapacity( count );
	for( uint32 i = 0; i <  count; i++ )
	{
		InsertBack( src[ i ] );
	}
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::Remove( uint32 idx )
{
	Destruct( &GetElement( idx ) );
    
	if( idx + 1 < GetCount() )
	{
		ShiftForward_( idx + 1 );
	}
    
	--count_;
}

//-----------------------------------------------------------------------------
// Doesn't preserve order.
//-----------------------------------------------------------------------------
template< class T > inline void
Vector< T >::FastRemove( uint32 idx )
{
	Assert( IsValidIndex( idx ) );
	Destruct( &GetElement( idx ) );
    
	if( idx + 1 != count_ )
	{
		MemMove( &GetElement( idx ), &GetElement( count_ - 1 ), sizeof( T ) );
	}
    
	--count_;
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::RemoveMultiple( uint32 idx, uint32 count )
{
	Assert( IsValidIndex( idx ) );
	Assert( idx + count <= count_ );
    
	for( uint32 i = idx; i < idx + count; ++i )
	{
		Destruct( &GetElement( i ) );
	}
    
	ShiftForward_( idx + count, count );
	count_ -= count;
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::RemoveFront( )
{
	Destruct( &GetElement( 0 ) );
	ShiftForward_( 1 );
	--count_;
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::RemoveBack( )
{
	Destruct( &GetElement( count_ - 1 ) );
	--count_;
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::RemoveAll( )
{
	for( uint32 i = 0; i < count_; ++i )
	{
		Destruct( &GetElement( i ) );
	}
    
	count_ = 0;
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::RemoveAndDelete( uint32 idx )
{
	SAFE_DELETE( GetElement( idx ) );
	Remove( idx );
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::RemoveAndDeleteFront( )
{
	SAFE_DELETE( GetFront() );
	RemoveFront();
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::RemoveAndDeleteBack( )
{
	SAFE_DELETE( GetBack() );
	RemoveBack();
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::RemoveAndDeleteAll( )
{
	while( count_ )
	{
		SAFE_DELETE( GetBack() );
		RemoveBack();
	}
}

//------------------------------------------------------------------------------

template< class T > inline T
Vector< T >::Pop( uint32 idx )
{
	T buffer = GetElement( idx );
    
	Remove( idx );
	return buffer;
}

//------------------------------------------------------------------------------

template< class T > inline T
Vector< T >::PopFront( )
{
	T buffer = GetFront();
    
	RemoveFront();
	return buffer;
}

//------------------------------------------------------------------------------

template< class T > inline T
Vector< T >::PopBack( )
{
	T buffer = GetBack();
    
	RemoveBack();
	return buffer;
}

//-----------------------------------------------------------------------------
// Free all memory.
//-----------------------------------------------------------------------------
template< class T > inline void
Vector< T >::Purge( )
{
	RemoveAll();
	dMemory_.Purge();
}

//------------------------------------------------------------------------------

template< class T > void
Vector< T >::PurgeAndDeleteAll ( )
{
	for( uint32 i = 0; i < GetCount(); i++ )
	{
		SAFE_DELETE( GetElement( i ) );
	}
    
	Purge();
}

//------------------------------------------------------------------------------

template< class T > bool
Vector< T >::IsEqual( const Vector& vector ) const
{
	for( uint32 i = 0; i < vector.GetCount(); ++i )
	{
		if( GetElement( i ) != vector[ i ] )
		{
			return false;
		}
	}
    
	return true;
}

//-----------------------------------------------------------------------------
// Returns 'NOT_FOUND' on fail.
//-----------------------------------------------------------------------------
template< class T > int32
Vector< T >::FindFirst( const T& src ) const
{
	for( uint32 i = 0; i <  GetCount(); ++i )
	{
		if( GetElement( i ) == src )
		{
			return i;
		}
	}
    
	return NOT_FOUND;
}

//-----------------------------------------------------------------------------
// Returns true on success.
//-----------------------------------------------------------------------------
template< class T > bool
Vector< T >::FindFirstAndRemove( const T& src )
{
	int32 idx = FindFirst( src );
	if( idx != -1 )
	{
		Remove( idx );
		return true;
	}
	
	return false;
}

//------------------------------------------------------------------------------

template< class T > int32
Vector< T >::Find( const T& src, uint32 idx ) const
{
	for( uint32 i = 0; i <  GetCount(); ++i )
	{
		if( GetElement( i ) == src )
		{
			if( !idx )
			{
				return i;
			}
            
			--idx;
		}
	}
    
	return -1;
}

//------------------------------------------------------------------------------

template< class T > void
Vector< T >::FindAll( Vector< uint32 >* idxVec, T const& src ) const
{
	idxVec->RemoveAll();
    
	for( uint32 i = 0; i <  count_; ++i )
	{
		if( GetElement( i ) == src )
		{
			idxVec->InsertBack( i );
		}
	}
}

//------------------------------------------------------------------------------

template< class T > inline T*
Vector< T >::GetMemory( ) const
{
	return dMemory_.GetMemory();
}

//-----------------------------------------------------------------------------
// Use to tell the vector how many elements we have after a direct memory modify.
//-----------------------------------------------------------------------------
template< class T > inline void
Vector< T >::OverrideCount( uint32 newCount )
{
	count_ = newCount;
}

//------------------------------------------------------------------------------

template< class T > inline bool
Vector< T >::IsValidIndex( uint32 i ) const
{
	return dMemory_.IsValidIndex( i ) && i < GetCount();
}

// ------------------------------ Private Methods ------------------------------
// NOTE: Make sure you change 'count_' AFTER you call these methods.
//-----------------------------------------------------------------------------
template< class T > inline void
Vector< T >::ShiftForward_( uint32 idx )
{
	Assert( IsValidIndex( idx ) && idx != 0 );
    
	uint32 moveCount = GetCount() - idx;
	memmove( &GetElement( idx - 1 ), &GetElement( idx ), moveCount * sizeof( T ) );
    
#ifdef _DEBUG
	// Initialize the 'new' memory to a debug value.
	MemSet( &GetElement( GetCount() - 1 ), 0xDD, sizeof( T ) );
#endif
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::ShiftForward_( uint32 idx, uint32 amount )
{
	Assert( IsValidIndex( idx ) );
	Assert( (int32)idx - (int32)amount > 0 );
	Assert( amount > 1 );
    
	uint32 moveCount = GetCount() - idx;
	memmove( &GetElement( idx - amount ), &GetElement( idx ), moveCount * sizeof( T ) );
    
#ifdef _DEBUG
	// Initialize the 'new' memory to a debug value.
	MemSet( &GetElement( GetCount() - amount ), 0xDD, amount * sizeof( T ) );
#endif
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::ShiftBackward_( uint32 idx )
{
	Assert( IsValidIndex( idx ) );
    
	uint32 moveCount = GetCount() - idx;
	memmove( &GetElement( idx + 1 ), &GetElement( idx ), moveCount * sizeof( T ) );
    
#ifdef _DEBUG
	// Initialize the 'new' memory to a debug value.
	MemSet( &GetElement( idx ), 0xDD, sizeof( T ) );
#endif
}

//------------------------------------------------------------------------------

template< class T > inline void
Vector< T >::ShiftBackward_( uint32 idx, uint32 amount )
{
	Assert( IsValidIndex( idx ) );
	Assert( amount > 1 );
    
	uint32 moveCount = GetCount() - idx;
	memmove( &GetElement( idx + amount ), &GetElement( idx ), moveCount * sizeof( T ) );
    
#ifdef _DEBUG
	// Initialize the 'new' memory to a debug value.
	MemSet( &GetElement( idx ), 0xDD, amount * sizeof( T ) );
#endif
}

//------------------------------------------------------------------------------

#endif
