//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"

class IServerEntity;

// Use to teach the entity list about an entitiy
#define LINK_SERVERENTITY_TO_CLASS( name, className )					\
	IServerEntity* CreateNew##className( )								\
	{																	\
		return new className;											\
	}																	\
																		\
	class className##Foo												\
	{																	\
		public:															\
		className##Foo( )												\
		{																\
			GetServerEntityList()->Learn( name, CreateNew##className );	\
		}																\
	};																	\
																		\
	static className##Foo _g_##className##Foo;

class IServerEntityList
{
public:
	~IServerEntityList( ) { ; }

	// Do not call this directly, use 'LINK_CLIENTENTITY_TO_CLASS'
	virtual void			Learn( const char* name, IServerEntity* (*createFn)() ) = 0;
	// Return true if we know about that entity
	virtual bool			Verify( const char* name ) = 0;
	virtual IServerEntity*	Create( const char* name ) = 0;

	virtual const char*		GetName( uint32 idx ) = 0;
	virtual uint32			GetCount( ) = 0;
};

// Accessor
extern IServerEntityList* GetServerEntityList( );