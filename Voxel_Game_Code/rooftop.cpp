//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "rooftop.h"

RoofTop::RoofTop( RoomInitData& initData )
	: BaseRoom( initData )
{
	doorHeight_ = 2;
	exteriorDoorType_ = DOOR_SINGLE;
}

RoofTop::~RoofTop( )
{
	;
}

void
RoofTop::BuildFoundation( uint32 thickness )
{
	BaseRoom::BuildFoundation( thickness );
}

void
RoofTop::BuildFloor( )
{
	uint32 sCubeBuf;
	int32 zOff = 0;
	for (uint32 y = min_.y; y <= max_.y; ++y)
	{
		for (uint32 x = min_.x; x <= max_.x; ++x)
		{
			sCubeBuf = 0;
			if (IsPointOnExteriorWall (x, y, (int32)min_.z + zOff + 1, min_.z, false))
			{
				SetSCubeTerrainType (&sCubeBuf, parentBuilding_->GetOuterWallMaterial ()); // makes sure floor isnt placed on exterior of building.
			}

			if (sCubeBuf == 0)
				SetSCubeTerrainType (&sCubeBuf, parentBuilding_->GetFloorMaterial ()); // interior floor

			GetWorld()->SetSCube(x, y, min_.z + zOff, sCubeBuf);
		}
	}
}

void
RoofTop::BuildWalls( )
{
	uint32 sCubeBuf;

	// open roof and parapet construction
	if (parentBuilding_->GetRoofType () == ROOF_OPENPARAPETS)
	{
		uint32 addVal = 0;
		if (GetLengthX () % 2 != 0 || (GetLengthX () + GetLengthY ()) % 2 != 0)
			++addVal; // helps make sure that parapet high points usually start in corners, which improves their look

		for (uint32 y = min_.y; y <= max_.y; ++y)
		{
			for (uint32 x = min_.x; x <= max_.x; ++x)
			{
				for (int32 zOff = 1; zOff < 2 + int32((x + y + addVal) % 2); ++zOff)
				{
					sCubeBuf = 0;
					if (IsPointOnExteriorWall (x, y, (int32)min_.z + zOff, min_.z, false))
					{
						SetSCubeTerrainType (&sCubeBuf, parentBuilding_->GetOuterWallMaterial ());
						GetWorld()->SetSCube(x, y, min_.z + zOff, sCubeBuf);
					}
				}
			}
		}
	}
	else
	{
		for (uint32 y = min_.y; y <= max_.y; ++y)
		{
			for (uint32 x = min_.x; x <= max_.x; ++x)
			{
				for (int32 zOff = 1; zOff < 2; ++zOff)
				{
					sCubeBuf = 0;
					if (IsPointOnExteriorWall (x, y, (int32)min_.z + zOff, min_.z, false))
					{
						SetSCubeTerrainType (&sCubeBuf, parentBuilding_->GetOuterWallMaterial ());
						GetWorld()->SetSCube(x, y, min_.z + zOff, sCubeBuf);
					}
				}
			}
		}
	}
}

void
RoofTop::PlaceObjects( )
{
	BaseRoom::PlaceObjects();
}

void
RoofTop::PlaceWindows( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance )
{
	; // do nothing since windows should not be placed here
}

DoorType 
RoofTop::GetConnectingDoorType( BaseRoom* childRoom )
{
	if( childRoom->GetType() == ROOM_ROOFTOP )
		return DOOR_NONE;
	if( childRoom->GetType() == ROOM_DUNGEONROOM )
		return DOOR_DOUBLE;
	if( childRoom->GetType() == ROOM_TREASUREROOM )
		return DOOR_DOUBLE;
	if( childRoom->GetType() == ROOM_HALLWAY && parentBuilding_->GetType() == BUILDING_DUNGEONTOWER )
		return DOOR_DOUBLE;
	return DOOR_SINGLE;
}

RoomType
RoofTop::GetType( )
{
	return ROOM_ROOFTOP;
}