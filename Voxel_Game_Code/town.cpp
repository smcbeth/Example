//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "town.h"
#include "zoneroadgenerator.h"
#include "roadnode.h"

Town::Town (uint32 xMin, uint32 yMin, bool doLevelTerrain)
	: BaseZone (xMin, yMin)
{
	// STODO: expand bounding box beyond sector area if neighboring areas can support a town
	// STODO: make town bounding box not conform exactly to sector lines
	population_ = 0;
			
	// shift town terrain toward average height level
	if( doLevelTerrain )
		LevelTerrain_ ();

	// generate slope map
	GenerateSlopeMap_ (slopeMap_, GetLengthX (), GetLengthY ());

	// define areas where buildings_ cannot be placed
	GenerateHazardExtents_ ();
}

Town::~Town ()
{
	buildings_.RemoveAndDeleteAll ();
	farms_.RemoveAndDeleteAll ();
}

bool
Town::PlaceBuildings ()
{
	uint32 numResidents;
	uint32 residentsToPlace = rand () % 7 + 7;
	uint32 idealPopulation = residentsToPlace;
	
	uint32 manorHouseQuota = 1;
	uint32 granaryQuota = 1;
	uint32 shopQuota = 0;
	// STODO: dynamically set these quotas based on num residents placed in homes
	if (residentsToPlace > 5)
		shopQuota = 1 + residentsToPlace / 15;

	// begin modifiy: needed only for roads at the moment
	GetWorld()->BeginModify();

	// create first road. STODO: formulize the creation of multiple roads
	roadGenerator_->AllocZoneRoadGrid( 32, 10 );
	roadGenerator_->AddRoadToZone( ROAD_PRIMARY, true, false );
	roadGenerator_->AddRoadToZone( ROAD_PRIMARY, false, false );
	roadGenerator_->AddRoadToZone( ROAD_PRIMARY, false, false );
	roadGenerator_->AddRoadToZone( ROAD_PRIMARY, false, false );

	uint32 failureCount = 0;

	if( roadNodes_.GetCount() == 0 )
	{
		GetWorld()->EndModify();
		return false;
	}
	
	while (population_ < idealPopulation)
	{
		BaseBuilding* building;
		BuildingSize size;
		uint32 buildingExtentsRadius = 10;
		bool attemptToCenterBuilding = false;

		// determine building type here
		numResidents = 0;
		if (manorHouseQuota >= 1)
		{
			attemptToCenterBuilding = true; // the manor house is the only building that should be centered in a town (for now)
			buildingExtentsRadius = 20;
			if (residentsToPlace < 12)
				size = BUILDING_SMALL;
			else if (residentsToPlace < 25)
				size = BUILDING_MEDIUM;
			else
				size = BUILDING_LARGE;

			building = new ManorHouse (this, size, numResidents);
		}
		else if (granaryQuota >= 1)
		{
			buildingExtentsRadius = 12;
			if (residentsToPlace < 12)
				size = BUILDING_SMALL;
			else if (residentsToPlace < 25)
				size = BUILDING_MEDIUM;
			else
				size = BUILDING_LARGE;

			building = new Granary (this, size, numResidents);

			roadGenerator_->AddRoadToZone( ROAD_PRIMARY, false, false );
		}
		else if (shopQuota >= 1)
		{
			buildingExtentsRadius = 10;
			if (residentsToPlace < 15)
				size = BUILDING_SMALL;
			else if (residentsToPlace < 30)
				size = BUILDING_MEDIUM;
			else
				size = BUILDING_LARGE;

			building = new Shop (this, size, numResidents);

			roadGenerator_->AddRoadToZone( ROAD_PRIMARY, false, false );
		}
		else
		{
			numResidents = 1;
			buildingExtentsRadius = 10;
			switch (rand () % 4)
			{
			case 0:
				size = BUILDING_SMALL;
				break;
			case 1:
				size = BUILDING_MEDIUM;
				numResidents += numResidents / 5;
				break;
			case 2:
				size = BUILDING_MEDIUM; // medium repeated to make it the most likely building size.
				if (numResidents >= 1)
					numResidents += 1 + numResidents / 5;
				break;
			case 3:
				size = BUILDING_LARGE;
				if (numResidents >= 1)
					numResidents += 1 + numResidents / 2;
				break;
			}

			building = new House (this, size, numResidents);
		}

		if (size == BUILDING_MEDIUM)
			buildingExtentsRadius = uint32(float32(buildingExtentsRadius) * 1.25f);
		else if (size == BUILDING_LARGE)
			buildingExtentsRadius =  uint32(float32(buildingExtentsRadius) * 1.5f);

		
		// set number of residents and reduce population_
		if (residentsToPlace == 1 && numResidents > 1)
			numResidents = 1;

		uint32 failedAddCount = 0;

		while ( true )
		{
			// attempt to place building.
			if (/*AttemptToPlaceBuilding_ (building, size, numResidents, buildingExtentsRadius, attemptToCenterBuilding)*/
				AttemptToPlaceBuildingConnectedToRoad_(building, size, numResidents, buildingExtentsRadius, attemptToCenterBuilding, 1, 10))
			{
				residentsToPlace -= numResidents;
				population_ += numResidents;
				
				if (building->GetType () == BUILDING_GRANARY)
					--granaryQuota;
				else if (building->GetType () == BUILDING_MANORHOUSE)
					--manorHouseQuota;
				else if (building->GetType () == BUILDING_SHOP)
					--shopQuota;

				break;
			}
			else
			{
				++failedAddCount; // building cannot be placed

				// STODO: these if checks and constructors are repeated. make a close method for basebuilding and children and have the destructor call close?
				if (building->GetType () == BUILDING_GRANARY)
				{
					delete building;
					building = new Granary (this, size, numResidents);
				}
				else if (building->GetType () == BUILDING_HOUSE)
				{
					delete building;
					building = new House (this, size, numResidents);
				}
				else if (building->GetType () == BUILDING_MANORHOUSE)
				{
					delete building;
					building = new ManorHouse (this, size, numResidents);
				}
				else if (building->GetType () == BUILDING_SHOP)
				{
					delete building;
					building = new Granary (this, size, numResidents);
				}
			}

			if( failedAddCount >= 10 )
			{
				if( population_ == 0 )
				{
					// end modify
					GetWorld()->EndModify();
					return false;
				}
				++failureCount;
				break;
			}
		}

		if( failureCount >= 10 )
			break;
	}

	// end modify
	GetWorld()->EndModify();

	if (population_ == 0)
		return false;
	return true;
}

void
Town::LevelTerrain_ ()
{
	const float32 maxSmoothingFactor = 0.75f;
	const uint32 aboveGroundHeightLimit = 10;
	BaseLevelTerrain_ (maxSmoothingFactor, aboveGroundHeightLimit);
}

void
Town::GenerateHazardExtents_ ()
{
	const float32 slopeThreshold = 0.32f;
	BaseGenerateHazardExtents_ (slopeThreshold);
}

Farm*
Town::GetFarm (uint32 idx)
{
	return farms_[idx];
}

uint32
Town::GetNumFarms ()
{
	return farms_.GetCount ();
}

uint32
Town::GetPopulation ()
{
	return population_;
}