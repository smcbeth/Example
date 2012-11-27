//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\nstdlib\octree.h"

// use as a placeholder to serialize nothing to disk when strings are involved
#define NOTHING			'\0'

// pass to 'SetNextThink(~)' for special behaviour
#define THINK_NEVER		-1.0f
#define THINK_ALWAYS	-2.0f

class Vector3D;
class SocketMessage;
class IStreamFile;
class ICollide;
class IMesh;
class IServerEntity;

typedef OcTree< IServerEntity* >::Node	ServerEntityNode;
typedef Vector< ServerEntityNode* >		ServerEntityLinks;

class IServerEntity
{
public:
	virtual ~IServerEntity( ) { ; }

	virtual const char*			GetEntityClassName( ) = 0;
	virtual uint32				GetUID( ) = 0;

	virtual void				Update( ) = 0;

	virtual void				Think( ) = 0;
	virtual void				SetNextThink( float32 deltaTime ) = 0;
	virtual float32				GetNextThink( ) = 0;

	virtual bool				ShouldSerializeToNetwork( ) = 0;
	virtual void				SerializeToNetwork( SocketMessage* socketMessage ) = 0;
	virtual void				UnserializeFromNetwork( SocketMessage* socketMessage ) = 0;

	virtual void				SerializeToHDD( IStreamFile* file ) = 0;
	virtual void				UnserializeFromHDD( IStreamFile* file ) = 0;

	virtual void				Spawn( ) = 0;
	virtual void				Despawn( ) = 0;

	virtual void				SetTranslation( const Vector3D& origin ) = 0;
	virtual Vector3D			GetTranslation( ) = 0;
	virtual float32				GetTranslationX( ) = 0;
	virtual float32				GetTranslationY( ) = 0;
	virtual float32				GetTranslationZ( ) = 0;

	virtual void				SetRotationYawPitchRoll( float32 yaw, float32 pitch, float32 roll ) = 0;
	virtual void				RotateX( float32 x ) = 0;
	virtual void				RotateY( float32 y ) = 0;
	virtual void				RotateZ( float32 z ) = 0;

	virtual void				AddBacklink( ServerEntityNode* node ) = 0;
	virtual ServerEntityNode*	GetBacklink( uint32 idx ) = 0;
	virtual uint32				GetBacklinkCount( ) = 0;
	virtual void				Unlink( ) = 0;

	virtual void				SetCollide( ICollide* collide ) = 0;
	virtual ICollide*			GetCollide( ) = 0;
};