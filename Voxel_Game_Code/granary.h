//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "basezone.h"
#include "basebuilding.h"

#include "storageroom.h"

class BaseZone;
enum BuildingSize;
enum Direction;

class Granary: public BaseBuilding
{
public:
	Granary (BaseZone* zone, BuildingSize size, uint32 numResidents);
	~Granary ();

	void			Build ();
	bool			PlaceRooms (Direction doorSide);

	BuildingType	GetType () const;

protected:
	bool LinkRooms_ ();
	void PlaceWindows_ ();

	void SetStorageRoomInitData_ (RoomInitData& initData, uint32 floorLevel);
};