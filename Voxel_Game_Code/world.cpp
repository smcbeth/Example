//---------- Copyright © 2010, Daedal Games, All rights reserved. -------------

#include "world.h"
#include "public\nstdlib\_math.h"
#include "public\iengine.h"
#include "public\isocket.h"
#include "public\ifilesystem.h"
#include "public\istreamfile.h"
#include "public\iphysicssystem.h"
#include "public\icollide.h"
#include "iserverentity.h"
#include "terrainhelper.h"
#include "serverentitylist.h"
#include "s_sector.h"

World::World( )
{
	length_			= 0.0f;
	depth_			= 0.0f;
	isWorldLoaded_	= false;
	isModifying_	= false;
}

World::~World( )
{
	Unload();
}

void
World::Update( )
{
	// update sectors & entities
	for( uint32 i = 0; i < sectors_.GetCount(); ++i )
	{
		sectors_[ i ]->Update();
	}
	for( uint32 i = 0; i < entities_.GetCount(); ++i )
	{
		entities_[ i ]->Update();
	}
}

void
World::Save( )
{
	// open save file
	IStreamFile* saveFile = FileAPI()->GetStreamFile( "saves\\save.dat", "wb" ); // CTODO: have this location be provided

	// compute sector offsets size (one uint32 for each sector in the world)
	uint32 worldSectorsAcross	= uint32(length_ / SECTOR_LENGTH);
	uint32 worldSectorsDeep		= uint32(depth_ / SECTOR_LENGTH);
	uint32 sectorOffsetsSize = Square( worldSectorsAcross ) * worldSectorsDeep * sizeof( uint32 );

	// seek passed header and offsets
	saveFile->Seek( SAVEFILE_HEADERSIZE + sectorOffsetsSize, SEEKTYPE_SET );

	// create buffer to store offsets
	uint32* sectorOffsets = (uint32*)malloc( sectorOffsetsSize );

	// serialize sectors
	for( uint32 i = 0; i < sectors_.GetCount(); ++i )
	{
		// remember the offset for later
		sectorOffsets[ i ] = saveFile->Tell();
	
		sectors_[ i ]->SerializeToHDD( saveFile );
	}

	// write entity count
	saveFile->Write( entities_.GetCount() );

	// serialize entities
	for( uint32 i = 0; i < entities_.GetCount(); ++i )
	{
		entities_[ i ]->SerializeToHDD( saveFile );
	}

	// seek to start and write header and offsets
	saveFile->Seek( 0, SEEKTYPE_SET );
	saveFile->Write( length_ );
	saveFile->Write( depth_ );
	saveFile->Write( sectorOffsets, 1, sectorOffsetsSize );

	// cleanup
	saveFile->Release();
	SAFE_FREE( sectorOffsets );
}

void
World::Load( )
{
	Assert( !isWorldLoaded_ );

	// open save file
	IStreamFile* loadFile = FileAPI()->GetStreamFile( "saves\\save.dat", "rb" );

	// set up physics environment
	PhysicsAPI()->SetGravity( Vector3D( 0.0f, 0.0f, -32.0876969f ) );
	//env_->SetAirFriction( 0.1f ); // CTODO:

	// read size
	loadFile->Read( length_ );
	loadFile->Read( depth_ );

	// compute world size in sectors
	uint32 worldSectorsAcross	= uint32(length_ / SECTOR_LENGTH);
	uint32 worldSectorsDeep		= uint32(depth_ / SECTOR_LENGTH);

	// compute sector offsets size (one uint32 for each sector in the world)
	uint32 sectorOffsetsSize = Square( worldSectorsAcross ) * worldSectorsDeep * sizeof( uint32 );

	// seek passed header and sector offsets
	loadFile->Seek( SAVEFILE_HEADERSIZE + sectorOffsetsSize, SEEKTYPE_SET );

	// init pvs octree
	pvsOcTree_.Init( length_, depth_, SECTOR_LENGTH );

	// init physics octree
	PhysicsAPI()->InitOcTree( length_, depth_, SECTOR_LENGTH );

	// unserialize sectors
	for( uint32 z = 0; z < worldSectorsDeep; ++z )
	{
		for( uint32 y = 0; y < worldSectorsAcross; ++y )
		{
			for( uint32 x = 0; x < worldSectorsAcross; ++x )
			{
				S_Sector* sector = new S_Sector;

				// unserialize
				sector->UnserializeFromHDD( loadFile );

				// add to sector list
				sectors_.InsertBack( sector );

				// link to pvs octree
		 		pvsOcTree_.AddObject( sector, sector->GetCollide()->GetExtents() );

				// 'spawn'
				sector->Spawn();
			}
		}
	}

#ifdef _DEBUG
	// make sure all the sectors got linked
	for( uint32 i = 0; i < sectors_.GetCount(); ++i )
	{
		if( !sectors_[ i ]->GetBacklinkCount() )
		{
			FAssertMsg( "World::Load(~): Unlinked sector found." );
		}
	}
#endif

	// read entity count
	uint32 entityCount;
	loadFile->Read( entityCount );

	// unserialize entities
	for( uint32 i = 0; i < entityCount; ++i )
	{
		// read entity name
		char entityName[ MAX_ENTITYNAME ];
		loadFile->Read( entityName );

		// create
		IServerEntity* entity = GetServerEntityList()->Create( entityName );

		// unserialize
		entity->UnserializeFromHDD( loadFile );

		// link to octree
		pvsOcTree_.AddObject( entity, entity->GetCollide()->GetExtents() );

		// add to entity list
		entities_.InsertBack( entity );

		// 'spawn'
		entity->Spawn();
	}

	isWorldLoaded_ = true;

	// cleanup
	loadFile->Release();
}

