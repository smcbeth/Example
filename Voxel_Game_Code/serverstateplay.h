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

class ISocket;

//-----------------------------------------------------------------------------
// Class ServerStatePlay
//-----------------------------------------------------------------------------
class ServerStatePlay: public BaseState, private SocketMessageHandler
{
public:
	ServerStatePlay( );
	~ServerStatePlay( );

	//-------------------------------------------------------------------------
	// Inherited from BaseState
	//-------------------------------------------------------------------------
	const char*	GetName( );

private:
	class ConnectedClient_;


	void		Initialize_( );
	void		Run_( );
	void		Shutdown_( );

	//-------------------------------------------------------------------------
	// Inherited from SocketMessageHandler
	//-------------------------------------------------------------------------
	void		OnSocketMessage( SocketMessage* socketMessage );


	ISocket*					socket_;			// our comm socket
	Vector< ConnectedClient_* >	connectedClients_;	// connected clients
};