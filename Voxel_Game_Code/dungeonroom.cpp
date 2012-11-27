//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "serverentitylist.h"
#include "s_basefurniture.h"
#include "dungeonroom.h"

DungeonRoom::DungeonRoom( RoomInitData& initData )
	: BaseRoom( initData )
{
	doorHeight_ = 2;
	exteriorDoorType_ = DOOR_DOUBLE;
}

DungeonRoom::~DungeonRoom()
{
	;
}

void
DungeonRoom::BuildFoundation( uint32 thickness )
{
	BaseRoom::BuildFoundation( thickness );
}

void
DungeonRoom::BuildFloor( )
{
	BaseRoom::BuildFloor();
}

void
DungeonRoom::BuildWalls( )
{
	BaseRoom::BuildWalls();
}

void
DungeonRoom::PlaceObjects( )
{
	BaseRoom::PlaceObjects();
}

void
DungeonRoom::PlaceWindows( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance )
{
	BaseRoom::PlaceWindows( heightAboveFloor, height, width, windowSpacing, placementChance );
}

DoorType 
DungeonRoom::GetConnectingDoorType( BaseRoom* childRoom )
{
	return DOOR_DOUBLE;
}

RoomType
DungeonRoom::GetType( )
{
	return ROOM_DUNGEONROOM;
}