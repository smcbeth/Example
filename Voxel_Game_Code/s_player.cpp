//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "public\nstdlib\platform.h"
#include "public\newport lib\convar.h"
#include "public\newport lib\camera.h"
#include "public\newport lib\force.h"
#include "public\iengine.h"
#include "public\itimesystem.h"
#include "public\iphysicssystem.h"
#include "public\isocket.h"
#include "public\istreamfile.h"
#include "serverentitylist.h"
#include "s_basecharacter.h"

// CTODO:
//static ConVar s_noclip("s_noclip", false, "Allows the use of the no-clip camera." );
//static ConVar s_noclipacceleration( "s_noclipacceleration", 50.0f, "Acceleration of the no-clip camera." );
//static ConVar s_noclipdeacceleration( "s_noclipdeacceleration", 50.0f, "Deacceleration of the no-clip camera." );
//static ConVar s_noclipmaxvelocity( "s_noclipmaxvelocity", 50.0f, "Maximum veloctiy of the no-clip camera." );

//-----------------------------------------------------------------------------
// Class S_Player
//-----------------------------------------------------------------------------
class S_Player: public S_BaseCharacter
{
public:
	S_Player( );
	~S_Player( );

	const char* GetEntityClassName( );

	void Update( );

	void Think( );

	void SerializeToNetwork( SocketMessage* socketMessage );
	void UnserializeFromNetwork( SocketMessage* socketMessage );

	void SerializeToHDD( IStreamFile* file );
	void UnserializeFromHDD( IStreamFile* file );

	void Spawn( );
	void Despawn( );

public:
	class InputHandler_;

	InputHandler_*	inputHandler_;	// handles raw input
};

LINK_SERVERENTITY_TO_CLASS( "player", S_Player );

//-----------------------------------------------------------------------------
// Class S_Player::InputHandler_
//-----------------------------------------------------------------------------
class S_Player::InputHandler_
{
public:
	InputHandler_( S_Player* player );
	~InputHandler_( );

	void KeyPressEvent( uint32 virtualKey, bool isDown );
	void MouseMoveEvent( int32 deltaX, int32 deltaY );
	void MouseClickEvent( MouseButton button, bool isDown );

private:
	S_Player*	player_;
};

//-----------------------------------------------------------------------------
//
// Class S_Player Implementation
//
//-----------------------------------------------------------------------------
S_Player::S_Player( )
{
	// set jump velocity
	SetJumpVelocity( Vector3D( 0.0f, 0.0f, 15.0f ) );

	// load / set our collide
	SetCollide( PhysicsAPI()->LoadCollideFromMDL( "human" ) );

	// create force to control movement
	// CTODO: why the fuck do i create it manually??
	Force* force = new Force;
	force->SetMaxVelocity( 9.0f );
	SetMoveForce( force );

	inputHandler_ = new InputHandler_( this );
}

S_Player::~S_Player( )
{
	delete inputHandler_;
}

const char*
S_Player::GetEntityClassName( )
{
	return "player";
}

void
S_Player::Update( )
{
	S_BaseCharacter::Update();

	// CTODO: make this not suck
	//
	//        currently doing this because we dont get notified of when our
	//        collide moved
	doSerializeToNetwork_ = true;
}

void
S_Player::Think( )
{
	S_BaseCharacter::Think();
}

void
S_Player::SerializeToNetwork( SocketMessage* socketMessage )
{
	S_BaseCharacter::SerializeToNetwork( socketMessage );
}

void
S_Player::UnserializeFromNetwork( SocketMessage* socketMessage )
{
	S_BaseCharacter::UnserializeFromNetwork( socketMessage );

	// replay input
	uint32 inputCount;
	socketMessage->Read( inputCount );

	while( inputCount )
	{
		InputType inputType;
		socketMessage->Read( *(uint8*)&inputType );

		if( inputType == INPUTTYPE_KEYPRESS )
		{
			uint32	virtualKey;
			bool	isDown;
			socketMessage->Read( virtualKey );
			socketMessage->Read( isDown );

			inputHandler_->KeyPressEvent( virtualKey, isDown );
		}
		else if( inputType == INPUTTYPE_MOUSECLICK )
		{
			MouseButton	mouseButton;
			bool		isDown;
			socketMessage->Read( *(uint8*)&mouseButton );
			socketMessage->Read( isDown );

			inputHandler_->MouseClickEvent( mouseButton, isDown );
		}
		else
		{
			// bad input type
			FAssert();
		}

		--inputCount;
	}
}

void
S_Player::SerializeToHDD( IStreamFile* file )
{
	S_BaseCharacter::SerializeToHDD( file );
}

void
S_Player::UnserializeFromHDD( IStreamFile* file )
{
	S_BaseCharacter::UnserializeFromHDD( file );
}

void
S_Player::Spawn( )
{
	S_BaseCharacter::Spawn();
}

void
S_Player::Despawn( )
{
	S_BaseCharacter::Despawn();
}

//-----------------------------------------------------------------------------
//
// Class S_Player::InputHandler_ Implementation
//
//-----------------------------------------------------------------------------
S_Player::InputHandler_::InputHandler_( S_Player* player )
{
	player_ = player;
}

S_Player::InputHandler_::~InputHandler_( )
{
	SetCapture( false );
}

void
S_Player::InputHandler_::KeyPressEvent( uint32 virtualKey, bool isDown )
{
	switch( virtualKey )
	{
	case 'W':
		player_->SetMovingForward( isDown );
		break;

	case 'S':
		player_->SetMovingBackward( isDown );
		break;

	case 'A':
		player_->SetMovingLeft( isDown );
		break;

	case 'D':
		player_->SetMovingRight( isDown );
		break;

	case 32: // spacebar
		if( isDown )
		{
			player_->Jump();
		}
		break;
	}
}

void
S_Player::InputHandler_::MouseMoveEvent( int32 deltaX, int32 deltaY )
{
	;
}

void
S_Player::InputHandler_::MouseClickEvent( MouseButton button, bool isDown )
{
	/* TODO:
	if( button == MOUSE_LBUTTON && isDown )
	{
		player_->Attack();
	}
	*/
}