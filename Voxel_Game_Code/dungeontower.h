//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "basebuilding.h"
#include "basezone.h"

#include "dungeonroom.h"
#include "hallway.h"
#include "rooftop.h"
#include "treasureroom.h"

class BaseZone;
enum BuildingSize;
enum Direction;

class DungeonTower: public BaseBuilding
{
public:
	DungeonTower (BaseZone* zone, BuildingSize size, uint32 numResidents);
	~DungeonTower ();

	void			Build ();
	bool			PlaceRooms (Direction doorSide);

	BuildingType	GetType () const;

private:
	void FinalizeRoomPlacement_ ();
	bool LinkRooms_ ();
	void PlaceWindows_ ();

	void SetDungeonRoomInitData_ (RoomInitData& initData, uint32 floorLevel, bool isMainRoom);
	void SetHallwayInitData_ (RoomInitData& initData, uint32 floorLevel);
	void SetRoofTopInitData_ (RoomInitData& initData, uint32 floorLevel, uint32 minLength, uint32 maxLength);
	void SetTreasureRoomInitData_ (RoomInitData& initData, uint32 floorLevel);
};