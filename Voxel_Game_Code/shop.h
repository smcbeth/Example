//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "basezone.h"
#include "basebuilding.h"

#include "displayroom.h"

class BaseZone;
enum BuildingSize;
enum Direction;

class Shop: public BaseBuilding
{
public:
	Shop (BaseZone* zone, BuildingSize size, uint32 numResidents);
	~Shop ();

	void			Build ();
	bool			PlaceRooms (Direction doorSide);

	BuildingType	GetType () const;

protected:
	bool LinkRooms_ ();
	void PlaceWindows_ ();

	void SetDisplayRoomInitData_ (RoomInitData& initData, uint32 floorLevel);
};