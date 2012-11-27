//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "zoneroadgenerator.h"
#include "public\nstdlib\_math.h"
#include "public\nstdlib\upoint3d.h"
#include "astar3d.h"
#include "basebuilding.h"
#include "roadnode.h"

struct ZoneRoadGenerator::GetAreaAdjacentToRoadNodeLoopData
{
	RoadNode* node;
	BaseArea* areaToBeInitialized;
	uint32 xLength;
	uint32 yLength;
	uint32 minDistanceFromRoad;
	uint32 maxDistanceFromRoad;
	uint32 startX;
	uint32 endX;
	uint32 startY;
	uint32 endY;
	int32 iterationStep;
};

ZoneRoadGenerator::ZoneRoadGenerator( BaseZone* zone )
	: RoadGenerator()
{
	Assert( zone->GetLengthX() > 0 && zone->GetLengthX() < 100000 );
	Assert( zone->GetLengthY() > 0 && zone->GetLengthY() < 100000 );

	zone_ = zone;
	roadMap_ = (Vector< Pair< RoadNode*,RoadNode* > >**)Calloc( zone_->GetLengthX() * zone_->GetLengthY(), sizeof( Vector< Pair< RoadNode*,RoadNode* > >* ) );
}

ZoneRoadGenerator::~ZoneRoadGenerator( )
{
	free( roadMap_ );
}

void
ZoneRoadGenerator::AllocZoneRoadGrid( uint32 gridSpacing, uint32 maxOffset )
{
	// grid spacing: space between each grid along x and y
	// maxOffset: max distance from center of each grid unit that each individual point is set to
	Assert( gridSpacing / 2 >= maxOffset );
	isZoneRoadGridInitialized_ = true;

	xGridElements_ = zone_->GetLengthX() / gridSpacing - 1;
	yGridElements_ = zone_->GetLengthY() / gridSpacing - 1;
	gridSpacing_ = gridSpacing;

	Assert( xGridElements_ > 0 );
	Assert( yGridElements_ > 0 );

	roadGrid_ = (UPoint3D*)Malloc( sizeof( UPoint3D ) * xGridElements_ * yGridElements_ );

	// init roadGrid_ data
	uint32 y = zone_->GetMin().y + gridSpacing / 2;
	for( uint32 yIndex = 0; yIndex < yGridElements_; ++yIndex )
	{
		uint32 x = zone_->GetMin().x + gridSpacing / 2;
		for( uint32 xIndex = 0; xIndex < xGridElements_; ++xIndex )
		{
			int32 xMod = (rand () % (maxOffset + 1)) - maxOffset / 2;
			int32 yMod = (rand () % (maxOffset + 1)) - maxOffset / 2;
			roadGrid_[xIndex + yIndex * xGridElements_].x = uint32((int32)x + xMod);
			roadGrid_[xIndex + yIndex * xGridElements_].y = uint32((int32)y + yMod);
			roadGrid_[xIndex + yIndex * xGridElements_].z = zone_->GetHeightAtPoint( uint32((int32)x + xMod), uint32((int32)y + yMod) ) + 1; // +1 is to allow proper a* pathing

			x += gridSpacing;
			Assert( x < zone_->GetMax().x );
		}
		y += gridSpacing;
		Assert( y < zone_->GetMax().y );
	}

	isConnectedElementsOfGrid_ = (bool*)Calloc( xGridElements_ * yGridElements_, sizeof( bool ) );
}

void
ZoneRoadGenerator::FreeZoneRoadGrid( )
{
	isZoneRoadGridInitialized_ = false;
	
	free( roadGrid_ );
	free( isConnectedElementsOfGrid_ );
	
	
	BaseZone* zone_ = NULL;
}

void
ZoneRoadGenerator::ClearZoneRoadMap( )
{
	for( uint32 i = 0; i < zone_->GetLengthX() * zone_->GetLengthY(); ++i )
	{
		if( roadMap_[i] != NULL )
			delete roadMap_[i];
	}
	ZeroMemory( roadMap_, zone_->GetLengthX() * zone_->GetLengthY() * sizeof( Vector< Pair< RoadNode*,RoadNode* > >* ) );
}

