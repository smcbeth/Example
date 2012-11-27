//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "iserverentity.h"

class ICollide;

class S_BaseEntity: public IServerEntity
{
public:
	S_BaseEntity( );
	virtual ~S_BaseEntity( );

	virtual const char*	GetEntityClassName( ) = 0;
	uint32				GetUID( );

	virtual void		Update( );

	virtual void		Think( );
	void				SetNextThink( float32 deltaTime );
	float32				GetNextThink( );

	bool				ShouldSerializeToNetwork( );
	virtual void		SerializeToNetwork( SocketMessage* socketMessage );
	virtual void		UnserializeFromNetwork( SocketMessage* socketMessage );

	virtual void		SerializeToHDD( IStreamFile* file );
	virtual void		UnserializeFromHDD( IStreamFile* file );

	virtual void		Spawn( );
	virtual void		Despawn( );

	void				SetTranslation( const Vector3D& origin );
	Vector3D			GetTranslation( );
	float32				GetTranslationX( );
	float32				GetTranslationY( );
	float32				GetTranslationZ( );

	void				SetRotationYawPitchRoll( float32 yaw, float32 pitch, float32 roll );
	void				RotateX( float32 x );
	void				RotateY( float32 y );
	void				RotateZ( float32 z );

	void				AddBacklink( ServerEntityNode* node );
	ServerEntityNode*	GetBacklink( uint32 idx );
	uint32				GetBacklinkCount( );
	void				Unlink( );

	void				SetCollide( ICollide* collide );
	ICollide*			GetCollide( );

protected:
	bool				doSerializeToNetwork_;	// new changes to send clients?

private:
	float32				nextThink_;				// next think time, in seconds
	ICollide*			collide_;				// our physics representation


	uint32				uid_;					// unique entitiy identifier, same on the server and client
	ServerEntityLinks	backlinks_;				// octree back-links
};