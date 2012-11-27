//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "stairhall.h"

StairHall::StairHall( RoomInitData& initData )
	: BaseRoom( initData )
{
	doorHeight_ = 2;
	exteriorDoorType_ = DOOR_SINGLE;
}

StairHall::~StairHall( )
{
	;
}

void
StairHall::BuildFoundation( uint32 thickness )
{
	BaseRoom::BuildFoundation( thickness );
}

void
StairHall::BuildFloor( )
{
	BaseRoom::BuildFloor();
}

void
StairHall::BuildWalls( )
{
	BaseRoom::BuildWalls();
}

void
StairHall::PlaceObjects( )
{
	BaseRoom::PlaceObjects();
}

void
StairHall::PlaceWindows( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance )
{
	BaseRoom::PlaceWindows( heightAboveFloor, height, width, windowSpacing, placementChance );
}

DoorType 
StairHall::GetConnectingDoorType( BaseRoom* childRoom )
{
	if( childRoom->GetType() == ROOM_HALLWAY )
		return DOOR_NONE;
	if ( childRoom->GetType() == ROOM_GREATHALL )
		return DOOR_NONE;
	if ( childRoom->GetType() == ROOM_LIVINGROOM )
		return DOOR_NONE;
	return DOOR_SINGLE;
}

RoomType
StairHall::GetType( )
{
	return ROOM_STAIRHALL;
}