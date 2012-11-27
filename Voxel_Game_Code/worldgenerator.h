//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\nstdlib\vector.h"
#include "public\nstdlib\upoint3d.h"
#include "terrainhelper.h"

// STODO: reduce jagged edges between different textures
// STODO: perform smoothing on all orientations.
// STODO: have slope map sample more locations to reduce speckling of terra types on sloped terrain.
	// STODO: have slope map sample locations in adjacent sectors.
// STODO: improve cliff sculpting.
// STODO: generate more reliable flat sections of terrain at different elevations besides near sea level. make this smoothing drastic in some places for town support. afterwards make town locations more restrictive.
// STODO: improve the tree generator.
// STODO: implement cacti and multiple tree types.
// STODO: add more terra types and resources. fine tune allocation of these types.
// STODO: perform smoothing on terrain touching old air, lava, water, and snow, by treating these types as air. for smoothing snow itself, make sure other snow isnt treated like air. snow smoothing must be done in second pass. increase snow depth slightly to compensate.
// STODO: idea for responsive terrain types: use bits stored in block data to determine certain states of a block such as frozen and scorched. ex: frozen + dirt = snow, frozen + water = ice.
	// STODO: implement heated(glowing ) and wet blocks near lava and aquifers.
// STODO: OPTIMIZATION:( save for last ) improve speed of terrain generation.

// STODO: make a HeightMap class that is inited with the raw noise height map
	// this heightmap can then be revised
	// allow for

// STODO: (from chris) maybe put those inlined methods in a .inl file (just delete this when you see it)

// SNOW: add GenerateDebugWorld();
// SNOW: make a heightmap class that allows you to get a height map of any size and access height values at a 2D point
	// HeightMap would return a map that reflected any changes made to the raw height. HeightMap will inherit from RawHeightMap

// noise defines
#define ARIDTY_DILATION				400
#define SMOOTHING_DILATION			600
#define HIGHLANDS_DILATION			650
#define HILLPEAK_DILATION			220
#define MOUNTAINPEAK_DILATION		530
#define VERYNARROW2D_DILATION		5
#define NARROW2D_DILATION			15
#define MEDIUM2D_DILATION			50
#define BROAD2D_DILATION			175
#define QUARTERMILE2D_DILATION		440
#define HALFMILE2D_DILATION			880
#define MILE2D_DILATION				1760
#define VERYNARROW3D_DILATION		4
#define NARROW3D_DILATION			14
#define MEDIUM3D_DILATION			36
#define BROAD3D_DILATION			50

#define MAXOCTAVES					3 // for 3 octaves

class S_Sector;
class NoiseGenerator2D;
class NoiseGenerator3D;

enum INDEX2DNOISE
{
	INDEX2D_ARIDTY = 0,
	INDEX2D_SMOOTHING,
	INDEX2D_HIGHLANDS,
	INDEX2D_HILLPEAK,
	INDEX2D_MOUNTAINPEAK,
	INDEX2D_VERYNARROW,
	INDEX2D_NARROW,
	INDEX2D_MEDIUM,
	INDEX2D_BROAD,
	INDEX2D_QUARTERMILE,
	INDEX2D_HALFMILE,
	INDEX2D_MILE
};

enum INDEX3DNOISE
{
	INDEX3D_VERYNARROW = 0,
	INDEX3D_NARROW,
	INDEX3D_MEDIUM,
	INDEX3D_BROAD
};

class WorldGenerator
{
public:
	WorldGenerator( );
	~WorldGenerator( );

	// Starting Point
	void				Generate( uint32 size, uint32 depth );

	// 2D Map Generation
	bool				GenerateHeightMap( uint32 oX, uint32 oZ, uint16* heightMap, uint32 lengthX, uint32 lengthY ); //returns true if the heightmap area is suitable for a town	
	void				GenerateSlopeMap( float32* slopeMap, uint16* heightMaps[], uint32 lengthX, uint32 lengthY );

	// Perlin Noise Preparation
	void				PrepTerraNoise( uint32 oX, uint32 oY, uint32 oZ, uint32 lengthX, uint32 lengthY, bool prep3DNoise );

	// Accessors
	uint32				GetWorldDepth( );
	uint32				GetSeaLevel( );
	uint32				GetWorldSize( );
	Vector< uint32 >*	GetTownSupportingAreas( );
	char*				GetTownSupportingAreasSparseArray( );

private:
	// Sector Generation
	S_Sector*			GenerateSector_( uint32 oX, uint32 oY, uint32 oZ, uint16* heightMaps[] );
	void				GSSetTerrainShapeAnd3DNoisePass_( uint32 oX, uint32 oY, uint32 oZ, uint32* sectorData );
	void				GSModifyTerrainWith3DNoisePass_( uint32 oX, uint32 oY, uint32 oZ, uint32* sectorData, uint16* heightMaps[] );
	void				GSSetTerrainTypesPass_( uint32 oX, uint32 oY, uint32 oZ, uint32* sectorData );

