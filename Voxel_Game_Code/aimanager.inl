FORCE_INLINE bool 
AIManager::GetPathUnknownZ( const UPoint3D& start, const UPoint2D& goal, Vector<UPoint3D>& path, uint32 pathingHeight, float32 heuristicCostMultiplier )
{
	return GetPathFindZ_( &AIManager::GetPath, start, goal, path, pathingHeight, heuristicCostMultiplier );
}

FORCE_INLINE bool
AIManager::GetShortRangePath( const UPoint3D& start, const UPoint3D& goal, Vector<UPoint3D>& path, uint32 pathingHeight, float32 heuristicCostMultiplier )
{
	Assert( path.GetCount() == 0 );

	return shortRangeSearcher_.Search( start, goal, path, pathingHeight, heuristicCostMultiplier );
}

FORCE_INLINE bool 
AIManager::GetShortRangePathUnknownZ( const UPoint3D& start, const UPoint2D& goal, Vector<UPoint3D>& path, uint32 pathingHeight, float32 heuristicCostMultiplier )
{
	return GetPathFindZ_( &AIManager::GetShortRangePath, start, goal, path, pathingHeight, heuristicCostMultiplier );
}