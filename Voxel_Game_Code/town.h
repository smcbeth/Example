//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "basezone.h"
#include "basebuilding.h"
#include "farm.h"
#include "granary.h"
#include "house.h"
#include "shop.h"
#include "manorhouse.h"

class Farm;

enum Direction;

class Town: public BaseZone
{
	// STODO: smooth town terrain toward average height
	// STODO: add roads and place buildings along roads
public:
	Town (uint32 xMin, uint32 yMin, bool doLevelTerrain);
	virtual ~Town ();

	bool PlaceBuildings ();

protected:
	void LevelTerrain_ ();
	void GenerateHazardExtents_ ();
	
public:
	Farm* GetFarm (uint32 idx);
	uint32 GetNumFarms ();
	uint32 GetPopulation ();

private:
	Vector<Farm*> farms_;
	uint32 population_;
};