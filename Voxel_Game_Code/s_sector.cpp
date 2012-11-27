//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "s_sector.h"
#include "public\nstdlib\vector3d.h"
#include "public\newport lib\trianglesoup.h"
#include "public\iengine.h"
#include "public\isocket.h"
#include "public\istreamfile.h"
#include "terrainhelper.h"
#include "scubegeom.h"

S_Sector::S_Sector( )
{
	compressedData_			= NULL;
	compressedDataSize_		= 0;
	data_					= NULL;
	origin_.Init();
	collide_				= NULL;
	isCompressedDataDirty_	= false;
	isModifying_			= false;
}

S_Sector::~S_Sector( )
{
	Unlink();

	EngineAPI()->FreeUID( uid_ );

	SAFE_FREE( data_ );
	SAFE_FREE( compressedData_ );
	SAFE_RELEASE( collide_ );
}

//-----------------------------------------------------------------------------
// These methods assume you mean for this class to handle the memory
// (in that it will be freed by this class)
//-----------------------------------------------------------------------------
void
S_Sector::Init( Vector3D origin, uint32* data )
{
	Assert( data );

	// must call first
	Init_();

	origin_	= origin;
	data_	= data;

	// regenerate compressed data
	isCompressedDataDirty_ = true;
	PurgeData();

	// must call last
	CreateCollide_();
	UpdateCollide_();
}

void
S_Sector::Init( Vector3D origin, uint32* compressedData, uint32 compressedDataSize )
{
	Assert( compressedData && compressedDataSize );

	// must call first
	Init_();

	origin_					= origin;
	compressedData_			= compressedData;
	compressedDataSize_		= compressedDataSize;
	isCompressedDataDirty_	= false;

	// must call last
	CreateCollide_();
	UpdateCollide_();
}

bool
S_Sector::GetIsModifying( )
{
	return isModifying_;
}

uint32*
S_Sector::BeginModify( )
{
	Assert( !isModifying_ );

	if( !data_ )
	{
		UnpackData_();
	}

	isCompressedDataDirty_	= true;
	isModifying_			= true;

	return data_;
}

//-----------------------------------------------------------------------------
// These should only be called between BeginModify(~ ) and EndModify(~ )
//-----------------------------------------------------------------------------
void
S_Sector::RemoveSCube( Point3D location )
{
	Assert( isModifying_ );
	Assert( data_ );

	FAssert(); // CTODO:
}

uint32*
S_Sector::GetData( )
{
	Assert( isModifying_ );
	Assert( data_ );

	return data_;
}

void
S_Sector::EndModify( )
{
	if( !isModifying_ )
	{
		FAssert();
		return;
	}

	isModifying_			= false;
	doSerializeToNetwork_	= true;

	// recompress data
	PurgeData();

	// CTODO: add option to end modify without recomputing collide
	//        this will help speed up world gen
	UpdateCollide_();
}

