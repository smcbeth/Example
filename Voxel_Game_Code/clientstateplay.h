//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\nstdlib\vector.h"
#include "public\newport lib\basestate.h"
#include "public\isocket.h"
#include "public\icliententity.h"

class ClientStatePlay: public BaseState, private SocketMessageHandler
{
public:
	ClientStatePlay( const IPAddress& serverIP, ISocket* socket );
	~ClientStatePlay( );

	//-------------------------------------------------------------------------
	// Inherited from BaseState
	//-------------------------------------------------------------------------
	const char*	GetName( );

	//-------------------------------------------------------------------------
	// Inherited from SocketMessageHandler
	//-------------------------------------------------------------------------
	void		OnSocketMessage( SocketMessage* socketMessage );

private:
	void		Initialize_( );
	void		Run_( );
	void		Shutdown_( );

	Vector< IClientEntity* >	entities_;
	IPAddress					serverIP_;
	ISocket*					socket_;
};