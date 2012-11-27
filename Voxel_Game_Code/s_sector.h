//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: Client version.
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\nstdlib\octree.h"
#include "definitions.h"
#include "iserverentity.h"

class Vector3D;
class Point3D;
class TriangleSoup;
class IMaterial;
class IMesh;
class IServerEntity;

// HACK: this is to keep our extents fitting cleanly inside an octree sector, since
// if our extents touched a boundary we would be registered to both
#define EXTENT_SHRINK_AMOUNT 0.1f

// CTODO: adjust origins so that we use the negative portion of our position floats as well for long-range accuracy
class S_Sector: public IServerEntity
{
public:
	S_Sector( );
	~S_Sector( );

	// These methods assume you mean for this class to handle the memory
	// (in that it will be freed by this class)
	void				Init( Vector3D origin, uint32* data );
	void				Init( Vector3D origin, uint32* compressedData, uint32 compressedDataSize );

	bool				GetIsModifying( );
	uint32*				BeginModify( );
	// These should only be called between BeginModify(~) and EndModify(~)
	void				RemoveSCube( Point3D location );
	uint32*				GetData( );
	void				EndModify( );

	void				ComputeVis( uint32** sectorDatas );
	
	uint32*				GetCompressedData( );
	uint32				GetCompressedDataSize( );

	// Free unpacked data to save memory, repacking if necessary
	void				PurgeData( );

	//-------------------------------------------------------------------------
	// Inherited from 'IServerEntity'
	//-------------------------------------------------------------------------
	const char*			GetEntityClassName( );
	uint32				GetUID( );

	void				Update( );

	void				Think( );
	void				SetNextThink( float32 deltaTime );
	float32				GetNextThink( );

	bool				ShouldSerializeToNetwork( );
	void				SerializeToNetwork( SocketMessage* socketMessage );
	void				UnserializeFromNetwork( SocketMessage* socketMessage );

	void				SerializeToHDD( IStreamFile* file );
	void				UnserializeFromHDD( IStreamFile* file );

	void				Spawn( );
	void				Despawn( );

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

	void				SetMesh( IMesh* mesh );
	IMesh*				GetMesh( );

	void				SetCollide( ICollide* collide );
	ICollide*			GetCollide( );

private:
	// Prevents duplicate constructor code
	void Init_( );
	void CreateCollide_( );
	void UpdateCollide_( );
	void PackData_( );
	void UnpackData_( );

	uint32*				compressedData_;					// packed scube data
	uint32				compressedDataSize_;
	uint32*				data_;								// unpacked scube data (will not always be in memory)
	uint32				uid_;								// our unique identifier

	Vector3D			origin_;							// central point
	TriangleSoup*		collide_;

	// vertex and index count for each terrain type
	uint32				vertexCounts_[ TERRAINTYPE_COUNT ];
	uint32				indexCounts_[ TERRAINTYPE_COUNT ];
	
	bool				isCompressedDataDirty_;
	bool				isModifying_;
	bool				doSerializeToNetwork_;				// clients need to be notified of some change?

	ServerEntityLinks	backlinks_;							// back-link to OcTree node (sectors should only have one link)
};