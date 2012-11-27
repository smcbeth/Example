//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "public\nstdlib\platform.h"
#include "public\nstdlib\vector.h"
#include "public\isocket.h"
#include "public\imaterialsystem.h"
#include "public\imaterial.h"
#include "public\imesh.h"
#include "definitions.h"
#include "c_baseentity.h"
#include "cliententitylist.h"
#include "scubegeom.h"

class C_Sector: public C_BaseEntity
{
public:
	C_Sector( );
	~C_Sector( );

	//-------------------------------------------------------------------------
	// Inherited from C_BaseEntity
	//-------------------------------------------------------------------------
	const char*	GetEntityClassName( );

	void		UnserializeFromNetwork( SocketMessage* socketMessage );

	void		Update( );

	void		Spawn( );
	void		Despawn( );

private:
	void		ComputeHull_( uint32* vertexCounts, uint32* indexCounts );

	uint32*		compressedData_;						// packed scube data
	uint32		compressedDataSize_;

	IMaterial*	materials_[ TERRAINTYPE_COUNT - 1 ];	// stores the materials for our terrain types (minus air)

	Vector3D	origin_;								// central point
};

LINK_CLIENTENTITY_TO_CLASS( "sector", C_Sector );

C_Sector::C_Sector( )
{
	compressedData_			= NULL;
	compressedDataSize_		= 0;

	// load our materials
	// CTODO: only load the materials we need?
	materials_[ 0 ] = MaterialAPI()->GetOrLoadMaterial( "oldair" );
	materials_[ 1 ] = MaterialAPI()->GetOrLoadMaterial( "water" );
	materials_[ 2 ] = MaterialAPI()->GetOrLoadMaterial( "lava" );
	materials_[ 3 ] = MaterialAPI()->GetOrLoadMaterial( "snow" );
	materials_[ 4 ] = MaterialAPI()->GetOrLoadMaterial( "stone" );
	materials_[ 5 ] = MaterialAPI()->GetOrLoadMaterial( "gravel" );
	materials_[ 6 ] = MaterialAPI()->GetOrLoadMaterial( "grass" );
	materials_[ 7 ] = MaterialAPI()->GetOrLoadMaterial( "dirt" );
	materials_[ 8 ] = MaterialAPI()->GetOrLoadMaterial( "clay" );
	materials_[ 9 ] = MaterialAPI()->GetOrLoadMaterial( "sand" );
	materials_[ 10 ] = MaterialAPI()->GetOrLoadMaterial( "tree" );
	materials_[ 11 ] = MaterialAPI()->GetOrLoadMaterial( "leaves" );
	materials_[ 12 ] = MaterialAPI()->GetOrLoadMaterial( "coalore" );
	materials_[ 13 ] = MaterialAPI()->GetOrLoadMaterial( "copperore" );
	materials_[ 14 ] = MaterialAPI()->GetOrLoadMaterial( "goldore" );
	materials_[ 15 ] = MaterialAPI()->GetOrLoadMaterial( "ironore" );
	materials_[ 16 ] = MaterialAPI()->GetOrLoadMaterial( "silverore" );
	materials_[ 17 ] = MaterialAPI()->GetOrLoadMaterial( "tinore" );
	materials_[ 18 ] = MaterialAPI()->GetOrLoadMaterial( "bronze" );
	materials_[ 19 ] = MaterialAPI()->GetOrLoadMaterial( "copper" );
	materials_[ 20 ] = MaterialAPI()->GetOrLoadMaterial( "gold" );
	materials_[ 21 ] = MaterialAPI()->GetOrLoadMaterial( "iron" );
	materials_[ 22 ] = MaterialAPI()->GetOrLoadMaterial( "lead" );
	materials_[ 23 ] = MaterialAPI()->GetOrLoadMaterial( "mithril" );
	materials_[ 24 ] = MaterialAPI()->GetOrLoadMaterial( "tin" );
	materials_[ 25 ] = MaterialAPI()->GetOrLoadMaterial( "roughbronze" );
	materials_[ 26 ] = MaterialAPI()->GetOrLoadMaterial( "roughiron" );
	materials_[ 27 ] = MaterialAPI()->GetOrLoadMaterial( "boards" );
	materials_[ 28 ] = MaterialAPI()->GetOrLoadMaterial( "brick" );
	materials_[ 29 ] = MaterialAPI()->GetOrLoadMaterial( "stonebrick" );
	materials_[ 30 ] = MaterialAPI()->GetOrLoadMaterial( "thatch" );
	materials_[ 31 ] = MaterialAPI()->GetOrLoadMaterial( "dirtroad" );
}