// CTODO: handle water properly
void
S_Sector::ComputeVis (uint32** sectorDatas)
{
	bool doPurge = false;
	if (!data_)
	{
		UnpackData_ ();
		doPurge = true;
	}

	uint32 x;
	uint32 y;
	uint32 z;

	uint32 loc = 0;
	unsigned char sCubeTerraType;
	unsigned char sCubeType;
	unsigned char sCubeRot;
	unsigned char sCubeSpin;

	// STODO: BUG: visibility issue occuring when an scube should be visible but isnt since it is not directly adjacent to any air blocks
	for (z = 0; z < SECTOR_BLENGTH; ++z)
	{
		for (y = 0; y < SECTOR_BLENGTH; ++y)
		{
			for (x = 0; x < SECTOR_BLENGTH; ++x)
			{
				sCubeTerraType	= GetSCubeTerrainType (BOX_CENTER);
				sCubeType		= GetSCubeType (BOX_CENTER);
				sCubeRot		= GetSCubeRotation (BOX_CENTER);
				sCubeSpin		= GetSCubeSpin (BOX_CENTER);

				// STODO: water should act like air, cept for other water
				if (sCubeTerraType == TERRA_AIR ||
					//sCubeTerraType == TERRA_WATER ||
					sCubeType == SCUBE_QUARTERCUBESTRANGETRIANGLE)
				{
					SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
					SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
					SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
					SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
					SetSCubeVisibility (&BOX_UP, VIS_BOTTOM);
					SetSCubeVisibility (&BOX_DOWN, VIS_TOP);
				}
				// this checks if the there is air and
				else if (sCubeType != SCUBE_CUBE &&
						(GetSCubeTerrainType (BOX_LEFT) == TERRA_AIR || GetSCubeType (BOX_LEFT) != SCUBE_CUBE ||
							GetSCubeTerrainType (BOX_RIGHT) == TERRA_AIR || GetSCubeType (BOX_RIGHT) != SCUBE_CUBE ||
							GetSCubeTerrainType (BOX_BACK) == TERRA_AIR || GetSCubeType (BOX_BACK) != SCUBE_CUBE ||
							GetSCubeTerrainType (BOX_FRONT) == TERRA_AIR || GetSCubeType (BOX_FRONT) != SCUBE_CUBE ||
							GetSCubeTerrainType (BOX_UP) == TERRA_AIR || GetSCubeType (BOX_UP) != SCUBE_CUBE || 
							GetSCubeTerrainType (BOX_DOWN) == TERRA_AIR || GetSCubeType (BOX_DOWN) != SCUBE_CUBE) &&
							(GetSCubeTerrainType (BOX_LEFT_FRONT_DOWN) != TERRA_OLDAIR &&		GetSCubeTerrainType (BOX_FRONT_DOWN) != TERRA_OLDAIR &&	GetSCubeTerrainType (BOX_RIGHT_FRONT_DOWN) != TERRA_OLDAIR &&
							GetSCubeTerrainType (BOX_LEFT_DOWN) != TERRA_OLDAIR &&		GetSCubeTerrainType (BOX_DOWN) != TERRA_OLDAIR &&			GetSCubeTerrainType (BOX_RIGHT_DOWN) != TERRA_OLDAIR &&
							GetSCubeTerrainType (BOX_LEFT_BACK_DOWN) != TERRA_OLDAIR &&	GetSCubeTerrainType (BOX_BACK_DOWN) != TERRA_OLDAIR &&	GetSCubeTerrainType (BOX_RIGHT_BACK_DOWN) != TERRA_OLDAIR &&
							GetSCubeTerrainType (BOX_LEFT_FRONT) != TERRA_OLDAIR &&		GetSCubeTerrainType (BOX_FRONT) != TERRA_OLDAIR &&		GetSCubeTerrainType (BOX_RIGHT_FRONT) != TERRA_OLDAIR &&
							GetSCubeTerrainType (BOX_LEFT) != TERRA_OLDAIR &&				GetSCubeTerrainType (BOX_CENTER) != TERRA_OLDAIR &&		GetSCubeTerrainType (BOX_RIGHT) != TERRA_OLDAIR &&
							GetSCubeTerrainType (BOX_LEFT_BACK) != TERRA_OLDAIR &&		GetSCubeTerrainType (BOX_BACK) != TERRA_OLDAIR &&			GetSCubeTerrainType (BOX_RIGHT_BACK) != TERRA_OLDAIR &&
							GetSCubeTerrainType (BOX_LEFT_FRONT_UP) != TERRA_OLDAIR &&	GetSCubeTerrainType (BOX_FRONT_UP) != TERRA_OLDAIR &&		GetSCubeTerrainType (BOX_RIGHT_FRONT_UP) != TERRA_OLDAIR &&
							GetSCubeTerrainType (BOX_LEFT_UP) != TERRA_OLDAIR &&			GetSCubeTerrainType (BOX_UP) != TERRA_OLDAIR &&			GetSCubeTerrainType (BOX_RIGHT_UP) != TERRA_OLDAIR &&
							GetSCubeTerrainType (BOX_LEFT_BACK_UP) != TERRA_OLDAIR &&		GetSCubeTerrainType (BOX_BACK_UP) != TERRA_OLDAIR &&		GetSCubeTerrainType (BOX_RIGHT_BACK_UP) != TERRA_OLDAIR))
				{
					if (sCubeType == SCUBE_QUARTERCUBEPYRAMID || sCubeType == SCUBE_QUARTERCUBECORNERPYRAMID || 
						sCubeType == SCUBE_HALFCUBE || sCubeType == SCUBE_HALFCUBEWEDGE)
					{
						if (sCubeRot != 2 || GetSCubeType (BOX_LEFT) != SCUBE_CUBE)
							SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
						if (sCubeRot != 3 || GetSCubeType (BOX_RIGHT) != SCUBE_CUBE)
							SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
						if (sCubeRot != 5 || GetSCubeType (BOX_BACK) != SCUBE_CUBE)
							SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
						if (sCubeRot != 4 || GetSCubeType (BOX_FRONT) != SCUBE_CUBE)
							SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
						if (sCubeRot != 1 || GetSCubeType (BOX_UP) != SCUBE_CUBE)
							SetSCubeVisibility (&BOX_UP, VIS_BOTTOM);
						if (sCubeRot != 0 || GetSCubeType (BOX_DOWN) != SCUBE_CUBE)
							SetSCubeVisibility (&BOX_DOWN, VIS_TOP);
					}
					else if (sCubeRot > 0) // prevents visibility bugs from occuring since rotations aren't currently handled for the following scubes.
					{
						SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
						SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
						SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
						SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
						SetSCubeVisibility (&BOX_UP, VIS_BOTTOM);
						SetSCubeVisibility (&BOX_DOWN, VIS_TOP);
					}
					else if (sCubeType == SCUBE_HALFCUBETRIANGLE)
					{
						// STODO: the three quarter cube cases are not working properly
						switch (sCubeSpin)
						{
						case 0:
							if ((GetSCubeType (BOX_LEFT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_LEFT) != 0) &&
								/*(GetSCubeType (BOX_LEFT) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_LEFT) != 3) && */
								(GetSCubeType (BOX_LEFT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_RIGHT) != 1))
							{
								SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
							}
							if ((GetSCubeType (BOX_RIGHT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_RIGHT) != 0) &&
								/*(GetSCubeType (BOX_RIGHT) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_RIGHT) != 0) && */
								(GetSCubeType (BOX_RIGHT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_RIGHT) != 0))
							{
								SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
							}
							if (GetSCubeType (BOX_BACK) != SCUBE_CUBE)
							{
								SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
							}
							SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
							break;
						case 1:
							SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
							if (GetSCubeType (BOX_RIGHT) != SCUBE_CUBE)
							{
								SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
							}
							if ((GetSCubeType (BOX_BACK) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_BACK) != 1) &&
								/*(GetSCubeType (BOX_BACK) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_BACK) != 0) && */
								(GetSCubeType (BOX_BACK) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_BACK) != 2))
							{
								SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
							}
							if ((GetSCubeType (BOX_FRONT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_FRONT) != 1) &&
								/*(GetSCubeType (BOX_FRONT) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_FRONT) != 1) && */
								(GetSCubeType (BOX_FRONT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_FRONT) != 1))
							{
								SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
							}
							break;
						case 2:
							if ((GetSCubeType (BOX_LEFT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_LEFT) != 2) &&
								/*(GetSCubeType (BOX_LEFT) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_LEFT) != 2) && */
								(GetSCubeType (BOX_LEFT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_LEFT) != 2))
							{
								SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
							}
							if ((GetSCubeType (BOX_RIGHT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_RIGHT) != 2) &&
								/*(GetSCubeType (BOX_RIGHT) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_RIGHT) != 0) && */
								(GetSCubeType (BOX_RIGHT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_RIGHT) != 3))
							{
								SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
							}
							SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
							if (GetSCubeType (BOX_FRONT) != SCUBE_CUBE)
							{
								SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
							}
							break;
						case 3:
							if (GetSCubeType (BOX_LEFT) != SCUBE_CUBE)
							{
								SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
							}
							SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
							if ((GetSCubeType (BOX_BACK) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_BACK) != 3) &&
								/*(GetSCubeType (BOX_BACK) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_BACK) != 3) && */
								(GetSCubeType (BOX_BACK) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_BACK) != 3))
							{
								SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
							}
							if ((GetSCubeType (BOX_FRONT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_FRONT) != 3) &&
								/*(GetSCubeType (BOX_FRONT) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_FRONT) != 1) && */
								(GetSCubeType (BOX_FRONT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_FRONT) != 0))
							{
								SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
							}
							break;
						}
						SetSCubeVisibility (&BOX_UP, VIS_BOTTOM);
						if (GetSCubeType (BOX_DOWN) != SCUBE_CUBE)
							SetSCubeVisibility (&BOX_DOWN, VIS_TOP);
					}
					else if (sCubeType == SCUBE_THREEQUARTERCUBE)
					{
						switch (sCubeSpin)
						{
						case 0:
							if ((GetSCubeType (BOX_LEFT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_LEFT) != 0) &&
								(GetSCubeType (BOX_LEFT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_LEFT) != 1))
							{
								SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
							}
							if (GetSCubeType (BOX_RIGHT) != SCUBE_CUBE)
							{
								SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
							}
							if (GetSCubeType (BOX_BACK) != SCUBE_CUBE)
							{
								SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
							}
							if ((GetSCubeType (BOX_FRONT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_FRONT) != 1) &&
								(GetSCubeType (BOX_FRONT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_FRONT) != 1))
							{
								SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
							}
							break;
						case 1:
							if ((GetSCubeType (BOX_LEFT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_LEFT) != 2) &&
								(GetSCubeType (BOX_LEFT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_LEFT) != 2))
							{
								SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
							}
							if (GetSCubeType (BOX_RIGHT) != SCUBE_CUBE)
							{
								SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
							}
							if ((GetSCubeType (BOX_BACK) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_BACK) != 1) &&
								(GetSCubeType (BOX_BACK) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_BACK) != 2))
							{
								SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
							}
							if (GetSCubeType (BOX_FRONT) != SCUBE_CUBE)
							{
								SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
							}
							break;
						case 2:
							if (GetSCubeType (BOX_LEFT) != SCUBE_CUBE)
							{
								SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
							}
							if ((GetSCubeType (BOX_RIGHT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_RIGHT) != 2) &&
								(GetSCubeType (BOX_RIGHT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_RIGHT) != 3))
							{
								SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
							}
							if ((GetSCubeType (BOX_BACK) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_BACK) != 3) &&
								(GetSCubeType (BOX_BACK) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_BACK) != 3))
							{
								SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
							}
							if (GetSCubeType (BOX_FRONT) != SCUBE_CUBE)
							{
								SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
							}
							break;
						case 3:
							if (GetSCubeType (BOX_LEFT) != SCUBE_CUBE)
							{
								SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
							}
							if ((GetSCubeType (BOX_RIGHT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_RIGHT) != 0) &&
								(GetSCubeType (BOX_RIGHT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_RIGHT) != 0))
							{
								SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
							}
							if (GetSCubeType (BOX_BACK) != SCUBE_CUBE)
							{
								SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
							}
							if ((GetSCubeType (BOX_FRONT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_FRONT) != 3) &&
								(GetSCubeType (BOX_FRONT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_FRONT) != 0))
							{
								SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
							}
							break;
						}
						if (GetSCubeType (BOX_UP) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_UP) != (sCubeSpin + 1) % 4)
							SetSCubeVisibility (&BOX_UP, VIS_BOTTOM);
						if (GetSCubeType (BOX_DOWN) != SCUBE_CUBE)
							SetSCubeVisibility (&BOX_DOWN, VIS_TOP);
					}
					else if (sCubeType == SCUBE_QUARTERCUBETRIANGLE)
					{	
						// STODO: the three quarter cube cases are not working when a threequarter cube is surrounded by quartercubetriangles and other scubes with the same connection surfaces
						switch (sCubeSpin)
						{
						case 0:
							if ((GetSCubeType (BOX_LEFT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_LEFT) != 0) &&
								(GetSCubeType (BOX_LEFT) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_LEFT) != 3) &&
								(GetSCubeType (BOX_LEFT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_LEFT) != 1))
							{
								SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
							}
							SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
							if ((GetSCubeType (BOX_BACK) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_BACK) != 3) &&
								(GetSCubeType (BOX_BACK) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_BACK) != 3) &&
								(GetSCubeType (BOX_BACK) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_BACK) != 3))
							{
								SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
							}
							SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
							break;
						case 1:
							SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
							if ((GetSCubeType (BOX_RIGHT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_RIGHT) != 0) &&
								(GetSCubeType (BOX_RIGHT) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_RIGHT) != 0) &&
								(GetSCubeType (BOX_RIGHT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_RIGHT) != 0))
							{
								SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
							}
							if ((GetSCubeType (BOX_BACK) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_BACK) != 1) &&
								(GetSCubeType (BOX_BACK) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_BACK) != 0) &&
								(GetSCubeType (BOX_BACK) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_BACK) != 2))
							{
								SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
							}
							SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
							break;
						case 2:
							SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
							if ((GetSCubeType (BOX_RIGHT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_RIGHT) != 2) &&
								(GetSCubeType (BOX_RIGHT) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_RIGHT) != 1) &&
								(GetSCubeType (BOX_RIGHT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_RIGHT) != 3))
							{
								SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
							}
							SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
							if ((GetSCubeType (BOX_FRONT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_FRONT) != 1) &&
								(GetSCubeType (BOX_FRONT) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_FRONT) != 1) &&
								(GetSCubeType (BOX_FRONT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_FRONT) != 1))
							{
								SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
							}
							break;
						case 3:
							if ((GetSCubeType (BOX_LEFT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_LEFT) != 2) &&
								(GetSCubeType (BOX_LEFT) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_LEFT) != 2) &&
								(GetSCubeType (BOX_LEFT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_LEFT) != 2))
							{
								SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
							}
							SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
							SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
							if ((GetSCubeType (BOX_FRONT) != SCUBE_HALFCUBETRIANGLE || GetSCubeSpin (BOX_FRONT) != 3) &&
								(GetSCubeType (BOX_FRONT) != SCUBE_THREEQUARTERCUBE || GetSCubeSpin (BOX_FRONT) != 2) &&
								(GetSCubeType (BOX_FRONT) != SCUBE_QUARTERCUBETRIANGLE || GetSCubeSpin (BOX_FRONT) != 0))
							{
								SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
							}
							break;
						}
						SetSCubeVisibility (&BOX_UP, VIS_BOTTOM);
						SetSCubeVisibility (&BOX_DOWN, VIS_TOP);
					}
					else if (sCubeType == SCUBE_QUARTERCUBEPYRAMID || sCubeType == SCUBE_QUARTERCUBECORNERPYRAMID || 
						sCubeType == SCUBE_HALFCUBE || sCubeType == SCUBE_HALFCUBEWEDGE)
					{
						if (sCubeRot != 2 || GetSCubeType (BOX_LEFT) != SCUBE_CUBE)
							SetSCubeVisibility (&BOX_LEFT, VIS_RIGHT);
						if (sCubeRot != 3 || GetSCubeType (BOX_RIGHT) != SCUBE_CUBE)
							SetSCubeVisibility (&BOX_RIGHT, VIS_LEFT);
						if (sCubeRot != 5 || GetSCubeType (BOX_BACK) != SCUBE_CUBE)
							SetSCubeVisibility (&BOX_BACK, VIS_FRONT);
						if (sCubeRot != 4 || GetSCubeType (BOX_FRONT) != SCUBE_CUBE)
							SetSCubeVisibility (&BOX_FRONT, VIS_BACK);
						if (sCubeRot != 1 || GetSCubeType (BOX_UP) != SCUBE_CUBE)
							SetSCubeVisibility (&BOX_UP, VIS_BOTTOM);
						if (sCubeRot != 0 || GetSCubeType (BOX_DOWN) != SCUBE_CUBE)
							SetSCubeVisibility (&BOX_DOWN, VIS_TOP);
					}
				}
			}
		}
	}

	isCompressedDataDirty_ = true;
	if (doPurge)
	{
		PurgeData ();
	}
}

