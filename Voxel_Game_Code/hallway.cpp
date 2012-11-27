//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "hallway.h"

Hallway::Hallway( RoomInitData& initData )
	: BaseRoom( initData )
{
	doorHeight_ = 2;
	exteriorDoorType_ = DOOR_SINGLE;
}

Hallway::~Hallway( )
{
	;
}

void
Hallway::BuildFoundation( uint32 thickness )
{
	BaseRoom::BuildFoundation( thickness );
}

void
Hallway::BuildFloor( )
{
	BaseRoom::BuildFloor();
}

void
Hallway::BuildWalls( )
{
	BaseRoom::BuildWalls();
}

void
Hallway::PlaceObjects( )
{
	BaseRoom::PlaceObjects();
}

void
Hallway::PlaceWindows( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance )
{
	BaseRoom::PlaceWindows( heightAboveFloor, height, width, windowSpacing, placementChance );
}

DoorType 
Hallway::GetConnectingDoorType( BaseRoom* childRoom )
{
	if( childRoom->GetType() == ROOM_HALLWAY )
		return DOOR_NONE;
	if ( childRoom->GetType() == ROOM_GREATHALL )
		return DOOR_NONE;
	if ( childRoom->GetType() == ROOM_STAIRHALL )
		return DOOR_NONE;
	if ( childRoom->GetType() == ROOM_DUNGEONROOM )
		return DOOR_DOUBLE;
	if ( childRoom->GetType() == ROOM_TREASUREROOM )
		return DOOR_DOUBLE;
	if( childRoom->GetType() == ROOM_ROOFTOP && parentBuilding_->GetType() == BUILDING_DUNGEONTOWER )
		return DOOR_DOUBLE;

	return DOOR_SINGLE;
}

RoomType
Hallway::GetType( )
{
	return ROOM_HALLWAY;
}