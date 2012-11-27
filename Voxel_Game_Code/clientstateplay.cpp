//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "clientstateplay.h"
#include "public\imaterialsystem.h"
#include "definitions.h"
#include "cliententitylist.h"

ClientStatePlay::ClientStatePlay( const IPAddress& serverIP, ISocket* socket )
{
	serverIP_	= serverIP;
	socket_		= socket;
	socket_->SetMessageHandler( this );
}

ClientStatePlay::~ClientStatePlay( )
{
	socket_->Release();
}

//-----------------------------------------------------------------------------
//
// Inherited from BaseState
//
//-----------------------------------------------------------------------------
const char*
ClientStatePlay::GetName( )
{
	return "ClientStatePlay";
}

//-----------------------------------------------------------------------------
//
// Inherited from SocketMessageHandler
//
//-----------------------------------------------------------------------------
void
ClientStatePlay::OnSocketMessage( SocketMessage* socketMessage )
{
	MessageType	messageType;
	socketMessage->Read( *(uint32*)&messageType );

	// initial data to set up our world representation?
	if( messageType == MESSAGETYPE_INITIALWORLDDATA )
	{
		uint32 length;
		uint32 depth;
		uint32 minSize;

		socketMessage->Read( length );
		socketMessage->Read( depth );
		socketMessage->Read( minSize );

		MaterialAPI()->InitOcTree( length, depth, minSize );
	}

	// new entity message?
	else if( messageType == MESSAGETYPE_NEWENTITY )
	{
		char	entityClassName[ MAX_ENTITYCLASSNAME ];
		uint32	entityUID;

		socketMessage->Read( entityClassName );
		socketMessage->Read( entityUID );

		// create and set UID
		IClientEntity* entity = GetClientEntityList()->Create( entityClassName );
		entity->SetUID( entityUID );

		// unserialize
		entity->UnserializeFromNetwork( socketMessage );

		// 'spawn' entity
		entity->Spawn();

		// add to list
		entities_.InsertBack( entity );
	}

	// entity update message?
	else if( messageType == MESSAGETYPE_ENTITYUPDATE )
	{
		uint32 entityUID;
		socketMessage->Read( entityUID );

		// find entity
		for( uint32 i = 0; i < entities_.GetCount(); ++ i )
		{
			if( entityUID == entities_[ i ]->GetUID() )
			{
				// unserialize
				entities_[ i ]->UnserializeFromNetwork( socketMessage );
				break;
			}
		}
	}

	// destroy entity message?
	else if( messageType == MESSAGETYPE_DESTROYENTITY )
	{
		uint32 entityUID;
		socketMessage->Read( entityUID );

		for( uint32 i = 0; i < entities_.GetCount(); ++i )
		{
			if( entityUID == entities_[ i ]->GetUID() )
			{
				// 'despawn'
				entities_[ i ]->Despawn();

				// destroy
				entities_.RemoveAndDelete( i );
				break;
			}
		}
	}
	// unrecognised message
	else
	{
		FAssert();
	}
}

void
ClientStatePlay::Initialize_( )
{
	isInitialized_ = true;
}

void
ClientStatePlay::Run_( )
{
	for( uint32 i = 0; i < entities_.GetCount(); ++i )
	{
		entities_[ i ]->Update();
	}

	static Vector< SocketMessage* > messages;
	for( uint32 i = 0; i < entities_.GetCount(); ++i )
	{
		if( entities_[ i ]->ShouldSerializeToNetwork() )
		{
			SocketMessage* socketMessage = new SocketMessage;
			socketMessage->Write( (uint32)MESSAGETYPE_ENTITYUPDATE );
			entities_[ i ]->SerializeToNetwork( socketMessage );
			messages.InsertBack( socketMessage );
		}
	}

	for( uint32 i = 0; i < messages.GetCount(); ++i )
	{
		messages[ i ]->SetReceiverIP( serverIP_ );
		socket_->Send( *messages[ i ] );
	}
	messages.PurgeAndDeleteAll();
}

void
ClientStatePlay::Shutdown_( )
{
	isShutdown_		= true;
	isInitialized_	= false;
}