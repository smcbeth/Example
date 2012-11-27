FORCEINLINE void
AStar3D::AddPointToAvoid( const UPoint3D& point, float32 avoidanceBlockRadius, float32 heuristicCostMultiplier )
{
	pointsToAvoid_.InsertBack( PointToAvoid( point, avoidanceBlockRadius, heuristicCostMultiplier ) );
}

FORCEINLINE void
AStar3D::ClearPointsToAvoid( )
{
	pointsToAvoid_.RemoveAll();
}

FORCEINLINE void
AStar3D::AddPathableArea( const BaseArea& area )
{
	pathableAreas_.InsertBack( area );
}

FORCEINLINE void
AStar3D::ClearPathableAreas( )
{
	pathableAreas_.RemoveAll();
}

FORCEINLINE void
AStar3D::AddUnpathableArea( const BaseArea& area )
{
	unpathableAreas_.InsertBack( area );
}

FORCEINLINE void
AStar3D::ClearUnpathableAreas( )
{
	unpathableAreas_.RemoveAll();
}

FORCE_INLINE AStar3D::AStar3DNode* 
AStar3D::GetNodeInMap_( uint32 x, uint32 y, uint32 z )
{
	Assert( x >= searchSpaceMin_.x );
	Assert( y >= searchSpaceMin_.y );
	Assert( z >= searchSpaceMin_.z );
	Assert( x - searchSpaceMin_.x < xDiameter_ );
	Assert( y - searchSpaceMin_.y < yDiameter_ );
	Assert( z - searchSpaceMin_.z < zDiameter_ );

	return nodeMap_[ x - searchSpaceMin_.x + (y - searchSpaceMin_.y) * xDiameter_ + (z - searchSpaceMin_.z) * (xDiameter_ * yDiameter_) ];
}

FORCE_INLINE void
AStar3D::SetNodeInMap_( uint32 x, uint32 y, uint32 z, AStar3DNode* node )
{
	Assert( x >= searchSpaceMin_.x );
	Assert( y >= searchSpaceMin_.y );
	Assert( z >= searchSpaceMin_.z );
	Assert( x - searchSpaceMin_.x < xDiameter_ );
	Assert( y - searchSpaceMin_.y < yDiameter_ );
	Assert( z - searchSpaceMin_.z < zDiameter_ );

	nodeMap_[ x - searchSpaceMin_.x + (y - searchSpaceMin_.y) * xDiameter_ + (z - searchSpaceMin_.z) * (xDiameter_ * yDiameter_) ] = node;
}

FORCE_INLINE bool
AStar3D::IsSearchPossible_( const UPoint3D& start, const UPoint3D& goal )
{
	// check if search is possible in given area
	if (start.x < searchSpaceMin_.x || start.x >= searchSpaceMin_.x + xDiameter_)
		return false;
	if (start.y < searchSpaceMin_.y || start.y >= searchSpaceMin_.y + yDiameter_)
		return false;
	if (start.z < searchSpaceMin_.z || start.z >= searchSpaceMin_.z + zDiameter_)
		return false;
	if (goal.x < searchSpaceMin_.x || goal.x >= searchSpaceMin_.x + xDiameter_)
		return false;
	if (goal.y < searchSpaceMin_.y || goal.y >= searchSpaceMin_.y + yDiameter_)
		return false;
	if (goal.z < searchSpaceMin_.z || goal.z >= searchSpaceMin_.z + zDiameter_)
		return false;

	return true;
}

FORCE_INLINE void
AStar3D::CloseOpenedSectors_()
{
	while (listOfOpenedSectors_.GetCount () > 0)
	{
		listOfOpenedSectors_.PopBack()->EndModify();  // Note- this is only thread safe with other A* searches and threads that do not open sectors.
	}
}