// this is to make the AABB fit cleanly inside an octree sector
// CTODO: fuck this shit
#define AABBSHRINKAMOUNT 0.01f

uint32*
S_Sector::GetCompressedData( )
{
	PurgeData();

	return compressedData_;
}

uint32
S_Sector::GetCompressedDataSize( )
{
	PurgeData();

	return compressedDataSize_;
}

//-----------------------------------------------------------------------------
// Free unpacked data to save memory, repacking if necessary
//-----------------------------------------------------------------------------
void
S_Sector::PurgeData( )
{
	Assert( !isModifying_ );

	if( !data_ )
	{
		return;
	}

	if( isCompressedDataDirty_ )
	{
		PackData_();
	}

	free( data_ );
	data_ = NULL;
}

//-----------------------------------------------------------------------------
//
// Inherited from 'IServerEntity'
//
//-----------------------------------------------------------------------------
const char*
S_Sector::GetEntityClassName( )
{
	return "sector";
}

uint32
S_Sector::GetUID( )
{
	return uid_;
}

void
S_Sector::Update( )
{
	;
}

void
S_Sector::Think( )
{
	FAssert();
}

void
S_Sector::SetNextThink( float32 deltaTime )
{
	FAssert();
}

float32
S_Sector::GetNextThink( )
{
	FAssert();
	return 0.0f;
}

