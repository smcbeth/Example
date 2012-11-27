//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "astar3d.h"
#include "public\nstdlib\_math.h"
//#include "public\nstdlib\sort.h"
#include "public\nstdlib\binaryheap.h"
#include "world.h"
#include "terrainhelper.h"

class AStar3D::AStar3DNode : public UPoint3D
{
public:
	AStar3DNode( uint32 x, uint32 y, uint32 z );
	AStar3DNode( const UPoint3D& point );
	AStar3DNode( const UPoint3D& point, float32 g, float32 h, float32 f, AStar3DNode* parent);
	
	bool operator<( const AStar3DNode& other ) const;
	bool operator>( const AStar3DNode& other ) const;

	float32			gCost; // cost for traveling along path
	float32			hCost; // distance estimate
	float32			fCost; // g + f cost. total cost along path. determines sorting order.

	AStar3DNode*	parentNode;
};

AStar3D::AStar3DNode::AStar3DNode( uint32 x, uint32 y, uint32 z )
	: UPoint3D( x, y, z ), gCost( 0 ), hCost( 0 ), fCost( 0 )
{
	;
}

AStar3D::AStar3DNode::AStar3DNode( const UPoint3D& point )
	: UPoint3D( point ), gCost( 0 ), hCost( 0 ), fCost( 0 )
{
	;
}

AStar3D::AStar3DNode::AStar3DNode( const UPoint3D& point, float32 g, float32 h, float32 f, AStar3DNode* parent )
	: UPoint3D( point ), gCost( g ), hCost( h ), fCost( f ), parentNode( parent )
{
	;
}
	
bool
AStar3D::AStar3DNode::operator< (const AStar3DNode& other) const
{
	return fCost < other.fCost;
}

bool
AStar3D::AStar3DNode::operator> (const AStar3DNode& other) const
{
	return fCost > other.fCost;
}

AStar3D::PointToAvoid::PointToAvoid( const UPoint3D& point, float32 avoidanceBlockRadius, float32 heuristicCostMultiplier )
	: UPoint3D( point ), avoidanceBlockRadius( avoidanceBlockRadius ), heuristicCostMultiplier( heuristicCostMultiplier )
{
	;
}

AStar3D::AStar3D( bool doUseFastSqrt )
	: xDiameter_( 0 ), yDiameter_( 0 ), zDiameter_( 0 ), doUseFastSqrt_( doUseFastSqrt ), searchSpaceMin_()
{
	;
}

AStar3D::~AStar3D()
{
	;
}

void
AStar3D::AllocMem( uint32 xDiameter, uint32 yDiameter, uint32 zDiameter )
{
	xDiameter_ = xDiameter;
	yDiameter_ = yDiameter;
	zDiameter_ = zDiameter;

	searchSpaceMin_.x = 0;
	searchSpaceMin_.y = 0;
	searchSpaceMin_.z = 0;

	nodeMap_ = (AStar3DNode**)Calloc( xDiameter * yDiameter * zDiameter, sizeof (AStar3DNode*) ); // returns NULL
}

void
AStar3D::ClearMem( )
{
	ZeroMemory( nodeMap_, sizeof (AStar3DNode*) * xDiameter_ * yDiameter_ * zDiameter_ );
}

void
AStar3D::FreeMem( )
{
	if (nodeMap_)
		Free (nodeMap_);
}

/*
void
AStar3D::Init( bool doTraverseAir, bool doTraverseWater, bool doTraverseLava, uint32 maxJumpDist, uint32 maxClimbDist, uint32 pathingWidth, uint32 pathingHeight )
{
	Assert( maxJumpDist < 1000 );
	Assert( maxClimbDist < 1000 );

	// STODO:
}
*/

void
AStar3D::BeginModifyStartAndGoalSectors( const UPoint3D& start, const UPoint3D& goal )
{
	{
		S_Sector* startSector = GetWorld()->GetSector( start.x, start.y, start.z );
		if( startSector->GetIsModifying() )
			startSector->BeginModify();
	}
	{
		S_Sector* goalSector = GetWorld()->GetSector( goal.x, goal.y, goal.z );
		if( goalSector->GetIsModifying() )
			goalSector->BeginModify();
	}
}