void
World::Unload( )
{
	length_	= 0.0f;
	depth_	= 0.0f;

	sectors_.RemoveAndDeleteAll();
	entities_.RemoveAndDeleteAll();

	isWorldLoaded_ = false;
}

uint32
World::GetLength( )
{
	return length_;
}

uint32
World::GetDepth( )
{
	return depth_;
}

uint32
World::GetMinSize( )
{
	return SECTOR_LENGTH;
}

IServerEntity*
World::CreateEntity( const char* name )
{
	IServerEntity* entity = GetServerEntityList()->Create( name );
	entities_.InsertBack( entity );
	return entity;
}

void
World::DestroyEntity( IServerEntity* entity )
{
	entities_.FindFirstAndRemove( entity );
	delete entity;
}

void
World::ComputePVSMessages( Vector< SocketMessage* >& dest )
{
	// CTODO: this sucks shit

	for( uint32 i = 0; i < sectors_.GetCount(); ++i )
	{
		SocketMessage* socketMessage = new SocketMessage;
		socketMessage->Write( (uint32)MESSAGETYPE_NEWENTITY );
		socketMessage->Write( sectors_[ i ]->GetEntityClassName() );
		sectors_[ i ]->SerializeToNetwork( socketMessage );
		dest.InsertBack( socketMessage );
	}

	for( uint32 i = 0; i < entities_.GetCount(); ++i )
	{
		SocketMessage* socketMessage = new SocketMessage;
		socketMessage->Write( (uint32)MESSAGETYPE_NEWENTITY );
		socketMessage->Write( entities_[ i ]->GetEntityClassName() );
		entities_[ i ]->SerializeToNetwork( socketMessage );
		dest.InsertBack( socketMessage );
	}
}

void
World::ComputeKeyframeMessages( Vector< SocketMessage* >& dest )
{
	// serialize all sectors and entities

	for( uint32 i = 0; i < sectors_.GetCount(); ++i )
	{
		SocketMessage* socketMessage = new SocketMessage;
		socketMessage->Write( (uint32)MESSAGETYPE_ENTITYUPDATE );
		sectors_[ i ]->SerializeToNetwork( socketMessage );
		dest.InsertBack( socketMessage );
	}

	for( uint32 i = 0; i < entities_.GetCount(); ++i )
	{
		SocketMessage* socketMessage = new SocketMessage;
		socketMessage->Write( (uint32)MESSAGETYPE_ENTITYUPDATE );
		entities_[ i ]->SerializeToNetwork( socketMessage );
		dest.InsertBack( socketMessage );
	}
}

void
World::ComputeDeltaMessages( Vector< SocketMessage* >& dest )
{
	// serialize all sectors and entities that request it

	for( uint32 i = 0; i < sectors_.GetCount(); ++i )
	{
		if( sectors_[ i ]->ShouldSerializeToNetwork() )
		{
			SocketMessage* socketMessage = new SocketMessage;
			socketMessage->Write( (uint32)MESSAGETYPE_ENTITYUPDATE );
			sectors_[ i ]->SerializeToNetwork( socketMessage );
			dest.InsertBack( socketMessage );
		}
	}

	for( uint32 i = 0; i < entities_.GetCount(); ++i )
	{
		if( entities_[ i ]->ShouldSerializeToNetwork() )
		{
			SocketMessage* socketMessage = new SocketMessage;
			socketMessage->Write( (uint32)MESSAGETYPE_ENTITYUPDATE );
			entities_[ i ]->SerializeToNetwork( socketMessage );
			dest.InsertBack( socketMessage );
		}
	}
}

