//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "serverentitylist.h"
#include "s_basefurniture.h"
#include "bedroom.h"

Bedroom::Bedroom( RoomInitData& initData )
	: BaseRoom( initData )
{
	doorHeight_ = 2;
	exteriorDoorType_ = DOOR_SINGLE;
}

Bedroom::~Bedroom( )
{
	;
}

void
Bedroom::BuildFoundation( uint32 thickness )
{
	BaseRoom::BuildFoundation( thickness );
}

void
Bedroom::BuildFloor( )
{
	BaseRoom::BuildFloor();
}

void
Bedroom::BuildWalls( )
{
	BaseRoom::BuildWalls();
}

void
Bedroom::PlaceObjects( )
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
Bedroom::PlaceWindows( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance )
{
	BaseRoom::PlaceWindows( heightAboveFloor, height, width, windowSpacing, placementChance );
}

DoorType 
Bedroom::GetConnectingDoorType( BaseRoom* childRoom )
{
	return DOOR_SINGLE;
}

RoomType
Bedroom::GetType( )
{
	return ROOM_BEDROOM;
}