bool
AStar3D::Search( const UPoint3D& start, const UPoint3D& goal, Vector<UPoint3D>& path, uint32 pathingHeight, float32 heuristicCostMultiplier )
{
	Assert( path.GetCount() == 0 );
	// heuristicCostMultipler: 
	//	value of 0: equivalent to dikjstra's algorithm. search space expands radially.
	//	value of 1: standard A*. largest value that can reliably give an optimal path.
	//	values > 1: directs and narrows search space with increasing number. results in suboptimal paths. see "A* search algorithm" on wikipedia for explaination.

	// STODO: PERFORMANCE: optimize this code as much as possible. performance is critical here.

	// STODO: PERFORMANCE: implement a binary heap for storing and sorting open nodes(or pointers to open nodes, see below)

	// STODO: PERFORMANCE: evaluate the speed of pathing when storing objects directly on the vector(or binary heap) compared to storing pointers to objects on the heap

	Assert( heuristicCostMultiplier >= 0.0f );
	Assert( GetWorld()->GetIsModifying() );

	// init search space min with the midpoint between the start and goal representing the center of the search space
	searchSpaceMin_.x = (Min( start.x, goal.x ) + (Max( start.x, goal.x ) - Min( start.x, goal.x )) / 2) - xDiameter_ / 2;
	searchSpaceMin_.y = (Min( start.y, goal.y ) + (Max( start.y, goal.y ) - Min( start.y, goal.y )) / 2) - yDiameter_ / 2;
	searchSpaceMin_.z = (Min( start.z, goal.z ) + (Max( start.z, goal.z ) - Min( start.z, goal.z )) / 2) - zDiameter_ / 2;

	if( !IsSearchPossible_( start, goal ) )
		return false;

	Vector<AStar3DNode*> traversedNodes; // storage location for nodes that have been traversed
	//Vector<AStar3DNode*> openNodes;
	BinaryHeap<AStar3DNode> openNodes(&AStar3DNode::operator<);

	{
		// insert first open node
		float32 distance;
		if( doUseFastSqrt_ )
			distance = start.GetDistanceFast( goal );
		else
			distance = start.GetDistance( goal );

		openNodes.Insert( new AStar3DNode( start, 0.0f, distance, distance, NODE_NULL ) );	
	}

	// open first sector. STODO: possibly make a method that can pre-open sectors at the start and goal nodes
	{
		S_Sector* sector = GetWorld()->GetSector( start.x, start.y, start.z );
		if (!sector->GetIsModifying())
		{
			sector->BeginModify();
			listOfOpenedSectors_.InsertBack( sector );
		}
	}

	while (openNodes.GetCount () > 0)
	{
		//Sort ( openNodes.GetMemory( ), openNodes.GetCount( ), true ); // Note- this sort is preliminary. STODO: consider using a binary heap instead of a quicksorted vector
		AStar3DNode* bestNode = openNodes.PopFront(); // this node has the lowest fCost value

		if( *bestNode == goal )
		{
			ConstructFinalPath_( bestNode, path );
			CloseOpenedSectors_();

			traversedNodes.RemoveAndDeleteAll();
			openNodes.RemoveAndDeleteAll();
			return true;
		}

		// remove best node from open points list and add to closed points
		traversedNodes.InsertBack( bestNode );
		SetNodeInMap_( bestNode->x, bestNode->y, bestNode->z, bestNode ); // add traversed node to node map

		// supersample nearby points if not already in nodeMap_
		if ( bestNode->x == searchSpaceMin_.x || bestNode->x == searchSpaceMin_.x + xDiameter_ - 1  ||
			bestNode->y == searchSpaceMin_.y || bestNode->y == searchSpaceMin_.y + yDiameter_ - 1 ||
			bestNode->z == searchSpaceMin_.z || bestNode->z == searchSpaceMin_.z + zDiameter_ - 1 )
		{
			continue; // prevent accessing memory outside array
		}

		// supersample surrounding locations
		for( uint32 zSample = bestNode->z - 1; zSample <= bestNode->z + 1; ++zSample )
		{
			for( uint32 ySample = bestNode->y - 1; ySample <= bestNode->y + 1; ++ySample )
			{
				for( uint32 xSample = bestNode->x - 1; xSample <= bestNode->x + 1; ++xSample )
				{
					if ( xSample == bestNode->x && ySample == bestNode->y && zSample == bestNode->z )
						continue;

					AStar3DNode* accessedNode = GetNodeInMap_( xSample, ySample, zSample );

					if( accessedNode == NODE_CLOSED )
						continue;

					UPoint3D sampledPoint( xSample, ySample, zSample );

					Assert (accessedNode == NODE_NULL || *accessedNode == sampledPoint);
		
					// STODO: be creative about modifying gCost for new nodes. 
						//		gCost should increase over difficult terrain
						//		gCost should increase more when going up in elevation that when going down
						//		gCost should be significantly higher if climbing or other obstacle traversal is involved at a block
						//		gCost could be lower on roads than other terrain

					float32 newGCost = bestNode->gCost;
					
					if( doUseFastSqrt_ )
						newGCost += sampledPoint.GetDistanceFast( *bestNode );
					else
						newGCost += sampledPoint.GetDistance( *bestNode );

					// check if surrounding location is not in map, add if so
					if ( accessedNode == NODE_NULL )
					{
						if( IsPointTraversible_( sampledPoint, pathingHeight ) )
						{
							// compute h cost
							float32 newHCost = ComputeHCost_( sampledPoint, goal, heuristicCostMultiplier );

							accessedNode = new AStar3DNode( sampledPoint, newGCost, newHCost, newGCost + newHCost, bestNode );
							openNodes.Insert( accessedNode );
						}
						else
						{
							accessedNode = NODE_CLOSED; // marks spot on map as unpathable
						}

						SetNodeInMap_( xSample, ySample, zSample, accessedNode );
					}
					else if ( accessedNode->gCost > newGCost/* * 1.001f*/ ) // Note- this check easily succumbs to floating point error. STODO: make another search method(or class) that skips this step
					{
						// case for nodes that are already traversed or open that can have an improved g cost by linking to the current best node
						accessedNode->gCost = newGCost;
						accessedNode->fCost = accessedNode->hCost + newGCost;
						accessedNode->parentNode = bestNode;
						openNodes.ResortExistingElement(accessedNode);
					}
				}
			}
		}
	}

	CloseOpenedSectors_();

	traversedNodes.RemoveAndDeleteAll();
	openNodes.RemoveAndDeleteAll();

	return false;
}