bool
S_Sector::ShouldSerializeToNetwork( )
{
	return doSerializeToNetwork_;
}

void
S_Sector::SerializeToNetwork( SocketMessage* socketMessage )
{
	socketMessage->Write( GetUID() );
	socketMessage->Write( origin_ );

	if( collide_ )
	{
		socketMessage->Write( collide_->GetExtents().mins );
		socketMessage->Write( collide_->GetExtents().maxs );
	}
	else
	{
		socketMessage->Write( Vector3D( 0.0f, 0.0f, 0.0f ) );
		socketMessage->Write( Vector3D( 0.0f, 0.0f, 0.0f ) );
	}

	socketMessage->Write( (void*)vertexCounts_, TERRAINTYPE_COUNT * sizeof( uint32 ) );
	socketMessage->Write( (void*)indexCounts_, TERRAINTYPE_COUNT * sizeof( uint32 ) );
	socketMessage->Write( GetCompressedDataSize() );
	socketMessage->Write( (void*)GetCompressedData(), GetCompressedDataSize() );

	doSerializeToNetwork_ = false;
}

void
S_Sector::UnserializeFromNetwork( SocketMessage* socketMessage )
{
	;
}

void
S_Sector::SerializeToHDD( IStreamFile* file )
{
	file->Write( origin_ );
	file->Write( GetCompressedDataSize() );
	file->Write( GetCompressedData(), sizeof( uint32 ), GetCompressedDataSize() / 4 );
}

