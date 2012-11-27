//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\nstdlib\vector.h"
#include "world.h"
#include "astar3d.h"

class UPoint3D;
class UPoint2D;

// STODO: possibly allow for pathing to optionally work off a heightmap

class AIManager
{
public:
	AIManager( );
	~AIManager( );

	// can perform many searches at a time
	bool GetPath( const UPoint3D& start, const UPoint3D& goal, Vector<UPoint3D>& path, uint32 pathingHeight, float32 heuristicCostMultiplier = 1.00f );
	bool GetPathUnknownZ( const UPoint3D& start, const UPoint2D& goal, Vector<UPoint3D>& path, uint32 pathingHeight, float32 heuristicCostMultiplier = 1.00f );
	
	// can perform one search at a time
	bool GetShortRangePath( const UPoint3D& start, const UPoint3D& goal, Vector<UPoint3D>& path, uint32 pathingHeight, float32 heuristicCostMultiplier = 1.00f );
	bool GetShortRangePathUnknownZ( const UPoint3D& start, const UPoint2D& goal, Vector<UPoint3D>& path, uint32 pathingHeight, float32 heuristicCostMultiplier = 1.00f );
	
private:
	typedef bool (AIManager::*GetPathMethod)( const UPoint3D&, const UPoint3D&, Vector<UPoint3D>&, uint32, float32 );

	bool GetPathFindZ_( GetPathMethod method, const UPoint3D& start, const UPoint2D& goal, Vector<UPoint3D>& path, uint32 pathingHeight, float32 heuristicCostMultiplier );

	AStar3D shortRangeSearcher_;
	//AStar3D mediumRangeSearcher_;
	//AStar3D longRangeSearcher_;
};

// Accessor
FORCE_INLINE AIManager*
GetAIManager( )
{
	extern AIManager* _g_AIManagerInstance;
	Assert( _g_AIManagerInstance );
	return _g_AIManagerInstance;
}

#include "aimanager.inl"