bool
AStar3D::LeastResistanceSearch( const UPoint3D& start, const UPoint3D& goal, Vector<UPoint3D>& path, float32 pathOfLeastResistanceWeight, float32 heuristicCostMultiplier )
{
	Assert( path.GetCount() == 0 );
	Assert( pathOfLeastResistanceWeight >= 0.0f );
	Assert( pathOfLeastResistanceWeight <= 1.0f );
	Assert( heuristicCostMultiplier >= 0.0f );
	Assert( GetWorld()->GetIsModifying() );

	return false;
}

// ------------------------------------------- Private Methods ----------------------------------------------------

float32
AStar3D::ComputeHCost_( const UPoint3D& currentPoint, const UPoint3D& goal, float32 heuristicCostMultiplier )
{
	float32 hCost = heuristicCostMultiplier;
	if( doUseFastSqrt_ )
	{
		hCost *= currentPoint.GetDistance( goal );

		// add additional h cost for being near points that are to be avoided
		for( uint32 i = 0; i < pointsToAvoid_.GetCount(); ++i )
		{
			float32 distance = currentPoint.GetDistance( pointsToAvoid_[i] );
			if( distance < pointsToAvoid_[i].avoidanceBlockRadius )
				hCost += (pointsToAvoid_[i].avoidanceBlockRadius - distance) * pointsToAvoid_[i].heuristicCostMultiplier;
		}
	}
	else
	{
		hCost *= currentPoint.GetDistanceFast( goal );

		// add additional h cost for being near points that are to be avoided
		for( uint32 i = 0; i < pointsToAvoid_.GetCount(); ++i )
		{
			float32 distance = currentPoint.GetDistanceFast( pointsToAvoid_[i] );
			if( distance < pointsToAvoid_[i].avoidanceBlockRadius )
				hCost += (pointsToAvoid_[i].avoidanceBlockRadius - distance) * pointsToAvoid_[i].heuristicCostMultiplier;
		}
	}

	return hCost;
}