void
S_Sector::UnserializeFromHDD( IStreamFile* file )
{
	Vector3D origin;
	file->Read( origin );

	uint32 compressedDataSize;
	file->Read( compressedDataSize );

	uint32* compressedData = (uint32*)malloc( compressedDataSize );
	file->Read( compressedData, sizeof( uint32 ), compressedDataSize / sizeof( uint32 ) );

	Init( origin, compressedData, compressedDataSize );
}

void
S_Sector::Spawn( )
{
	collide_->SetIsSimulated( true );
	// make sure our collide got linked, and only once, to the Physics System octree
	AssertMsg( collide_->GetBacklinkCount() == 1, "S_Sector collide linked multiple times to Physics System octree." );
}

void
S_Sector::Despawn( )
{
	collide_->SetIsSimulated( false );
}

void
S_Sector::SetTranslation( const Vector3D& origin )
{
	;
}

Vector3D
S_Sector::GetTranslation( )
{
	return origin_;
}

float32
S_Sector::GetTranslationX( )
{
	return origin_.x;
}

float32
S_Sector::GetTranslationY( )
{
	return origin_.y;
}

float32
S_Sector::GetTranslationZ( )
{
	return origin_.z;
}

void
S_Sector::SetRotationYawPitchRoll( float32 yaw, float32 pitch, float32 roll )
{
	;
}