void
World::UnserializeEntityUpdate( SocketMessage* socketMessage )
{
	uint32 uid;
	socketMessage->Read( uid );

	// find the entity and unserialize
	for( uint32 i = 0; i < entities_.GetCount(); ++i )
	{
		if( uid == entities_[ i ]->GetUID() )
		{
			entities_[ i ]->UnserializeFromNetwork( socketMessage );
			return;
		}
	}

	// entity uid could not be found
	FAssert();
}

//-----------------------------------------------------------------------------
// Slow but simple methods for modifying sector data
// Coordinates start at the bottom-back-left corner of the world
//-----------------------------------------------------------------------------
bool
World::GetIsModifying( )
{
	return isModifying_;
}

void
World::BeginModify( )
{
	Assert( !isModifying_ );
	isModifying_ = true;
}

void
World::SetSCube( uint32 bX, uint32 bY, uint32 bZ, uint32 cubeData )
{
	Assert( isModifying_ );
	Assert( GetSCubeTerrainType( cubeData ) < TERRATYPE_COUNT );

	// get the sector containing sCube
	S_Sector* sector = GetSector( bX, bY, bZ );
	
	// sector unopen?
	if( !sector->GetIsModifying() )
	{
		// open the sector for writing
		sector->BeginModify();
		openedSectors_.InsertBack( sector );
	}

	// get the sectors data
	uint32* cubeDatas = sector->GetData();

	// translate the block coords from world space to sector space
	bX -= uint32((sector->GetTranslationX() - (SECTOR_LENGTH / 2.0f)) / SCUBE_LENGTH);
	bY -= uint32((sector->GetTranslationY() - (SECTOR_LENGTH / 2.0f)) / SCUBE_LENGTH);
	bZ -= uint32((sector->GetTranslationZ() - (SECTOR_LENGTH / 2.0f)) / SCUBE_LENGTH);

	// write the sCube
	cubeDatas[ bX + (bY * SECTOR_BLENGTH) + (bZ * Square( SECTOR_BLENGTH )) ] = cubeData;
}

uint32
World::GetSCube( uint32 bX, uint32 bY, uint32 bZ, S_Sector* openedSector )
{
	// Note- openedSector is used to optimize and avoid redundancy in AStar3D
	// STODO: is there a better way of doing this? and the way AStar3D is multithreaded could affect how this is used

	Assert( isModifying_ );

	// get the sector containing sCube
	S_Sector* sector = GetSector( bX, bY, bZ );
	
	// sector unopen?
	if( !sector->GetIsModifying() )
	{
		// open the sector for writing
		sector->BeginModify();
		openedSectors_.InsertBack( sector );
		openedSector = sector;
	}

	// get the sectors data
	uint32* cubeDatas = sector->GetData();

	// translate the block coords from world space to sector space
	bX -= uint32((sector->GetTranslationX() - (SECTOR_LENGTH / 2.0f)) / SCUBE_LENGTH);
	bY -= uint32((sector->GetTranslationY() - (SECTOR_LENGTH / 2.0f)) / SCUBE_LENGTH);
	bZ -= uint32((sector->GetTranslationZ() - (SECTOR_LENGTH / 2.0f)) / SCUBE_LENGTH);

	// return the sCube
	return cubeDatas[ bX + (bY * SECTOR_BLENGTH) + (bZ * SECTOR_BAREA) ];
}

void
World::EndModify( )
{
	Assert( isModifying_ );

	// close out any sectors we opened
	while( openedSectors_.GetCount() )
	{
		openedSectors_.PopBack()->EndModify();
	}

	isModifying_ = false;
}

//-----------------------------------------------------------------------------
// The following methods should only be called from "WorldGenerator"
//-----------------------------------------------------------------------------
void
World::SetSize( float32 length, float32 depth )
{
	length_	= length;
	depth_	= depth;
}

void
World::AddSector( S_Sector* sector )
{
	sectors_.InsertBack( sector );
}

S_Sector*
World::GetSector( uint32 bX, uint32 bY, uint32 bZ )
{
	// compute length of world in sectors
	uint32 worldSectorsAcross = uint32(length_ / (SECTOR_BLENGTH * SCUBE_LENGTH));

	bX /= SECTOR_BLENGTH;
	bY /= SECTOR_BLENGTH;
	bZ /= SECTOR_BLENGTH;

	return sectors_[ bX + (bY * worldSectorsAcross) + (bZ * Square( worldSectorsAcross )) ];
}