	// Sector Postprocessing
	void				FillHeightMaps_( uint32 oX, uint32 oY, uint32 oZ, uint16* heightMaps[] );
	void				ReviseSectors_( uint32 oX, uint32 oY );
	void				ReviseHeightMap_( uint32 oX, uint32 oZ );
	void				SmoothSectorsAndFindOldAir_( );

	// Terrain Smoothing
	void				TerrainSmoother_( uint32** sectorDatas );
	void				TSAddCubesPass_( uint32** sectorDatas );
	void				TSAddSCubesToCubesPass_( uint32** sectorDatas );
	void				TSAddSCubesToSCubesPass_( uint32** sectorDatas );
	void				TSPostPass_( uint32** sectorDatas );
	bool				IsBlockSmoothable_( uint32 block );
	bool				IsBlockSmoothableBuildingMat_( uint32 block );
	bool				IsBlockSmoothableToOnlySelf_( uint32 block );

	// Tree Generation
	void				TreeGenerator_( uint32 loc, uint32 height, float32 aridty, uint32* sectorData );

	// Old Air
	void				FindOldAir_( uint32 oX, uint32 oY, uint32 oZ, uint32** sectorDatas );
	void				ReplaceOldAir_( UPoint3D point, int32 centerOX, int32 centerOY, int32 centerOZ );

	// Configuration
	void				LoadConfig_( );


	// STODO: clean up some of these variables.
	uint32				depth_;
	uint32				depthMod_;							// controls terrain thickness, sea level, etc... allows depth_ to be changed easily.
	uint32				seaLevel_;
	uint32				upperLavaLevel_;
	uint32				lowerLavaLevel_;
	uint32				size_;
	uint32				sectorsWritten_;
	uint32				seed_;
	uint16*				heightMap_;
	uint32*				sectorOffsets_;
	
	// STODO: clean this up/ find a better way to do this
	NoiseGenerator3D*	veryNarrow3DNoiseObj_;
	NoiseGenerator3D*	narrow3DNoiseObj_;
	NoiseGenerator3D*	medium3DNoiseObj_;
	NoiseGenerator3D*	broad3DNoiseObj_;

	NoiseGenerator2D*	aridty2DNoiseObj_;
	NoiseGenerator2D*	smoothing2DNoiseObj_;
	NoiseGenerator2D*	highlands2DNoiseObj_;
	NoiseGenerator2D*	hillPeak2DNoiseObj_;
	NoiseGenerator2D*	mountain2DNoiseObj_;
	NoiseGenerator2D*	smoothingNoiseObj_;
	NoiseGenerator2D*	hillPeakNoiseObj_;
	NoiseGenerator2D*	mountainNoiseObj_;
	NoiseGenerator2D*	veryNarrow2DNoiseObj_;
	NoiseGenerator2D*	narrow2DNoiseObj_;
	NoiseGenerator2D*	medium2DNoiseObj_;
	NoiseGenerator2D*	broad2DNoiseObj_;
	NoiseGenerator2D*	quarterMile2DNoiseObj_;
	NoiseGenerator2D*	halfMile2DNoiseObj_;
	NoiseGenerator2D*	mile2DNoiseObj_;

	float32*			veryNarrow3DNoise_;
	float32*			narrow3DNoise_;
	float32*			medium3DNoise_;
	float32*			broad3DNoise_;

	float32*			aridty_;
	float32*			veryNarrow2DNoise_;
	float32*			narrow2DNoise_;
	float32*			medium2DNoise_;
	float32*			broad2DNoise_;
	float32*			quarterMile2DNoise_;
	float32*			halfMile2DNoise_;
	float32*			mile2DNoise_;

	float32*			slopeMap_;

	Vector< uint32 >	townSupportingAreas_;				// stores x-z index of sectors, y is ignored.
	char*				townSupportingAreasSparseArray_;	// stores true and false values in 2D area based on GenerateHeightMap returns.

};

// Accessor
FORCE_INLINE WorldGenerator*
WorldGeneratorAPI( )
{
	extern WorldGenerator* _g_WorldGeneratorInstance;
	return _g_WorldGeneratorInstance;
}

// STODO: potentially use a bit for IsBlockSmoothable for more versatility
FORCE_INLINE bool
WorldGenerator::IsBlockSmoothable_( uint32 block )
{
	TERRATYPE type = GetSCubeTerrainType( block );
	if( type == TERRA_AIR ||
		type == TERRA_OLDAIR ||
		type == TERRA_WATER ||
		type == TERRA_LAVA ||
		type == TERRA_TREE ||
		type == TERRA_BOARDS ||
		type == TERRA_BRICK ||
		type == TERRA_STONEBRICK)
	{
		return false;
	}
	return true;
}

FORCE_INLINE bool
WorldGenerator::IsBlockSmoothableBuildingMat_( uint32 block )
{
	return GetSCubeTerrainType( block ) == TERRA_THATCH;
}

// SNOW: add this to terrain smoother. possibly attempt to level adjacent terrain to the level of the road

FORCE_INLINE bool
WorldGenerator::IsBlockSmoothableToOnlySelf_( uint32 block )
{
	return GetSCubeTerrainType( block ) == TERRA_DIRTROAD;
}
