//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "s_basemobile.h"
#include "public\nstdlib\matrix.h"
#include "public\nstdlib\gmath.h"
#include "public\newport lib\force.h"
#include "public\istreamfile.h"
#include "public\isocket.h"
#include "public\icollide.h"

S_BaseMobile::S_BaseMobile( )
{
	jumpVelocity_.Init();

	facingYaw_			= 0.0f;
	facingPitch_		= 0.0f;
	facingRoll_			= 0.0f;

	isMovingForward_	= false;
	isMovingBackward_	= false;
	isMovingLeft_		= false;
	isMovingRight_		= false;

	moveForce_			= NULL;
}

S_BaseMobile::~S_BaseMobile( )
{
	;
}

void
S_BaseMobile::SetVelocity( const Vector3D& velocity )
{
	if( GetCollide() )
	{
		GetCollide()->SetVelocity( velocity );
	}
}

Vector3D
S_BaseMobile::GetVelocity( )
{
	if( GetCollide() )
	{
		return GetCollide()->GetVelocity();
	}
}

void
S_BaseMobile::SetFacing( float32 yaw, float32 pitch, float32 roll )
{
	facingYaw_		= yaw;
	facingPitch_	= pitch;
	facingRoll_		= roll;
}

void
S_BaseMobile::YawFacing( float32 amount )
{
	facingYaw_ += amount;
}

void
S_BaseMobile::PitchFacing( float32 amount )
{
	facingPitch_ += amount;
}

void
S_BaseMobile::RollFacing( float32 amount )
{
	facingRoll_ += amount;
}

void
S_BaseMobile::GetFacing( Vector3D& out )
{
	FAssert();
}

float32
S_BaseMobile::GetFacingYaw( )
{
	return facingYaw_;
}

float32
S_BaseMobile::GetFacingPitch( )
{
	return facingPitch_;
}

float32
S_BaseMobile::GetFacingRoll( )
{
	return facingRoll_;
}

void
S_BaseMobile::SetMovingForward( bool isMovingForward )
{
	isMovingForward_ = isMovingForward;
}

void
S_BaseMobile::SetMovingBackward( bool isMovingBackward )
{
	isMovingBackward_ = isMovingBackward;
}

void
S_BaseMobile::SetMovingLeft( bool isMovingLeft )
{
	isMovingLeft_ = isMovingLeft;
}

void
S_BaseMobile::SetMovingRight( bool isMovingRight )
{
	isMovingRight_ = isMovingRight;
}

void
S_BaseMobile::SetJumpVelocity( const Vector3D& velocity )
{
	jumpVelocity_ = velocity;
}

void
S_BaseMobile::Jump( )
{
	GetCollide()->AddVelocity( jumpVelocity_ );
}

void
S_BaseMobile::SetMoveForce( Force* force )
{
	Assert( !moveForce_ );
	Assert( GetCollide() );

	GetCollide()->AddForce( force );
	moveForce_ = force;
}

Force*
S_BaseMobile::GetMoveForce( )
{
	return moveForce_;
}

//-----------------------------------------------------------------------------
//
// Inherited from BaseEntity
//
//-----------------------------------------------------------------------------
void
S_BaseMobile::Update( )
{
	S_BaseEntity::Update();

	if( moveForce_ )
	{
		// handle movement

		// compute vectors pointing camera 'right' and 'forward'
		Matrix facingMatrix;
		MatrixYawPitchRoll( facingYaw_, facingPitch_, facingRoll_, facingMatrix );
	
		Vector3D right( 1.0f, 0.0f, 0.0f );
		right *= facingMatrix;
	
		Vector3D forward( 0.0f, 1.0f, 0.0f );
		forward *= facingMatrix;

		// compute new velocity
		Vector3D newVelocity;
		newVelocity.Init();
		if( isMovingForward_ )
		{
			newVelocity += forward;
		}
		if( isMovingBackward_ )
		{
			newVelocity -= forward;
		}
		if( isMovingRight_ )
		{
			newVelocity += right;
		}
		if( isMovingLeft_ )
		{
			newVelocity -= right;
		}

		// set to max velocity
		if( !newVelocity.IsNull() )
		{
			newVelocity.Normalize();
			newVelocity *= moveForce_->GetMaxVelocity();
		}

		moveForce_->SetVelocity( newVelocity );
	}
}

void
S_BaseMobile::Think( )
{
	S_BaseEntity::Think();
}

void
S_BaseMobile::UnserializeFromNetwork( SocketMessage* socketMessage )
{
 	socketMessage->Read( facingYaw_ );
	socketMessage->Read( facingPitch_ );
	socketMessage->Read( facingRoll_ );
}

void
S_BaseMobile::SerializeToHDD( IStreamFile* file )
{
	S_BaseEntity::SerializeToHDD( file );

	file->Write( jumpVelocity_ );
	file->Write( facingYaw_ );
	file->Write( facingPitch_ );
	file->Write( facingRoll_ );
}

void
S_BaseMobile::UnserializeFromHDD( IStreamFile* file )
{
	S_BaseEntity::UnserializeFromHDD( file );

	// 'S_BaseEntity' unserialized our collide, so here we attach the movement force
	GetCollide()->AddForce( GetMoveForce() );

	file->Read( jumpVelocity_ );
	file->Read( facingYaw_ );
	file->Read( facingPitch_ );
	file->Read( facingRoll_ );
}

void
S_BaseMobile::Spawn( )
{
	S_BaseEntity::Spawn();
}

void
S_BaseMobile::Despawn( )
{
	S_BaseEntity::Despawn();
}