//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "basebuilding.h"

class BaseBuilding;
class BaseRoom;

struct RoomLinkerInitData
{
	BaseBuilding*	building;
	uint32			maxStairsPerFloor;
	uint32			maxParentRooms;
	uint32			maxChildRooms;
	float32			doorPlaceChance;
	float32			stairPlaceChance;
	float32			spiralStairTypeChance;
};

// SNOW: add building failure possibilities to the roomlinker. this is especially useful for perfecting the linking of smaller buildings. this failure can occur when forced linking is used
	// example: a small (not dungeon) building should fail when a stairway intersects with a door. 
	// preventing miscellaneous linking bugs like this is vital to the look and feel of the interiors of buildings
	// STODO: consider doing a linking pass after each and every room is placed. this would allow for failures to not force an entire redo of the building.
		// have the linking pass treat the room that was just placed as a child room and all other rooms as parent rooms

class RoomLinker
{
public:
	explicit RoomLinker( RoomLinkerInitData& initData );
	~RoomLinker();

	// STODO: simplify this method as much as possible, it will only get more complex later.
	bool Link();
	
private:
	uint32	AttemptLink_( BaseRoom* parentRoom, BaseRoom* childRoom, uint32 pRoomFloorLevel, uint32 cRoomFloorLevel );
	uint32	AttemptHorizontalLink_( BaseRoom* parentRoom, BaseRoom* childRoom, uint32 floorLevel );
	uint32	AttemptVerticalLink_( BaseRoom* parentRoom, BaseRoom* childRoom, uint32 pRoomFloorLevel, uint32 cRoomFloorLevel, bool doIgnoreMaxStairsPerFloor );
	void	ConditionallyRemoveChildRoom_( uint32 childRoomIndex, uint32 floorLevel );
	void	UpdateLinkingFactors_( uint32 linkingCount, uint32& failedLinkCount );
	
	// RoomLinkerInitData variables
	const BaseBuilding*			building_;
	const uint32				maxStairsPerFloor_;
	const uint32				maxChildRooms_;
	const uint32				maxParentRooms_;
	const float32				doorPlaceChance_;
	const float32				stairPlaceChance_;
	const float32				spiralStairTypeChance_;

	// other variables
	bool						likelyLinking_;
	bool						forcedLinkingLevel1_;	// STODO: make sure that forcedLinking never has to be made true
	bool						forcedLinkingLevel2_;
	Vector<Vector<BaseRoom*>*>	childRooms_;			// rooms that can potentially be linked to from parent rooms
	Vector<Vector<BaseRoom*>*>	parentRooms_;			// rooms that can potentially link to child rooms
	Vector<uint32>				unlinkedRoomsPerFloor_;
	Vector<uint32>				stairsPerFloor_;
};