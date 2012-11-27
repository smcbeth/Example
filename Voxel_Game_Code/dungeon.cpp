//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "dungeon.h"
#include "worldgenerator.h"

Dungeon::Dungeon (uint32 xMin, uint32 yMin, bool doLevelTerrain)
	: BaseZone (xMin, yMin)
{
	// STODO: to make this work: move some functionality out of the town constructor and move it to town method calls in towngenerator
	// shift town terrain toward average height level
	if( doLevelTerrain )
		LevelTerrain_ ();

	// generate slope map
	GenerateSlopeMap_ (slopeMap_, GetLengthX (), GetLengthY ());

	// define areas where buildings_ cannot be placed
	GenerateHazardExtents_ ();
}

Dungeon::~Dungeon ()
{
	buildings_.RemoveAndDeleteAll ();
}

bool
Dungeon::PlaceBuildings ()
{
	bool firstPass = true;
	uint32 addCount = 0;
	uint32 failedAddCount = 0;
	
	BaseBuilding* building;
	BuildingSize size;

	while (addCount < 1)
	{
		Direction doorSide = Direction(rand () % 4);
		uint32 extentsRadius;

		// determine building type and size
		if (rand () % 2 == 0)
		{
			size = BUILDING_SMALL;
			extentsRadius = 20;
		}
		else if (rand () % 4 <= 2 || WorldGeneratorAPI ()->GetWorldSize () <= 2048)
		{
			size = BUILDING_MEDIUM;
			extentsRadius = 25;
		}
		else
		{
			size = BUILDING_LARGE;
			extentsRadius = 30;
		}

		building = new DungeonTower (this, size, 0);
		
		// attempt to place building.
		if (AttemptToPlaceBuilding_ (building, size, 0, extentsRadius, true))
			++addCount;
		else
			++failedAddCount; // building cannot be placed

		if (failedAddCount > 10)
			break; // no more buildings can be placed (most likely)
	}

	if (addCount == 0)
		return false;
	return true;
}

void
Dungeon::LevelTerrain_ ()
{
	const float32 maxSmoothingFactor = 0.60f;
	const uint32 aboveGroundHeightLimit = 10;
	BaseLevelTerrain_ (maxSmoothingFactor, aboveGroundHeightLimit);
}

void
Dungeon::GenerateHazardExtents_ ()
{
	const float32 slopeThreshold = 0.40f;
	BaseGenerateHazardExtents_ (slopeThreshold);
}