//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "server.h"
#include "public\iengine.h"
#include "serverstateplay.h"

Server::Server( )
{
	;
}

Server::~Server( )
{
	;
}

void
Server::Init( )
{
	// initial state
	EngineAPI()->RunState( new ServerStatePlay );
}