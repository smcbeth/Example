//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "clientstateconnect.h"
#include "public\nstdlib\stools.h"
#include "public\iengine.h"
#include "public\inetworksystem.h"
#include "definitions.h"
#include "cliententitylist.h"
#include "clientstatemainmenu.h"
#include "clientstateplay.h"

ClientStateConnect::ClientStateConnect( const IPAddress& serverIP )
{
	serverIP_ = serverIP;
}

ClientStateConnect::~ClientStateConnect( )
{
	;
}
 
//-----------------------------------------------------------------------------
//
// Inherited from BaseState
//
//-----------------------------------------------------------------------------
const char*
ClientStateConnect::GetName( )
{
	return "ClientStateConnect";
}

//-----------------------------------------------------------------------------
//
// Inherited from SocketMessageHandler
//
//-----------------------------------------------------------------------------
void
ClientStateConnect::OnSocketMessage( SocketMessage* socketMessage )
{
	switch( commStateType_ )
	{
	case COMMSTATETYPE_CONNECT:
		CommState_Connect_( socketMessage );
		break;

	case COMMSTATETYPE_VERIFYENTITIES:
		CommState_VerifyEntities_( socketMessage );
		break;
	}
}

void
ClientStateConnect::Initialize_( )
{
	// hardcode server ip // CTODO:
	serverIP_.Init( 127, 0, 0, 1, 777 );

	// create socket
	socket_ = NetworkAPI()->CreateSocket();
	socket_->Init( "Client Socket", 778 );
	socket_->SetMessageHandler( this );

	// set initial comm state
	commStateType_ = COMMSTATETYPE_CONNECT;

	// send connection request
	SocketMessage socketMessage;
	socketMessage.SetReceiverIP( serverIP_ );
	socketMessage.Write( (uint32)MESSAGETYPE_CONNECTIONREQUST );
	socket_->Send( socketMessage );

	isInitialized_ = true;
}

void
ClientStateConnect::Run_( )
{
	;
}

void
ClientStateConnect::Shutdown_( )
{
	isShutdown_		= true;
	isInitialized_	= false;
}

void
ClientStateConnect::CommState_Connect_( SocketMessage* socketMessage )
{
	MessageType messageType;
	socketMessage->Read( *(uint32*)&messageType );

	switch( messageType )
	{
	// connection accepted?
	case MESSAGETYPE_CONNECTIONREQUEST_ACCEPTED:
		commStateType_ = COMMSTATETYPE_VERIFYENTITIES;
		break;

	// connection rejected?
	case MESSAGETYPE_CONNECTIONREQUEST_DENIED:
		ConnectionError_();
		break;

	// bad message
	default:
		ConnectionError_();
	}
}

void
ClientStateConnect::CommState_VerifyEntities_( SocketMessage* socketMessage )
{
	MessageType messageType;
	socketMessage->Read( *(uint32*)&messageType );

	if( messageType == MESSAGETYPE_VERIFYENTITY )
	{
		char entityName[ MAX_ENTITYNAME ];
		socketMessage->Read( entityName );

		// entity failed verification?
		if( !GetClientEntityList()->Verify( entityName ) )
		{
			// tell the server
			SocketMessage message;
			message.SetReceiverIP( serverIP_ );
			message.Write( (uint32)MESSAGETYPE_VERIFYENTITY_FAILURE );
			socket_->Send( message );

			ConnectionError_();
		}
	}

	else if( messageType == MESSAGETYPE_VERIFYENTITY_FINISHED )
	{
		// start the 'play' state
		EngineAPI()->RunState( new ClientStatePlay( serverIP_, socket_ ) );
		Shutdown();
	}
}

void
ClientStateConnect::ConnectionError_( )
{
	// go back to main menu & display error message
	ClientStateMainMenu* clientStateMainMenu = new ClientStateMainMenu;
	clientStateMainMenu->DisplayConnectFailureMessage();
	EngineAPI()->RunState( clientStateMainMenu );

	// stop listening to socket messages
	commStateType_ = COMMSTATETYPE_NONE;

	Shutdown();
}