bool
ZoneRoadGenerator::AddRoadToZone( RoadType type, bool doCreateEntrance, bool doAllowDiagonalPlacement, float32 aStarHeuristicCostMult )
{
	Assert( GetWorld()->GetIsModifying() );
	Assert( isZoneRoadGridInitialized_ );

	UPoint3D firstPoint;
	UPoint3D* currentPathingPoint = &firstPoint;
	UPoint3D* nextPathingPoint;

	uint32 xGridIndex;
	uint32 yGridIndex;
	uint32 xEndGridIndex;
	uint32 yEndGridIndex;
	
	if( doCreateEntrance )
	{
		if ( !AddRoadToZoneEntrancePrep_( type, currentPathingPoint, nextPathingPoint, xGridIndex, yGridIndex, xEndGridIndex, yEndGridIndex ) )
			return false;
	}
	else
	{
		if ( !AddRoadToZoneNormalPrep_( type, currentPathingPoint, nextPathingPoint, xGridIndex, yGridIndex, xEndGridIndex, yEndGridIndex, doAllowDiagonalPlacement ) )
			return false;
	}

	RoadNode* lastParent = NULL; // used to link new road nodes to the previous node

	AStar3D searcher( false );
	searcher.AllocMem( gridSpacing_ * 3, gridSpacing_ * 3, zone_->GetMax().z - zone_->GetMin().z );

	// add building bounds as unpathable areas
	for( uint32 i = 0; i < zone_->GetNumBuildings(); ++i )
	{
		searcher.AddUnpathableArea( *zone_->GetBuilding( i ) );
	}
	
	while( true )
	{
		// connect currentPathingPoint to nextPathingPoint by road
		if( !ConnectPointsInZone( type, *currentPathingPoint, *nextPathingPoint, lastParent, &searcher, aStarHeuristicCostMult ) )
			return false;

		// mark current grid indices as connected
		isConnectedElementsOfGrid_[xGridIndex + yGridIndex * xGridElements_] = true;

		// increment or deincrement grid indices
		if ( xGridIndex < xEndGridIndex )
			++xGridIndex;
		else if( xGridIndex > xEndGridIndex )
			--xGridIndex;
		if ( yGridIndex < yEndGridIndex )
			++yGridIndex;
		else if( yGridIndex > yEndGridIndex )
			--yGridIndex;

		if( xGridIndex == xEndGridIndex && yGridIndex == yEndGridIndex )
			break;

		// STODO: add a termination factor when intersecting with a road node that already exists at a grid point
		// move to next point
		currentPathingPoint = nextPathingPoint;
		nextPathingPoint = &roadGrid_[xGridIndex + yGridIndex * xGridElements_];

		searcher.ClearMem();
	}
	searcher.FreeMem();

	return true;
}

bool
ZoneRoadGenerator::ConnectPointsInZone( RoadType type, const UPoint3D& point, const UPoint3D& otherPoint, RoadNode* lastParent, AStar3D* searcher, float32 aStarHeuristicCostMult )
{
	Assert( GetWorld()->GetIsModifying() );
	bool doDeleteSearcher = ConditionallyInitSearcherObject_( searcher, point, otherPoint );
	searcher->AddPathableArea( *zone_ );
	Vector<UPoint3D> path;

	// a* between current and next pathing point, continue till road has reached at least the center of the map
	if( !searcher->Search( point, otherPoint, path, 2, aStarHeuristicCostMult ) )
	{
		if( doDeleteSearcher )
			delete searcher;

		return false; // STODO: handle this case better by not just returing false immediately. (currently this can result in an incomplete road being generated)
	}

	for( uint32 i = 0; i < path.GetCount(); ++i )
	{
		UPoint3D currentRoadPoint = GetNextPointInAStarPath_( path, lastParent, i );

		// make sure two nodes never share the same position. newRoadNode will be set to NULL if another node does not exist at that point
		RoadNode* newRoadNode = GetNodeAtLocationOnRoadMap( currentRoadPoint ); // STODO: is the old node conditionally overwritten with the new non positional data? (ex: width)

		if( newRoadNode == NULL )
		{
			newRoadNode = new RoadNode( currentRoadPoint, type );

			// fill activeZone's road map for building placement and road to building linking purposes
			if( lastParent != NULL )
			{
				// link nodes
				lastParent->AddChildNode( *newRoadNode, true );

				// fill map
				lastParent->FillRoadMap( *newRoadNode, roadMap_, zone_->GetLengthX(), zone_->GetLengthY(), zone_->GetMin().x, zone_->GetMin().y );
			}

			zone_->AddRoadNode( newRoadNode ); // SNOW: lengthen road if the point after the next will continue on the same direction and slope		
		}
		lastParent = newRoadNode;
	}

	searcher->ClearPathableAreas();

	if( doDeleteSearcher )
		delete searcher;

	return true;
}

