//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: A* Search algorithm that can be performed on game terrain with modifiable obstacles
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\upoint3d.h"
#include "basearea.h"
#include "s_sector.h"

// Note- possibly have two options for traversed node storage. octree vs 3D array. the 3D array should be best for small search spaces so implement it first

// definitions for node pointer types
#define NODE_NULL 0
#define	NODE_CLOSED (AStar3DNode*)1

class AStar3D
{
public:
	AStar3D( bool doUseFastSqrt );
	~AStar3D( );

	void				AllocMem( uint32 xDiameter, uint32 yDiameter, uint32 zDiameter );
	void				ClearMem( );
	void				FreeMem( );
	//void Init( bool doTraverseAir, bool doTraverseWater, bool doTraverseLava, uint32 maxJumpDist, uint32 maxClimbDist, uint32 pathingWidth, uint32 pathingHeight );

	void				BeginModifyStartAndGoalSectors( const UPoint3D& start, const UPoint3D& goal );

	bool				Search( const UPoint3D& start, const UPoint3D& goal, Vector<UPoint3D>& path, uint32 pathingHeight, float32 heuristicCostMultiplier = 1.0f );

	// STODO: implement least resistance by simply increasing the hCost and gCost a fixed amount for going up or down a slope.
	bool				LeastResistanceSearch( const UPoint3D& start, const UPoint3D& goal, Vector<UPoint3D>& path, float32 pathOfLeastResistanceWeight, float32 heuristicCostMultiplier = 1.0f );

	void				AddPointToAvoid( const UPoint3D& point, float32 avoidanceBlockRadius, float32 heuristicCostMultiplier );
	void				ClearPointsToAvoid( );

	void				AddPathableArea( const BaseArea& area );
	void				ClearPathableAreas( );

	void				AddUnpathableArea( const BaseArea& area );
	void				ClearUnpathableAreas( );

private:

	class AStar3DNode;
	class PointToAvoid;

	float32				ComputeHCost_( const UPoint3D& currentPoint, const UPoint3D& goal, float32 heuristicCostMultiplier );
	
	AStar3DNode*		GetNodeInMap_( uint32 x, uint32 y, uint32 z );
	void				SetNodeInMap_( uint32 x, uint32 y, uint32 z, AStar3DNode* node );

	void				ConstructFinalPath_( AStar3DNode* goal, Vector<UPoint3D>& path );
	void				ConstructFinalPathBackwardsFast_( AStar3DNode* goal, Vector<UPoint3D>& path );

	bool				IsSearchPossible_( const UPoint3D& start, const UPoint3D& goal );
	bool				IsPointTraversible_( const UPoint3D& sampledPoint, uint32 pathingHeight );

	

	void				CloseOpenedSectors_( );

	uint32							xDiameter_;
	uint32							yDiameter_;
	uint32							zDiameter_;

	bool							doUseFastSqrt_;

	UPoint3D						searchSpaceMin_;
	AStar3DNode**					nodeMap_; // STODO: PERFORMANCE: clearing this buffer is expensive and if large will not fit on cache. however, this allows for O(1)  
										// access within the inner loop. smaller data structures will have O(log n) access at best, but might still be faster 
										// when calls to ClearMem and cache misses are factored in.
										// ideas: bitpacked array: 
										//	pros: reasonably fast access, small footprint, allows for resuable memory 
										//	cons: reordering data with every insertion
										// octree:
										//  pros: logn access, larger footprint, memory expands dynamically with search space
										//  cons: memory is not reusable. more memory usage than bitpacked array
										// STODO: look into some kind of map data structure like a hash map for this storage.
										// STODO: make a more directed post on game dev
	Vector< S_Sector* >				listOfOpenedSectors_;
	Vector< PointToAvoid >			pointsToAvoid_;
	Vector< BaseArea >				pathableAreas_; // NOTE: when there are no pathable areas, everything but unpathable areas are pathable. when there is one or more, only the pathable areas can be pathed.
	Vector< BaseArea >				unpathableAreas_;
	Vector< Vector< UPoint3D >* >	finalPaths_; // stores all paths generated until cleared
	
};

class AStar3D::PointToAvoid : public UPoint3D
{
public:
	PointToAvoid( const UPoint3D& point, float32 avoidanceBlockRadius, float32 heuristicCostMultiplier );
	float32 avoidanceBlockRadius;
	float32 heuristicCostMultiplier;
};

#include "astar3d.inl"
