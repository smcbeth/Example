//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"

// CTODO: I sure hope we can find a way to get rid of all this. - chris
#define BOX_LEFT_FRONT_DOWN		sectorDatas[((x + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH)	+ ((y + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 3 +( (z + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 9	][(x + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH	+ ((y + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH	+( (z + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BAREA	]
#define BOX_FRONT_DOWN			sectorDatas[1											+ ((y + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 3 +( (z + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 9	][x											+ ((y + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH	+( (z + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BAREA	]
#define BOX_RIGHT_FRONT_DOWN	sectorDatas[((x + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH)	+ ((y + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 3 +( (z + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 9	][(x + 1) % SECTOR_BLENGTH					+ ((y + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH	+( (z + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BAREA	]
#define BOX_LEFT_DOWN			sectorDatas[((x + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH)	+ 3												+( (z + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 9	][(x + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH	+ y * SECTOR_BLENGTH											+( (z + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BAREA	]
#define BOX_DOWN				sectorDatas[1											+ 3												+( (z + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 9	][x											+ y * SECTOR_BLENGTH											+( (z + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BAREA	]
#define BOX_RIGHT_DOWN			sectorDatas[((x + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH)	+ 3												+( (z + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 9	][(x + 1) % SECTOR_BLENGTH					+ y * SECTOR_BLENGTH											+( (z + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BAREA	]
#define BOX_LEFT_BACK_DOWN		sectorDatas[((x + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH)	+ ((y + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 3 +( (z + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 9	][(x + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH	+ ((y + 1) % SECTOR_BLENGTH)	* SECTOR_BLENGTH					+( (z + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BAREA	]
#define BOX_BACK_DOWN			sectorDatas[1											+ ((y + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 3 +( (z + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 9	][x											+ ((y + 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH					+( (z + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BAREA	]
#define BOX_RIGHT_BACK_DOWN		sectorDatas[((x + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH)	+ ((y + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 3 +( (z + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 9	][(x + 1) % SECTOR_BLENGTH					+ ((y + 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH					+( (z + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BAREA	]
#define BOX_LEFT_FRONT			sectorDatas[((x + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH)	+ ((y + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 3 + 9												][(x + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH	+ ((y + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH	+ z * SECTOR_BAREA											]
#define BOX_FRONT				sectorDatas[1											+ ((y + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 3 + 9												][x											+ ((y + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH	+ z * SECTOR_BAREA											]
#define BOX_RIGHT_FRONT			sectorDatas[((x + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH)	+ ((y + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 3 + 9												][(x + 1) % SECTOR_BLENGTH					+ ((y + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH	+ z * SECTOR_BAREA											]
#define BOX_LEFT				sectorDatas[((x + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH)	+ 3												+ 9												][(x + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH	+ y * SECTOR_BLENGTH											+ z * SECTOR_BAREA											]
#define BOX_CENTER				sectorDatas[1											+ 3												+ 9												][x											+ y * SECTOR_BLENGTH											+ z * SECTOR_BAREA											]
#define BOX_RIGHT				sectorDatas[((x + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH)	+ 3												+ 9												][(x + 1) % SECTOR_BLENGTH					+ y * SECTOR_BLENGTH											+ z * SECTOR_BAREA											]
#define BOX_LEFT_BACK			sectorDatas[((x + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH)	+ ((y + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 3 + 9												][(x + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH	+ ((y + 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH					+ z * SECTOR_BAREA											]
#define BOX_BACK				sectorDatas[1											+ ((y + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 3 + 9												][x											+ ((y + 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH					+ z * SECTOR_BAREA											]
#define BOX_RIGHT_BACK			sectorDatas[((x + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH)	+ ((y + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 3 + 9												][(x + 1) % SECTOR_BLENGTH					+ ((y + 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH					+ z * SECTOR_BAREA											]
#define BOX_LEFT_FRONT_UP		sectorDatas[((x + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH)	+ ((y + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 3 +( (z + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 9	][(x + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH	+ ((y + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH	+( (z + 1) % SECTOR_BLENGTH) * SECTOR_BAREA					]
#define BOX_FRONT_UP			sectorDatas[1											+ ((y + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 3 +( (z + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 9	][x											+ ((y + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH	+( (z + 1) % SECTOR_BLENGTH) * SECTOR_BAREA					]
#define BOX_RIGHT_FRONT_UP		sectorDatas[((x + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH)	+ ((y + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH) * 3 +( (z + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 9	][(x + 1) % SECTOR_BLENGTH					+ ((y + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH	+( (z + 1) % SECTOR_BLENGTH) * SECTOR_BAREA					]
#define BOX_LEFT_UP				sectorDatas[((x + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH)	+ 3												+( (z + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 9	][(x + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH	+ y * SECTOR_BLENGTH											+( (z + 1) % SECTOR_BLENGTH) * SECTOR_BAREA					]
#define BOX_UP					sectorDatas[1											+ 3												+( (z + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 9	][x											+ y * SECTOR_BLENGTH											+( (z + 1) % SECTOR_BLENGTH) * SECTOR_BAREA					]
#define BOX_RIGHT_UP			sectorDatas[((x + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH)	+ 3												+( (z + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 9	][(x + 1) % SECTOR_BLENGTH					+ y * SECTOR_BLENGTH											+( (z + 1) % SECTOR_BLENGTH) * SECTOR_BAREA					]
#define BOX_LEFT_BACK_UP		sectorDatas[((x + SECTOR_BLENGTH - 1) / SECTOR_BLENGTH)	+ ((y + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 3 +( (z + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 9	][(x + SECTOR_BLENGTH - 1) % SECTOR_BLENGTH	+ ((y + 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH					+( (z + 1) % SECTOR_BLENGTH) * SECTOR_BAREA					]
#define BOX_BACK_UP				sectorDatas[1											+ ((y + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 3 +( (z + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 9	][x											+ ((y + 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH					+( (z + 1) % SECTOR_BLENGTH) * SECTOR_BAREA					]
#define BOX_RIGHT_BACK_UP		sectorDatas[((x + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH)	+ ((y + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 3 +( (z + SECTOR_BLENGTH + 1) / SECTOR_BLENGTH) * 9	][(x + 1) % SECTOR_BLENGTH					+ ((y + 1) % SECTOR_BLENGTH) * SECTOR_BLENGTH					+( (z + 1) % SECTOR_BLENGTH) * SECTOR_BAREA					]

