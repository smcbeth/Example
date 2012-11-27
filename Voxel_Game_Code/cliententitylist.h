//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"

class IClientEntity;

// Use to teach the entity list about an entitiy
#define LINK_CLIENTENTITY_TO_CLASS( name, className )					\
	IClientEntity* CreateNew##className( )								\
	{																	\
		return new className;											\
	}																	\
																		\
	class className##Foo												\
	{																	\
		public:															\
		className##Foo( )												\
		{																\
			GetClientEntityList()->Learn( name, CreateNew##className );	\
		}																\
	};																	\
																		\
	static className##Foo _g_##className##Foo;

class IClientEntityList
{
public:
	~IClientEntityList( ) { ; }

	// Do not call this directly, use 'LINK_CLIENTENTITY_TO_CLASS'
	virtual void			Learn( const char* name, IClientEntity* (*createFn)() ) = 0;
	// Return true if we know about that entity
	virtual bool			Verify( const char* name ) = 0;
	virtual  IClientEntity*	Create( const char* name ) = 0;
};

// Accessor
extern IClientEntityList* GetClientEntityList( );