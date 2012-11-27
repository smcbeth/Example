//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "roadgenerator.h"
#include "public\nstdlib\_math.h"
#include "public\nstdlib\upoint3d.h"
#include "astar3d.h"
#include "world.h"
#include "basezone.h"
#include "roadnode.h"

RoadGenerator::RoadGenerator( )
{
	;
}

RoadGenerator::~RoadGenerator( )
{
	;
}

bool
RoadGenerator::ConnectPoints( RoadType type, const UPoint3D& firstPoint, const UPoint3D& secondPoint, AStar3D* searcher, float32 aStarHeuristicCostMult )
{
	Assert( GetWorld()->GetIsModifying() );

	// STODO: store the nodes created here somewhere else, like world
	bool doDeleteSearcher = ConditionallyInitSearcherObject_( searcher, firstPoint, secondPoint );

	Vector<UPoint3D> path;
	
	// a* between current and next pathing point, continue till road has reached at least the center of the map
	if( !searcher->Search( firstPoint, secondPoint, path, 2, aStarHeuristicCostMult ) )
	{
		if( doDeleteSearcher )
			delete searcher;

		return false; // STODO: handle this case better by not just returing false immediately. (currently this can result in an incomplete road being generated)
	}
			
	Vector<RoadNode> nodes;

	RoadNode* lastParent = NULL;
		
	for( uint32 i = 0; i < path.GetCount(); ++i )
	{
		UPoint3D currentRoadPoint = GetNextPointInAStarPath_( path, lastParent, i );

		// make sure two nodes never share the same position. newRoadNode will be set to NULL if another node does not exist at that point
		nodes.InsertBack( RoadNode( currentRoadPoint, type ) );
		RoadNode* newRoadNode = &nodes.GetBack();

		// fill activeZone's road map for building placement and road to building linking purposes
		if( lastParent != NULL )
		{
			// link nodes
			lastParent->AddChildNode( *newRoadNode, true );
		}

		//zone_->AddRoadNode( newRoadNode ); // STODO: store the nodes somewhere		
		lastParent = newRoadNode;
	}	

	searcher->FreeMem();

	// build out road
	for( uint32 i = 0; i < nodes.GetCount(); ++i )
	{
		for( uint32 j = 0; j < nodes[i].GetNumChildNodes(); ++j )
		{
			nodes[i].BuildOrDoIntersection( *nodes[i].GetChildNode( j ), false );
		}
	}

	if( doDeleteSearcher )
		delete searcher;

	return true;
}

bool
RoadGenerator::ConnectZones( RoadType type, BaseZone* firstZone, BaseZone* secondZone, float32 aStarHeuristicCostMult, const Vector<BaseArea*>* unpathableAreas )
{
	Assert( GetWorld()->GetIsModifying() );

	const UPoint3D* firstZoneEntrance = NULL;
	const UPoint3D* secondZoneEntrance = NULL;

	AStar3D* searcher = new AStar3D( false );
	searcher->AddPointToAvoid( firstZone->GetCenterPoint(), float32(Max(firstZone->GetLengthX(), firstZone->GetLengthY())) * 0.8f, aStarHeuristicCostMult * 3 );
	searcher->AddUnpathableArea( BaseArea( UPoint3D( firstZone->GetMin().x + 1, firstZone->GetMin().y + 1, firstZone->GetMin().z ), 
											UPoint3D( firstZone->GetMax().x - 1, firstZone->GetMax().y - 1, firstZone->GetMax().z ) ) );

	// add additional unpathable areas if available
	if( unpathableAreas != NULL )
	{
		for( uint32 i = 0; i < unpathableAreas->GetCount(); ++i )
		{
			const BaseArea& area = *unpathableAreas->GetElement( i );

			if( area != *firstZone && area != *secondZone )
				searcher->AddUnpathableArea( area );
		}
	}

	if( firstZone != secondZone )
	{
		// case for connecting entrances of different zones
		// find closest entrances to other zone
		firstZoneEntrance = &FindClosestZoneEntranceToZone_( firstZone, secondZone );
		secondZoneEntrance = &FindClosestZoneEntranceToZone_( secondZone, firstZone );

		searcher->AddPointToAvoid( secondZone->GetCenterPoint(), float32(Max(secondZone->GetLengthX(), secondZone->GetLengthY())) * 0.80f, aStarHeuristicCostMult * 3 );
		searcher->AddUnpathableArea( BaseArea( UPoint3D( secondZone->GetMin().x + 1, secondZone->GetMin().y + 1, secondZone->GetMin().z ), 
											UPoint3D( secondZone->GetMax().x - 1, secondZone->GetMax().y - 1, secondZone->GetMax().z ) ) );

		const int32 minSearchSpaceLength = 20;
		searcher->AllocMem( uint32(Max( Abs( int32(firstZoneEntrance->x) - int32(secondZoneEntrance->x) ) * 2, minSearchSpaceLength )), 
			uint32(Max( Abs( int32(firstZoneEntrance->y) - int32(secondZoneEntrance->y) ) * 2, minSearchSpaceLength )),
			uint32(Max( Abs( int32(firstZoneEntrance->z) - int32(secondZoneEntrance->z) ) * 2, minSearchSpaceLength * 2 )) );
	}
	else
	{
		// case for connecting entrances of the same zone
		Assert( firstZone->GetNumEntrances() >= 2 );

		// randomly pick both entrances
		firstZoneEntrance = &firstZone->GetEntrance( rand() % firstZone->GetNumEntrances() );
		for( uint32 i = rand() % firstZone->GetNumEntrances(); i < firstZone->GetNumEntrances(); ++i )
		{
			if( &firstZone->GetEntrance( i ) != firstZoneEntrance )
			{
				secondZoneEntrance = &firstZone->GetEntrance( i );
				break;
			}
		}
		if( secondZoneEntrance == NULL )
		{
			// loop through all entrances to ensure success
			for( uint32 i = 0; i < firstZone->GetNumEntrances(); ++i )
			{
				if( &firstZone->GetEntrance( i ) != firstZoneEntrance )
				{
					secondZoneEntrance = &firstZone->GetEntrance( i );
					break;
				}
			}
		}

		const int32 minSearchSpaceLength = 20;
		searcher->AllocMem( uint32(Max(Max( Abs( int32(firstZoneEntrance->x) - int32(secondZoneEntrance->x) ) * 2, minSearchSpaceLength ), Abs( int32(firstZoneEntrance->y) - int32(secondZoneEntrance->y) ) * 2)), 
			uint32(Max(Max( Abs( int32(firstZoneEntrance->y) - int32(secondZoneEntrance->y) ) * 2, minSearchSpaceLength ), Abs( int32(firstZoneEntrance->x) - int32(secondZoneEntrance->x) ) * 2)),
			uint32(Max( Abs( int32(firstZoneEntrance->z) - int32(secondZoneEntrance->z) ) * 2, minSearchSpaceLength * 2 )) );
	}

	return ConnectPoints( type, *firstZoneEntrance, *secondZoneEntrance, searcher, aStarHeuristicCostMult );

	delete searcher;

	// TODO: link to zone roads once the roads generated here can be stored somewhere
}

