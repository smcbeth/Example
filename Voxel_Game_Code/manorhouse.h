//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "basezone.h"
#include "basebuilding.h"

#include "bedroom.h"
#include "greathall.h"
#include "hallway.h"
#include "kitchen.h"
#include "library.h"
#include "office.h"
#include "stairhall.h"
#include "treasuryroom.h"

class BaseZone;
enum BuildingSize;
enum Direction;

class ManorHouse: public BaseBuilding
{
public:
	ManorHouse (BaseZone* zone, BuildingSize size, uint32 numResidents);
	~ManorHouse ();

	void			Build ();
	bool			PlaceRooms (Direction doorSide);

	BuildingType	GetType () const;

protected:
	bool LinkRooms_ ();
	void PlaceWindows_ ();

	void SetBedroomInitData_ (RoomInitData& initData, uint32 floorLevel);
	void SetGreatHallInitData_ (RoomInitData& initData, uint32 floorLevel);
	void SetHallwayInitData_ (RoomInitData& initData, uint32 floorLevel, bool doAllowLongHallway);
	void SetKitchenInitData_ (RoomInitData& initData, uint32 floorLevel);
	void SetLibraryInitData_ (RoomInitData& initData, uint32 floorLevel);
	void SetOfficeInitData_ (RoomInitData& initData, uint32 floorLevel);
	void SetStairHallInitData_ (RoomInitData& initData, uint32 floorLevel);
	void SetTreasuryRoomInitData_ (RoomInitData& initData, uint32 floorLevel);
};