void
S_Sector::RotateX( float32 x )
{
	;
}

void
S_Sector::RotateY( float32 y )
{
	;
}

void
S_Sector::RotateZ( float32 z )
{
	;
}

void
S_Sector::AddBacklink( ServerEntityNode* node )
{
	AssertMsg( !backlinks_.GetCount(), "S_Sector being linked multiple times to world octree." );
	backlinks_.InsertBack( node );
}

ServerEntityNode*
S_Sector::GetBacklink( uint32 idx )
{
	return backlinks_[ idx ];
}

uint32
S_Sector::GetBacklinkCount( )
{
	return backlinks_.GetCount();
}

void
S_Sector::Unlink( )
{
	while( backlinks_.GetCount() )
	{
		backlinks_.PopBack()->RemoveObject( this );
	}
}

void
S_Sector::SetMesh( IMesh* mesh )
{
	FAssert();
}

IMesh*
S_Sector::GetMesh( )
{
	FAssert();
	return NULL;
}

void
S_Sector::SetCollide( ICollide* collide )
{
	FAssert();
}

ICollide*
S_Sector::GetCollide( )
{
	Assert( collide_ );
	return collide_;
}

//-----------------------------------------------------------------------------
// Prevents duplicate constructor code
//-----------------------------------------------------------------------------
void
S_Sector::Init_( )
{
	compressedData_			= NULL;
	compressedDataSize_		= 0;
	data_					= NULL;
	origin_.Init();
	collide_				= NULL;
	isCompressedDataDirty_	= false;
	isModifying_			= false;
	doSerializeToNetwork_	= false;

	uid_ = EngineAPI()->NewUID();
}

