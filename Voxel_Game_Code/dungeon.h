//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "basezone.h"
#include "dungeontower.h"

// STODO: add platforming features to dungeons to make them more fun.
	// ex: pits, platforms to jump on, hard to reach doors, water filled rooms, etc...

class Dungeon: public BaseZone
{
public:
	Dungeon (uint32 xMin, uint32 yMin, bool doLevelTerrain);
	~Dungeon ();

	bool PlaceBuildings ();

protected:
	void LevelTerrain_ ();
	void GenerateHazardExtents_ ();
};