//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "basezone.h"
#include "basebuilding.h"

#include "bedroom.h"
#include "kitchen.h"
#include "livingroom.h"

class BaseZone;
enum BuildingSize;
enum Direction;

class House: public BaseBuilding
{
public:
	House (BaseZone* zone, BuildingSize size, uint32 numResidents);
	~House ();

	void			Build ();
	bool			PlaceRooms (Direction doorSide);

	BuildingType	GetType () const;

protected:
	bool LinkRooms_ ();
	void PlaceWindows_ ();

	void SetBedroomInitData_ (RoomInitData& initData, uint32 floorLevel, bool isMainRoom);
	void SetKitchenInitData_ (RoomInitData& initData, uint32 floorLevel);
	void SetLivingRoomInitData_ (RoomInitData& initData, uint32 floorLevel, bool isMainRoom);
};