//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "basearea.h"
#include "basezone.h"
#include "town.h"

class BaseZone;

enum LandType
{
	LAND_LAWN = 0,
	LAND_WHEATCROP,
	LAND_PASTURE,
	LAND_TRAININGGROUND,
	LAND_CEMETERY,
	LAND_PARK
};

class Land: public BaseArea
{
	Land ();
	~Land ();

	BaseZone* parentZone_;
	UPoint3D min_;
	UPoint3D max_;
	LandType type_;
};