void
S_Sector::CreateCollide_( )
{
	collide_ = new TriangleSoup;

	// compute initial extents
	Vector3D mins(
		origin_.x - (SECTOR_LENGTH / 2.0f),
		origin_.y - (SECTOR_LENGTH / 2.0f),
		origin_.z - (SECTOR_LENGTH / 2.0f) );

	Vector3D maxs(
		origin_.x + (SECTOR_LENGTH / 2.0f),
		origin_.y + (SECTOR_LENGTH / 2.0f),
		origin_.z + (SECTOR_LENGTH / 2.0f) );

	// shrink the extents (see comment for 'EXTENT_SHRINK_AMOUNT')
	mins += EXTENT_SHRINK_AMOUNT;
	maxs -= EXTENT_SHRINK_AMOUNT;

	collide_->SetExtents( Extents3D( mins, maxs ) );
}

void
S_Sector::UpdateCollide_( )
{
	// make sure compressed data is up to date
	PackData_();

	// compute vertex and index counts
	ZeroMemory( vertexCounts_, sizeof( uint32 ) * TERRAINTYPE_COUNT );
	ZeroMemory( indexCounts_, sizeof( uint32 ) * TERRAINTYPE_COUNT );

	GetSCubeGeom()->ComputeVertexAndIndexCounts(
		compressedData_,
		vertexCounts_,
		indexCounts_ );

	// compute total vertex and index count
	uint32 vertexCount	= 0;
	uint32 indexCount	= 0;
	for( uint32 i = 0; i < TERRAINTYPE_COUNT; ++i )
	{
		vertexCount	+= vertexCounts_[ i ];
		indexCount	+= indexCounts_[ i ];
	}

	// update collide
	float32* collideMemory = collide_->BeginModify( indexCount / 3 );

	// CTODO: remove this once we put in all the scubes
	//
	//       set all points to the origin, because we dont fill the structure
	//       completely in and it would mess up extent calculation and other stuff
	for( uint32 i = 0; i < indexCount * 3; i += 3 )
	{
		collideMemory[ i ] = origin_.x;
		collideMemory[ i + 1 ] = origin_.y;
		collideMemory[ i + 2 ] = origin_.z;
	}

	GetSCubeGeom()->GenerateCollide(
		collideMemory,
		compressedData_,
		origin_ );

	// supply the extents if we wrote no triangles
	//
	// this is because in the case that no triangles are written we cannot
	// compute proper extents and the sectors will not be properly registered
	// to the octree (which should only be done once to save time)
	if( indexCount == 0 )
	{
		Extents3D extents;

		extents.mins.x = origin_.x - (SECTOR_LENGTH / 2.0f) + EXTENT_SHRINK_AMOUNT;
		extents.mins.y = origin_.y - (SECTOR_LENGTH / 2.0f) + EXTENT_SHRINK_AMOUNT;
		extents.mins.z = origin_.z - (SECTOR_LENGTH / 2.0f) + EXTENT_SHRINK_AMOUNT;

		extents.maxs.x = origin_.x + (SECTOR_LENGTH / 2.0f) - EXTENT_SHRINK_AMOUNT;
		extents.maxs.y = origin_.y + (SECTOR_LENGTH / 2.0f) - EXTENT_SHRINK_AMOUNT;
		extents.maxs.z = origin_.z + (SECTOR_LENGTH / 2.0f) - EXTENT_SHRINK_AMOUNT;

		collide_->EndModify( extents );
	}
	// compute the extents like normal
	else
	{
		collide_->EndModify( );

		// shrink the extents (see comment for 'EXTENT_SHRINK_AMOUNT')
		Extents3D extents = collide_->GetExtents( );

		extents.mins.x += EXTENT_SHRINK_AMOUNT;
		extents.mins.y += EXTENT_SHRINK_AMOUNT;
		extents.mins.z += EXTENT_SHRINK_AMOUNT;

		extents.maxs.x -= EXTENT_SHRINK_AMOUNT;
		extents.maxs.y -= EXTENT_SHRINK_AMOUNT;
		extents.maxs.z -= EXTENT_SHRINK_AMOUNT;

		collide_->SetExtents( extents );
	}
}