void
ZoneRoadGenerator::ConnectRoadToFrontDoor( RoadNode* roadNode, BaseBuilding* building, float32 aStarHeuristicCostMult )
{
	UPoint3D goalPoint = building->GetFrontDoorPos();
	++goalPoint.z;

	UPoint3D intermediateGoalPoint = goalPoint;

	uint32 intermediateGoalPointOffset = 2 + rand() % 3;

	switch( building->GetDoorSide() )
	{
	case DIRECTION_SOUTH:
		--goalPoint.y;
		intermediateGoalPoint.y -= intermediateGoalPointOffset;
		break;
	case DIRECTION_NORTH:
		++goalPoint.y;
		intermediateGoalPoint.y += intermediateGoalPointOffset;
		break;
	case DIRECTION_WEST:
		--goalPoint.x;
		intermediateGoalPoint.x -= intermediateGoalPointOffset;
		break;
	case DIRECTION_EAST:
		++goalPoint.x;
		intermediateGoalPoint.x += intermediateGoalPointOffset;
		break;
	}

	intermediateGoalPoint.z = zone_->GetHeightAtPoint( intermediateGoalPoint.x, intermediateGoalPoint.y ) + 1;

	ConnectPointsInZone( ROAD_SECONDARY, *roadNode, intermediateGoalPoint, NULL, NULL, aStarHeuristicCostMult );
	ConnectPointsInZone( ROAD_SECONDARY, intermediateGoalPoint, goalPoint, NULL, NULL, aStarHeuristicCostMult );
}

RoadNode*
ZoneRoadGenerator::GetNodeAtLocationOnRoadMap( const UPoint3D& point )
{
	Assert( isZoneRoadGridInitialized_ );

	// find node at point by investigating the roadMap at that point.
	Vector< Pair< RoadNode*, RoadNode* > >* mapLocation = GetDataFromRoadMap( point.x, point.y );
	
	if( mapLocation != NULL )
	{
		for( uint32 i = 0; i < mapLocation->GetCount(); ++i )
		{
			if( *mapLocation->GetElement( i ).first == point )
				return mapLocation->GetElement( i ).first;
			else if( *mapLocation->GetElement( i ).second == point )
				return mapLocation->GetElement( i ).second;
		}
	}

	return NULL;
}