// follow this order for fastest supersampling
#define LEFT_FRONT_DOWN		sectorData[(x - 1) + (y - 1) * SECTOR_BLENGTH + (z - 1) * SECTOR_BAREA]
#define FRONT_DOWN			sectorData[x + ((y - 1) * SECTOR_BLENGTH) + (z - 1) * SECTOR_BAREA]
#define RIGHT_FRONT_DOWN	sectorData[(x + 1) + (y - 1) * SECTOR_BLENGTH + (z - 1) * SECTOR_BAREA]
#define LEFT_DOWN			sectorData[(x - 1) + (y * SECTOR_BLENGTH) + (z - 1) * SECTOR_BAREA]
#define DOWN				sectorData[x + (y * SECTOR_BLENGTH) + (z - 1) * SECTOR_BAREA]
#define RIGHT_DOWN			sectorData[(x + 1) + (y * SECTOR_BLENGTH) + (z - 1) * SECTOR_BAREA]
#define LEFT_BACK_DOWN		sectorData[(x - 1) + (y + 1) * SECTOR_BLENGTH + (z - 1) * SECTOR_BAREA]
#define BACK_DOWN			sectorData[x + ((y + 1) * SECTOR_BLENGTH) + (z - 1) * SECTOR_BAREA]
#define RIGHT_BACK_DOWN		sectorData[(x + 1) + (y + 1) * SECTOR_BLENGTH + (z - 1) * SECTOR_BAREA]
#define LEFT_FRONT			sectorData[(x - 1) + (y - 1) * SECTOR_BLENGTH + z * SECTOR_BAREA]
#define FRONT				sectorData[x + ((y - 1) * SECTOR_BLENGTH) + z * SECTOR_BAREA]
#define RIGHT_FRONT			sectorData[(x + 1) + (y - 1) * SECTOR_BLENGTH + z * SECTOR_BAREA]
#define LEFT				sectorData[(x - 1) + (y * SECTOR_BLENGTH) + z * SECTOR_BAREA]
#define CENTER				sectorData[x + y * SECTOR_BLENGTH + z * SECTOR_BAREA]
#define RIGHT				sectorData[(x + 1) + (y * SECTOR_BLENGTH) + z * SECTOR_BAREA]
#define LEFT_BACK			sectorData[(x - 1) + (y + 1) * SECTOR_BLENGTH + z * SECTOR_BAREA]
#define BACK				sectorData[x + ((y + 1) * SECTOR_BLENGTH) + z * SECTOR_BAREA]
#define RIGHT_BACK			sectorData[(x + 1) + (y + 1) * SECTOR_BLENGTH + z * SECTOR_BAREA]
#define LEFT_FRONT_UP		sectorData[(x - 1) + (y - 1) * SECTOR_BLENGTH + (z + 1) * SECTOR_BAREA]
#define FRONT_UP			sectorData[x + ((y - 1) * SECTOR_BLENGTH) + (z + 1) * SECTOR_BAREA]
#define RIGHT_FRONT_UP		sectorData[(x + 1) + (y - 1) * SECTOR_BLENGTH + (z + 1) * SECTOR_BAREA]
#define LEFT_UP				sectorData[(x - 1) + (y * SECTOR_BLENGTH) + (z + 1) * SECTOR_BAREA]
#define UP					sectorData[x + (y * SECTOR_BLENGTH) + (z + 1) * SECTOR_BAREA]
#define RIGHT_UP			sectorData[(x + 1) + (y * SECTOR_BLENGTH) + (z + 1) * SECTOR_BAREA]
#define LEFT_BACK_UP		sectorData[(x - 1) + (y + 1) * SECTOR_BLENGTH + (z + 1) * SECTOR_BAREA]
#define BACK_UP				sectorData[x + ((y + 1) * SECTOR_BLENGTH) + (z + 1) * SECTOR_BAREA]
#define RIGHT_BACK_UP		sectorData[(x + 1) + (y + 1) * SECTOR_BLENGTH + (z + 1) * SECTOR_BAREA]