void
AStar3D::ConstructFinalPath_( AStar3DNode* goal, Vector<UPoint3D>& path )
{
	Assert (goal != NODE_NULL);
	Assert( path.GetCount() == 0 );
	Vector<UPoint3D*> tmpReversePath;

	AStar3DNode* currentNode = goal;
	while ( currentNode != NULL )
	{		
		tmpReversePath.InsertBack( currentNode ); // Note- goal will be at front of list and start will be at end	
		currentNode = currentNode->parentNode;
	}

	// put path points in order from start to goal
	for( int32 i = tmpReversePath.GetCount() - 1; i >= 0; --i )
	{
		path.InsertBack( UPoint3D( *tmpReversePath[i] ) );
	}
}

void
AStar3D::ConstructFinalPathBackwardsFast_( AStar3DNode* goal, Vector<UPoint3D>& path )
{
	Assert (goal != NODE_NULL);
	Assert( path.GetCount() == 0 );

	AStar3DNode* currentNode = goal;
	while ( currentNode != NULL )
	{		
		path.InsertBack( UPoint3D( *currentNode ) ); // Note- goal will be at front of list and start will be at end	
		currentNode = currentNode->parentNode;
	}
}

bool
AStar3D::IsPointTraversible_( const UPoint3D& sampledPoint, uint32 pathingHeight )
{
	// STODO: in the future expand upon the definition of what is pathable.

	// add to open points or closed points
	S_Sector* openedSector = NULL;
	uint32 sampledSCubeBuf = GetWorld()->GetSCube( sampledPoint.x, sampledPoint.y, sampledPoint.z, openedSector );
	if( openedSector != NULL )
		listOfOpenedSectors_.InsertBack( openedSector );
	openedSector = NULL;
	uint32 sampledSCubeBufDown = GetWorld()->GetSCube( sampledPoint.x, sampledPoint.y, sampledPoint.z - 1, openedSector );
	if( openedSector != NULL )
		listOfOpenedSectors_.InsertBack( openedSector );

	// for now check if the sampled location is air and if the sampled down location is ground to determine if it is pathable.
	if( GetSCubeTerrainType( sampledSCubeBuf ) == TERRA_AIR && GetSCubeTerrainType( sampledSCubeBufDown ) != TERRA_AIR ) // STODO: add checks for water, lava, others in some cases
	{
		for( uint32 zOffset = 1; zOffset < pathingHeight; ++zOffset )
		{
			openedSector = NULL;
			bool isBlockClear = GetSCubeTerrainType( GetWorld()->GetSCube( sampledPoint.x, sampledPoint.y, sampledPoint.z + zOffset, openedSector ) ) == TERRA_AIR;
			if( openedSector != NULL )
				listOfOpenedSectors_.InsertBack( openedSector );
			if( !isBlockClear )
				return false;
		}

		if( pathableAreas_.GetCount() > 0 )
		{
			bool isPathable = false;
			for( uint32 i = 0; i < pathableAreas_.GetCount(); ++i )
			{
				const BaseArea& area = pathableAreas_[i];
				if( IntersectBoxWithPoint( sampledPoint.x, sampledPoint.y, sampledPoint.z, area.GetMin(), area.GetMax() ) )
				{
					isPathable = true;
					break;
				}
			}
			if( !isPathable )
				return false;
		}

		// check if point intersects with any unpathable areas. STODO: use octree
		for( uint32 i = 0; i < unpathableAreas_.GetCount(); ++i )
		{
			const BaseArea& area = unpathableAreas_[i];
			if( IntersectBoxWithPoint( sampledPoint.x, sampledPoint.y, sampledPoint.z, area.GetMin(), area.GetMax() ) )
				return false;
		}

		return true;
	}
	return false;
}