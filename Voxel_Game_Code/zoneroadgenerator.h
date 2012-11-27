//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\nstdlib\vector.h"
#include "public\nstdlib\pair.h"
#include "basezone.h"
#include "roadgenerator.h"

class BaseBuilding;
class BaseArea;
class AStar3D;

// STODO: add methods to get road map information for building placement

// SNOW: allow for the placement of axis aligned(on the grid) only roads, or possibly diagonal only roads

class ZoneRoadGenerator : public RoadGenerator // SNOW: refactor and clean up once its working for adding a single road
{
public:
	ZoneRoadGenerator( BaseZone* zone );
	virtual ~ZoneRoadGenerator( );

	void AllocZoneRoadGrid( uint32 gridSpacing, uint32 maxOffset );
	void FreeZoneRoadGrid( );

	void ClearZoneRoadMap( );

	bool AddRoadToZone( RoadType type, bool doCreateEntrance, bool doAllowDiagonalPlacement, float32 aStarHeuristicCostMult = 1.0f ); // STODO: Note- the first road generated in a zone that connects to the exterior will probably have to determine the exterior connecting point of the zone
	bool ConnectPointsInZone( RoadType type, const UPoint3D& point, const UPoint3D& otherPoint, RoadNode* lastParent = NULL, AStar3D* searcher = NULL, float32 aStarHeuristicCostMult = 1.0f );

	void ConnectRoadToFrontDoor( RoadNode* roadNode, BaseBuilding* building, float32 aStarHeuristicCostMult = 1.0f ); // STODO:

	RoadNode* GetNodeAtLocationOnRoadMap( const UPoint3D& point );
	bool Does3DAreaIntersectWithRoads( const BaseArea& area );
	bool Does2DAreaIntersectWithRoads( uint32 minX, uint32 maxX, uint32 minY, uint32 maxY );
	bool GetAreaAdjacentToRoadNode( RoadNode* node, BaseArea& areaToBeInitialized, uint32 xLength, uint32 yLength, uint32 minDistanceFromRoad, uint32 maxDistanceFromRoad );
	//bool GetAreaBetweenRoadNodesAndCreateNewRoadNode ( RoadNode* parentNode, RoadNode* childNode,  BaseArea& areaToBeInitialized, RoadNode* newNode); // STODO: (maybe)
	RoadNode* GetNearestRoadNodeToArea( const BaseArea& area );
	//Pair< RoadNode*, RoadNode* >* GetNearestRoadSegmentToArea( const BaseArea& area );

	Vector< Pair< RoadNode*, RoadNode* > >* GetDataFromRoadMap( uint32 xPos, uint32 yPos );

private:

	struct GetAreaAdjacentToRoadNodeLoopData;

	bool GetAreaAdjacentToRoadNodeLoop_( GetAreaAdjacentToRoadNodeLoopData& data );

	bool AddRoadToZoneEntrancePrep_( RoadType type, UPoint3D*& currentPathingPoint, UPoint3D*& nextPathingPoint, uint32& xGridIndex, uint32& yGridIndex, uint32& xEndGridIndex, uint32& yEndGridIndex );
	bool AddRoadToZoneNormalPrep_( RoadType type, UPoint3D*& currentPathingPoint, UPoint3D*& nextPathingPoint, uint32& xGridIndex, uint32& yGridIndex, uint32& xEndGridIndex, uint32& yEndGridIndex, bool doAllowDiagonalPlacement );
	void ConditionallyModifyEndGridIndices_( uint32& xEndGridIndex, uint32& yEndGridIndex, uint32 x, uint32 y, RoadType type );

	bool GetListOfRoadSegmentsThatIntersectWithArea_( const BaseArea& area, Vector< Pair< RoadNode*, RoadNode* > >& listToFill ); // Note- this returned vector must be deleted manually if the return is not NULL
	
	BaseZone* zone_; // Note- it is important for the size of the zone to not change while the road grid is active
	
	UPoint3D* roadGrid_;
	bool* isConnectedElementsOfGrid_;
	bool isZoneRoadGridInitialized_;
	uint32 xGridElements_;
	uint32 yGridElements_;
	uint32 gridSpacing_;

	Vector< Pair< RoadNode*, RoadNode* > >** roadMap_; // 2D representation of zone containing NULL or Road* for every location on the map to allow easy building to road connectivity
};

FORCE_INLINE Vector< Pair< RoadNode*, RoadNode* > >*
ZoneRoadGenerator::GetDataFromRoadMap( uint32 xPos, uint32 yPos )
{
	Assert( xPos >= zone_->GetMin().x );
	Assert( yPos >= zone_->GetMin().y );
	Assert( xPos <= zone_->GetMax().x );
	Assert( yPos <= zone_->GetMax().y );

	return roadMap_[(xPos - zone_->GetMin().x) + (yPos - zone_->GetMin().y) * zone_->GetLengthX()];
}