bool
ZoneRoadGenerator::Does3DAreaIntersectWithRoads( const BaseArea& area )
{
	Assert( isZoneRoadGridInitialized_ );
	Assert( area.GetMin().x >= zone_->GetMin().x );
	Assert( area.GetMin().y >= zone_->GetMin().y );
	Assert( area.GetMax().x <= zone_->GetMax().x );
	Assert( area.GetMax().y <= zone_->GetMax().y );

	for( uint32 y = area.GetMin().y; y <= area.GetMax().y; ++y )
	{
		for( uint32 x = area.GetMin().x; x <= area.GetMax().x; ++x )
		{
			Vector< Pair< RoadNode*, RoadNode* > >* mapLocation = GetDataFromRoadMap( x, y );
			if(  mapLocation != NULL )
			{
				// check of z levels to make sure area can intersect with road
				for( uint32 i = 0; i < mapLocation->GetCount(); ++i )
				{
					// Note- STODO: this does not check if the line segment truly intersects with the area in all cases for vertically diagonal road segments.
					Pair< RoadNode*, RoadNode* >& pair = mapLocation->GetElement( i );
					if( (pair.first->z >= area.GetMin().z && pair.first->z <= area.GetMax().z) ||
						(pair.second->z >= area.GetMin().z && pair.second->z <= area.GetMax().z) )
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool
ZoneRoadGenerator::Does2DAreaIntersectWithRoads( uint32 minX, uint32 maxX, uint32 minY, uint32 maxY )
{
	Assert( isZoneRoadGridInitialized_ );
	Assert( minX >= zone_->GetMin().x );
	Assert( minY >= zone_->GetMin().y );
	Assert( maxX <= zone_->GetMax().x );
	Assert( maxY <= zone_->GetMax().y );

	for( uint32 y = minY; y <= maxY; ++y )
	{
		for( uint32 x = minX; x <= maxX; ++x )
		{
			if(  GetDataFromRoadMap( x, y ) != NULL )
				return true;
		}
	}
	return false;
}

bool
ZoneRoadGenerator::GetAreaAdjacentToRoadNode( RoadNode* node, BaseArea& areaToBeInitialized, uint32 xLength, uint32 yLength, uint32 minDistanceFromRoad, uint32 maxDistanceFromRoad )
{
	// Note- min and max distance from road refer to the road blocks not the node itself. this offset is road node width / 2
	Assert( isZoneRoadGridInitialized_ );
	Assert( minDistanceFromRoad <= maxDistanceFromRoad );

	// Note- this method does not initialize areaToBeInitialized.z
	GetAreaAdjacentToRoadNodeLoopData data;
	data.node = node;
	data.areaToBeInitialized = &areaToBeInitialized;
	data.xLength = xLength;
	data.yLength = yLength;
	data.minDistanceFromRoad = minDistanceFromRoad;
	data.maxDistanceFromRoad = maxDistanceFromRoad;

	// the positions of x and y take into account the position
	data.startX = ((node->x - xLength) - node->GetWidth() / 2) - maxDistanceFromRoad;
	data.endX = node->x + node->GetWidth() / 2 + maxDistanceFromRoad;
	data.startX += rand () % (data.endX - data.startX);
	data.startY = ((node->y - yLength) - node->GetWidth() / 2) - maxDistanceFromRoad;
	data.endY = node->y + node->GetWidth() / 2 + maxDistanceFromRoad;
	data.startY += rand () % (data.endY - data.startY);

	data.iterationStep = (rand () % 2) * 2 - 1;

	// make sure area that is to be created will not exceed zone bounds
	// x axis step
	if( data.startX < zone_->GetMin().x )
		data.startX = zone_->GetMin().x;
	if( data.startX > zone_->GetMax().x - xLength )
		data.startX = zone_->GetMax().x - xLength;
	if( data.endX < zone_->GetMin().x )
		data.endX = zone_->GetMin().x;
	if( data.endX > zone_->GetMax().x - xLength )
		data.endX = zone_->GetMax().x - xLength;
	
	// y axis step
	if( data.startY < zone_->GetMin().y )
		data.startY = zone_->GetMin().y;
	if( data.startY > zone_->GetMax().y - yLength )
		data.startY = zone_->GetMax().y - yLength;
	if( data.endY < zone_->GetMin().y )
		data.endY = zone_->GetMin().y;
	if( data.endY > zone_->GetMax().y - yLength )
		data.endY = zone_->GetMax().y - yLength;
	
	if( GetAreaAdjacentToRoadNodeLoop_( data ) )
		return true;

	data.iterationStep = -data.iterationStep;

	if( GetAreaAdjacentToRoadNodeLoop_( data ) )
		return true;

	return false;
}

RoadNode*
ZoneRoadGenerator::GetNearestRoadNodeToArea( const BaseArea& area )
{
	Assert( isZoneRoadGridInitialized_ );

	BaseArea modifiableArea( area );
	UPoint3D centerOfArea;
	centerOfArea.x = area.GetCenterX();
	centerOfArea.y = area.GetCenterY();
	centerOfArea.z = area.GetCenterZ();

	while( true )
	{
		Vector< Pair< RoadNode*, RoadNode* > > listToFill;

		if( GetListOfRoadSegmentsThatIntersectWithArea_( modifiableArea, listToFill ) )
		{
			float32 minDistance = 1000000000.0f;
			RoadNode* closestRoadNode = NULL;
			for( uint32 i = 0; i < listToFill.GetCount(); ++i )
			{
				float32 newDistance = listToFill[i].first->GetDistanceFast( centerOfArea );
				if( newDistance < minDistance )
				{
					minDistance = newDistance;
					closestRoadNode = listToFill[i].first;
				}

				newDistance = listToFill[i].second->GetDistanceFast( centerOfArea );
				if( newDistance < minDistance )
				{
					minDistance = newDistance;
					closestRoadNode = listToFill[i].second;
				}
			}
			Assert( closestRoadNode != NULL );
			break;
		}
		else
		{
			modifiableArea.SetMinX( modifiableArea.GetMin().x - 1 );
			modifiableArea.SetMinY( modifiableArea.GetMin().y - 1 );
			modifiableArea.SetMaxX( modifiableArea.GetMax().x + 1 );
			modifiableArea.SetMaxY( modifiableArea.GetMax().y + 1 );
		}
	}

	return NULL;
}

/*
Pair< RoadNode*, RoadNode* >*
RoadGenerator::GetNearestRoadSegmentToArea( const BaseArea& area )
{
	Assert( isZoneRoadGridInitialized_ );



	return NULL;
}
*/

/*-------------------------------------------------------------------------------------------------------
	Private Methods
---------------------------------------------------------------------------------------------------------*/

bool
ZoneRoadGenerator::GetAreaAdjacentToRoadNodeLoop_( GetAreaAdjacentToRoadNodeLoopData& data )
{
	// randomly sample a space around the node
	BaseArea tmpArea;
	for( uint32 y = data.startY; y != data.endY ; ++y )
	{
		tmpArea.SetMinY( y );
		tmpArea.SetMaxY( y + data.yLength - 1 );
		for( uint32 x = data.startX; x != data.endX ; ++x )
		{
			tmpArea.SetMinX( x );
			tmpArea.SetMaxX( x + data.xLength - 1 );

			Assert( tmpArea.GetMin().x >= zone_->GetMin().x );
			Assert( tmpArea.GetMin().y >= zone_->GetMin().y );
			Assert( tmpArea.GetMax().x <= zone_->GetMax().x );
			Assert( tmpArea.GetMax().y <= zone_->GetMax().y );
			Assert( tmpArea.GetMin().x < tmpArea.GetMax().x );
			Assert( tmpArea.GetMin().y < tmpArea.GetMax().y );

			if( uint32(Min( Abs( int32(tmpArea.GetMin().x) - int32(data.node->x) ), Abs( int32(tmpArea.GetMax().x) - int32(data.node->x) ) )) + 1 < data.minDistanceFromRoad + data.node->GetWidth() / 2 && 
				uint32(Min( Abs( int32(tmpArea.GetMin().y) - int32(data.node->y) ), Abs( int32(tmpArea.GetMax().y) - int32(data.node->y) ) )) + 1 < data.minDistanceFromRoad + data.node->GetWidth() / 2 )
			{
				continue;
			}

			// TODO: this is inaccurate and uses magic numbers. forces another IsBuildingBoundsPlaceable check once bounds are more finalized
			tmpArea.SetMinZ( zone_->GetHeightAtPoint(tmpArea.GetCenterPoint().x, tmpArea.GetCenterPoint().y ) - 50 );
			tmpArea.SetMaxZ( tmpArea.GetMin().z + 100 );

			// make sure building bounds are placeable
			if( zone_->IsBuildingBoundsPlaceable( tmpArea.GetMin(), tmpArea.GetMax() ) )
			{
				*data.areaToBeInitialized = tmpArea;
				return true;
			}
		}
	}

	return false;
}

bool
ZoneRoadGenerator::AddRoadToZoneEntrancePrep_( RoadType type, UPoint3D*& currentPathingPoint, UPoint3D*& nextPathingPoint, uint32& xGridIndex, uint32& yGridIndex, uint32& xEndGridIndex, uint32& yEndGridIndex )
{
	// STODO: setting of zone entrances here is preliminary. more control of this may need to be added elsewhere
	switch( rand () % 4 )
	{
	case 0: // south
		xGridIndex = xGridElements_ / 3 + rand() % xGridElements_ / 3;
		yGridIndex = 0;
		xEndGridIndex = xGridIndex;
		yEndGridIndex = yGridElements_ / 3 + rand() % (yGridElements_ / 2) + yGridElements_ / 10;
		nextPathingPoint = &roadGrid_[xGridIndex + yGridIndex * xGridElements_];
		currentPathingPoint->x = nextPathingPoint->x;
		currentPathingPoint->y = zone_->GetMin().y;
		break;
	case 1: // north
		xGridIndex = xGridElements_ / 3 + rand() % xGridElements_ / 3;
		yGridIndex = yGridElements_ - 1;
		xEndGridIndex = xGridIndex;
		yEndGridIndex = yGridElements_ - (yGridElements_ / 3 + rand() % (yGridElements_ / 2) + yGridElements_ / 10);
		nextPathingPoint = &roadGrid_[xGridIndex + yGridIndex * xGridElements_];
		currentPathingPoint->x = nextPathingPoint->x;
		currentPathingPoint->y = zone_->GetMax().y;
		break;
	case 2: // west
		xGridIndex = 0;
		yGridIndex = yGridElements_ / 3 + rand() % (yGridElements_ / 3);
		xEndGridIndex = xGridElements_ / 3 + rand() % (xGridElements_ / 2) + xGridElements_ / 10;
		yEndGridIndex = yGridIndex;
		nextPathingPoint = &roadGrid_[xGridIndex + yGridIndex * xGridElements_];
		currentPathingPoint->x = zone_->GetMin().x;
		currentPathingPoint->y = nextPathingPoint->y;
		break;
	case 3: // east
		xGridIndex = xGridElements_ - 1;
		yGridIndex = yGridElements_ / 3 + rand() % (yGridElements_ / 3);
		xEndGridIndex = xGridElements_ - (xGridElements_ / 3 + rand() % (xGridElements_ / 2) + xGridElements_ / 10);
		yEndGridIndex = yGridIndex;
		nextPathingPoint = &roadGrid_[xGridIndex + yGridIndex * xGridElements_];
		currentPathingPoint->x = zone_->GetMax().x;
		currentPathingPoint->y = nextPathingPoint->y;
		break;
	}
	currentPathingPoint->z = zone_->GetHeightAtPoint( currentPathingPoint->x, currentPathingPoint->y ) + 1;
	zone_->AddEntrance( *currentPathingPoint );

	return true;
}

bool
ZoneRoadGenerator::AddRoadToZoneNormalPrep_( RoadType type, UPoint3D*& currentPathingPoint, UPoint3D*& nextPathingPoint, uint32& xGridIndex, uint32& yGridIndex, uint32& xEndGridIndex, uint32& yEndGridIndex, bool doAllowDiagonalPlacement )
{
	bool doRandLoop = true;
	
	uint32 count = 0;
	while( count < 15 )
	{
		if( count > 10 )
			doRandLoop = false;
		bool isStartingPositionForNonRandLoopSet = false;

		for( uint32 y = rand() % yGridElements_; y < yGridElements_; ++y )
		{
			if( !doRandLoop && !isStartingPositionForNonRandLoopSet )
				y = 0;
			for( uint32 x = rand() % xGridElements_; x < xGridElements_; ++x )
			{
				// make sure start point does not intersect with building bounds
				if( zone_->DoesPointIntersectWithBuildingBounds( roadGrid_[x + y * xGridElements_] ) )
					continue;

				if( !doRandLoop && !isStartingPositionForNonRandLoopSet )
				{
					x = 0;
					isStartingPositionForNonRandLoopSet = true;
				}

				// make sure beginning point is already connected in the grid
				if( isConnectedElementsOfGrid_[x + y * xGridElements_] )
				{
					// find adjacent position in road grid
					for( int32 ySample = (int32)y - 1 + rand() % 3; ySample <= (int32)y + 1; ++ySample )
					{
						if( ySample < 0 || ySample >= (int32)yGridElements_ )
							continue;

						for( int32 xSample = (int32)x - 1 + rand() % 3; xSample <= (int32)x + 1; ++xSample )
						{
							if( xSample < 0 || xSample >= (int32)xGridElements_ )
								continue;
							if( xSample == (int32)x && ySample == (int32)y )
								continue;
							if( !doAllowDiagonalPlacement && xSample != x && ySample != y )
								continue;

							// make sure end point does not intersect with building bounds
							if( zone_->DoesPointIntersectWithBuildingBounds( roadGrid_[(uint32)xSample + (uint32)ySample * xGridElements_] ) )
								continue;
							
							if( !isConnectedElementsOfGrid_[(uint32)xSample + (uint32)ySample * xGridElements_] )
							{
								xGridIndex = x;
								yGridIndex = y;
								currentPathingPoint = &roadGrid_[xGridIndex + yGridIndex * xGridElements_];

								nextPathingPoint = &roadGrid_[(uint32)xSample + (uint32)ySample * xGridElements_];

								xEndGridIndex = xSample;
								yEndGridIndex = ySample;

								// attempt to extend end indices in the same direction
								ConditionallyModifyEndGridIndices_( xEndGridIndex, yEndGridIndex, x, y, type );

								return true;
							}
						}
					}
				}
			}
		}
		++count;
	}
	return false;
}

void
ZoneRoadGenerator::ConditionallyModifyEndGridIndices_( uint32& xEndGridIndex, uint32& yEndGridIndex, uint32 x, uint32 y, RoadType type )
{
	if( xEndGridIndex < x && yEndGridIndex < y )
	{
		while( xEndGridIndex > 0 && yEndGridIndex > 0 &&
			!isConnectedElementsOfGrid_[(uint32)xEndGridIndex + (uint32)yEndGridIndex * xGridElements_] &&
			(type == ROAD_PRIMARY && rand() % 6 <= 4 || type == ROAD_SECONDARY && rand() % 4 <= 2) ) // STODO: export rand checks here to method that takes in road type, grid spacing grid elements into account
		{
			if( zone_->DoesPointIntersectWithBuildingBounds( roadGrid_[ (xEndGridIndex - 1) + (yEndGridIndex - 1) * xGridElements_ ] ) )
				break;

			--xEndGridIndex;
			--yEndGridIndex;
		}
	}
	else if( xEndGridIndex == x && yEndGridIndex < y )
	{
		while( yEndGridIndex > 0 &&
			!isConnectedElementsOfGrid_[(uint32)xEndGridIndex + (uint32)yEndGridIndex * xGridElements_] &&
			(type == ROAD_PRIMARY && rand() % 6 <= 4 || type == ROAD_SECONDARY && rand() % 4 <= 2) )
		{
			if( zone_->DoesPointIntersectWithBuildingBounds( roadGrid_[ (xEndGridIndex) + (yEndGridIndex - 1) * xGridElements_ ] ) )
				break;

			--yEndGridIndex;
		}
	}
	else if( xEndGridIndex > x && yEndGridIndex < y )
	{
		while( xEndGridIndex < xGridElements_ - 1 && yEndGridIndex > 0 &&
			!isConnectedElementsOfGrid_[(uint32)xEndGridIndex + (uint32)yEndGridIndex * xGridElements_] &&
			(type == ROAD_PRIMARY && rand() % 6 <= 4 || type == ROAD_SECONDARY && rand() % 4 <= 2) )
		{
			if( zone_->DoesPointIntersectWithBuildingBounds( roadGrid_[ (xEndGridIndex + 1) + (yEndGridIndex - 1) * xGridElements_ ] ) )
				break;

			++xEndGridIndex;
			--yEndGridIndex;
		}
	}
	else if( xEndGridIndex < x && yEndGridIndex == y )
	{
		while( xEndGridIndex > 0 &&
			!isConnectedElementsOfGrid_[(uint32)xEndGridIndex + (uint32)yEndGridIndex * xGridElements_] &&
			(type == ROAD_PRIMARY && rand() % 6 <= 4 || type == ROAD_SECONDARY && rand() % 4 <= 2) )
		{
			if( zone_->DoesPointIntersectWithBuildingBounds( roadGrid_[ (xEndGridIndex - 1) + (yEndGridIndex) * xGridElements_ ] ) )
				break;

			--xEndGridIndex;
		}
	}
	else if( xEndGridIndex > x && yEndGridIndex == y )
	{
		while( xEndGridIndex < xGridElements_ - 1 &&
			!isConnectedElementsOfGrid_[(uint32)xEndGridIndex + (uint32)yEndGridIndex * xGridElements_] &&
			(type == ROAD_PRIMARY && rand() % 6 <= 4 || type == ROAD_SECONDARY && rand() % 4 <= 2) )
		{
			if( zone_->DoesPointIntersectWithBuildingBounds( roadGrid_[ (xEndGridIndex + 1) + (yEndGridIndex) * xGridElements_ ] ) )
				break;

			++xEndGridIndex;
		}
	}
	else if( xEndGridIndex < x && yEndGridIndex > y )
	{
		while( xEndGridIndex > 0 && yEndGridIndex < yGridElements_ - 1 &&
			!isConnectedElementsOfGrid_[(uint32)xEndGridIndex + (uint32)yEndGridIndex * xGridElements_] &&
			(type == ROAD_PRIMARY && rand() % 6 <= 4 || type == ROAD_SECONDARY && rand() % 4 <= 2) ) // STODO: export rand checks here to method that takes in road type, grid spacing grid elements into account
		{
			if( zone_->DoesPointIntersectWithBuildingBounds( roadGrid_[ (xEndGridIndex - 1) + (yEndGridIndex + 1) * xGridElements_ ] ) )
				break;

			--xEndGridIndex;
			++yEndGridIndex;
		}
	}
	else if( xEndGridIndex == x && yEndGridIndex > y )
	{
		while( yEndGridIndex < yGridElements_ - 1 &&
			!isConnectedElementsOfGrid_[(uint32)xEndGridIndex + (uint32)yEndGridIndex * xGridElements_] &&
			(type == ROAD_PRIMARY && rand() % 6 <= 4 || type == ROAD_SECONDARY && rand() % 4 <= 2) )
		{
			if( zone_->DoesPointIntersectWithBuildingBounds( roadGrid_[ (xEndGridIndex) + (yEndGridIndex + 1) * xGridElements_ ] ) )
				break;

			++yEndGridIndex;
		}
	}
	else
	{
		while( xEndGridIndex < xGridElements_ - 1 && yEndGridIndex < yGridElements_ - 1 &&
			!isConnectedElementsOfGrid_[(uint32)xEndGridIndex + (uint32)yEndGridIndex * xGridElements_] &&
			(type == ROAD_PRIMARY && rand() % 6 <= 4 || type == ROAD_SECONDARY && rand() % 4 <= 2) )
		{
			if( zone_->DoesPointIntersectWithBuildingBounds( roadGrid_[ (xEndGridIndex + 1) + (yEndGridIndex + 1) * xGridElements_ ] ) )
				break;

			++xEndGridIndex;
			++yEndGridIndex;
		}
	}
}

bool
ZoneRoadGenerator::GetListOfRoadSegmentsThatIntersectWithArea_( const BaseArea& area, Vector< Pair< RoadNode*, RoadNode* > >& listToFill )
{
	Assert( isZoneRoadGridInitialized_ );

	for( uint32 y = area.GetMin().y; y <= area.GetMax().y; ++y )
	{
		for( uint32 x = area.GetMin().x; x <= area.GetMax().x; ++x )
		{
			Vector< Pair< RoadNode*, RoadNode* > >* mapLocation = GetDataFromRoadMap( x, y );
			if(  mapLocation != NULL )
			{
				// check of z levels to make sure area can intersect with road
				for( uint32 i = 0; i < mapLocation->GetCount(); ++i )
				{
					// Note- STODO: this does not check if the line segment truly intersects with the area in all cases for vertically diagonal road segments.
					Pair< RoadNode*, RoadNode* >& pair = mapLocation->GetElement( i );
					if( (pair.first->z >= area.GetMin().z && pair.first->z <= area.GetMax().z) ||
						(pair.second->z >= area.GetMin().z && pair.second->z <= area.GetMax().z) )
					{
						listToFill.InsertBack( pair );
					}
				}
			}
		}
	}

	return listToFill.GetCount() > 0;
}
