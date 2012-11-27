//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\nstdlib\vector.h"

class UPoint3D;
class RoadNode;
class BaseArea;
class BaseZone;
class AStar3D;

enum RoadType;

// STODO: need way of leveling terrain up to the road
// STODO: will need to get heighmap data always
// STODO: add methods to get road map information for building placement

// SNOW: allow for the placement of axis aligned(on the grid) only roads, or possibly diagonal only roads

class RoadGenerator
{
public:
	RoadGenerator( );
	virtual ~RoadGenerator( );

	// creates a stretch of road nodes between two points
	bool ConnectPoints( RoadType type, const UPoint3D& firstPoint, const UPoint3D& secondPoint, AStar3D* searcher = NULL, float32 aStarHeuristicCostMult = 1.0f );

	// creates a stretch of road nodes between two entrances of the two zones
	bool ConnectZones( RoadType type, BaseZone* firstZone, BaseZone* secondZone, float32 aStarHeuristicCostMult = 1.0f, const Vector<BaseArea*>* unpathableAreas = NULL );

	// similar to ConnectPoints but will link the connected roads
	bool ConnectRoadToRoad( RoadType type, RoadNode* firstRoadNode, RoadNode* secondRoadNode, float32 aStarHeuristicCostMult = 1.0f );

protected:

	const UPoint3D& FindClosestZoneEntranceToZone_( BaseZone* zoneWithEntrance, BaseZone* otherZone );

	UPoint3D GetNextPointInAStarPath_( const Vector<UPoint3D>& path, RoadNode* lastParent, uint32& pathIndex );

	bool ConditionallyInitSearcherObject_( AStar3D*& searcher, const UPoint3D& point, const UPoint3D& otherPoint );

};