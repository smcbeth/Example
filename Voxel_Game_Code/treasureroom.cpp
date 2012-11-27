//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "treasureroom.h"

TreasureRoom::TreasureRoom( RoomInitData& initData )
	: BaseRoom( initData )
{
	doorHeight_ = 2;
	exteriorDoorType_ = DOOR_SINGLE;
}

TreasureRoom::~TreasureRoom()
{
	;
}

void
TreasureRoom::BuildFoundation( uint32 thickness )
{
	BaseRoom::BuildFoundation( thickness );
}

void
TreasureRoom::BuildFloor( )
{
	BaseRoom::BuildFloor();
}

void
TreasureRoom::BuildWalls( )
{
	BaseRoom::BuildWalls();
}

void
TreasureRoom::PlaceObjects( )
{
	BaseRoom::PlaceObjects();
}

void
TreasureRoom::PlaceWindows( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance )
{
	BaseRoom::PlaceWindows( heightAboveFloor, height, width, windowSpacing, placementChance );
}

DoorType 
TreasureRoom::GetConnectingDoorType( BaseRoom* childRoom )
{
	return DOOR_DOUBLE;
}

RoomType
TreasureRoom::GetType( )
{
	return ROOM_TREASUREROOM;
}