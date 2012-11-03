//
//  Platform.h
//  Empire
//
//  Created by Sean McBeth on 5/3/12.
//  Copyright (c) 2012 Sean McBeth. All rights reserved.
//

#ifndef _Empire_Platform_h
#define _Empire_Platform_h

#include <assert.h>
#include <stdlib.h>			// For 'malloc(~)' and 'free(~)' (don't use cstdlib because it just
                            //   places the functions in namespaces).
#include <new>				// For new, placement new and delete.

//-----------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------
#define CACHELINE_SIZE		64	// Typical size. TODO: Get this info from the architecture.

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------
typedef signed char			int8;
typedef signed short		int16;
typedef signed long			int32;
typedef signed long long	int64;

typedef unsigned char		uint8;
typedef unsigned short		uint16;
typedef unsigned long		uint32;
typedef unsigned long long	uint64;

typedef float				float32;
typedef double				float64;

//-----------------------------------------------------------------------------
// Debugging / Build Optimization
//-----------------------------------------------------------------------------
#define Assert assert
//#define AssertMsg(Expression, Message) (void)( (!!(Expression)) || (_wassert(_CRT_WIDE(#Message), _CRT_WIDE(__FILE__), __LINE__), 0) )

// These asserts will always fail
#define FAssert() assert (0)
//#define FAssertMsg(Message) AssertMsg(0, Message)

//-----------------------------------------------------------------------------
// Memory
//-----------------------------------------------------------------------------
// Warning- Never use any of these directly!
// NOTE: Use malloc/free only when there is a need to efficiently realloc.
#define Malloc	malloc
#define Calloc	calloc
#define Realloc	realloc
#define Free	free
#define MemSet	memset
#define MemMove	memmove

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------
#ifndef NULL
#define NULL 0
#endif

#define SAFE_FREE(pointer)	\
if( pointer )               \
{                           \
    free( pointer );        \
    pointer = NULL;         \
}

#define SAFE_DELETE(pointer)	\
if( pointer )                   \
{                               \
    delete pointer;             \
    pointer = NULL;             \
}

#define SAFE_RELEASE(pointer)	\
if( pointer )                   \
{                               \
    pointer->Release();         \
    pointer = NULL;             \
}

//-----------------------------------------------------------------------------
// Explicit Construction / Destruction
//-----------------------------------------------------------------------------
template<class T> inline void
Construct (T* pMemory)
{
	new (pMemory) T;
}

template<class T> inline void
CopyConstruct (T* pMemory, T const& src)
{
	new (pMemory) T(src);
}

template<class T> inline void
Destruct (T* pMemory)
{
	pMemory->~T ();
    
#ifdef _DEBUG
	// Set the memory to a recognisable string of 'D's.
	memset (pMemory, 0xDD, sizeof(T));
#endif
}

#endif
