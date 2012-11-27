//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "public\nstdlib\platform.h"
#include "public\newport lib\camera.h"
#include "public\isocket.h"
#include "public\ifilesystem.h"
#include "public\itaggedfile.h"
#include "public\iinputsystem.h"
#include "public\iinputeventhandler.h"
#include "c_baseanimated.h"
#include "cliententitylist.h"

struct InputEvent
{
	InputType type;

	union
	{
		uint32		virtualKey;
		MouseButton	mouseButton;
	};

	bool isDown;
};

//-----------------------------------------------------------------------------
// Class C_Player
//-----------------------------------------------------------------------------
class C_Player: public C_BaseAnimated
{
public:
	C_Player( );
	~C_Player( );

	void			AddKeyPressEvent( uint32 virtualKey, bool isDown );
	void			AddMouseClickEvent( MouseButton mouseButton, bool isDown );

	void			SetFacing( float32 yaw, float32 pitch, float32 roll );
	void			YawFacing( float32 amount );
	void			PitchFacing( float32 amount );
	void			RollFacing( float32 amount );
	void			GetFacing( Vector3D& out );
	float32			GetFacingYaw( );
	float32			GetFacingPitch( );
	float32			GetFacingRoll( );

	//-------------------------------------------------------------------------
	// Inherited from C_BaseEntity
	//-------------------------------------------------------------------------
	const char*	GetEntityClassName( );

	void		Update( );

	void		SerializeToNetwork( SocketMessage* socketMessage );
	void		UnserializeFromNetwork( SocketMessage* socketMessage );

	void		Spawn( );
	void		Despawn( );

private:
	class InputHandler_;

	void UpdateCamera_( );

	float32					facingYaw_;
	float32					facingPitch_;
	float32					facingRoll_;

	InputHandler_*			inputHandler_;	// handles raw input
	Camera					camera_;		// our view, the scene is rendered from this perspective

	Vector< InputEvent* >	inputEvents_;
};

LINK_CLIENTENTITY_TO_CLASS( "player", C_Player );

//-----------------------------------------------------------------------------
// Class C_Player::InputHandler_
//-----------------------------------------------------------------------------
class C_Player::InputHandler_: public IInputEventHandler
{
public:
	InputHandler_( C_Player* player );
	~InputHandler_( );

	void SetCapture( bool capture );
	bool GetCapture( );

	void KeyPressEvent( uint32 virtualKey, bool isDown );
	void MouseMoveEvent( int32 deltaX, int32 deltaY );
	void MouseClickEvent( MouseButton button, bool isDown );

private:
	C_Player*	player_;
	bool		doCapture_;
};

//-----------------------------------------------------------------------------
//
// Class C_Player Implementation
//
//-----------------------------------------------------------------------------
C_Player::C_Player( )
{
	// init camera (use config file to determine settings)
	// CTODO: use convars
	ITaggedFile* configFile = FileAPI()->GetTaggedFile( CONFIGFILE_NAME );
	configFile->SetTags( "graphics", "fov" );
	float32 fov = configFile->GetFloat32Data();
	configFile->SetTags( "graphics", "neardistance" );
	float32 nearDistance = configFile->GetFloat32Data();
	configFile->SetTags( "graphics", "fardistance" );
	float32 farDistance = configFile->GetFloat32Data();
	configFile->Release();

	camera_.InitPerspective( fov, nearDistance, farDistance );

	facingYaw_		= 0.0f;
	facingPitch_	= 0.0f;
	facingRoll_		= 0.0f;

	inputHandler_ = new InputHandler_( this );
}

C_Player::~C_Player( )
{
	delete inputHandler_;
}

void
C_Player::AddKeyPressEvent( uint32 virtualKey, bool isDown )
{
	InputEvent* inputEvent = new InputEvent;
	inputEvent->type = INPUTTYPE_KEYPRESS;
	inputEvent->virtualKey = virtualKey;
	inputEvent->isDown = isDown;

	inputEvents_.InsertBack( inputEvent );
	doSerializeToNetwork_ = true;
}

void
C_Player::AddMouseClickEvent( MouseButton mouseButton, bool isDown )
{
	InputEvent* inputEvent = new InputEvent;
	inputEvent->type = INPUTTYPE_MOUSECLICK;
	inputEvent->mouseButton = mouseButton;
	inputEvent->isDown = isDown;

	inputEvents_.InsertBack( inputEvent );
	doSerializeToNetwork_ = true;
}

void
C_Player::SetFacing( float32 yaw, float32 pitch, float32 roll )
{
	facingYaw_		= yaw;
	facingPitch_	= pitch;
	facingRoll_		= roll;
}

