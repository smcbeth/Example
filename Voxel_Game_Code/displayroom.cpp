//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "serverentitylist.h"
#include "s_basefurniture.h"
#include "displayroom.h"

DisplayRoom::DisplayRoom( RoomInitData& initData )
	: BaseRoom( initData )
{
	doorHeight_ = 2;
	exteriorDoorType_ = DOOR_SINGLE;
}

DisplayRoom::~DisplayRoom( )
{
	;
}

void
DisplayRoom::BuildFoundation( uint32 thickness )
{
	BaseRoom::BuildFoundation( thickness );
}

void
DisplayRoom::BuildFloor( )
{
	BaseRoom::BuildFloor();
}

void
DisplayRoom::BuildWalls( )
{
	BaseRoom::BuildWalls();
}

void
DisplayRoom::PlaceObjects( )
{
	if (rand () % 2 == 0)
	{
		S_BaseFurniture* table = (S_BaseFurniture*)GetServerEntityList()->Create( "table" );
		table->SetBOrigin( UPoint3D( min_.x + 1 + (rand () % GetLengthX() - 1), min_.y + 1 + (rand () % GetLengthY() - 1), min_.z ) );
	}
	else
	{
		S_BaseFurniture* chair = (S_BaseFurniture*)GetServerEntityList()->Create( "chair" );
		chair->SetBOrigin( UPoint3D( min_.x + 1 + (rand () % GetLengthX() - 1), min_.y + 1 + (rand () % GetLengthY() - 1), min_.z ) );
	}

	BaseRoom::PlaceObjects();
}

void
DisplayRoom::PlaceWindows( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance )
{
	BaseRoom::PlaceWindows( heightAboveFloor, height, width, windowSpacing, placementChance );
}

DoorType 
DisplayRoom::GetConnectingDoorType( BaseRoom* childRoom )
{
	return DOOR_SINGLE;
}

RoomType
DisplayRoom::GetType( )
{
	return ROOM_DISPLAYROOM;
}