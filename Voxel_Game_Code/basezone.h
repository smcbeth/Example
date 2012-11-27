//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "basearea.h"

class RoadNode;
class ZoneRoadGenerator;
class BaseBuilding;
enum BuildingSize;
enum Direction;

enum HazardType
{
	HAZARD_SLOPE = 0,
	HAZARD_WATER,
	HAZARD_LAVA,
	HAZARD_MULTI
};

class Hazard
{
public:
	UPoint3D min;
	UPoint3D max;
	HazardType hazardType;
};

class BaseZone: public BaseArea
{
public:
					BaseZone( uint32 xMin, uint32 yMin );
	virtual			~BaseZone( );

	virtual bool	PlaceBuildings( );

	void			Build( );

	uint32			GetHeightAtPoint( uint32 x, uint32 y );
	uint32			GetAverageHeightWithinBounds( const UPoint3D& min, const UPoint3D& max );
	bool			IsBuildingBoundsPlaceable( const UPoint3D& bMin, const UPoint3D& bMax );

	bool			DoesPointIntersectWithBuildingBounds( const UPoint3D& point );

protected:
	virtual void	LevelTerrain_( );
	virtual void	GenerateHazardExtents_( );

	bool			AttemptToPlaceBuilding_( BaseBuilding* building, BuildingSize size, uint32 numResidents, uint32 extentsRadius, bool attemptToCenterBuilding );
	bool			AttemptToPlaceBuildingConnectedToRoad_( BaseBuilding* building, BuildingSize size, uint32 numResidents, uint32 extentsRadius, bool attemptToCenterBuilding, uint32 minDistFromRoad, uint32 maxDistFromRoad );
	
	void			BaseLevelTerrain_( float32 maxSmoothingFactor, uint32 aboveGroundHeightLimit );
	void			GenerateSlopeMap_( float32* slopeMap, uint32 lengthX, uint32 lengthY ); // STODO: merge w/ world gen
	void			GenerateRevisedHeightMap_( uint32 xMin, uint32 yMin, uint32 heightOffset = 0 ); // STODO: merge w/ world gen
	void			BaseGenerateHazardExtents_( float32 slopeThreshold );
	
public:
	const Hazard&	GetHazard( uint32 idx );
	BaseBuilding*	GetBuilding( uint32 idx );
	uint32			GetNumBuildings( );
	uint32			GetNumHazards( );
	uint32			GetTerraHeightAtLoc( uint32 x, uint32 y );
	// uint32 GetPopulation (); STODO:

	void			AddEntrance( const UPoint3D& point );
	uint32			GetNumEntrances( );
	const UPoint3D& GetEntrance( uint32 index );
	void			AddRoadNode( RoadNode* road );
	uint32			GetNumRoadNodes( );
	RoadNode&		GetRoadNode( uint32 index );

protected:
	Vector<UPoint3D> entrances_; // points on exterior of bounds that can be connected to by roads. entrances_[0] is main entrance.
	Vector<BaseBuilding*> buildings_;
	Vector<RoadNode*> connectingRoadNodeForBuildings_;
	Vector<Hazard> hazards_;
	Vector<RoadNode*> roadNodes_;
	ZoneRoadGenerator* roadGenerator_;
	float32* slopeMap_;
	uint16* heightMap_;
	uint16* revisedHeightMap_;
};