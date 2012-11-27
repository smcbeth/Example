//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\_math.h"
#include "public\nstdlib\vector.h"
#include "public\nstdlib\list.h"

#include "world.h"
#include "terrainhelper.h"
#include "s_sector.h"
#include "iworldgenerator.h"
#include "town.h"
#include "dungeon.h"

#include "astar3d.h"

// STODO: to perfect all aspects of town generation add more failure cases. for example if a building has a stair that intersects with a door, redo the building

// STODO: add rooms that are smaller than the min room size in a post pass (after rooms have been linked)... ex: closets.

// STODO: make defensive buildings: keeps, towers, walls, parapets, etc...
// STODO: make underground buildings. (some may integrate with existing caves)
class TownGenerator
{
public:
	TownGenerator ();
	~TownGenerator ();

	void Generate ();
	
private:

	// STODO: add diagonal walls to rooms, later add thin walls. possibly do thin roofs sooner.
	
	Vector<Town*> towns_;
	Vector<Dungeon*> dungeons_;

	// these data structures are filled in WorldGenerator
	Vector<uint32>* townSupportingAreas_; 
	Vector<bool> isSmoothedTownSupportingAreas_;
	char* townSupportingAreasSparseArray_;
};