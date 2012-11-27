//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\nstdlib\vector.h"
#include "public\nstdlib\vector3d.h"
#include "public\nstdlib\octree.h"
#include "definitions.h"

#define SAVEFILE_HEADERSIZE (2 * sizeof( uint32 ))

class SocketMessage;
class IServerEntity;
class S_Sector;

class World
{
public:
	World( );
	~World( );

	void			Update( );

	void			Save( );
	void			Load( );
	void			Unload( );

	uint32			GetLength( );
	uint32			GetDepth( );
	uint32			GetMinSize( );

	IServerEntity*	CreateEntity( const char* name );
	void			DestroyEntity( IServerEntity* entity );

	void			ComputePVSMessages( Vector< SocketMessage* >& dest );
	void			ComputeKeyframeMessages( Vector< SocketMessage* >& dest );
	void			ComputeDeltaMessages( Vector< SocketMessage* >& dest );

	void			UnserializeEntityUpdate( SocketMessage* socketMessage );

	// Slow but simple methods for modifying sector data
	// Coordinates start at the bottom-back-left corner of the world
	bool			GetIsModifying( );
	void			BeginModify( );
	void			SetSCube( uint32 bX, uint32 bY, uint32 bZ, uint32 cubeData );
	uint32			GetSCube( uint32 bX, uint32 bY, uint32 bZ, S_Sector* openedSector = NULL );
	void			EndModify( );

	// The following methods should only be called from "WorldGenerator"
	void			SetSize( float32 length, float32 depth );
	void			AddSector( S_Sector* sector );
	S_Sector*		GetSector( uint32 bX, uint32 bY, uint32 bZ );
	void			ComputeVis( );
		
private:
	// CTODO: put these fucking things in block count so we can use a uint32
	//        dont use floats where we dont have to
	float32						length_;		// the length of the world
	float32						depth_;			// the depth of the world

	Vector< S_Sector* >			sectors_;		// every sector in the world (expects sectors to be ordered in a specific way)
	Vector< S_Sector* >			openedSectors_;	// tracks sectors opened during a modify

	Vector< IServerEntity* >	entities_;		// all 'spawned' entities
	OcTree< IServerEntity* >	pvsOcTree_;		// octree for computing client pvs

	bool						isWorldLoaded_;	// is world loaded?
	bool						isModifying_;	// is world being modifid using 'BeginModify(~)'?
};

// Accessor
FORCE_INLINE World*
GetWorld( )
{
	extern World* _g_WorldInstance;
	Assert( _g_WorldInstance );
	return _g_WorldInstance;
}