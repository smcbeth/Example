//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\newport lib\basestate.h"
#include "public\isocket.h"

class ClientStateConnect: public BaseState, private SocketMessageHandler
{
public:
	ClientStateConnect( const IPAddress& serverIP );
	~ClientStateConnect( );

	//-------------------------------------------------------------------------
	// Inherited from BaseState
	//-------------------------------------------------------------------------
	const char*	GetName( );

	//-------------------------------------------------------------------------
	// Inherited from SocketMessageHandler
	//-------------------------------------------------------------------------
	void OnSocketMessage( SocketMessage* socketMessage );

private:
	enum CommStateType
	{
		COMMSTATETYPE_NONE,
		COMMSTATETYPE_CONNECT,			// connecting to server
		COMMSTATETYPE_VERIFYENTITIES	// verifiying entities
	};

	void Initialize_( );
	void Run_( );
	void Shutdown_( );

	void CommState_Connect_( SocketMessage* socketMessage );
	void CommState_VerifyEntities_( SocketMessage* socketMessage );
	void ConnectionError_( );

	IPAddress		serverIP_;
	ISocket*		socket_;
	CommStateType	commStateType_;
};