//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\newport lib\basestate.h"

class ClientStateMainMenu: public BaseState
{
public:
	ClientStateMainMenu( );
	~ClientStateMainMenu( );

	void		DisplayConnectFailureMessage( );

	//-------------------------------------------------------------------------
	// Inherited from BaseState
	//-------------------------------------------------------------------------
	const char*	GetName( );

private:
	void		Initialize_( );
	void		Run_( );
	void		Shutdown_( );
};