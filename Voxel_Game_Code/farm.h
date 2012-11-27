//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "basezone.h"

class Farm: public BaseZone
{
public:
	Farm (uint32 xMin, uint32 yMin);
	~Farm ();

protected:
	void LevelTerrain_ ();
	void GenerateHazardExtents_ ();
};