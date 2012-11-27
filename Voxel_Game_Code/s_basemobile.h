//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: Anything that can move on it's own inherits from this.
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\nstdlib\vector3d.h"
#include "s_baseentity.h"

class SocketMessage;
class Force;

class S_BaseMobile: public S_BaseEntity
{
public:
	S_BaseMobile( );
	virtual ~S_BaseMobile( );

	void			SetVelocity( const Vector3D& velocity );
	Vector3D		GetVelocity( );

	void			SetFacing( float32 yaw, float32 pitch, float32 roll );
	void			YawFacing( float32 amount );
	void			PitchFacing( float32 amount );
	void			RollFacing( float32 amount );
	void			GetFacing( Vector3D& out );
	float32			GetFacingYaw( );
	float32			GetFacingPitch( );
	float32			GetFacingRoll( );

	void			SetMovingForward( bool isMovingForward );
	void			SetMovingBackward( bool isMovingBackward );
	void			SetMovingRight( bool isMovingRight );
	void			SetMovingLeft( bool isMovingLeft );

	void			SetJumpVelocity( const Vector3D& velocity );
	void			Jump( );

	void			SetMoveForce( Force* force );
	Force*			GetMoveForce( );

	void			SetNoClip( bool noClip );
	bool			GetNoClip( );

	//-------------------------------------------------------------------------
	// Inherited from BaseEntity
	//-------------------------------------------------------------------------
	virtual void	Update( );

	virtual void	Think( );

	virtual void	UnserializeFromNetwork( SocketMessage* socketMessage );

	virtual void	SerializeToHDD( IStreamFile* file );
	virtual void	UnserializeFromHDD( IStreamFile* file );

	virtual void	Spawn( );
	virtual void	Despawn( );

private:
	Vector3D	jumpVelocity_;

	float32		facingYaw_;
	float32		facingPitch_;
	float32		facingRoll_;

	bool		isMovingForward_;
	bool		isMovingBackward_;
	bool		isMovingRight_;
	bool		isMovingLeft_;

	Force*		moveForce_;			// the force we use to push our collide around
};