void
C_Player::YawFacing( float32 amount )
{
	facingYaw_ += amount;
}

void
C_Player::PitchFacing( float32 amount )
{
	facingPitch_ += amount;
}

void
C_Player::RollFacing( float32 amount )
{
	facingRoll_ += amount;
}

void
C_Player::GetFacing( Vector3D& out )
{
	Assert( 0 );
}

float32
C_Player::GetFacingYaw( )
{
	return facingYaw_;
}

float32
C_Player::GetFacingPitch( )
{
	return facingPitch_;
}

float32
C_Player::GetFacingRoll( )
{
	return facingRoll_;
}

const char*
C_Player::GetEntityClassName( )
{
	return "player";
}

void
C_Player::Update( )
{
	C_BaseEntity::Update();

	// CTODO: this is here because 
	doSerializeToNetwork_ = true;

	UpdateCamera_();
}

void
C_Player::SerializeToNetwork( SocketMessage* socketMessage )
{
	C_BaseEntity::SerializeToNetwork( socketMessage );

	// serialize facing
	// this is because we don't send mouse movement, instead, we just send
	// the direction the player is facing
	socketMessage->Write( facingYaw_ );
	socketMessage->Write( facingPitch_ );
	socketMessage->Write( facingRoll_ );

	// serialize input - don't send over mouse movement
	socketMessage->Write( inputEvents_.GetCount() );
	for( uint32 i = 0; i < inputEvents_.GetCount(); ++i )
	{
		socketMessage->Write( (uint8)inputEvents_[ i ]->type );
		if( inputEvents_[ i ]->type == INPUTTYPE_KEYPRESS )
		{
			socketMessage->Write( inputEvents_[ i ]->virtualKey );
		}
		else if( inputEvents_[ i ]->type == INPUTTYPE_MOUSECLICK )
		{
			socketMessage->Write( (uint8)inputEvents_[ i ]->mouseButton );
		}
		socketMessage->Write( inputEvents_[ i ]->isDown );
	}
	inputEvents_.RemoveAndDeleteAll();
}

void
C_Player::UnserializeFromNetwork( SocketMessage* socketMessage )
{
	C_BaseEntity::UnserializeFromNetwork( socketMessage );
}

void
C_Player::Spawn( )
{
	C_BaseEntity::Spawn();

	// CTODO:
	//SetMesh( MaterialAPI()->GetOrLoadMesh( "human" ) );

	MaterialAPI()->SetCamera( &camera_ );
	inputHandler_->SetCapture( true );
}

void
C_Player::Despawn( )
{
	C_BaseEntity::Despawn();

	MaterialAPI()->SetCamera( NULL );
}

void
C_Player::UpdateCamera_( )
{
	// update the camera to our 'facing'
	camera_.SetRotation( GetFacingYaw(), GetFacingPitch(), GetFacingRoll() );

	// compute new camera origin using transform matrix
	Vector3D origin;
	origin.Init();
	origin *= GetTransform();
	// HACK: get this height from somewhere else
	origin.z += 3.0f;

	camera_.SetOrigin( origin );
}

//-----------------------------------------------------------------------------
//
// Class C_Player::InputHandler_ Implementation
//
//-----------------------------------------------------------------------------
C_Player::InputHandler_::InputHandler_( C_Player* player )
{
	player_		= player;
	doCapture_	= false;
}

C_Player::InputHandler_::~InputHandler_( )
{
	SetCapture( false );
}

void
C_Player::InputHandler_::SetCapture( bool capture )
{
	if( doCapture_ == capture )
	{
		return;
	}

	if( capture )
	{
		InputAPI()->SetCursorCapture( true );
		InputAPI()->AddInputEventHandler( this );
	}
	else
	{
		InputAPI()->SetCursorCapture( true );
		InputAPI()->RemoveInputEventHandler( this );
	}

	doCapture_ = capture;
}

bool
C_Player::InputHandler_::GetCapture( )
{
	return doCapture_;
}

void
C_Player::InputHandler_::KeyPressEvent( uint32 virtualKey, bool isDown )
{
	player_->AddKeyPressEvent( virtualKey, isDown );
}

void
C_Player::InputHandler_::MouseMoveEvent( int32 deltaX, int32 deltaY )
{
	player_->YawFacing( float32(deltaX) * InputAPI()->GetSensitivity() );
	player_->PitchFacing( float32(deltaY) * InputAPI()->GetSensitivity() );
}

void
C_Player::InputHandler_::MouseClickEvent( MouseButton button, bool isDown )
{
	player_->AddMouseClickEvent( button, isDown );
}