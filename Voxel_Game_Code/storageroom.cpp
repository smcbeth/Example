//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "storageroom.h"

StorageRoom::StorageRoom( RoomInitData& initData )
	: BaseRoom( initData )
{
	doorHeight_ = 2;
	exteriorDoorType_ = DOOR_SINGLE;
}

StorageRoom::~StorageRoom( )
{
	;
}

void
StorageRoom::BuildFoundation( uint32 thickness )
{
	BaseRoom::BuildFoundation( thickness );
}

void
StorageRoom::BuildFloor( )
{
	BaseRoom::BuildFloor();
}

void
StorageRoom::BuildWalls( )
{
	BaseRoom::BuildWalls();
}

void
StorageRoom::PlaceObjects( )
{
	BaseRoom::PlaceObjects();
}

void
StorageRoom::PlaceWindows( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance )
{
	BaseRoom::PlaceWindows( heightAboveFloor, height, width, windowSpacing, placementChance );
}

DoorType 
StorageRoom::GetConnectingDoorType( BaseRoom* childRoom )
{
	return DOOR_SINGLE;
}

RoomType
StorageRoom::GetType( )
{
	return ROOM_STORAGEROOM;
}