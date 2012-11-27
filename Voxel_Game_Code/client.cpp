//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "public\iengine.h"
#include "client.h"
#include "clientstatemainmenu.h"

Client::Client( )
{
	;
}

Client::~Client( )
{
	;
}

void
Client::Init( )
{
	// run the initial state
	EngineAPI()->RunState( new ClientStateMainMenu );
}