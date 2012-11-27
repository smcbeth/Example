//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: A* Search algorithm that can be performed on a heightmap representing the surface of the terrain
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\math_.h"
#include "public\nstdlib\upoint3d.h"

class AStar2D
{
public:
	AStar2D( );
	~AStar2D( );

	void Init( bool doTraverseWater, uint32 maxJumpDist, uint32 maxClimbDist, uint32 pathingWidth, uint16* heightMap, uint32 heightMapXLength, uint32 heightMapYLength);

	bool Search( UPoint3D& start, UPoint3D& goal, uint32 maxSearchDist);
	bool WeightedSearch( UPoint3D& start, UPoint3D& goal, float pathOfLeastResistanceWeight, uint32 maxSearchDist );

private:

	uint32 heightMapXLength;
	uint32 heightMapYLength;
	uint16* heightMap_; // STODO: make a height map class that this A* search can use
};