// STODO: fill this out so we can know at a glance
// Terrain Data Representation( uint32 )
// (We can further compress this when we care to)
//
// (where 0 is highbit )
// Bit(s) 0 - 3   4: Simplecube Type	(0: Cube, 1:  )
// Bit(s) 4 - 11  8: Terrain Type		
// Bit(s) 16 - 23 8: Rotation			
// Bit(s) 24 - 29 6: Visibility

// follow this order for fastest supersampling in a 3x3 box of sectors
// this set of defines is intended for terrain smoothing and any other algorithms that need to supersample to adjacent sectors. first [] access is for the 3x3 box of sectors. second [] access is for the sector data. 
// blocks will only be accessed at a max distance of one block from the border of the center sector. this allows for some optimizations in the access calculations when there is no chance of leaving the center sector along one or more axes.

// CTODO: update the naming conventions of the enums

// The various block types
enum SCUBETYPE
{
	SCUBE_CUBE = 0,
	SCUBE_THREEQUARTERCUBE,
	SCUBE_HALFCUBE,
	SCUBE_HALFCUBETRIANGLE,
	SCUBE_HALFCUBEWEDGE,
	SCUBE_QUARTERCUBEPYRAMID,
	SCUBE_QUARTERCUBECORNERPYRAMID,
	SCUBE_QUARTERCUBETRIANGLE,
	SCUBE_QUARTERCUBESTRANGETRIANGLE
};

// PERFORMANCE: Order needs to determine the order that the renderer draws
// Note- Air must be zero. Many optimizations are done with this fact.
// The various block materials
enum TERRATYPE
{
	TERRA_AIR = 0,
	TERRA_OLDAIR,
	TERRA_WATER,
	TERRA_LAVA,
	TERRA_SNOW,
	TERRA_STONE,
	TERRA_GRAVEL,
	TERRA_GRASS,
	TERRA_DIRT,
	TERRA_CLAY,
	TERRA_SAND,
	TERRA_TREE,
	TERRA_LEAVES,
	TERRA_COALORE,
	TERRA_COPPERORE,
	TERRA_GOLDORE,
	TERRA_IRONORE,
	TERRA_SILVERORE,
	TERRA_TINORE,
	TERRA_BRONZE,
	TERRA_COPPER,
	TERRA_GOLD,
	TERRA_IRON,
	TERRA_LEAD,
	TERRA_MITHRIL,
	TERRA_TIN,
	TERRA_ROUGHBRONZE,
	TERRA_ROUGHIRON,
	TERRA_BOARDS,
	TERRA_BRICK,
	TERRA_STONEBRICK,
	TERRA_THATCH,
	TERRA_DIRTROAD,
	TERRATYPE_COUNT		// must be last
};

enum TREEPART
{
	TREE_STALK = 0,
	TREE_LIMB,
	TREE_ROOT,
	TREE_EXTENSION	// used for widening of trees
};

// Visibility bits
enum VISTYPE
{
	VIS_LEFT	= 1 << 0,
	VIS_RIGHT	= 1 << 1,
	VIS_FRONT	= 1 << 2,
	VIS_BACK	= 1 << 3,
	VIS_TOP		= 1 << 4,
	VIS_BOTTOM	= 1 << 5
};

// CTODO: would it be faster to always return and take ints?
// CTODO: change these to work with references

void		SetSCubeType( uint32* cubeData, uint32 type );
void		SetSCubeTerrainType( uint32* cubeData, uint32 terrainType );
void		SetSCubeRotation( uint32* cubeData, uint32 rotation );
void		SetSCubeSpin( uint32* cubeData, uint32 spin );
void		SetSCubeVisibility( uint32* cubeData, uint32 visability );
void		SetSCubeHealth( uint32* cubeData, uint32 health );

SCUBETYPE	GetSCubeType( uint32 sCube );
TERRATYPE	GetSCubeTerrainType( uint32 sCube );
uint8		GetSCubeRotation( uint32 sCube );
uint8		GetSCubeSpin( uint32 sCube );
uint8		GetSCubeVisibility( uint32 sCube );
uint8		GetSCubeHealth( uint32 sCube );

void		ClearSCubeType( uint32* sCube );
void		ClearSCubeTerrainType( uint32* sCube );
void		ClearSCubeRotation( uint32* sCube );
void		ClearSCubeSpin( uint32* sCube );
void		ClearSCubeVisibility( uint32* sCube );
void		ClearSCubeHealth( uint32* sCube );

bool		IsTerrainTypeABuildingMaterial( TERRATYPE type );

uint8		ComputeTerraFlavor( int32 x, int32 y, int32 z );

#include "terrainhelper.inl"