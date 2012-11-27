//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"

class Client
{
public:
	Client( );
	~Client( );

	void Init( );
};

// Accessor
FORCE_INLINE Client* LocalClient( )
{
	extern Client* _g_LocalClientInstance;
	Assert( _g_LocalClientInstance );
	return _g_LocalClientInstance;
}