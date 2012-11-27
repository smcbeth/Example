//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "towngenerator.h"
#include "worldgenerator.h"
#include "roadgenerator.h"
#include "roadnode.h"

TownGenerator::TownGenerator ()
{
	;
}

TownGenerator::~TownGenerator ()
{
	if (towns_.GetCount () > 0)
		towns_.RemoveAndDeleteAll ();

	if (dungeons_.GetCount () > 0)
		dungeons_.RemoveAndDeleteAll ();
}

void
TownGenerator::Generate ()
{
	// STODO: have two options for building symmetry: symmetrical section placement and centered section placement.
	uint32 worldSize = WorldGeneratorAPI ()->GetWorldSize ();	
	uint32 numTownsLeft = 3; //(worldSize / (SECTOR_BLENGTH * 2)) + 1; // STODO: change this in future for larger maps
	uint32 numDungeonsLeft = 1;
	townSupportingAreas_ = WorldGeneratorAPI ()->GetTownSupportingAreas ();
	for( uint32 i = 0; i < townSupportingAreas_->GetCount(); ++i )
	{
		isSmoothedTownSupportingAreas_.InsertBack(false); // this ensures terrain is smoothed in a zone only once
	}

	// STODO: use this to shift the bounds of a town/dungeon away from sector borders
	townSupportingAreasSparseArray_ = WorldGeneratorAPI ()->GetTownSupportingAreasSparseArray (); 

	uint32 unsuccessfulAddCount = 0;
	while (numTownsLeft > 0 || numDungeonsLeft > 0)
	{
		if (townSupportingAreas_->GetCount () == 0)
			break;

		uint32 randIndex = rand () % townSupportingAreas_->GetCount ();

		// check if town is to be placed
		uint32 xMin = (townSupportingAreas_->GetElement (randIndex) % (worldSize / SECTOR_BLENGTH)) * SECTOR_BLENGTH;
		uint32 yMin = (townSupportingAreas_->GetElement (randIndex) / (worldSize / SECTOR_BLENGTH)) * SECTOR_BLENGTH;
		bool doLevelTerrain = !isSmoothedTownSupportingAreas_[randIndex];

		if (rand () % 2 == 0 && numDungeonsLeft > 0)
		{
			Dungeon* dungeon = new Dungeon (xMin, yMin, doLevelTerrain);
			isSmoothedTownSupportingAreas_[randIndex] = true;
			if (dungeon->PlaceBuildings ())
			{
				dungeon->Build ();
				dungeons_.InsertBack (dungeon);
				townSupportingAreas_->Remove (randIndex);
				isSmoothedTownSupportingAreas_.Remove( randIndex );
				--numDungeonsLeft;
			}
			else
			{
				delete dungeon;
				++unsuccessfulAddCount;
			}
		}
		else if (numTownsLeft > 0)
		{
			Town* town = new Town (xMin, yMin, doLevelTerrain);
			isSmoothedTownSupportingAreas_[randIndex] = true;
			if (town->PlaceBuildings ())
			{
				town->Build ();
				towns_.InsertBack (town);
				townSupportingAreas_->Remove (randIndex);
				isSmoothedTownSupportingAreas_.Remove( randIndex );
				--numTownsLeft;
			}
			else
			{
				delete town;
				++unsuccessfulAddCount;
			}
		}

		if (unsuccessfulAddCount > 25)
			break;
	}

	// STODO: this is preliminary code:
	// connect roads between town
	GetWorld()->BeginModify();
	RoadGenerator rGen;
	Vector<BaseArea*> areas;
	for( uint32 i = 0; i < towns_.GetCount(); ++i )
	{
		areas.InsertBack( towns_[i] );
	}
	for( uint32 i = 0; i < dungeons_.GetCount(); ++i )
	{
		areas.InsertBack( dungeons_[i] );
	}
	if( towns_.GetCount() >= 2 )
	{
		rGen.ConnectZones(ROAD_PRIMARY, towns_[0], towns_[1], 1.0f, &areas); // STODO: add avoidance points for all zones as well
		if( towns_.GetCount() >= 3 )
		{
			rGen.ConnectZones(ROAD_PRIMARY, towns_[0], towns_[2], 1.0f, &areas);
			rGen.ConnectZones(ROAD_PRIMARY, towns_[1], towns_[2], 1.0f, &areas);
		}
	}
	GetWorld()->EndModify();

	// DEBUG_TOWNGENERATOR:
	/*
	GetWorld()->BeginModify();
	AStar3D search;
	search.AllocMem( 150, 150, 120 );
	
	// note: make goal.z unreachable and time it
	for (uint32 i = 0; i < 1; ++i)
	{
		UPoint3D start = dungeons_.GetBack()->GetBuilding(0)->GetRoom(dungeons_.GetBack()->GetBuilding(0)->GetNumRooms() - 1)->GetCenterPoint();
		start.z = dungeons_.GetBack()->GetBuilding(0)->GetRoom(dungeons_.GetBack()->GetBuilding(0)->GetNumRooms() - 1)->GetMinZ() + 1;
		UPoint3D goal = towns_.GetFront()->GetBuilding(0)->GetRoom(towns_.GetFront()->GetBuilding(0)->GetNumRooms() - 1)->GetCenterPoint();
		goal.z = towns_.GetFront()->GetBuilding(0)->GetRoom(towns_.GetFront()->GetBuilding(0)->GetNumRooms() - 1)->GetMinZ() + 1;

		if( search.Search( start, goal, 2, 1.0f ) )
		{
			Vector<UPoint3D>* lastPath = search.GetLastPath();
			
			// DEBUG_ASTAR3D: show path
			for (uint32 i = 0; i < lastPath->GetCount(); ++i)
			{
				UPoint3D& point = lastPath->GetElement( i );
				uint32 sCubeBuf = 0;
				SetSCubeTerrainType (&sCubeBuf, TERRA_LAVA);
				GetWorld()->SetSCube  (point.x, point.y, point.z, sCubeBuf);
			}
		}
		search.ClearMem();
	}
	
	search.FreeMem();
	GetWorld()->EndModify();
	*/
	// DEBUG_ASTAR3D: end test search

}