bool
RoadGenerator::ConnectRoadToRoad( RoadType type, RoadNode* firstRoadNode, RoadNode* secondRoadNode, float32 aStarHeuristicCostMult )
{
	return ConnectPoints( type, *firstRoadNode, *secondRoadNode, NULL, aStarHeuristicCostMult );
	// TODO: link to roads once the roads generated here can be stored somewhere
}


/*-------------------------------------------------------------------------------------------------------
	Private Methods
---------------------------------------------------------------------------------------------------------*/


const UPoint3D&
RoadGenerator::FindClosestZoneEntranceToZone_( BaseZone* zoneWithEntrance, BaseZone* otherZone )
{
	Assert( zoneWithEntrance != otherZone );
	Assert( zoneWithEntrance->GetNumEntrances() > 0 );
	const UPoint3D* bestZoneEntrance;
	float32 minDistance = 1000000000.0f;
	for( uint32 i = 0; i < zoneWithEntrance->GetNumEntrances(); ++i )
	{
		float32 tmpDistance = zoneWithEntrance->GetEntrance( i ).GetDistanceFast( otherZone->GetCenterPoint() );
		if( tmpDistance < minDistance )
			bestZoneEntrance = &zoneWithEntrance->GetEntrance( i );
	}

	return *bestZoneEntrance;
}

UPoint3D
RoadGenerator::GetNextPointInAStarPath_( const Vector<UPoint3D>& path, RoadNode* lastParent, uint32& pathIndex )
{
	UPoint3D point;
	const uint32 maxRoadSegmentLength = 10; // allows skipping over path points that are in a straight line up to this number
	for( uint32 j = 0; j < maxRoadSegmentLength - 1 && pathIndex < path.GetCount(); ++j )
	{
		point = path.GetElement( pathIndex );
		if( pathIndex == 0 || pathIndex == path.GetCount() - 1 )
			break; // always make a node at the beginning and end of the path

		Point3D currentSlope( int32(point.x) - int32(lastParent->x),
			int32(point.y) - int32(lastParent->y),
			int32(point.z) - int32(lastParent->z) );

		const UPoint3D& nextRoadPoint = path.GetElement( pathIndex );

		Point3D nextSlope( int32(nextRoadPoint.x) - int32(nextRoadPoint.x),
			int32(nextRoadPoint.y) - int32(nextRoadPoint.y),
			int32(nextRoadPoint.z) - int32(nextRoadPoint.z) );

		if( currentSlope != nextSlope )
			break;

		++pathIndex;
	}
	--point.z; // offset by 1 from A* path to set road in the ground, not above it.

	return point;
}

bool
RoadGenerator::ConditionallyInitSearcherObject_( AStar3D*& searcher, const UPoint3D& point, const UPoint3D& otherPoint )
{
	if( searcher == NULL )
	{
		searcher = new AStar3D( false );
		{
			const uint32 minSearchSpaceLength = 20;
			searcher->AllocMem( uint32(Max( Abs( int32(point.x) - int32(otherPoint.x) ) * 2, minSearchSpaceLength )), 
				uint32(Max( Abs( int32(point.y) - int32(otherPoint.y) ) * 2, minSearchSpaceLength )),
				uint32(Max( Abs( int32(point.z) - int32(otherPoint.z) ) * 2, minSearchSpaceLength * 2 )) );
		}
		return true;
	}
	return false;
}