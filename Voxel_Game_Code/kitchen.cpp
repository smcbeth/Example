//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "serverentitylist.h"
#include "s_basefurniture.h"
#include "kitchen.h"

Kitchen::Kitchen( RoomInitData& initData )
	: BaseRoom( initData )
{
	doorHeight_ = 2;
	exteriorDoorType_ = DOOR_SINGLE;
}

Kitchen::~Kitchen()
{
	;
}

void
Kitchen::BuildFoundation( uint32 thickness )
{
	BaseRoom::BuildFoundation( thickness );
}

void
Kitchen::BuildFloor( )
{
	BaseRoom::BuildFloor();
}

void
Kitchen::BuildWalls( )
{
	BaseRoom::BuildWalls();
}

void
Kitchen::PlaceObjects( )
{
	BaseRoom::PlaceObjects();
}

void
Kitchen::PlaceWindows( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance )
{
	BaseRoom::PlaceWindows( heightAboveFloor, height, width, windowSpacing, placementChance );
}

DoorType 
Kitchen::GetConnectingDoorType( BaseRoom* childRoom )
{
	if( childRoom->GetType() == ROOM_LIVINGROOM )
		return DOOR_NONE;
	return DOOR_SINGLE;
}

RoomType
Kitchen::GetType( )
{
	return ROOM_KITCHEN;
}