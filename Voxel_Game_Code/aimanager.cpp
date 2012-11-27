//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "aimanager.h"
#include "public\nstdlib\upoint2d.h"
#include "public\nstdlib\upoint3d.h"
#include "terrainhelper.h"
#include "astar3d.h"

AIManager::AIManager()
	: shortRangeSearcher_( true )
{
	shortRangeSearcher_.AllocMem( 12, 12, 8 );
}

AIManager::~AIManager()
{
	;
}

bool
AIManager::GetPath( const UPoint3D& start, const UPoint3D& goal, Vector<UPoint3D>& path, uint32 pathingHeight, float32 heuristicCostMultiplier )
{
	Assert( path.GetCount() == 0 );

	AStar3D searcher( false );
	{
		uint32 xLength = Abs( int32(start.x) - int32(goal.x) ) * 2;
		uint32 yLength = Abs( int32(start.y) - int32(goal.y) ) * 2;
		uint32 zLength = Abs( int32(start.z) - int32(goal.z) ) * 2;

		const uint32 minLength = 5;

		searcher.AllocMem( Max( Max( xLength, Max( yLength / 2, zLength / 2 ) ), minLength ), 
						   Max( Max( yLength, Max( xLength / 2, zLength / 2 ) ), minLength ),
						   Max( Max( zLength, Max( xLength / 2, yLength / 2 ) ), minLength ) );
	}
	return searcher.Search( start, goal, path, pathingHeight, heuristicCostMultiplier );
}

bool
AIManager::GetPathFindZ_( GetPathMethod method, const UPoint3D& start, const UPoint2D& goal, Vector<UPoint3D>& path, uint32 pathingHeight, float32 heuristicCostMultiplier )
{
	// TODO: add pathing height checker
	uint32 z = start.z;
	if( GetWorld()->GetSCube( goal.x, goal.y, z ) == TERRA_AIR )
	{
		--z;
		while( GetWorld()->GetSCube( goal.x, goal.y, z ) == TERRA_AIR )
		{
			--z;
		}
		return (this->*method)( start, UPoint3D( goal.x, goal.y, z + 1 ), path, pathingHeight, heuristicCostMultiplier );
	}

	++z;
	while( GetWorld()->GetSCube( goal.x, goal.y, z ) != TERRA_AIR )
	{
		++z;
	}
	return (this->*method)( start, UPoint3D( goal.x, goal.y, z ), path, pathingHeight, heuristicCostMultiplier );
}