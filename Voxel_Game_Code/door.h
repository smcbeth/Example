//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\gmath.h"

#include "world.h"
#include "basearea.h"
#include "terrainhelper.h"

enum DoorType
{
	DOOR_NONE = 0, // open division between rooms
	DOOR_SINGLE,
	DOOR_DOUBLE
};

class Door
{
public:
	Door (UPoint3D& min, UPoint3D& max, Direction side, DoorType type, TERRATYPE floorMaterial, uint32 ceilingZLevel);
	~Door ();

	void Build ();

	UPoint3D& GetMin ();
	UPoint3D& GetMax ();
	Direction GetSide ();
	DoorType GetType ();
	TERRATYPE GetFloorMaterial ();
	
private:
	UPoint3D min_;
	UPoint3D max_;
	Direction side_;
	DoorType type_;
	TERRATYPE floorMaterial_;
	uint32 ceilingZLevel_;
};