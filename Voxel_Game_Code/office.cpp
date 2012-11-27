//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "serverentitylist.h"
#include "s_basefurniture.h"
#include "office.h"

Office::Office( RoomInitData& initData )
	: BaseRoom( initData )
{
	doorHeight_ = 2;
	exteriorDoorType_ = DOOR_SINGLE;
}

Office::~Office()
{
	;
}

void
Office::BuildFoundation( uint32 thickness )
{
	BaseRoom::BuildFoundation( thickness );
}

void
Office::BuildFloor( )
{
	BaseRoom::BuildFloor();
}

void
Office::BuildWalls( )
{
	BaseRoom::BuildWalls();
}

void
Office::PlaceObjects( )
{
	UPoint3D tablePoint = UPoint3D( min_.x + 2 + (rand () % GetLengthX() - 2), min_.y + 2 + (rand () % GetLengthY() - 2), min_.z );
	
	S_BaseFurniture* table = (S_BaseFurniture*)GetServerEntityList()->Create( "table" );
	table->SetBOrigin( tablePoint );

	S_BaseFurniture* chair = (S_BaseFurniture*)GetServerEntityList()->Create( "chair" );
	switch (rand () % 4)
	{
	case 0:
		chair->SetBOrigin( UPoint3D( tablePoint.x - 1, tablePoint.y, min_.z ) );
		break;
	case 1:
		chair->SetBOrigin( UPoint3D( tablePoint.x + 1, tablePoint.y, min_.z ) );
		break;
	case 2:
		chair->SetBOrigin( UPoint3D( tablePoint.x, tablePoint.y - 1, min_.z ) );
		break;
	case 3:
		chair->SetBOrigin( UPoint3D( tablePoint.x, tablePoint.y + 1, min_.z ) );
		break;
	}

	BaseRoom::PlaceObjects();
}

void
Office::PlaceWindows( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance )
{
	BaseRoom::PlaceWindows( heightAboveFloor, height, width, windowSpacing, placementChance );
}

DoorType 
Office::GetConnectingDoorType( BaseRoom* childRoom )
{
	return DOOR_SINGLE;
}

RoomType
Office::GetType( )
{
	return ROOM_OFFICE;
}