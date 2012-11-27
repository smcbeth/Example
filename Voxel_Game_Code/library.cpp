//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "serverentitylist.h"
#include "s_basefurniture.h"
#include "library.h"

Library::Library( RoomInitData& initData )
	: BaseRoom( initData )
{
	doorHeight_ = 2;
	exteriorDoorType_ = DOOR_SINGLE;
}

Library::~Library()
{
	;
}

void
Library::BuildFoundation( uint32 thickness )
{
	BaseRoom::BuildFoundation( thickness );
}

void
Library::BuildFloor( )
{
	BaseRoom::BuildFloor();
}

void
Library::BuildWalls( )
{
	BaseRoom::BuildWalls();
}

void
Library::PlaceObjects( )
{
	UPoint3D tablePoint = UPoint3D( min_.x + 2 + (rand () % GetLengthX() - 2), min_.y + 2 + (rand () % GetLengthY() - 2), min_.z );
	
	S_BaseFurniture* table = (S_BaseFurniture*)GetServerEntityList()->Create( "table" );
	table->SetBOrigin( tablePoint );

	S_BaseFurniture* chair1 = (S_BaseFurniture*)GetServerEntityList()->Create( "chair" );
	S_BaseFurniture* chair2 = (S_BaseFurniture*)GetServerEntityList()->Create( "chair" );

	if (rand () % 2 == 0)
	{	
		chair1->SetBOrigin( UPoint3D( tablePoint.x - 1, tablePoint.y, min_.z ) );
		chair2->SetBOrigin( UPoint3D( tablePoint.x + 1, tablePoint.y, min_.z ) );
	}
	else
	{
		chair1->SetBOrigin( UPoint3D( tablePoint.x, tablePoint.y - 1, min_.z ) );
		chair2->SetBOrigin( UPoint3D( tablePoint.x, tablePoint.y + 1, min_.z ) );
	}

	BaseRoom::PlaceObjects();
}

void
Library::PlaceWindows( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance )
{
	BaseRoom::PlaceWindows( heightAboveFloor, height, width, windowSpacing, placementChance );
}

DoorType 
Library::GetConnectingDoorType( BaseRoom* childRoom )
{
	return DOOR_SINGLE;
}

RoomType
Library::GetType( )
{
	return ROOM_LIBRARY;
}