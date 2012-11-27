//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "serverstateplay.h"
#include "public\nstdlib\stools.h"
#include "public\nstdlib\_string.h"
#include "public\iengine.h"
#include "public\inetworksystem.h"
#include "definitions.h"
#include "world.h"
#include "serverentitylist.h"

//-----------------------------------------------------------------------------
//
// Class ServerStatePlay::ConnectedClient_
//
//-----------------------------------------------------------------------------
class ServerStatePlay::ConnectedClient_
{
public:
	char		name_[ 64 ];		// character name
	IPAddress	ipAddress_;			// IP address

	bool		doSendKeyframe_;	// should send all entity data to client?
};

//-----------------------------------------------------------------------------
//
// Class ServerStatePlay
//
//-----------------------------------------------------------------------------
ServerStatePlay::ServerStatePlay( )
{
	;
}

ServerStatePlay::~ServerStatePlay( )
{
	;
}

//-----------------------------------------------------------------------------
//
// Inherited from BaseState
//
//-----------------------------------------------------------------------------
const char*
ServerStatePlay::GetName( )
{
	return "ServerStatePlay";
}

void
ServerStatePlay::Initialize_( )
{
	// create socket
	socket_ = NetworkAPI()->CreateSocket();
	socket_->Init( "Server Socket", 777 );
	socket_->SetMessageHandler( this );

	// load the world
	GetWorld()->Load();

	isInitialized_ = true;
}

void
ServerStatePlay::Run_( )
{
	GetWorld()->Update();

	static Vector< SocketMessage* > messages;
	for( uint32 i = 0; i < connectedClients_.GetCount(); ++i )
	{
		// compute messages related to pvs
		// (creation and destruction of clientside representations)

		// CTODO: make this shit not suck
		//       we need to know what entities the client knows about
		if( connectedClients_[ i ]->doSendKeyframe_ )
		{
			GetWorld()->ComputePVSMessages( messages );
			connectedClients_[ i ]->doSendKeyframe_ = false;
		}

		// compute entity deltas messages
		GetWorld()->ComputeDeltaMessages( messages );

		// send queued messages
		for( uint32 j = 0; j < messages.GetCount(); ++j )
		{
			// fill in receiver ip
			messages[ j ]->SetReceiverIP( connectedClients_[ i ]->ipAddress_ );

			// send
			socket_->Send( *messages[ j ] );
		}
		messages.RemoveAndDeleteAll();
	}
}

void
ServerStatePlay::Shutdown_( )
{
	// unload world
	GetWorld()->Unload();

	isShutdown_		= true;
	isInitialized_	= false;
}

//-----------------------------------------------------------------------------
//
// Inherited from SocketMessageHandler
//
//-----------------------------------------------------------------------------
void
ServerStatePlay::OnSocketMessage( SocketMessage* socketMessage )
{
	MessageType messageType;
	socketMessage->Read( *(uint32*)&messageType );

	// client connection request?
	if( messageType == MESSAGETYPE_CONNECTIONREQUST )
	{
		// add the client to 'connectedClients_'
		ConnectedClient_* connectedClient_	= new ConnectedClient_;
		connectedClient_->ipAddress_		= socketMessage->GetSenderIP();
		connectedClient_->doSendKeyframe_	= true;

		connectedClients_.InsertBack( connectedClient_ );

		// send acceptance response
		SocketMessage message;
		message.SetReceiverIP( connectedClient_->ipAddress_ );
		message.Write( (uint32)MESSAGETYPE_CONNECTIONREQUEST_ACCEPTED );
		socket_->Send( message );

		// verify entities
		// CTODO:
		message.Init();
		message.Write( (uint32)MESSAGETYPE_VERIFYENTITY_FINISHED );
		socket_->Send( message );
		/*
		for( uint32 i = 0; i < GetServerEntityList()->GetCount(); ++i )
		{
			message.Init();
			message.Write( (uint32)MESSAGETYPE_VERIFYENTITY );
			message.Write( GetServerEntityList()->GetName( i ) );
			socket_->Send( message );
		}
		*/

		// CTODO: fucking gay again
		// let the client switch into the play state here, i dont think i need this sleep!
		Sleep( 100 );

		// sent initial world data
		message.Init();
		message.Write( (uint32)MESSAGETYPE_INITIALWORLDDATA );
		message.Write( GetWorld()->GetLength() );
		message.Write( GetWorld()->GetDepth() );
		message.Write( GetWorld()->GetMinSize() );
		socket_->Send( message );
	}

	// client disconnected?
	else if( messageType == MESSAGETYPE_DISCONNECTION_NOTIFICATION )
	{
		FAssert(); // CTODO:
	}

	// client entity update?
	else if( messageType == MESSAGETYPE_ENTITYUPDATE )
	{
		GetWorld()->UnserializeEntityUpdate( socketMessage );
	}

	// unrecognised message
	else
	{
		FAssert();
	}
}