void
World::ComputeVis( )
{
	uint32* sectorDatas[ 27 ];
	S_Sector* sectors[ 27 ];

	for( uint32 i = 0; i < 27; ++i )
	{
		sectors[ i ] = NULL;
	}

	// make a sector filled with air.
	uint32* airSector = (uint32*)Calloc( SECTOR_BVOLUME, sizeof( uint32 ) );

	for( uint32 oZ = 0; oZ < depth_ / SCUBE_LENGTH; oZ += SECTOR_BLENGTH )
	{
		for( uint32 oY = 0; oY < length_ / SCUBE_LENGTH; oY += SECTOR_BLENGTH )
		{
			for( uint32 oX = 0; oX < length_ / SCUBE_LENGTH; oX += SECTOR_BLENGTH )
			{
				// Have all sectorDatas point to a sector filled with air by default. 
				// This is done so the terrain smoother algoritm does not have to worry about world extents.
				for( uint32 i = 0; i < 27; ++i )
				{
					sectorDatas[ i ] = airSector;
				}

				// Call begin modify on every sector in 3x3x3 cube based on conditions.
				for( int32 oZMod = 0 - int32(SECTOR_BLENGTH); oZMod <= SECTOR_BLENGTH; oZMod += SECTOR_BLENGTH )
				{
					for( int32 oYMod = 0 - int32(SECTOR_BLENGTH); oYMod <= SECTOR_BLENGTH; oYMod += SECTOR_BLENGTH )
					{
						for( int32 oXMod = 0 - int32(SECTOR_BLENGTH); oXMod <= SECTOR_BLENGTH; oXMod += SECTOR_BLENGTH )
						{
							if( (oXMod < 0 && oX == 0 ) ||
								(oYMod < 0 && oY == 0) ||
								(oZMod < 0 && oZ == 0) ||
								(oXMod > 0 && oX == length_ / SCUBE_LENGTH - SECTOR_BLENGTH) ||
								(oYMod > 0 && oY == length_ / SCUBE_LENGTH - SECTOR_BLENGTH) ||
								(oZMod > 0 && oZ == depth_ / SCUBE_LENGTH - SECTOR_BLENGTH) )
							{
								continue;
							}

							sectors[ (oXMod + SECTOR_BLENGTH + (oYMod + SECTOR_BLENGTH) * 3 + (oZMod + SECTOR_BLENGTH) * 9) / SECTOR_BLENGTH ] = 
								GetWorld()->GetSector( uint32(int32(oX) + oXMod), uint32(int32(oY) + oYMod), uint32(int32(oZ) + oZMod) );

							if( !sectors[ (oXMod + SECTOR_BLENGTH + (oYMod + SECTOR_BLENGTH) * 3 + (oZMod + SECTOR_BLENGTH ) * 9 ) / SECTOR_BLENGTH ]->GetIsModifying() )
							{
								sectorDatas[ (oXMod + SECTOR_BLENGTH + (oYMod + SECTOR_BLENGTH) * 3 + (oZMod + SECTOR_BLENGTH) * 9) / SECTOR_BLENGTH ] = 
									sectors[ (oXMod + SECTOR_BLENGTH + (oYMod + SECTOR_BLENGTH) * 3 + (oZMod + SECTOR_BLENGTH) * 9) / SECTOR_BLENGTH ]->BeginModify(); 
							}
							else
							{
								sectorDatas[ (oXMod + SECTOR_BLENGTH + (oYMod + SECTOR_BLENGTH ) * 3 + (oZMod + SECTOR_BLENGTH) * 9) / SECTOR_BLENGTH ] = 
									sectors[ (oXMod + SECTOR_BLENGTH + (oYMod + SECTOR_BLENGTH ) * 3 + (oZMod + SECTOR_BLENGTH) * 9) / SECTOR_BLENGTH ]->GetData(); 
							}			
						}
					}
				}

				// Add scubes to surface of terrain.
				sectors[ 13 ]->ComputeVis( sectorDatas );

				// Only call end modify on a max of 9 sectors at a time.
				for( uint32 i = 0; i < 9; ++i )
				{
					if( sectors[ i * 3 ] != NULL && sectors[i * 3]->GetIsModifying() )
					{
						sectors[ i * 3 ]->EndModify(); // in a 3x3x3 cube with x,y,z, x equals 0.
						sectors[ i * 3 ] = NULL;
					}
				}
			}
			// Call end modify on all remaining sectors if the end of the map is reached along the x axis.
			for( uint32 i = 0; i < 27; ++i )
			{
				if( sectors[ i ] != NULL && sectors[ i ]->GetIsModifying() )
				{
					sectors[ i ]->EndModify();
					sectors[ i ] = NULL;
				}
			}
		}
	}

	free( airSector );
}