C_Sector::~C_Sector( )
{
	SAFE_FREE( compressedData_ );
}

//-----------------------------------------------------------------------------
//
// Inherited from C_BaseEntity
//
//-----------------------------------------------------------------------------
const char*
C_Sector::GetEntityClassName( )
{
	return "sector";
}

void
C_Sector::UnserializeFromNetwork( SocketMessage* socketMessage )
{
	// C_Sector is one of the rare classes that does not call back to the base class here
	//C_BaseEntity::UnserializeFromNetwork( socketMessage );

	// blow away any 'compressedData'
	SAFE_FREE( compressedData_ );

	// vertex and index count for each terrain type
	uint32 vertexCounts[ TERRAINTYPE_COUNT ];
	uint32 indexCounts[ TERRAINTYPE_COUNT ];

	socketMessage->Read( origin_ );
	socketMessage->Read( extents_.mins );
	socketMessage->Read( extents_.maxs );
	socketMessage->Read( (void*)vertexCounts, TERRAINTYPE_COUNT * sizeof( uint32 ) );
	socketMessage->Read( (void*)indexCounts, TERRAINTYPE_COUNT * sizeof( uint32 ) );
	socketMessage->Read( compressedDataSize_ );
	compressedData_ = (uint32*)Malloc( compressedDataSize_ );
	socketMessage->Read( (void*)compressedData_, compressedDataSize_ );

	ComputeHull_( vertexCounts, indexCounts );
}

void
C_Sector::Update( )
{
	C_BaseEntity::Update();
}

void
C_Sector::Spawn( )
{
	C_BaseEntity::Spawn();
	// make sure we got linked only once to the material system octree
	AssertMsg( GetBacklinkCount() == 1, "C_Sector linked multiple times to Material System octree." );
}

void
C_Sector::Despawn( )
{
	C_BaseEntity::Despawn();
}

void
C_Sector::ComputeHull_( uint32* vertexCounts, uint32* indexCounts )
{
	// compute total vertex and index count
	uint32 vertexCount = 0;
	uint32 indexCount = 0;
	for( uint32 i = 0; i < TERRAINTYPE_COUNT; ++i )
	{
		vertexCount += vertexCounts[ i ];
		indexCount += indexCounts[ i ];
	}

	// any geometry?
	if( vertexCount && indexCount )
	{
		// create new mesh
		IMesh* mesh = MaterialAPI()->CreateMesh(
			"",
			VF_POSITION | VF_BASETEXTURE,
			vertexCount,
			indexCount,
			true,
			false );

		// add geometry to mesh
		MeshLockData meshLockData;
		mesh->Lock( &meshLockData );

		GetSCubeGeom()->GenerateSectorHull(
			meshLockData.vertexMemory,
			meshLockData.indexMemory,
			compressedData_,
			origin_,
			vertexCounts,
			indexCounts );

		mesh->Unlock( vertexCount, indexCount );

		// add passes to mesh
		uint32 baseVertex = 0;
		uint32 baseIndex = 0;
		for( uint32 i = 0; i < TERRAINTYPE_COUNT; ++i )
		{
			// terrain type has geometry?
			if( vertexCounts[ i ] && indexCounts[ i ] )
			{
				// add pass
				mesh->AddPass(
					baseVertex,
					baseIndex,
					indexCounts[ i ],
					vertexCounts[ i ],
					materials_[ i - 1 ] );

				baseVertex += vertexCounts[ i ];
				baseIndex += indexCounts[ i ];
			}
		}

		SetMesh( mesh );
	}
}