void
S_Sector::PackData_( )
{
	// attempted to repack during a modify, this assert is to prevent unexpected behavior
	Assert( !isModifying_ );

	if( !data_ || isModifying_ )
	{
		return;
	}

	// blow away any old compressed data
	if( compressedData_ )
	{
		free( compressedData_ );
		compressedDataSize_ = 0;
	}

	// compress the data
	// CTODO: get this buffer from somewhere so we don't have to recreate it every time...
	// (in the absolute worst case, our algorithm will double the data size so allocate allot of tmp memory just in case)
	uint32*	compressedDataBuf			= (uint32*)malloc( SECTOR_BVOLUME * 2 * sizeof( uint32 ) );
	uint32	compressedDataSCubeCount	= 0;
	uint32	dataPos						= 0;

	while( dataPos < SECTOR_BVOLUME )
	{
		uint32 sCube = data_[ dataPos ];
		uint32 sCubeCount = 0;

		while( dataPos < SECTOR_BVOLUME && data_[ dataPos ] == sCube )
		{
			++dataPos;
			++sCubeCount;
		}
		
		compressedDataBuf[ compressedDataSCubeCount++ ] = sCube;
		compressedDataBuf[ compressedDataSCubeCount++ ] = sCubeCount;
	}

	// update the compressed data size
	compressedDataSize_ = compressedDataSCubeCount * sizeof( uint32 );

	// copy the data from our buffer to it's final location
	compressedData_ = (uint32*)malloc( compressedDataSize_ );
	N_CopyMemory( compressedDataBuf, compressedData_, compressedDataSize_ );

	isCompressedDataDirty_ = false;

	// cleanup
	free( compressedDataBuf );

#if 0 // SPEW: set to spew the packed data
	char convertBuff[ 12 ];
	SPEW( "Sector::PackData(~): Begin Packed Sector Data\n" );
	SPEW( "Origin( x, y, z ): " );
	SPEW( ConvertToString( oX_, convertBuff ) );
	SPEW( ", " );
	SPEW( ConvertToString( oY_, convertBuff ) );
	SPEW( ", " );
	SPEW( ConvertToString( oZ_, convertBuff ) );
	SPEW( "\nSize: " );
	SPEW( ConvertToString( compressedDataSize_, convertBuff ) );
	SPEW( "\n" );
	for( uint32 i = 0; i < compressedDataSize_ / sizeof( uint32 ); )
	{
		SPEW( "Block: " );
		SPEW( ConvertToString( compressedData_[ i++ ], convertBuff ) );
		SPEW( "( " );
		SPEW( ConvertToString( compressedData_[ i++ ], convertBuff ) );
		SPEW( " count )" );
		SPEW( "\n" );
	}
	SPEW( "Sector::PackData(~): End Packed Sector Data\n\n" );
#endif
}

void
S_Sector::UnpackData_( )
{
	if( data_ != NULL )
	{
		return;
	}

	uint32 sCubesWritten		= 0;
	uint32 compressedDataPos	= 0;
	uint32 sCube;
	uint32 copies;

	data_ = (uint32*)malloc( SECTOR_BVOLUME * sizeof( uint32 ) );
	Assert( data_ );

	while( compressedDataPos < compressedDataSize_ / 4 )
	{
		// these always come in pairs
		sCube = compressedData_[ compressedDataPos++ ];
		copies = compressedData_[ compressedDataPos++ ];

		while( copies )
		{
			data_[ sCubesWritten++ ] = sCube;
			--copies;
		}
	}

	Assert( sCubesWritten <= SECTOR_BVOLUME );
}