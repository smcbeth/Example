//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "serverentitylist.h"
#include "s_basefurniture.h"
#include "treasuryroom.h"

TreasuryRoom::TreasuryRoom( RoomInitData& initData )
	: BaseRoom( initData )
{
	doorHeight_ = 2;
	exteriorDoorType_ = DOOR_SINGLE;
}

TreasuryRoom::~TreasuryRoom( )
{
	;
}

void
TreasuryRoom::BuildFoundation( uint32 thickness )
{
	BaseRoom::BuildFoundation( thickness );
}

void
TreasuryRoom::BuildFloor( )
{
	BaseRoom::BuildFloor();
}

void
TreasuryRoom::BuildWalls( )
{
	BaseRoom::BuildWalls();
}

void
TreasuryRoom::PlaceObjects( )
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
TreasuryRoom::PlaceWindows( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance )
{
	BaseRoom::PlaceWindows( heightAboveFloor, height, width, windowSpacing, placementChance );
}

DoorType 
TreasuryRoom::GetConnectingDoorType( BaseRoom* childRoom )
{
	return DOOR_SINGLE;
}

RoomType
TreasuryRoom::GetType( )
{
	return ROOM_TREASURYROOM;
}