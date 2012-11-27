//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "basebuilding.h"
#include "worldgenerator.h"

BaseBuilding::BaseBuilding (BaseZone* zone, BuildingSize size, uint32 numResidents)
	: parentZone_ (zone), size_ (size), numResidents_ (numResidents)
{
	numFloors_ = 1;

	numBasements_ = 0; // STODO: add basements to some buildings.
	ceilingHeight_ = 4;
	stairWidth_ = 1;
	minHallwayWidth_ = 4;
	maxHallwayWidth_ = 4;
	minRoomAndBuildingBoundSeparation_ = 5;
	maxRoomAndBuildingBoundSeparation_ = 5;

	innerWallMaterial_ = TERRA_BOARDS;
	outerWallMaterial_ = TERRA_BOARDS;
	floorMaterial_ = TERRA_BOARDS;
	roofMaterial_ = TERRA_THATCH;
	stairMaterial_ = TERRA_BOARDS;
	roofType_ = ROOF_NORMAL;
	roofFillType_ = ROOFFILL_SOLID;
	maxRoofHeight_ = 5;
}

BaseBuilding::~BaseBuilding ()
{
	rooms_.RemoveAndDeleteAll ();
	openWalls_.RemoveAndDeleteAll ();
	openAreas_.RemoveAndDeleteAll ();
}

void
BaseBuilding::SetBounds (const UPoint3D& entrance, const UPoint3D& buildingMin, const UPoint3D& buildingMax, BuildingSize size, Direction doorSide)
{
	min_ = buildingMin;
	max_ = buildingMax;

	groundFloorLevel_ = parentZone_->GetAverageHeightWithinBounds (min_, max_);

	min_.z = groundFloorLevel_ - 10;
	max_.z = min_.z + (ceilingHeight_ * numFloors_) + 10;

	Assert (max_.z < WorldGeneratorAPI ()->GetWorldDepth ());
}

bool
BaseBuilding::PlaceRooms (Direction doorSide)
{
	Assert (0);
	return false;
}

void
BaseBuilding::Build ()
{
	const uint32 maxLevelingDist = 6;
	const uint32 levelingStartDist = 1;
	const uint32 foundationThickness = 1;
	BaseBuild_ (maxLevelingDist, levelingStartDist, foundationThickness);
}

uint32 
BaseBuilding::GetNumRoomsContainingPoint (uint32 x, uint32 y, uint32 z, uint32 roomMinZ) const
{
	// Note- this method will only compare against rooms that match the roomMinZ parameter

	Assert (x > 0 && y > 0 && z > 0);
	Assert (x < 1000000 && y < 1000000 && z < 1000000);

	uint32 numRoomsContainingPoint = 0;

	for (uint32 i = 0; i < rooms_.GetCount (); ++i)
	{
		BaseRoom* room = rooms_[i];

		if (room->GetMin ().z != roomMinZ)
			continue; // TODO refine this check when multiple floor spanning rooms are added

		if (IntersectBoxWithPoint (UPoint3D (x, y, z), room->GetMin (), room->GetMax ()))
			++numRoomsContainingPoint;
	}

	return numRoomsContainingPoint;
}

bool
BaseBuilding::IntersectPointWithRoomsFast (uint32 x, uint32 y, uint32 z, uint32 roomMinZ) const
{
	Assert (x > 0 && y > 0 && z > 0);
	Assert (x < 1000000 && y < 1000000 && z < 1000000);

	for (uint32 i = 0; i < rooms_.GetCount (); ++i)
	{
		BaseRoom* room = rooms_[i];

		if (room->GetMin ().z != roomMinZ)
			continue; // TODO refine this check when multiple floor spanning rooms are added

		if (IntersectBoxWithPoint (UPoint3D (x, y, z), room->GetMin (), room->GetMax ()))
			return true;
	}

	return false;
}

bool
BaseBuilding::IntersectPointWithRooms (uint32 x, uint32 y, uint32 z, bool doExcludeRoomBorders, BaseRoom** intersectedRoomPtr) const
{
	Assert (x > 0 && y > 0 && z > 0);
	Assert (x < 1000000 && y < 1000000 && z < 1000000);

	bool retVal = false;
	int32 maxFloorLevel = int32((z - groundFloorLevel_) / ceilingHeight_ + 1);
	int32 minFloorLevel = maxFloorLevel - 1;

	for (uint32 i = 0; i < rooms_.GetCount (); ++i)
	{
		BaseRoom* room = rooms_[i];

		if (rooms_[i]->GetFloorLevel () > maxFloorLevel || rooms_[i]->GetFloorLevel () < minFloorLevel)
			continue;

		if (doExcludeRoomBorders)
		{
			UPoint3D tmpRoomMin (room->GetMin ());
			UPoint3D tmpRoomMax (room->GetMax ());
			tmpRoomMin += 1;
			tmpRoomMax -= 1;
			retVal = IntersectBoxWithPoint (UPoint3D (x, y, z), tmpRoomMin, tmpRoomMax);

			Assert (tmpRoomMin.x <= tmpRoomMax.x && tmpRoomMin.y <= tmpRoomMax.y && tmpRoomMin.z <= tmpRoomMax.z);
		}
		else
		{
			retVal = IntersectBoxWithPoint (UPoint3D (x, y, z), room->GetMin (), room->GetMax ());
		}

		if (retVal)
		{
			if (intersectedRoomPtr != NULL)
				*intersectedRoomPtr = room;
			break;
		}
	}

	return retVal;
}

void
BaseBuilding::RemoveWindowsThatIntersectWithPoint (uint32 x, uint32 y, uint32 z, uint32 roomMinZ) const
{
	Assert (x > 0 && y > 0 && z > 0);
	Assert (x < 1000000 && y < 1000000 && z < 1000000);

	for (uint32 i = 0; i < rooms_.GetCount (); ++i)
	{
		BaseRoom* room = rooms_[i];

		if (room->GetMin ().z != roomMinZ)
			continue; // TODO refine this check when multiple floor spanning rooms are added

		for (uint32 j = 0; j < room->GetNumWindows (); ++j)
		{
			if (IntersectBoxWithPoint (UPoint3D (x, y, z), room->GetWindow (j)->GetMin (), room->GetWindow (j)->GetMax ()))
				room->RemoveWindow (j);
		}
	}
}

IntersectType 
BaseBuilding::IntersectAreaWithRooms (const UPoint3D& areaMin, const UPoint3D& areaMax, bool doExcludeRoomBorders, BaseRoom** intersectedRoomPtr) const
{
	// this method will test if the the space of an area intersects with rooms and what form of intersection it takes
	// when doExcludeRoomBorders is enabled, each room min and max will be reduced in size along the x and y bounds by 1. 
		// This is useful because rooms share borders in the current implementation.

	// intersectedRoomPtr is set to the BaseRoom* of the room the area is intersected with if there is an intersection and if the double pointer is not null

	// Note- when an inside or encapsulate intersection is found, the loop will break
	// Note- STODO: multiple intersections are not noted and the intersectedRoomPtr is set to the most recently intersected room
	if (rooms_.GetCount () == 0)
		return INTERSECTTYPE_OUTSIDE;

	int32 maxFloorLevel = int32((areaMin.z - groundFloorLevel_) / ceilingHeight_ + 1);
	int32 minFloorLevel = maxFloorLevel - 1;

	IntersectType type = INTERSECTTYPE_OUTSIDE;

	for (uint32 i = 0; i < rooms_.GetCount (); ++i)
	{
		BaseRoom* room = rooms_[i];
		
		IntersectType tmpType;

		if (rooms_[i]->GetFloorLevel () > maxFloorLevel || rooms_[i]->GetFloorLevel () < minFloorLevel)
			continue;

		if (doExcludeRoomBorders)
		{
			tmpType = IntersectAABoxWithAABox (areaMin, areaMax, room->GetMin (), room->GetMax ());

			if (tmpType == INTERSECTTYPE_INTERSECT)
			{
				if (areaMax.x - areaMin.x == 0 || areaMax.y - areaMin.y == 0)
				{
					// wall case: exclude wall ends
					if (DoesWallIntersectWithAreaOnlyOnEndOfWall_ (areaMin, areaMax, room->GetMin (), room->GetMax ()))
						tmpType = INTERSECTTYPE_OUTSIDE;
				}
				else
				{
					// area case:
					if (DoesBoundsIntersectOnlyOnBorderOfArea_ (areaMin, areaMax, room->GetMin (), room->GetMax ()))
						tmpType = INTERSECTTYPE_OUTSIDE;
				}
			}

			//Assert (tmpRoomMin.x <= tmpRoomMax.x && tmpRoomMin.y <= tmpRoomMax.y && tmpRoomMin.z <= tmpRoomMax.z);
		}
		else
		{
			tmpType = IntersectAABoxWithAABox (areaMin, areaMax, room->GetMin (), room->GetMax ());
		}

		if (tmpType != INTERSECTTYPE_OUTSIDE)
		{
			if (intersectedRoomPtr != NULL)
				*intersectedRoomPtr = room;

			if (type != tmpType)
				type = tmpType;

			if (type == INTERSECTTYPE_INSIDE || type == INTERSECTTYPE_ENCAPSULATE)
				break;
		}
	}

	return type;
}

bool 
BaseBuilding::IntersectAreaWithRoomsFast (const UPoint3D& areaMin, const UPoint3D& areaMax, bool doExcludeRoomBorders, BaseRoom** intersectedRoomPtr) const
{
	// this method will test if the the space of an area intersects with rooms and what form of intersection it takes
	// when doExcludeRoomBorders is enabled, each room min and max will be reduced in size along the x and y bounds by 1. 
		// This is useful because rooms share borders in the current implementation.

	// intersectedRoomPtr is set to the BaseRoom* of the room the area is intersected with if there is an intersection and if the double pointer is not null

	if (rooms_.GetCount () == 0)
		return false;

	bool retVal = false;

	int32 maxFloorLevel = int32((areaMin.z - groundFloorLevel_) / ceilingHeight_ + 1);
	int32 minFloorLevel = maxFloorLevel - 1;

	for (uint32 i = 0; i < rooms_.GetCount (); ++i)
	{
		BaseRoom* room = rooms_[i];

		if (rooms_[i]->GetFloorLevel () > maxFloorLevel || rooms_[i]->GetFloorLevel () < minFloorLevel)
			continue;

		if (doExcludeRoomBorders)
		{
			UPoint3D tmpRoomMin (room->GetMin ());
			UPoint3D tmpRoomMax (room->GetMax ());

			// selectively narrow room borders based on dimensions of area
			if (areaMax.x - areaMin.x + 1 >= 3)
			{
				tmpRoomMin.x += 1;
				tmpRoomMax.x -= 1;
			}
			if (areaMax.y - areaMin.y + 1 >= 3)
			{
				tmpRoomMin.y += 1;
				tmpRoomMax.y -= 1;
			}
			tmpRoomMin.z += 1;
			tmpRoomMax.z -= 1;

			retVal = IntersectAABoxWithAABoxFast (areaMin, areaMax, tmpRoomMin, tmpRoomMax);

			Assert (tmpRoomMin.x <= tmpRoomMax.x && tmpRoomMin.y <= tmpRoomMax.y && tmpRoomMin.z <= tmpRoomMax.z);
		}
		else
		{
			retVal = IntersectAABoxWithAABoxFast (areaMin, areaMax, room->GetMin (), room->GetMax ());
		}

		if (retVal)
		{
			if (intersectedRoomPtr != NULL)
				*intersectedRoomPtr = room;
			break;
		}
	}

	return retVal;
}

BaseRoom*
BaseBuilding::GetRoom (uint32 idx) const
{
	return rooms_[idx];
}

const UPoint3D&
BaseBuilding::GetEntrance () const
{
	return entrance_;
}

const UPoint3D&
BaseBuilding::GetFrontDoorPos () const
{
	return frontDoorPos_;
}

uint32
BaseBuilding::GetNumRooms () const
{
	return rooms_.GetCount ();
}

uint32
BaseBuilding::GetNumFloors () const
{
	return numFloors_;
}

uint32 
BaseBuilding::GetGroundFloorLevel () const
{
	return groundFloorLevel_;
}

uint32
BaseBuilding::GetCeilingHeight () const
{
	return ceilingHeight_;
}

uint32
BaseBuilding::GetMaxRoofHeight () const
{
	return maxRoofHeight_;
}

uint32
BaseBuilding::GetStairWidth () const
{
	return stairWidth_;
}

uint32
BaseBuilding::GetMinHallWayWidth () const
{
	return minHallwayWidth_;
}

uint32
BaseBuilding::GetMaxHallwayWidth () const
{
	return maxHallwayWidth_;
}

uint32
BaseBuilding::GetRandHallwayWidth () const
{
	Assert (minHallwayWidth_ <= maxHallwayWidth_);
	return minHallwayWidth_ + rand () % (maxHallwayWidth_ - minHallwayWidth_ + 1);
}

BuildingSize
BaseBuilding::GetSize () const
{
	return size_;
}

TERRATYPE
BaseBuilding::GetFloorMaterial () const
{
	return floorMaterial_;
}

TERRATYPE
BaseBuilding::GetInnerWallMaterial () const
{
	return innerWallMaterial_;
}

TERRATYPE
BaseBuilding::GetOuterWallMaterial () const
{
	return outerWallMaterial_;
}

TERRATYPE
BaseBuilding::GetRoofMaterial () const
{
	return roofMaterial_;
}

TERRATYPE
BaseBuilding::GetStairMaterial () const
{
	return stairMaterial_;
}

RoofType
BaseBuilding::GetRoofType () const
{
	return roofType_;
}

RoofFillType
BaseBuilding::GetRoofFillType () const
{
	return roofFillType_;
}

Direction
BaseBuilding::GetDoorSide () const
{
	return doorSide_;
}

BuildingType
BaseBuilding::GetType () const
{
	return BUILDING_UNKNOWN;
}

void
BaseBuilding::SetFDoorPos (UPoint3D& point)
{
	frontDoorPos_ = point;
	frontDoorPos_.z = groundFloorLevel_;
}

bool
BaseBuilding::LinkRooms_ ()
{
	RoomLinkerInitData initData;
	initData.building = this;
	initData.maxStairsPerFloor = 1;
	initData.maxStairsPerFloor = 1;
	initData.maxParentRooms = 2;
	initData.maxChildRooms = 3;
	initData.doorPlaceChance = 0.4f;
	initData.stairPlaceChance = 1.0f;
	initData.spiralStairTypeChance = 0.33f;
	return BaseLinkRooms_ (initData);
}

void
BaseBuilding::PlaceWindows_ ()
{
	const uint32 heightAboveFloor = 2;
	const uint32 height = 1;
	const uint32 width = 1;
	const uint32 windowSpacing = 2;
	const float32 placementChance = 0.35f;
	BasePlaceWindows_ (heightAboveFloor, height, width, windowSpacing, placementChance);
}

void 
BaseBuilding::BaseBuild_ (uint32 maxLevelingDist, uint32 levelingStartDist, uint32 foundationThickness)
{
	Assert (rooms_.GetCount () > 0);

	// STODO: place items in building space

	GetWorld()->BeginModify (); // Note- not thread safe unless BeginModify has a lock

	// level terrain
	LevelTerrain_ (maxLevelingDist, levelingStartDist);

	// construct every room in the building
	for (uint32 i = 0; i < rooms_.GetCount (); ++i)
	{
		rooms_[i]->BuildFoundation (foundationThickness);
		rooms_[i]->BuildFloor ();
		rooms_[i]->BuildWalls ();
	}

	// construct doors, stairs and windows.
	for (uint32 i = 0; i < rooms_.GetCount (); ++i)
		rooms_[i]->PlaceObjects ();

	BuildRoofs_ ();

	GetWorld()->EndModify ();
}

void BaseBuilding::BuildRoofs_ ()
{
	// STODO: use rooms instead of areas to build roofs for error free roof building. be mindful of rooftop type. 
		// use current floor rooms for inside roof area and rooms above that floor for inside higher roof area. 
		// areaZLevel would be replaced by the ceiling level for the rooms

	// STODO: generate roof data after room placement and before the build phase. in order to do this, a vector of sparse arrays would be needed, one for each z level
	// STODO: clean up this method
	const int32 startingMinLevel = 100000;
	const int32 higherRoofLevel = 99999;

	Assert (higherRoofLevel < startingMinLevel);
	Assert (higherRoofLevel > maxRoofHeight_ + 1);

	if (roofType_ == ROOF_NONE)
		return;

	Assert (maxRoofHeight_ > 0);
	// STODO: allow for different roof types by modifing how rooflevels are calculated
	// create 2D sparse array the size of the building bounds and set each value in it to the roof height (construct at appropriate z level - since this can vary)
	// Note- sparse array scheme: 
	//	99999 for inside higher roof area. (skipped)
	//	-1 for outside roof area. 
	//	0 for inside roof area but not yet set. 
	//	1+ for each roof height level starting at 1 for the edges.
	Vector<int32> roofSparseArray;
	roofSparseArray.SetCapacity (GetLengthX () * GetLengthY ());
	roofSparseArray.OverrideCount (GetLengthX () * GetLengthY ());
	Vector<int32> roofSparseArrayTmp;
	roofSparseArrayTmp.SetCapacity (GetLengthX () * GetLengthY ());
	roofSparseArrayTmp.OverrideCount (GetLengthX () * GetLengthY ());

	Assert (roofSparseArray.GetCount () > 0);
	Assert (roofSparseArrayTmp.GetCount () > 0);

	while (openAreas_.GetCount () > 0)
	{
		// find highest and lowest z level out of all areas. this is currently used for placing attic roof types on all but the highest areas
		uint32 maxAreaZLevel = 0;
		uint32 minAreaZLevel = startingMinLevel;
		for (int32 i = 0; i < (int32)openAreas_.GetCount (); ++i)
		{
			// skip and remove first floor areas
			if (openAreas_[i]->GetMin ().z == groundFloorLevel_)
			{
				openAreas_.RemoveAndDelete (i);
				--i;
				continue;
			}

			if (openAreas_[i]->GetMin ().z > maxAreaZLevel)
				maxAreaZLevel = openAreas_[i]->GetMin ().z;
			if (openAreas_[i]->GetMin ().z < minAreaZLevel)
				minAreaZLevel = openAreas_[i]->GetMin ().z;
		}

		if (openAreas_.GetCount () == 0)
			break;

		Assert (minAreaZLevel < startingMinLevel);

		// initialize sparse array with -1s to represent exterior bounds
		for (uint32 y = 0; y < GetLengthY (); ++y)
		{
			for (uint32 x = 0; x < GetLengthX (); ++x)
				roofSparseArray[x + y * GetLengthX ()] = -1;
		}

		// initialize roofSparseArray
		// only operate on one z level at a time (minimum z level) and ignore all other areas at a different z level
		for (int32 i = 0; i < (int32)openAreas_.GetCount (); ++i)
		{
			uint32 minX;
			uint32 minY;
			uint32 maxX;
			uint32 maxY;

			// keep areas at least one distant from building bounds to simplify supersampling checks below
			Assert (openAreas_[i]->GetMin ().x > min_.x);
			Assert (openAreas_[i]->GetMin ().y > min_.y);
			Assert (openAreas_[i]->GetMax ().x < max_.x);
			Assert (openAreas_[i]->GetMax ().y < max_.y);

			if (openAreas_[i]->GetMin ().z > minAreaZLevel)
			{
				// initialize sparse array with 99999's for higher roof areas. these values will be ignored allowing for a better roof look
				minX = openAreas_[i]->GetMin ().x - min_.x;
				minY = openAreas_[i]->GetMin ().y - min_.y;
				maxX = openAreas_[i]->GetMax ().x - min_.x;
				maxY = openAreas_[i]->GetMax ().y - min_.y;
				for (uint32 y = minY; y <= maxY; ++y)
				{
					for (uint32 x = minX; x <= maxX; ++x)
						roofSparseArray[x + y * GetLengthX ()] = higherRoofLevel;
				}
			}
			else
			{
				// initialize sparse array with 0's for internal roof areas
				minX = openAreas_[i]->GetMin ().x - min_.x;
				minY = openAreas_[i]->GetMin ().y - min_.y;
				maxX = openAreas_[i]->GetMax ().x - min_.x;
				maxY = openAreas_[i]->GetMax ().y - min_.y;
				for (uint32 y = minY; y <= maxY; ++y)
				{
					for (uint32 x = minX; x <= maxX; ++x)
						roofSparseArray[x + y * GetLengthX ()] = 0;
				}

				openAreas_.RemoveAndDelete (i);
				--i;
			}	
		}

		bool isSparseArrayChanged = true;
		bool isSparseArrayNotFullySet = true;
		
		while (isSparseArrayNotFullySet)
		{
			bool doPerformFailurePass;
			if (!isSparseArrayChanged)
				doPerformFailurePass = true; // treats maxRoofHeight as 1 and treats -2 values as -1 values
			else
				doPerformFailurePass = false;

			isSparseArrayChanged = false;
			isSparseArrayNotFullySet = false;

			// initialize sparse array with interior roof levels
			// edges of the sparse array are avoided to simplify calculations
			roofSparseArrayTmp = roofSparseArray;
			for (uint32 y = 1; y < GetLengthY () - 1; ++y)
			{
				for (uint32 x = 1; x < GetLengthX () - 1; ++x)
				{
					if (roofSparseArray[x + y * GetLengthX ()] == 0)
					{
						isSparseArrayNotFullySet = true;
						int32 minAdjacentRoofLevel = startingMinLevel;

						// supersample nearby locations to get minAdjacentRoofLevel, ignore zero values
						if (roofSparseArray[(x - 1) + (y - 1) * GetLengthX ()] < minAdjacentRoofLevel && roofSparseArray[(x - 1) + (y - 1) * GetLengthX ()] != 0)
							minAdjacentRoofLevel = roofSparseArray[(x - 1) + (y - 1) * GetLengthX ()];
						if (roofSparseArray[x + (y - 1) * GetLengthX ()] < minAdjacentRoofLevel && roofSparseArray[x + (y - 1) * GetLengthX ()] != 0)
							minAdjacentRoofLevel = roofSparseArray[x + (y - 1) * GetLengthX ()];
						if (roofSparseArray[(x + 1) + (y - 1) * GetLengthX ()] < minAdjacentRoofLevel && roofSparseArray[(x + 1) + (y - 1) * GetLengthX ()] != 0)
							minAdjacentRoofLevel = roofSparseArray[(x + 1) + (y - 1) * GetLengthX ()];
						if (roofSparseArray[(x - 1) + y * GetLengthX ()] < minAdjacentRoofLevel && roofSparseArray[(x - 1) + y * GetLengthX ()] != 0)
							minAdjacentRoofLevel = roofSparseArray[(x - 1) + y * GetLengthX ()];
						if (roofSparseArray[(x + 1) + y * GetLengthX ()] < minAdjacentRoofLevel && roofSparseArray[(x + 1) + y * GetLengthX ()] != 0)
							minAdjacentRoofLevel = roofSparseArray[(x + 1) + y * GetLengthX ()];
						if (roofSparseArray[(x - 1) + (y + 1) * GetLengthX ()] < minAdjacentRoofLevel && roofSparseArray[(x - 1) + (y + 1) * GetLengthX ()] != 0)
							minAdjacentRoofLevel = roofSparseArray[(x - 1) + (y + 1) * GetLengthX ()];
						if (roofSparseArray[x + (y + 1) * GetLengthX ()] < minAdjacentRoofLevel && roofSparseArray[x + (y + 1) * GetLengthX ()] != 0)
							minAdjacentRoofLevel = roofSparseArray[x + (y + 1) * GetLengthX ()];
						if (roofSparseArray[(x + 1) + (y + 1) * GetLengthX ()] < minAdjacentRoofLevel && roofSparseArray[(x + 1) + (y + 1) * GetLengthX ()] != 0)
							minAdjacentRoofLevel = roofSparseArray[(x + 1) + (y + 1) * GetLengthX ()];

						if (doPerformFailurePass)
						{
							if (minAdjacentRoofLevel == -1 || minAdjacentRoofLevel == higherRoofLevel)
								roofSparseArrayTmp[x + y * GetLengthX ()] = 1;
							else
								roofSparseArrayTmp[x + y * GetLengthX ()] = 2;
							isSparseArrayChanged = true;
						}
						else if (minAdjacentRoofLevel == -1)
						{
							roofSparseArrayTmp[x + y * GetLengthX ()] = 1;
							isSparseArrayChanged = true;
						}
						else if (minAdjacentRoofLevel > 0)
						{
							if (minAdjacentRoofLevel == startingMinLevel || minAdjacentRoofLevel == higherRoofLevel)
							{
								continue;
							}
							else if (minAdjacentRoofLevel <= (int32)maxRoofHeight_)
							{
								roofSparseArrayTmp[x + y * GetLengthX ()] = minAdjacentRoofLevel + 1;
								isSparseArrayChanged = true;
							}
							else
							{
								roofSparseArrayTmp[x + y * GetLengthX ()] = maxRoofHeight_ + 1;
								isSparseArrayChanged = true;
							}
						}
					}
				}
			}
			roofSparseArray = roofSparseArrayTmp;
		}

		// build roof levels
		for (uint32 y = 1; y < GetLengthY () - 1; ++y)
		{
			for (uint32 x = 1; x < GetLengthX () - 1; ++x)
			{
				if (roofSparseArray[x + y * GetLengthX ()] <= 0 || roofSparseArray[x + y * GetLengthX ()] == higherRoofLevel)
					continue;

				// set to attic type for roof levels lower than the max roof level, but not for roof types equal to the outer wall material
				bool useAtticType = minAreaZLevel < maxAreaZLevel && roofMaterial_ != outerWallMaterial_; // STODO: dont rely on this

				if (roofSparseArray[x + y * GetLengthX ()] == 1 && roofFillType_ != ROOFFILL_ATTIC && !useAtticType)
					continue; // allows for wall extension for attic roof type

				if (IntersectPointWithRoomsFast (min_.x + x, min_.y + y, minAreaZLevel + 1, minAreaZLevel))
					continue; // prevent roof geometry from being placed over existing rooms

				for (uint32 z = 1; z <= (uint32)roofSparseArray[x + y * GetLengthX ()]; ++z)
				{
					uint32 sCubeBuf = 0;
					int32 zOffset = 0;
							
					if (!useAtticType && roofFillType_ == ROOFFILL_VAULTED)
					{
						if (z == roofSparseArray[x + y * GetLengthX ()])
						{
							SetSCubeTerrainType (&sCubeBuf, roofMaterial_);
						}
						else
						{
							if (GetSCubeTerrainType (GetWorld()->GetSCube (min_.x + x, min_.y + y, minAreaZLevel + z - 3)) == innerWallMaterial_)
								SetSCubeTerrainType (&sCubeBuf, innerWallMaterial_); // elevate walls to avoid gaps between walls and vaulted ceiling
							else
								SetSCubeTerrainType (&sCubeBuf, TERRA_AIR);
						}

					}
					else if (!useAtticType && roofFillType_ == ROOFFILL_SOLID)
					{
						if (z != 1)
							SetSCubeTerrainType (&sCubeBuf, roofMaterial_);
						else
							continue; // this prevents low hanging ceilings. STODO: this should not be necessary
					}
					else if (useAtticType || roofFillType_ == ROOFFILL_ATTIC)
					{
						zOffset = 1;
						// Note- must raise wall height by one to support attic floor
						if (z == 1)
						{
							if (GetSCubeTerrainType (GetWorld()->GetSCube (min_.x + x, min_.y + y, minAreaZLevel + z - 3 + zOffset)) == outerWallMaterial_)
							{
								SetSCubeTerrainType (&sCubeBuf, outerWallMaterial_);
							}
							else
							{
								SetSCubeTerrainType (&sCubeBuf, floorMaterial_);
							}
						}
						else if (z == roofSparseArray[x + y * GetLengthX ()])
						{
							SetSCubeTerrainType (&sCubeBuf, roofMaterial_);
						}
						else
						{
							SetSCubeTerrainType (&sCubeBuf, TERRA_AIR);
						}
					}
					else
					{
						Assert (0);
					}

					// HACK: windows shouldnt be removed like this during the build phase. in addition this is a roundabout method since sampling is done in this fashion
					RemoveWindowsThatIntersectWithPoint (min_.x + x - 1, min_.y + y - 1, minAreaZLevel + z - 2 + zOffset, minAreaZLevel);
					RemoveWindowsThatIntersectWithPoint (min_.x + x + 1, min_.y + y - 1, minAreaZLevel + z - 2 + zOffset, minAreaZLevel);
					RemoveWindowsThatIntersectWithPoint (min_.x + x - 1, min_.y + y + 1, minAreaZLevel + z - 2 + zOffset, minAreaZLevel);
					RemoveWindowsThatIntersectWithPoint (min_.x + x + 1, min_.y + y + 1, minAreaZLevel + z - 2 + zOffset, minAreaZLevel);

					GetWorld()->SetSCube (min_.x + x, min_.y + y, minAreaZLevel + z - 2 + zOffset, sCubeBuf);
				}
			}
		}
	}

	// OLD:
	/*
	for (uint32 i = 0; i < openAreas_.GetCount (); ++i)
	{
		RoomPlacementSpace* openArea = openAreas_[i];

		// dont add roofs over ground floor areas
		if (openAreas_[i]->GetMin ().z == groundFloorLevel_)
			continue;

		if (roofType_ == ROOF_NORMAL || roofType_ == ROOF_OPEN || roofType_ == ROOF_OPENPARAPETS)
		{
			for (uint32 y = openArea->GetMin ().y; y <= openArea->GetMax ().y; ++y)
			{
				for (uint32 x = openArea->GetMin ().x; x <= openArea->GetMax ().x; ++x)
				{
					uint32 intOff = 0;
					for (uint32 z = openArea->GetMin ().z; z <= openArea->GetMin ().z + maxRoofHeight_; ++z)
					{
						if (openArea->GetMin ().x + intOff >= openArea->GetMax ().x - intOff || openArea->GetMin ().y + intOff >= openArea->GetMax ().y - intOff)
							break;

						// HACK: the following if checks are work arounds to deal with the intersection of multiple roof areas
						// STODO: break on these continues to gather why these blocks are being placed or just redesign the roof placing algorithm
						if (GetSCubeTerrainType (GetWorld()->GetSCube (x, y, z)) != TERRA_AIR)
							break; // dont place roof blocks in existing blocks
						if (GetSCubeType (GetWorld()->GetSCube (x, y, z - 1)) != SCUBE_CUBE)
							break; // dont place roof blocks over existing roof tiles

						uint32 sCubeBuf = 0;
						SetSCubeTerrainType (&sCubeBuf, roofMaterial_);

						// roof corners
						if (x == openArea->GetMin ().x + intOff && y == openArea->GetMin ().y + intOff)
						{
							SetSCubeType (&sCubeBuf, SCUBE_QUARTERCUBECORNERPYRAMID);
							SetSCubeSpin (&sCubeBuf, 2);
							GetWorld()->SetSCube(x, y, z, sCubeBuf);
						}
						else if (x == openArea->GetMax ().x - intOff && y == openArea->GetMin ().y + intOff)
						{
							SetSCubeType (&sCubeBuf, SCUBE_QUARTERCUBECORNERPYRAMID);
							SetSCubeSpin (&sCubeBuf, 1);
							GetWorld()->SetSCube(x, y, z, sCubeBuf);
						}
						else if (x == openArea->GetMin ().x + intOff && y == openArea->GetMax ().y - intOff)
						{
							SetSCubeType (&sCubeBuf, SCUBE_QUARTERCUBECORNERPYRAMID);
							SetSCubeSpin (&sCubeBuf, 3);
							GetWorld()->SetSCube(x, y, z, sCubeBuf);
						}
						else if (x == openArea->GetMax ().x - intOff && y == openArea->GetMax ().y - intOff)
						{
							SetSCubeType (&sCubeBuf, SCUBE_QUARTERCUBECORNERPYRAMID);
							SetSCubeSpin (&sCubeBuf, 0);
							GetWorld()->SetSCube(x, y, z, sCubeBuf);
						}

						// roof sides
						else if (x == openArea->GetMin ().x + intOff && y >= (openArea->GetMin ().y + intOff) && y <= (openArea->GetMax ().y - intOff))
						{
							SetSCubeType (&sCubeBuf, SCUBE_HALFCUBETRIANGLE);
							SetSCubeSpin (&sCubeBuf, 1);
							GetWorld()->SetSCube(x, y, z, sCubeBuf);
						}
						else if (x == openArea->GetMax ().x - intOff && y >= (openArea->GetMin ().y + intOff) && y <= (openArea->GetMax ().y - intOff))
						{
							SetSCubeType (&sCubeBuf, SCUBE_HALFCUBETRIANGLE);
							SetSCubeSpin (&sCubeBuf, 3);
							GetWorld()->SetSCube(x, y, z, sCubeBuf);
						}
						else if (y == openArea->GetMin ().y + intOff && x >= (openArea->GetMin ().x + intOff) && x <= (openArea->GetMax ().x - intOff))
						{
							SetSCubeType (&sCubeBuf, SCUBE_HALFCUBETRIANGLE);
							SetSCubeSpin (&sCubeBuf, 0);
							GetWorld()->SetSCube(x, y, z, sCubeBuf);
						}
						else if (y == openArea->GetMax ().y - intOff && x >= (openArea->GetMin ().x + intOff) && x <= (openArea->GetMax ().x - intOff))
						{
							SetSCubeType (&sCubeBuf, SCUBE_HALFCUBETRIANGLE);
							SetSCubeSpin (&sCubeBuf, 2);
							GetWorld()->SetSCube(x, y, z, sCubeBuf);
						}

						// interior
						else if (x >= (openArea->GetMin ().x + intOff) && x <= (openArea->GetMax ().x - intOff) && y >= (openArea->GetMin ().y + intOff) && y <= (openArea->GetMax ().y - intOff))
						{
							sCubeBuf = 0;
							switch (roofFillType_)
							{
							case ROOFFILL_VAULTED:
								if (x == (openArea->GetMin ().x + intOff) + 1 || x == (openArea->GetMax ().x - intOff) - 1 || y == (openArea->GetMin ().y + intOff) + 1 || y == (openArea->GetMax ().y - intOff) - 1)
									SetSCubeTerrainType (&sCubeBuf, roofMaterial_);
								else if (z < openArea->GetMin ().z + maxRoofHeight_)
									SetSCubeTerrainType (&sCubeBuf, TERRA_AIR);
								else
									SetSCubeTerrainType (&sCubeBuf, roofMaterial_);
								break;
							case ROOFFILL_SOLID:
								SetSCubeTerrainType (&sCubeBuf, roofMaterial_);
								break;
							case ROOFFILL_ATTIC: // STODO: make the attic a room if it is big enough
								if (z == openArea->GetMin ().z)
									SetSCubeTerrainType (&sCubeBuf, floorMaterial_);
								else if (x == (openArea->GetMin ().x + intOff) + 1 || x == (openArea->GetMax ().x - intOff) - 1 || y == (openArea->GetMin ().y + intOff) + 1 || y == (openArea->GetMax ().y - intOff) - 1)
									SetSCubeTerrainType (&sCubeBuf, roofMaterial_);
								else if (z == openArea->GetMin ().z + maxRoofHeight_)
									SetSCubeTerrainType (&sCubeBuf, roofMaterial_);
								else
									SetSCubeTerrainType (&sCubeBuf, TERRA_AIR);
								break;
							}
							GetWorld()->SetSCube(x, y, z, sCubeBuf);
						}
							
						++intOff;
					}
				}
			}
		}
	}
	*/
}

void
BaseBuilding::FinalizeBuildingBounds_ (bool doReduceBoundsSizeOnly)
{
	// this method will reduce extraneous building bound space and ensure that BaseBuilding::LevelTerrain_ will have proper smoothing
	UPoint3D minRoomsBounds (rooms_[0]->GetMin ());
	UPoint3D maxRoomsBounds (rooms_[0]->GetMax ());

	for (uint32 i = 1; i < rooms_.GetCount (); ++i)
	{
		BaseRoom* room = rooms_[i];
		minRoomsBounds.x = Min (minRoomsBounds.x, room->GetMin ().x);
		minRoomsBounds.y = Min (minRoomsBounds.y, room->GetMin ().y);

		maxRoomsBounds.x = Max (maxRoomsBounds.x, room->GetMax ().x);
		maxRoomsBounds.y = Max (maxRoomsBounds.y, room->GetMax ().y);
	}

	// modify building bounds if it violates the min or max room and building bound separation values
	if (!doReduceBoundsSizeOnly)
	{
		if (min_.x > minRoomsBounds.x - minRoomAndBuildingBoundSeparation_)
			min_.x = minRoomsBounds.x - minRoomAndBuildingBoundSeparation_;
		if (min_.y > minRoomsBounds.y - minRoomAndBuildingBoundSeparation_)
			min_.y = minRoomsBounds.y - minRoomAndBuildingBoundSeparation_;
	}

	if (min_.x < minRoomsBounds.x - maxRoomAndBuildingBoundSeparation_)
		min_.x = minRoomsBounds.x - maxRoomAndBuildingBoundSeparation_;
	if (min_.y < minRoomsBounds.y - maxRoomAndBuildingBoundSeparation_)
		min_.y = minRoomsBounds.y - maxRoomAndBuildingBoundSeparation_;

	if (!doReduceBoundsSizeOnly)
	{
		if (max_.x < maxRoomsBounds.x + minRoomAndBuildingBoundSeparation_)
			max_.x = maxRoomsBounds.x + minRoomAndBuildingBoundSeparation_;
		if (max_.y < maxRoomsBounds.y + minRoomAndBuildingBoundSeparation_)
			max_.y = maxRoomsBounds.y + minRoomAndBuildingBoundSeparation_;
	}

	if (max_.x > maxRoomsBounds.x + maxRoomAndBuildingBoundSeparation_)
		max_.x = maxRoomsBounds.x + maxRoomAndBuildingBoundSeparation_;
	if (max_.y > maxRoomsBounds.y + maxRoomAndBuildingBoundSeparation_)
		max_.y = maxRoomsBounds.y + maxRoomAndBuildingBoundSeparation_;

	// shift entrance position if it doesnt match up with the building borders
	switch (doorSide_)
	{
	case DIRECTION_SOUTH:
		if (entrance_.y != min_.y)
			entrance_.y = min_.y;
		break;
	case DIRECTION_NORTH:
		if (entrance_.y != max_.y)
			entrance_.y = max_.y;
		break;
	case DIRECTION_WEST:
		if (entrance_.x != min_.x)
			entrance_.x = min_.x;
		break;
	case DIRECTION_EAST:
		if (entrance_.x != max_.x)
			entrance_.x = max_.x;
		break;
	}

	// if the new building bounds violate the zone bounds, modify the building bounds
	// STODO: prevent this code from ever being used
	if (parentZone_->GetMin ().x > min_.x)
		min_.x = parentZone_->GetMin ().x;
	if (parentZone_->GetMin ().y > min_.y)
		min_.y = parentZone_->GetMin ().y;

	if (parentZone_->GetMax ().x < max_.x)
		max_.x = parentZone_->GetMax ().x;
	if (parentZone_->GetMax ().y < max_.y)
		max_.y = parentZone_->GetMax ().y;
}

bool
BaseBuilding::PlaceRoom_ (BaseRoom* room, bool doAllowImaginaryAreasToBeUsed, bool doAllowImaginaryTypeCombination, bool doPlaceFirstRoomOnNewFloor, 
							float32 doAttemptToPreserveBoxShapeOnFloorOneChance, float32 chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne, 
							float32 chanceOfSkipPlacementSpaceOnFloorOne, float32 chanceOfSkipPlacementSpaceOnOtherFloors)
{
	// STODO: pass these in as parameters
	const uint32 reattemptsAfterFailure = 15; // Note- this will greatly affect performance when failures occur	
	uint32 failureCount = 0;
	uint32 floorLevel = room->GetFloorLevel ();
	bool addMainRoom = rooms_.GetCount () == 0;

	Assert (reattemptsAfterFailure > 1);
	Assert (floorLevel < 200);

	if (rooms_.GetCount () > 0)
		UpdateRoomPlacementSpaces_ (floorLevel - 1, floorLevel, true, doPlaceFirstRoomOnNewFloor, doAllowImaginaryTypeCombination, false, room);

	for (uint32 failureCount = 0; failureCount < reattemptsAfterFailure; ++failureCount)
	{
		if (rooms_.GetCount () > 0 && openAreas_.GetCount () == 0 && floorLevel > 1)
			return false;

		bool doAttemptToPreserveBoxShapeOnFloorOne;
		float32 chanceOfSkippingSpace;
		if (floorLevel == 1)
		{
			doAttemptToPreserveBoxShapeOnFloorOne = FRandom () <= doAttemptToPreserveBoxShapeOnFloorOneChance;
			chanceOfSkippingSpace = chanceOfSkipPlacementSpaceOnFloorOne;
		}
		else
		{
			doAttemptToPreserveBoxShapeOnFloorOne = true;
			chanceOfSkippingSpace = chanceOfSkipPlacementSpaceOnOtherFloors;
		}

		if (SetRoomBounds_ (room, chanceOfSkippingSpace, chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne, doAttemptToPreserveBoxShapeOnFloorOne, doAllowImaginaryAreasToBeUsed))
		{
			AddRoomToBuilding_ (room);

			return true;
		}

		if (rooms_.GetCount () > 0 && (failureCount + 1) % 3 == 0)
			UpdateRoomPlacementSpaces_ (floorLevel - 1, floorLevel, true, doPlaceFirstRoomOnNewFloor, doAllowImaginaryTypeCombination, false, room); // change room placement spaces since the current ones have failed
	}

	return false;
}

uint32
BaseBuilding::GetMaxNumRoomsOnFloor_ (uint32 floorLevel, uint32 maxNumRoomsOnFloor, uint32 maxNumFloorTwoRooms, uint32 reduceAmount, float32 reduceNumRoomsPerFloorChance)
{
	if (floorLevel == 2)
	{
		maxNumRoomsOnFloor = maxNumFloorTwoRooms;
	}
	else if (floorLevel > 2 && FRandom () < reduceNumRoomsPerFloorChance)
	{
		if (maxNumRoomsOnFloor > reduceAmount)
			maxNumRoomsOnFloor -= reduceAmount; // Note- if extra rooms are present they will be added to
	}
	return maxNumRoomsOnFloor;
}

void
BaseBuilding::FinalizeRoomPlacement_ ()
{
	// add rooftop rooms if roofType_ allows it using openArea_ bounds,
	// add areas that can later be used for building roofs

	// generate open areas for all floors above floor 1. PERFORMANCE: this may be slow for large buildings, since the entire building is scanned
	UpdateRoomPlacementSpaces_ (0, numFloors_, false, false, true, true);

	FinalizeBuildingBounds_ (false);
}

bool
BaseBuilding::SetRoomBounds_ (BaseRoom* room, float32 chanceOfSkippingSpace, float32 chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne, bool doAttemptToPreserveBoxShapeOnFloorOne, bool doAllowImaginaryAreasToBeUsed)
{
	// chanceOfSkippingSpace is used as the chance to not skip over a RoomPlacementSpace provided that there is at least one additional
	//		RoomPlacementSpace to place a room in. 
	//			if chanceOfSkippingSpace is 0.0, the first (closest to main room) space will always be chosen.
	//			if chanceOfSkippingSpace is 1.0, the last space will always be chosen. 
	//			it is not advisable to use values close to 1 since failure chance increases and building shape becomes erratic

	// doPreserveBoxShapeOnFloorOne: when true, will avoid using openWalls to place a room and will do so only if openAreas are unplaceable.
	//		when false or if openAreas are not usable, will combine openWalls and openAreas into a single vector and sort it by distance from main room.

	BaseArea roomBounds;
	uint32 floorLevel = room->GetFloorLevel ();

	Assert (floorLevel > 0);
	Assert (chanceOfSkippingSpace >= 0.0f && chanceOfSkippingSpace <= 1.0f);

	const uint32 failureTolerance = 15;
	const uint32 failureThresholdForIncreasedPlacementChance = 10;

	if (floorLevel > 1 && openAreas_.GetCount () == 0)
		return false;
	
	for (uint32 failureCount = 0; failureCount <= failureTolerance; ++failureCount)
	{
		// set z level based on floor level
		roomBounds.SetMin (0, 0, groundFloorLevel_ + ceilingHeight_ * (floorLevel - 1));
		roomBounds.SetMax (0, 0, roomBounds.GetMin ().z + ceilingHeight_);

		if (rooms_.GetCount () == 0) // case for first room placed
		{
			uint32 lengthX = room->GetRandLength ();
			uint32 lengthY = room->GetRandLength ();

			// place bounds differently for each doorSide_ case
			// first axis
			if (doorSide_ == DIRECTION_SOUTH || doorSide_ == DIRECTION_NORTH)
			{
				roomBounds.SetMinX (GetCenterX () - lengthX / 2);
				roomBounds.SetMaxX (roomBounds.GetMin ().x + lengthX);
			}
			else
			{
				roomBounds.SetMinY (GetCenterY () - lengthY / 2);
				roomBounds.SetMaxY (roomBounds.GetMin ().y + lengthY);
			}

			// other axis
			switch (doorSide_)
			{
			case DIRECTION_SOUTH:
				roomBounds.SetMinY (min_.y + minRoomAndBuildingBoundSeparation_);
				roomBounds.SetMaxY (roomBounds.GetMin ().y + lengthY);
				break;
			case DIRECTION_NORTH:
				roomBounds.SetMaxY (max_.y - minRoomAndBuildingBoundSeparation_);
				roomBounds.SetMinY (roomBounds.GetMax ().y - lengthY);
				break;
			case DIRECTION_WEST:
				roomBounds.SetMinX (min_.x + minRoomAndBuildingBoundSeparation_);
				roomBounds.SetMaxX (roomBounds.GetMin ().x + lengthX);
				break;
			case DIRECTION_EAST:
				roomBounds.SetMaxX (max_.x - minRoomAndBuildingBoundSeparation_);
				roomBounds.SetMinX (roomBounds.GetMax ().x - lengthX);
				break;
			}

			if (IsRoomPlaceable_ (roomBounds.GetMin (), roomBounds.GetMax (), true))
			{
				room->SetMin (roomBounds.GetMin ());
				room->SetMax (roomBounds.GetMax ());
				return true;
			}
			return false;
		}
		else
		{
			Assert (rooms_.GetCount () > 0);

			bool doLoop = true;
			bool isOpenAreaOnlyPassPerformed = false;
			
			while (doLoop)
			{	
				doLoop = false;

				Vector<RoomPlacementSpace*> openSpaces;
				Vector<RoomPlacementSpace*>* openSpacesPtr;

				uint32 openAreasOnFloor = 0;
				for (uint32 i = 0; i < openAreas_.GetCount (); ++i)
				{
					if (openAreas_[i]->GetMin ().z == roomBounds.GetMin ().z)
						++openAreasOnFloor;
				}

				if (floorLevel == 1 && openWalls_.GetCount () > 0 && 
					(!doAttemptToPreserveBoxShapeOnFloorOne || openAreasOnFloor == 0 || failureCount >= failureThresholdForIncreasedPlacementChance))
				{
					// use both open walls and open areas to place rooms in with the only preference being distance from center point to center
					openSpaces.InsertBack (openWalls_);
					openSpaces.InsertBack (openAreas_);
					Sort (openSpaces.GetMemory (), openSpaces.GetCount (), true);
					openSpacesPtr = &openSpaces;
				}
				else if (!isOpenAreaOnlyPassPerformed)
				{
					// only use open areas to place rooms in
					isOpenAreaOnlyPassPerformed = true;
					doLoop = true;
					openSpacesPtr = &openAreas_;
				}
				else
				{
					break;
				}

				// iterate through open areas to find room locations
				// STODO: allow configuration and test the different possible uses of mostTrueImaginaryTypePermitted and doForceSetRoomBoundsInArea
					// the use of these two variables allow for significant changes to the look and structure of buildings
				ImaginaryType mostTrueImaginaryTypePermitted = IMAGINARY_FALSE; // set to PARTIAL after failure, set to TRUE after failure of PARTIAL and doForceSetRoomBoundsInArea == true
				bool doForceSetRoomBoundsInArea = false; // will be set to false after an unsuccessful true pass. will be set to true after an unsuccessful false pass after either IMAGINARY_PARTIAL and IMAGINARY_TRUE have been tried
				while (true)
				{
					for (uint32 i = 0; i < openSpacesPtr->GetCount (); ++i)
					{
						RoomPlacementSpace* openSpace = openSpacesPtr->GetElement (i);
						if (roomBounds.GetMin ().z != openSpace->GetMin ().z)
							continue;
						if (mostTrueImaginaryTypePermitted == IMAGINARY_FALSE && 
							(openSpace->imaginaryType == IMAGINARY_PARTIAL || openSpace->imaginaryType == IMAGINARY_TRUE))
							continue;
						if (mostTrueImaginaryTypePermitted == IMAGINARY_PARTIAL && openSpace->imaginaryType == IMAGINARY_TRUE)
							continue;
						if (FRandom () < chanceOfSkippingSpace && (doAttemptToPreserveBoxShapeOnFloorOne || i < openSpacesPtr->GetCount () - 1))
							continue; // STODO: reconsider the logic of this check

						uint32 minAreaWallLength = Min (openSpace->GetLengthX (), openSpace->GetLengthY ());
						bool doAllowPerpendicularExpansion = floorLevel == 1 && FRandom () < chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne;
						bool doAllowParallelExpansion = floorLevel == 1 && FRandom () < chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne;

						if (openSpace->GetLengthX () > 1 && openSpace->GetLengthY () > 1) // area case
						{
							if (SetRoomBoundsInArea_ (room, roomBounds, openSpace, doForceSetRoomBoundsInArea, doAllowPerpendicularExpansion, doAllowParallelExpansion))
								return true;
						}
						else // wall case
						{
							RoomPlacementSpace* tmpArea = new RoomPlacementSpace (*openSpace);

							if (openSpace->GetLengthX () == 1)
							{
								Assert (openSpace->attachmentSide == DIRECTION_WEST || openSpace->attachmentSide == DIRECTION_EAST);
								if (openSpace->attachmentSide == DIRECTION_WEST)
									tmpArea->SetMaxX (openSpace->GetMin ().x + room->GetMaxLength () * 2 - 1); 
								else if (openSpace->attachmentSide == DIRECTION_EAST)
									tmpArea->SetMinX (openSpace->GetMin ().x - room->GetMaxLength () * 2 + 1);
							}
							else if (openSpace->GetLengthY () == 1)
							{
								Assert (openSpace->attachmentSide == DIRECTION_SOUTH || openSpace->attachmentSide == DIRECTION_NORTH);
								if (openSpace->attachmentSide == DIRECTION_SOUTH)
									tmpArea->SetMaxY (openSpace->GetMin ().y + room->GetMaxLength () * 2 - 1);
								else if (openSpace->attachmentSide == DIRECTION_NORTH)
									tmpArea->SetMinY (openSpace->GetMin ().y - room->GetMaxLength () * 2 + 1);
							}

							if (SetRoomBoundsInArea_ (room, roomBounds, tmpArea, doForceSetRoomBoundsInArea, doAllowPerpendicularExpansion, doAllowParallelExpansion))
							{
								delete tmpArea;
								return true;
							}
							delete tmpArea;
						}
					}

					// Note- this code block strongly affects the structure of buildings, see definition of mostTrueImaginaryTypePermitted and doForceSetRoomBoundsInArea
					if (mostTrueImaginaryTypePermitted == IMAGINARY_FALSE)
					{
						mostTrueImaginaryTypePermitted = IMAGINARY_PARTIAL; // avoid setting doForceSetRoomBoundsInArea to true here for a more connected building structure
					}
					else if (mostTrueImaginaryTypePermitted == IMAGINARY_PARTIAL)
					{
						if (!doForceSetRoomBoundsInArea)
						{
							doForceSetRoomBoundsInArea = true;
						}
						else
						{
							// fully imaginary spaces are not used on the first floor to prevent rooms from being placed that are not connected rooms that they should be connected to
							if (floorLevel == 1 || !doAllowImaginaryAreasToBeUsed)
								break;
							mostTrueImaginaryTypePermitted = IMAGINARY_TRUE;
						}
					}
					else
					{
						if (!doForceSetRoomBoundsInArea)
							doForceSetRoomBoundsInArea = true;
						else
							break;
					}
				}
				doAttemptToPreserveBoxShapeOnFloorOne = false;
			}
		}
	}

	return false;
}

bool
BaseBuilding::SetRoomBoundsInArea_ (BaseRoom* room, BaseArea& roomBounds, RoomPlacementSpace* area, bool doForceSetting, bool doAllowPerpendicularExpansion, bool doAllowParallelExpansion)
{
	// doAllowPerpendicularExpansion enables room bounds to ignore the length of the area along the perpendicular to connecting wall axis
	// doAllowParallelExpansion enables room bounds to ignore the length of the area along the parallel to connecting wall axis

	// STODO: export some methods here
	Assert (rooms_.GetCount () > 0);
	Assert (area->attachmentSide <= DIRECTION_EAST);

	roomBounds.SetMinZ (area->GetMin ().z);
	roomBounds.SetMaxZ (area->GetMax ().z);

	// determine first axis (perpendicular to connecting wall)
	{
		uint32 firstAxisLength = firstAxisLength = room->GetRandLength ();

		if (area->attachmentSide == DIRECTION_SOUTH || area->attachmentSide == DIRECTION_NORTH)
		{
			if (room->GetMinLength () > area->GetLengthY () && !doAllowPerpendicularExpansion)
				return false;

			if (firstAxisLength + room->GetMinLength () > area->GetLengthY () && area->GetLengthY () > room->GetMinLength () * 2)
				firstAxisLength -= (firstAxisLength + room->GetMinLength ()) - area->GetLengthY (); // adjust axis length to allow another room to fit in the space alloted

			if (area->attachmentSide == DIRECTION_SOUTH)
			{
				roomBounds.SetMinY (area->GetMin ().y);
				if (room->GetMaxLength () >= area->GetLengthY () && !doAllowPerpendicularExpansion)
					roomBounds.SetMaxY (area->GetMax ().y); // old: still useful for different building types: roomBounds.SetMaxY (roomBounds.GetMin ().y + Min (firstAxisLength, area->GetLengthY ()) - 1);
				else
					roomBounds.SetMaxY (roomBounds.GetMin ().y + firstAxisLength - 1);
			}
			else
			{
				roomBounds.SetMaxY (area->GetMax ().y);
				if (room->GetMaxLength () >= area->GetLengthY () && !doAllowPerpendicularExpansion)
					roomBounds.SetMinY (area->GetMin ().y); // old: roomBounds.SetMinY (roomBounds.GetMax ().y - Min (firstAxisLength, area->GetLengthY ()) + 1);
				else
					roomBounds.SetMinY (roomBounds.GetMax ().y - firstAxisLength + 1);
			}

			if (roomBounds.GetLengthY () > area->GetLengthY () && !doAllowPerpendicularExpansion)
				return false;
		}
		else
		{
			if (room->GetMinLength () > area->GetLengthX () && !doAllowPerpendicularExpansion)
				return false;

			if (firstAxisLength + room->GetMinLength () > area->GetLengthX () && area->GetLengthX () > room->GetMinLength () * 2)
				firstAxisLength -= (firstAxisLength + room->GetMinLength ()) - area->GetLengthX (); // adjust axis length to allow another room to fit in the space alloted 

			if (area->attachmentSide == DIRECTION_WEST)
			{
				roomBounds.SetMinX (area->GetMin ().x);
				if (room->GetMaxLength () >= area->GetLengthX () && !doAllowPerpendicularExpansion)
					roomBounds.SetMaxX (area->GetMax ().x); // old: still useful for different building types: roomBounds.SetMaxX (roomBounds.GetMin ().x + Min (firstAxisLength, area->GetLengthX ()) - 1);
				else
					roomBounds.SetMaxX (roomBounds.GetMin ().x + firstAxisLength - 1);
			}
			else
			{
				roomBounds.SetMaxX (area->GetMax ().x);
				if (room->GetMaxLength () >= area->GetLengthX () && !doAllowPerpendicularExpansion)
					roomBounds.SetMinX (area->GetMin ().x); // old: roomBounds.SetMinX (roomBounds.GetMax ().x - Min (firstAxisLength, area->GetLengthX ()) + 1);
				else
					roomBounds.SetMinX (roomBounds.GetMax ().x - firstAxisLength + 1);
			}

			if (roomBounds.GetLengthX () > area->GetLengthX () && !doAllowPerpendicularExpansion)
				return false;
		}
	}

	// determine second axis (parallel to connecting wall)
	{
		bool doUseHallwayWidth;
		uint32 secondAxisLength;
		if (room->GetType () == ROOM_HALLWAY)
		{
			secondAxisLength = GetRandHallwayWidth ();
			doUseHallwayWidth = true;
		}
		else
		{
			secondAxisLength = room->GetRandLength ();
			doUseHallwayWidth = false;
		}

		if (area->attachmentSide == DIRECTION_SOUTH || area->attachmentSide == DIRECTION_NORTH)
		{
			if (!doUseHallwayWidth && room->GetMinLength () > area->GetLengthX () && !doAllowParallelExpansion)
				return false;

			if (!doUseHallwayWidth && room->GetMaxLength () >= area->GetLengthX () && !doAllowParallelExpansion)
			{
				// STODO: consider having less rectangular buildings use this kind of call: roomBounds.SetMinX (roomBounds.GetMax ().x - Min (firstAxisLength, area->GetLengthX ()) + 1);
				roomBounds.SetMinX (area->GetMin ().x);
				roomBounds.SetMaxX (area->GetMax ().x);
			}
			else
			{
				// STODO: (maybe) allow rooms to be centered along a wall in an area

				// STODO: this code will always result in having a room take the minimum possible bounds. when this isnt necessary, allow the bounds to be larger
				if (!doUseHallwayWidth && secondAxisLength + room->GetMinLength () > area->GetLengthX () && area->GetLengthX () > room->GetMinLength () * 2 && secondAxisLength < area->GetLengthX ())
					secondAxisLength -= (secondAxisLength + room->GetMinLength ()) - area->GetLengthX (); // adjust axis length to allow another room to fit in the space alloted 

				// sample 3 points along the perpendicular edge to the connecting wall on both the min and max side of the connecting wall. choose the one with the most supporting points
					// or, if the number of supporting points are equal, randomly pick a side
				// STODO: move this code to a method to avoid repeated code structure. this structure is repeated below as well
				uint32 numMinSidePointsSupportedByPerpendicularWall = 0;
				uint32 numMaxSidePointsSupportedByPerpendicularWall = 0;
				if (area->attachmentSide == DIRECTION_SOUTH)
				{
					if (IntersectPointWithRoomsFast (area->GetMin ().x, roomBounds.GetMin ().y + 1, area->GetMin ().z + 1, area->GetMin ().z))
					{
						++numMinSidePointsSupportedByPerpendicularWall;

						if (IntersectPointWithRoomsFast (area->GetMin ().x, roomBounds.GetCenterPoint ().y, area->GetMin ().z + 1, area->GetMin ().z))
						{
							++numMinSidePointsSupportedByPerpendicularWall;

							if (IntersectPointWithRoomsFast (area->GetMin ().x, roomBounds.GetMax ().y - 1, area->GetMin ().z + 1, area->GetMin ().z))
								++numMinSidePointsSupportedByPerpendicularWall;
						}
					}

					if (IntersectPointWithRoomsFast (area->GetMax ().x, roomBounds.GetMin ().y + 1, area->GetMin ().z + 1, area->GetMin ().z))
					{
						++numMaxSidePointsSupportedByPerpendicularWall;

						if (IntersectPointWithRoomsFast (area->GetMax ().x, roomBounds.GetCenterPoint ().y, area->GetMin ().z + 1, area->GetMin ().z))
						{
							++numMaxSidePointsSupportedByPerpendicularWall;

							if (IntersectPointWithRoomsFast (area->GetMax ().x, roomBounds.GetMax ().y - 1, area->GetMin ().z + 1, area->GetMin ().z))
								++numMaxSidePointsSupportedByPerpendicularWall;
						}
					}
				}
				else
				{
					if (IntersectPointWithRoomsFast (area->GetMin ().x, roomBounds.GetMax ().y - 1, area->GetMin ().z + 1, area->GetMin ().z))
					{
						++numMinSidePointsSupportedByPerpendicularWall;

						if (IntersectPointWithRoomsFast (area->GetMin ().x, roomBounds.GetCenterPoint ().y, area->GetMin ().z + 1, area->GetMin ().z))
						{
							++numMinSidePointsSupportedByPerpendicularWall;

							if (IntersectPointWithRoomsFast (area->GetMin ().x, roomBounds.GetMin ().y + 1, area->GetMin ().z + 1, area->GetMin ().z))
								++numMinSidePointsSupportedByPerpendicularWall;
						}
					}

					if (IntersectPointWithRoomsFast (area->GetMax ().x, roomBounds.GetMax ().y - 1, area->GetMin ().z + 1, area->GetMin ().z))
					{
						++numMaxSidePointsSupportedByPerpendicularWall;

						if (IntersectPointWithRoomsFast (area->GetMax ().x, roomBounds.GetCenterPoint ().y, area->GetMin ().z + 1, area->GetMin ().z))
						{
							++numMaxSidePointsSupportedByPerpendicularWall;

							if (IntersectPointWithRoomsFast (area->GetMax ().x, roomBounds.GetMin ().y + 1, area->GetMin ().z + 1, area->GetMin ().z))
								++numMaxSidePointsSupportedByPerpendicularWall;
						}
					}
				}


				if (numMinSidePointsSupportedByPerpendicularWall > numMaxSidePointsSupportedByPerpendicularWall ||
					(numMinSidePointsSupportedByPerpendicularWall == numMaxSidePointsSupportedByPerpendicularWall && rand () % 2 == 0))
				{
					roomBounds.SetMinX (area->GetMin ().x);
					roomBounds.SetMaxX (area->GetMin ().x + secondAxisLength - 1);
				}
				else
				{
					roomBounds.SetMaxX (area->GetMax ().x);
					roomBounds.SetMinX (area->GetMax ().x - secondAxisLength + 1);
				}
			}

			if (roomBounds.GetLengthX () > area->GetLengthX () && !doAllowParallelExpansion)
				return false;
		}
		else if (area->attachmentSide == DIRECTION_WEST || area->attachmentSide == DIRECTION_EAST)
		{
			if (!doUseHallwayWidth && room->GetMinLength () > area->GetLengthY () && !doAllowParallelExpansion)
				return false;

			if (!doUseHallwayWidth && room->GetMaxLength () >= area->GetLengthY () && !doAllowParallelExpansion)
			{
				roomBounds.SetMinY (area->GetMin ().y);
				roomBounds.SetMaxY (area->GetMax ().y);
			}
			else
			{
				if (!doUseHallwayWidth && secondAxisLength + room->GetMinLength () > area->GetLengthY () && area->GetLengthY () > room->GetMinLength () * 2 && secondAxisLength < area->GetLengthY ())
					secondAxisLength -= (secondAxisLength + room->GetMinLength ()) - area->GetLengthY (); // adjust axis length to allow another room to fit in the space alloted

				// sample 3 points along the perpendicular edge to the connecting wall on both the min and max side of the connecting wall. choose the one with the most supporting points
					// or, if the number of supporting points are equal, randomly pick a side
				uint32 numMinSidePointsSupportedByPerpendicularWall = 0;
				uint32 numMaxSidePointsSupportedByPerpendicularWall = 0;
				if (area->attachmentSide == DIRECTION_SOUTH)
				{
					if (IntersectPointWithRoomsFast (roomBounds.GetMin ().x + 1, area->GetMin ().y, area->GetMin ().z + 1, area->GetMin ().z))
					{
						++numMinSidePointsSupportedByPerpendicularWall;

						if (IntersectPointWithRoomsFast (roomBounds.GetCenterPoint ().x, area->GetMin ().y, area->GetMin ().z + 1, area->GetMin ().z))
						{
							++numMinSidePointsSupportedByPerpendicularWall;

							if (IntersectPointWithRoomsFast (roomBounds.GetMax ().x - 1, area->GetMin ().y, area->GetMin ().z + 1, area->GetMin ().z))
								++numMinSidePointsSupportedByPerpendicularWall;
						}
					}

					if (IntersectPointWithRoomsFast (roomBounds.GetMin ().x + 1, area->GetMax ().y, area->GetMin ().z + 1, area->GetMin ().z))
					{
						++numMaxSidePointsSupportedByPerpendicularWall;

						if (IntersectPointWithRoomsFast (roomBounds.GetCenterPoint ().x, area->GetMax ().y, area->GetMin ().z + 1, area->GetMin ().z))
						{
							++numMaxSidePointsSupportedByPerpendicularWall;

							if (IntersectPointWithRoomsFast (roomBounds.GetMax ().x - 1, area->GetMax ().y, area->GetMin ().z + 1, area->GetMin ().z))
								++numMaxSidePointsSupportedByPerpendicularWall;
						}
					}
				}
				else
				{
					if (IntersectPointWithRoomsFast (roomBounds.GetMax ().x - 1, area->GetMin ().y, area->GetMin ().z + 1, area->GetMin ().z))
					{
						++numMinSidePointsSupportedByPerpendicularWall;

						if (IntersectPointWithRoomsFast (roomBounds.GetCenterPoint ().x, area->GetMin ().y, area->GetMin ().z + 1, area->GetMin ().z))
						{
							++numMinSidePointsSupportedByPerpendicularWall;

							if (IntersectPointWithRoomsFast (roomBounds.GetMin ().x + 1, area->GetMin ().y, area->GetMin ().z + 1, area->GetMin ().z))
								++numMinSidePointsSupportedByPerpendicularWall;
						}
					}

					if (IntersectPointWithRoomsFast (roomBounds.GetMax ().x - 1, area->GetMax ().y, area->GetMin ().z + 1, area->GetMin ().z))
					{
						++numMaxSidePointsSupportedByPerpendicularWall;

						if (IntersectPointWithRoomsFast (roomBounds.GetCenterPoint ().x, area->GetMax ().y, area->GetMin ().z + 1, area->GetMin ().z))
						{
							++numMaxSidePointsSupportedByPerpendicularWall;

							if (IntersectPointWithRoomsFast (roomBounds.GetMin ().x + 1, area->GetMax ().y, area->GetMin ().z + 1, area->GetMin ().z))
								++numMaxSidePointsSupportedByPerpendicularWall;
						}
					}
				}

				if (numMinSidePointsSupportedByPerpendicularWall > numMaxSidePointsSupportedByPerpendicularWall ||
					(numMinSidePointsSupportedByPerpendicularWall == numMaxSidePointsSupportedByPerpendicularWall && rand () % 2 == 0))
				{
					roomBounds.SetMinY (area->GetMin ().y);
					roomBounds.SetMaxY (area->GetMin ().y + secondAxisLength - 1);
				}
				else
				{
					roomBounds.SetMaxY (area->GetMax ().y);
					roomBounds.SetMinY (area->GetMax ().y - secondAxisLength + 1);
				}
			}

			if (roomBounds.GetLengthY () > area->GetLengthY () && !doAllowParallelExpansion)
				return false;
		}
	}

	// make sure gaps aren't forming where rooms can't be placed
	if (!doForceSetting)
	{
		// return false if gaps are forming where additional rooms (of the same size cannot be placed) 
		// STODO: improve this. possibly take minimum possible room size of the building into account
		// STODO: easy improvement for some cases: dont perform this check when this room is the last one placed in the building
		if ((area->GetLengthX () != roomBounds.GetLengthX () && area->GetLengthX () - roomBounds.GetLengthX () < minHallwayWidth_) ||
			(area->GetLengthY () != roomBounds.GetLengthY () && area->GetLengthY () - roomBounds.GetLengthY () < minHallwayWidth_))
		{
			return false;
		}
	}

	if ((area->GetLengthX () != roomBounds.GetLengthX () && area->GetLengthX () - roomBounds.GetLengthX () == 1) ||
		(area->GetLengthY () != roomBounds.GetLengthY () && area->GetLengthY () - roomBounds.GetLengthY () == 1))
	{
		return false; // avoid leaving gaps of one block since this will sometimes give bad results
	}

	if (IsRoomPlaceable_ (roomBounds.GetMin (), roomBounds.GetMax (), true))
	{
		room->SetMin (roomBounds.GetMin ());
		room->SetMax (roomBounds.GetMax ());

		// this code will cause a failure if the new room will not connect to any other rooms on the floor
		uint32 numRoomsOnFloor = 0;
		bool isBoundsConnectedToOtherRoomOnFloor = false;
		for (uint32 i = 0; i < rooms_.GetCount (); ++i)
		{
			if (rooms_[i]->GetMin ().z != roomBounds.GetMin ().z)
				continue;

			++numRoomsOnFloor;

			const uint32 doorWidth = 1;
			if (room->IsRoomHorizontallyLinkableToOtherRoom (rooms_[i], doorWidth))
			{
				isBoundsConnectedToOtherRoomOnFloor = true; // STODO: this doesnt work in the case where the rooms only touch on the corner
				break;
			}
		}

		if (numRoomsOnFloor > 0 && !isBoundsConnectedToOtherRoomOnFloor)
		{
			room->SetMin (UPoint3D (0, 0, 0));
			room->SetMax (UPoint3D (0, 0, 0));
			return false; 					
		}

		Assert (roomBounds.GetMin ().x > 0 && roomBounds.GetMin ().x < WorldGeneratorAPI ()->GetWorldSize ());
		Assert (roomBounds.GetMin ().y > 0 && roomBounds.GetMin ().y < WorldGeneratorAPI ()->GetWorldSize ());
		Assert (roomBounds.GetMin ().z > 0 && roomBounds.GetMin ().z < WorldGeneratorAPI ()->GetWorldDepth ());
		Assert (roomBounds.GetMax ().x > 0 && roomBounds.GetMin ().x < WorldGeneratorAPI ()->GetWorldSize ());
		Assert (roomBounds.GetMax ().y > 0 && roomBounds.GetMin ().y < WorldGeneratorAPI ()->GetWorldSize ());
		Assert (roomBounds.GetMax ().z > 0 && roomBounds.GetMin ().z < WorldGeneratorAPI ()->GetWorldDepth ());
		Assert (roomBounds.GetMin ().x < roomBounds.GetMax ().x);
		Assert (roomBounds.GetMin ().y < roomBounds.GetMax ().y);
		Assert (roomBounds.GetMin ().z < roomBounds.GetMax ().z);

		return true;
	}
	return false;
}

void 
BaseBuilding::AddRoomToBuilding_ (BaseRoom* room)
{
	room->debugRoomsIndex_ = rooms_.GetCount (); // DEBUG_BASEBUILDING:

	if (room->GetFloorLevel () > (int32)numFloors_)
		numFloors_ = room->GetFloorLevel ();

	Assert (room->GetMin().x >= min_.x + minRoomAndBuildingBoundSeparation_);
	Assert (room->GetMin().y >= min_.y + minRoomAndBuildingBoundSeparation_);
	Assert (room->GetMax().x <= max_.x - minRoomAndBuildingBoundSeparation_);
	Assert (room->GetMax().y <= max_.y - minRoomAndBuildingBoundSeparation_);
	Assert (rooms_.GetCount () < 10000);
	Assert (IsRoomPlaceable_ (room->GetMin (), room->GetMax (), true));

	rooms_.InsertBack (room);
}

void
BaseBuilding::UpdateRoomPlacementSpaces_ (int32 minFloorLevel, int32 floorLevel, bool createOnlyImaginaryWallsBelowFloorLevel, 
				bool doPlaceFirstRoomOnNewFloor, bool doAllowImaginaryTypeCombination, bool allowMaximumAreaCoverageForRoofs, BaseRoom* roomToBeAdded)
{
	// STODO: for better room placement capability on floors above the first, make a mode that only creates walls, and project areas out from these walls
		// this mode would be used after the normal area making pass has failed to place a room. there are certain cases where this will prevent bad building
		// geometry from occuring. projection of these areas would be done by checking for the first area that intersects with a perpendicular projection and
		// setting the max/min of that area to the intersected wall bounds.

	// this must be true for walls to be properly placed
	Assert (rooms_.GetCount () > 0);
	Assert (minFloorLevel < floorLevel);

	// PERFORMANCE: if performace is an issue with this method with large buildings, keep track of exterior rooms and iterate through those rooms only.
	openWalls_.RemoveAndDeleteAll ();
	openAreas_.RemoveAndDeleteAll ();
	
	bool isWallAdded = AddOpenWallsToRooms_ (minFloorLevel, floorLevel, createOnlyImaginaryWallsBelowFloorLevel, doPlaceFirstRoomOnNewFloor, roomToBeAdded);

	Assert (isWallAdded);
	
	bool doCombineImaginaryAndNonImaginaryAreas = floorLevel != 1 && doAllowImaginaryTypeCombination;
	CombineOpenWalls_ (doCombineImaginaryAndNonImaginaryAreas);

	DetermineDistFromPriorityPointForWallsAndAreas_ ();
	Sort (openWalls_.GetMemory (), openWalls_.GetCount (), true); // sort walls for better area placement

	CreateOpenAreas_ (doCombineImaginaryAndNonImaginaryAreas, allowMaximumAreaCoverageForRoofs);

	do
	{
		CombineOpenAreas_ (doCombineImaginaryAndNonImaginaryAreas);
	} while (RedefineSmallOpenAreas_ ());

	// STODO: for better results, possibly fail for the entire floor here if there is one or more open areas that are smaller than the min room size
	/*
	for (uint32 i = 0; i < areas_.GetCount (); ++i)
	{
		// check if an area exists that is too small
	}
	*/

	// sort open walls and areas by distance from the center of the ground floor of the building
	DetermineDistFromPriorityPointForWallsAndAreas_ ();

	Sort (openWalls_.GetMemory (), openWalls_.GetCount (), true); // second sort here since walls can be modified by area placement
	Sort (openAreas_.GetMemory (), openAreas_.GetCount (), true);
}

bool
BaseBuilding::AddOpenWallsToRooms_ (int32 minFloorLevel, int32 floorLevel, bool createOnlyImaginaryWallsBelowFloorLevel, bool doPlaceFirstRoomOnNewFloor, BaseRoom* roomToBeAdded)
{
	bool isAWallAdded = false;

	for (uint32 i = 0; i < rooms_.GetCount (); ++i)
	{
		if (rooms_[i]->GetFloorLevel () < minFloorLevel)
			continue;

		bool placeImaginaryWallOnly = false;

		if (createOnlyImaginaryWallsBelowFloorLevel && rooms_[i]->GetFloorLevel () < floorLevel)
			placeImaginaryWallOnly = true; // real walls will only be placed on the current floor level

		// attempt to add wall(s) for each side of the room
		if (AttemptToAddOpenWallToRoom_ (rooms_[i], rooms_[i]->GetMin ().x, rooms_[i]->GetMin ().y, DIRECTION_NORTH, placeImaginaryWallOnly, doPlaceFirstRoomOnNewFloor, roomToBeAdded)) // south wall
			isAWallAdded = true;
		if (AttemptToAddOpenWallToRoom_ (rooms_[i], rooms_[i]->GetMin ().x, rooms_[i]->GetMax ().y, DIRECTION_SOUTH, placeImaginaryWallOnly, doPlaceFirstRoomOnNewFloor, roomToBeAdded)) // north wall
			isAWallAdded = true;
		if (AttemptToAddOpenWallToRoom_ (rooms_[i], rooms_[i]->GetMin ().x, rooms_[i]->GetMin ().y, DIRECTION_EAST, placeImaginaryWallOnly, doPlaceFirstRoomOnNewFloor, roomToBeAdded)) // west wall
			isAWallAdded = true;
		if (AttemptToAddOpenWallToRoom_ (rooms_[i], rooms_[i]->GetMax ().x, rooms_[i]->GetMin ().y, DIRECTION_WEST, placeImaginaryWallOnly, doPlaceFirstRoomOnNewFloor, roomToBeAdded)) // east wall
			isAWallAdded = true;
	}

	return isAWallAdded;
}

bool
BaseBuilding::AttemptToAddOpenWallToRoom_ (BaseRoom* room, uint32 initialX, uint32 initialY, Direction attachmentSide, bool placeImaginaryWall, bool doPlaceFirstRoomOnNewFloor, BaseRoom* roomToBeAdded)
{
	// Note- this method is O(n^2)
	RoomPlacementSpace* wall;
	bool hasWallBeenCreated = false;
	bool recursiveHasWallBeenCreated = false;
	
	// avoids placing a wall where a front door is, but allows an imaginary wall to be placed above
	if (room->GetIsMainRoom () && !placeImaginaryWall)
	{
		switch (attachmentSide)
		{
		case DIRECTION_SOUTH:
			placeImaginaryWall = doorSide_ == DIRECTION_NORTH;
			break;
		case DIRECTION_NORTH:
			placeImaginaryWall = doorSide_ == DIRECTION_SOUTH;
			break;
		case DIRECTION_WEST:
			placeImaginaryWall = doorSide_ == DIRECTION_EAST;
			break;
		case DIRECTION_EAST:
			placeImaginaryWall = doorSide_ == DIRECTION_WEST;
			break;
		}
	}

	// this loop conditionally attempts to add a non imaginary wall, and then attempts to add an imaginary wall
	while (true)
	{
		bool doAddWall = false;	

		// iterate along room bounds, extendng wall one block at a time as long as the wall is still on the exterior of the building
		if (attachmentSide == DIRECTION_NORTH || attachmentSide == DIRECTION_SOUTH)
		{
			for (uint32 x = initialX; x <= room->GetMax ().x; ++x)
			{
				if (room->IsPointOnExteriorWall (x, initialY, room->GetMin ().z + 1, room->GetMin ().z, false) && (placeImaginaryWall || IsPointOnWallConnectable_ (room, attachmentSide, x, initialY)))
				{
					if (!hasWallBeenCreated)
					{
						wall = CreateOpenWall_ (room, x, initialY, room->GetMin ().z, room->GetMax ().z, room->GetIsMainRoom (), attachmentSide, roomToBeAdded);
						hasWallBeenCreated = true;
						doAddWall = true;
					}
					else
					{
						wall->SetMaxX (x); // elongate wall in x direction
					}
				}
				else
				{
					if (hasWallBeenCreated)
					{
						recursiveHasWallBeenCreated = AttemptToAddOpenWallToRoom_ (room, ++x, initialY, attachmentSide, placeImaginaryWall, doPlaceFirstRoomOnNewFloor, roomToBeAdded); // recursive call
						break;
					}
				}
			}
		}
		else
		{
			for (uint32 y = initialY; y <= room->GetMax ().y; ++y)
			{
				if (room->IsPointOnExteriorWall (initialX, y, room->GetMin ().z + 1, room->GetMin ().z, false) && (placeImaginaryWall || IsPointOnWallConnectable_ (room, attachmentSide, initialX, y)))
				{
					if (!hasWallBeenCreated)
					{
						wall = CreateOpenWall_ (room, initialX, y, room->GetMin ().z, room->GetMax ().z, room->GetIsMainRoom (), attachmentSide, roomToBeAdded);
						hasWallBeenCreated = true;
						doAddWall = true;
					}
					else
					{
						wall->SetMaxY (y); // elongate wall in y direction
					}
				}
				else
				{
					if (hasWallBeenCreated)
					{
						recursiveHasWallBeenCreated = AttemptToAddOpenWallToRoom_ (room, initialX, ++y, attachmentSide, placeImaginaryWall, doPlaceFirstRoomOnNewFloor, roomToBeAdded); // recursive call
						break;
					}
				}
			}
		}

		if (doAddWall)
		{
			if (wall->GetLengthX () > 1 || wall->GetLengthY () > 1)
			{
				if (placeImaginaryWall)
				{
					bool doMakeWallNotImaginary;
					if (doPlaceFirstRoomOnNewFloor && 
						(roomToBeAdded == NULL || 
						(room->GetIsLinkableToChildRoom (roomToBeAdded) && 
							(roomToBeAdded->GetType () != ROOM_STAIRHALL || room->GetCanContainStairs () == true)))) // STODO: clean this up
					{
						doMakeWallNotImaginary = true;
					}
					else
					{
						doMakeWallNotImaginary = false;
					}

					AddImaginaryWall_ (wall, true, doMakeWallNotImaginary);
					delete wall;
				}
				else
				{
					openWalls_.InsertBack (wall);
					hasWallBeenCreated = false;
				}
			}
			else
			{
				delete wall;
				hasWallBeenCreated = false;
			}
		}

		if (placeImaginaryWall)
			break;
		else
			placeImaginaryWall = true;
	}

	return hasWallBeenCreated || recursiveHasWallBeenCreated;
}

RoomPlacementSpace*
BaseBuilding::CreateOpenWall_ (BaseRoom* room, uint32 x, uint32 y, uint32 minZ, uint32 maxZ, bool isPriorityHigh, Direction attachmentSide, BaseRoom* roomToBeAdded)
{
	RoomPlacementSpace* wall = new RoomPlacementSpace ();
	wall->SetMin (x, y, minZ);
	wall->SetMax (wall->GetMin ());
	wall->SetMaxZ (maxZ);
	wall->attachmentSide = attachmentSide;

	if (roomToBeAdded == NULL || room->GetIsLinkableToChildRoom (roomToBeAdded))
		wall->imaginaryType = IMAGINARY_FALSE;
	else
		wall->imaginaryType = IMAGINARY_TRUE;

	Assert ((wall->GetLengthX () == 1 && wall->GetLengthY () == 1) ||
		((wall->GetLengthX () == 1 || wall->attachmentSide == DIRECTION_SOUTH || wall->attachmentSide == DIRECTION_NORTH) &&
		(wall->GetLengthY () == 1 || wall->attachmentSide == DIRECTION_WEST || wall->attachmentSide == DIRECTION_EAST)));

	return wall;
}

bool
BaseBuilding::AddImaginaryWall_ (RoomPlacementSpace* parentWall, bool raiseFloorLevelByOne, bool doMakeWallNotImaginary, uint32 recursiveDepth)
{
	// doMakeWallNotImaginary is used when the first room is being placed on a new floor
	Assert (recursiveDepth < 50);
	Assert (openWalls_.GetCount () < 50000);

	// PERFORMANCE: this only needs to be called once per floor level, not once after every room is added. a separate vector of walls would be needed
	// add additional wall on floor above using the wall just added
	RoomPlacementSpace* wall = parentWall;
	RoomPlacementSpace* imaginaryWall = new RoomPlacementSpace (*wall);

	if (doMakeWallNotImaginary)
		imaginaryWall->imaginaryType = IMAGINARY_FALSE;
	else
		imaginaryWall->imaginaryType = IMAGINARY_TRUE;

	if (raiseFloorLevelByOne)
	{
		imaginaryWall->SetMinZ (imaginaryWall->GetMax ().z);
		imaginaryWall->SetMaxZ (imaginaryWall->GetMin ().z + ceilingHeight_);
	}

	// check if imaginary wall intersects with any existing rooms and modify or delete the imaginary wall accordingly to make sure it does not intersect
	{
		BaseRoom* intersectedRoom = NULL;
		BaseRoom** intersectedRoomPtr = &intersectedRoom;

		IntersectType inType = IntersectAreaWithRooms (imaginaryWall->GetMin (), imaginaryWall->GetMax (), true, intersectedRoomPtr);

		Assert (inType != INTERSECTTYPE_ENCAPSULATE);

		if (inType == INTERSECTTYPE_INSIDE)
		{
			delete imaginaryWall;
			return false;
		}
		else if (inType == INTERSECTTYPE_INTERSECT)
		{
			Assert (intersectedRoom != NULL);
			Assert (DoesBoundsIntersectOnlyOnBorderOfArea_ (imaginaryWall->GetMin (), imaginaryWall->GetMax (), intersectedRoom->GetMin (), intersectedRoom->GetMax ()));
			Assert (!DoesWallIntersectWithAreaOnlyOnEndOfWall_ (imaginaryWall->GetMin (), imaginaryWall->GetMax (), intersectedRoom->GetMin (), intersectedRoom->GetMax ()));

			// this code performs a recursive call on the imaginary wall after intersection and the rest of the wall removed from the intersection
			bool retVal = false;
			// use this in a recursive call in case the imaginary wall can continue on the other side of the intersected room
			RoomPlacementSpace restOfWall (*imaginaryWall);

			if (imaginaryWall->GetLengthX () == 1)
			{
				if (imaginaryWall->GetMin ().y < intersectedRoom->GetMin ().y)
				{
					imaginaryWall->SetMaxY (intersectedRoom->GetMin ().y);
					restOfWall.SetMinY (intersectedRoom->GetMin ().y);
					if (restOfWall.GetLengthY () > 1)
						retVal = AddImaginaryWall_ (imaginaryWall, false, doMakeWallNotImaginary, recursiveDepth + 1) || AddImaginaryWall_ (&restOfWall, false, doMakeWallNotImaginary, recursiveDepth + 1);
					else
						retVal = AddImaginaryWall_ (imaginaryWall, false, doMakeWallNotImaginary, recursiveDepth + 1);
				}
				else if (imaginaryWall->GetMax ().y > intersectedRoom->GetMax ().y)
				{
					imaginaryWall->SetMinY (intersectedRoom->GetMax ().y);
					restOfWall.SetMaxY (intersectedRoom->GetMax ().y);
					if (restOfWall.GetLengthY () > 1)
						retVal = AddImaginaryWall_ (imaginaryWall, false, doMakeWallNotImaginary, recursiveDepth + 1) || AddImaginaryWall_ (&restOfWall, false, doMakeWallNotImaginary, recursiveDepth + 1);
					else
						retVal = AddImaginaryWall_ (imaginaryWall, false, doMakeWallNotImaginary, recursiveDepth + 1);
				}
				else
				{
					Assert (0);
				}
			}
			else
			{
				if (imaginaryWall->GetMin ().x < intersectedRoom->GetMin ().x)
				{
					imaginaryWall->SetMaxX (intersectedRoom->GetMin ().x);
					restOfWall.SetMinX (intersectedRoom->GetMin ().x);
					if (restOfWall.GetLengthX () > 1)
						retVal = AddImaginaryWall_ (imaginaryWall, false, doMakeWallNotImaginary, recursiveDepth + 1) || AddImaginaryWall_ (&restOfWall, false, doMakeWallNotImaginary, recursiveDepth + 1);
					else
						retVal = AddImaginaryWall_ (imaginaryWall, false, doMakeWallNotImaginary, recursiveDepth + 1);
				}
				else if (imaginaryWall->GetMax ().x > intersectedRoom->GetMax ().x)
				{
					imaginaryWall->SetMinX (intersectedRoom->GetMax ().x);
					restOfWall.SetMaxX (intersectedRoom->GetMax ().x);
					if (restOfWall.GetLengthX () > 1)
						retVal = AddImaginaryWall_ (imaginaryWall, false, doMakeWallNotImaginary, recursiveDepth + 1) || AddImaginaryWall_ (&restOfWall, false, doMakeWallNotImaginary, recursiveDepth + 1);
					else
						retVal = AddImaginaryWall_ (imaginaryWall, false, doMakeWallNotImaginary, recursiveDepth + 1);
				}
				else
				{
					Assert (0);
				}
			}
			return retVal; // the imaginary wall will have been inserted into open walls
		}
	}

	// flip attachment side since it must point toward the interior of the building, not away from it
	switch (imaginaryWall->attachmentSide)
	{
	case DIRECTION_SOUTH:
		imaginaryWall->attachmentSide = DIRECTION_NORTH;
		break;
	case DIRECTION_NORTH:
		imaginaryWall->attachmentSide = DIRECTION_SOUTH;
		break;
	case DIRECTION_WEST:
		imaginaryWall->attachmentSide = DIRECTION_EAST;
		break;
	case DIRECTION_EAST:
		imaginaryWall->attachmentSide = DIRECTION_WEST;
		break;
	}

	Assert ((imaginaryWall->GetLengthX () == 1 || imaginaryWall->attachmentSide == DIRECTION_SOUTH || imaginaryWall->attachmentSide == DIRECTION_NORTH) &&
		(imaginaryWall->GetLengthY () == 1 || imaginaryWall->attachmentSide == DIRECTION_WEST || imaginaryWall->attachmentSide == DIRECTION_EAST));

	openWalls_.InsertBack (imaginaryWall);

	Assert (openWalls_.GetCount () < 50000);

	return true;
}

void
BaseBuilding::CombineOpenWalls_ (bool doCombineImaginaryWithNonImaginary)
{
	// doCombineImaginaryWithNonImaginary is currently set to true on the first floor of buildings. this allows for more usable room placement spaces since imaginary spaces are not used for placing rooms on the first floor 

	bool isCombinationPerformed = true;
	while (isCombinationPerformed)
	{
		isCombinationPerformed = false;

		for (int32 i = 0; i < (int32)openWalls_.GetCount (); ++i)
		{
			for (int32 j = i + 1; j < (int32)openWalls_.GetCount (); ++j)
			{
				Assert (i >= 0);

				if (i == j || openWalls_[i]->GetMin ().z != openWalls_[j]->GetMin ().z)
					continue;
				if (openWalls_[i]->attachmentSide != openWalls_[j]->attachmentSide)
					continue;
				if (!doCombineImaginaryWithNonImaginary && 
					openWalls_[i]->imaginaryType != openWalls_[j]->imaginaryType && 
					(openWalls_[i]->imaginaryType == IMAGINARY_FALSE || openWalls_[j]->imaginaryType == IMAGINARY_FALSE))
					continue;
				if (!IntersectAABoxWithAABoxFast (openWalls_[i]->GetMin (), openWalls_[i]->GetMax (), openWalls_[j]->GetMin (), openWalls_[j]->GetMax ()))
					continue;

				bool doCombination = false;

				if (openWalls_[i]->GetMin ().x == openWalls_[j]->GetMax ().x && openWalls_[i]->GetMin ().y == openWalls_[i]->GetMax ().y && openWalls_[j]->GetMin ().y == openWalls_[j]->GetMax ().y)
				{
					openWalls_[i]->SetMinX (openWalls_[j]->GetMin ().x);
					doCombination = true;
				}
				else if (openWalls_[i]->GetMax ().x == openWalls_[j]->GetMin ().x && openWalls_[i]->GetMin ().y == openWalls_[i]->GetMax ().y && openWalls_[j]->GetMin ().y == openWalls_[j]->GetMax ().y)
				{
					openWalls_[i]->SetMaxX (openWalls_[j]->GetMax ().x);
					doCombination = true;
				}
				else if (openWalls_[i]->GetMin ().y == openWalls_[j]->GetMax ().y && openWalls_[i]->GetMin ().x == openWalls_[i]->GetMax ().x && openWalls_[j]->GetMin ().x == openWalls_[j]->GetMax ().x)
				{
					openWalls_[i]->SetMinY (openWalls_[j]->GetMin ().y);
					doCombination = true;
				}
				else if (openWalls_[i]->GetMax ().y == openWalls_[j]->GetMin ().y && openWalls_[i]->GetMin ().x == openWalls_[i]->GetMax ().x && openWalls_[j]->GetMin ().x == openWalls_[j]->GetMax ().x)
				{
					openWalls_[i]->SetMaxY (openWalls_[j]->GetMax ().y);
					doCombination = true;
				}

				if (doCombination)
				{
					isCombinationPerformed = true;
					if (openWalls_[i]->imaginaryType != openWalls_[i]->imaginaryType)
						openWalls_[i]->imaginaryType = IMAGINARY_PARTIAL;
					openWalls_.RemoveAndDelete (j);

					Assert ((openWalls_[i]->GetLengthX () == 1 && openWalls_[i]->GetLengthY () == 1) ||
						((openWalls_[i]->GetLengthX () == 1 || openWalls_[i]->attachmentSide == DIRECTION_SOUTH || openWalls_[i]->attachmentSide == DIRECTION_NORTH) &&
						(openWalls_[i]->GetLengthY () == 1 || openWalls_[i]->attachmentSide == DIRECTION_WEST || openWalls_[i]->attachmentSide == DIRECTION_EAST)));

					// DEBUG_BASEBUILDING:
					if( rooms_.GetCount() >= 7 )
					{	
						if(openWalls_[i]->GetMin() == UPoint3D(210, 212, 315) && openWalls_[i]->GetMax() == UPoint3D(236, 212, 320)) // area forming wall north
							int breakHere1 = 1;
						if(openWalls_[i]->GetMin() == UPoint3D(210, 205, 315) && openWalls_[i]->GetMax() == UPoint3D(210, 212, 320)) // area forming wall west
							int breakHere1 = 1;
						if(openWalls_[i]->GetMin() == UPoint3D(217, 205, 315) && openWalls_[i]->GetMax() == UPoint3D(217, 223, 320)) // intersecting wall east
							int breakHere1 = 1;
					}

					i = -1;
					break;
				}
			}
		}
	}
}

bool
BaseBuilding::CombineOpenAreas_ (bool doCombineImaginaryWithNonImaginary)
{
	bool isCombinationPerformed = true;
	while (isCombinationPerformed)
	{
		isCombinationPerformed = false;

		for (int32 i = 0; i < (int32)openAreas_.GetCount (); ++i)
		{
			for (int32 j = i + 1; j < (int32)openAreas_.GetCount (); ++j)
			{
				Assert (i >= 0);

				if (i == j || openAreas_[i]->GetMin ().z != openAreas_[j]->GetMin ().z)
					continue;
				if (!doCombineImaginaryWithNonImaginary && 
					(openAreas_[i]->imaginaryType != openAreas_[j]->imaginaryType && 
					openAreas_[i]->imaginaryType == IMAGINARY_TRUE || openAreas_[j]->imaginaryType == IMAGINARY_TRUE))
					continue;
				if (!IntersectAABoxWithAABoxFast (openAreas_[i]->GetMin (), openAreas_[i]->GetMax (), openAreas_[j]->GetMin (), openAreas_[j]->GetMax ()))
					continue;

				bool doCombination = false;

				bool combineAlongX;

				{
					// randomly choose which axis to combine along first, then do other axis if that fails
					combineAlongX = rand () % 2 == 0;
					for (uint32 k = 0; k <= 1; ++k)
					{
						if (combineAlongX)
						{
							if (openAreas_[i]->GetMin ().y == openAreas_[j]->GetMin ().y && openAreas_[i]->GetMax ().y == openAreas_[j]->GetMax ().y)
							{
								if (openAreas_[i]->GetMin ().x == openAreas_[j]->GetMax ().x)
								{
									openAreas_[i]->SetMinX (openAreas_[j]->GetMin ().x);
									doCombination = true;
								}
								else if (openAreas_[i]->GetMax ().x == openAreas_[j]->GetMin ().x)
								{
									openAreas_[i]->SetMaxX (openAreas_[j]->GetMax ().x);
									doCombination = true;
								}
							}
						}
						else
						{
							if (openAreas_[i]->GetMin ().x == openAreas_[j]->GetMin ().x && openAreas_[i]->GetMax ().x == openAreas_[j]->GetMax ().x)
							{
								if (openAreas_[i]->GetMin ().y == openAreas_[j]->GetMax ().y)
								{
									openAreas_[i]->SetMinY (openAreas_[j]->GetMin ().y);
									doCombination = true;
								}
								else if (openAreas_[i]->GetMax ().y == openAreas_[j]->GetMin ().y)
								{
									openAreas_[i]->SetMaxY (openAreas_[j]->GetMax ().y);
									doCombination = true;
								}
							}
						}
						if (doCombination)
							break;
						combineAlongX = !combineAlongX;
					}
				}

				if (doCombination)
				{
					isCombinationPerformed = true;

					Direction attachmentSide;

					// set attachment side by picking the least imaginary area's attachment side or if they are equal, pick at random
					if (openAreas_[i]->imaginaryType != openAreas_[j]->imaginaryType)
					{
						if (openAreas_[i]->imaginaryType == IMAGINARY_FALSE)
						{
							attachmentSide = openAreas_[i]->attachmentSide;
						}
						else if (openAreas_[i]->imaginaryType == IMAGINARY_PARTIAL)
						{
							if (openAreas_[j]->imaginaryType == IMAGINARY_TRUE)
								attachmentSide = openAreas_[i]->attachmentSide;
							else
								attachmentSide = openAreas_[j]->attachmentSide;
						}
						else
						{
							attachmentSide = openAreas_[j]->attachmentSide;
						}

						openAreas_[i]->imaginaryType = IMAGINARY_PARTIAL;
					}
					else
					{
						if (rand () % 2)
							attachmentSide = openAreas_[i]->attachmentSide;
						else
							attachmentSide = openAreas_[j]->attachmentSide;
					}

					/*
					// STODO: make sure that this old code is useless and delete it
					if (combineAlongX)
					{
						if (openAreas_[i]->attachmentSide == DIRECTION_WEST || openAreas_[j]->attachmentSide == DIRECTION_WEST)
							attachmentSide = DIRECTION_WEST;
						else
							attachmentSide = DIRECTION_EAST;
					}
					else
					{
						if (openAreas_[i]->attachmentSide == DIRECTION_SOUTH || openAreas_[j]->attachmentSide == DIRECTION_SOUTH)
							attachmentSide = DIRECTION_SOUTH;
						else
							attachmentSide = DIRECTION_NORTH;
					}
					*/

					openAreas_[i]->attachmentSide = attachmentSide;	

					openAreas_.RemoveAndDelete (j);
					//--j;
					i = -1; // this is slower than just decrementing j, but should work better
					break;
				}
			}
		}
	}

	return isCombinationPerformed;
}

bool
BaseBuilding::RedefineSmallOpenAreas_ ()
{
	// this method finds open areas that have a smaller width than the min room size and attempts to redefine these areas by using space from adjacent areas

	// Note- unlike similar algorithms, it is necessary that all open areas are compared to each other twice
	bool isModificationPerformed = false;
	for (uint32 i = 0; i < openAreas_.GetCount (); ++i)
	{
		for (uint32 j = 0; j < openAreas_.GetCount (); ++j) 
		{
			if (i == j)
				continue;

			// STODO: make this check smarter and possibly pass in parameters that change its behavior
			if (openAreas_[i]->imaginaryType == IMAGINARY_TRUE && openAreas_[j]->imaginaryType != IMAGINARY_TRUE)
				continue; // makes sure to not increase the size of fully imaginary areas while decreasing the size of partial or non imaginary areas. 		

			if ((int32)openAreas_[i]->GetLengthY () < ((int32)openAreas_[j]->GetLengthX () - (int32)openAreas_[i]->GetLengthX ()) && 
				IsAreaFullyAttachedToOtherAreaByWall_ (openAreas_[i]->GetMin (), openAreas_[i]->GetMax (), openAreas_[j]->GetMin (), openAreas_[j]->GetMax (), true))
			{
				bool successfulModification = false;

				// modify other area bounds to allow room for extension of offending area
				if (openAreas_[i]->GetMin ().x == openAreas_[j]->GetMin ().x && openAreas_[i]->GetMax ().x <= openAreas_[j]->GetMax ().x - openAreas_[i]->GetLengthY ())
				{
					openAreas_[j]->SetMinX (openAreas_[i]->GetMax ().x);
					successfulModification = true;
				}
				else if (openAreas_[i]->GetMax ().x == openAreas_[j]->GetMax ().x && openAreas_[i]->GetMin ().x >= openAreas_[j]->GetMin ().x + openAreas_[i]->GetLengthY ())
				{
					openAreas_[j]->SetMaxX (openAreas_[i]->GetMin ().x);
					successfulModification = true;
				}
				else if (openAreas_[i]->GetMin ().x >= openAreas_[j]->GetMin ().x + openAreas_[i]->GetLengthY () && openAreas_[i]->GetMax ().x <= openAreas_[j]->GetMax ().x - openAreas_[i]->GetLengthY ())
				{
					// create new area and modify other area
					RoomPlacementSpace* newArea = new RoomPlacementSpace (*openAreas_[j]);
					openAreas_[j]->SetMinX (openAreas_[i]->GetMax ().x);
					newArea->SetMaxX (openAreas_[i]->GetMin ().x);
					successfulModification = true;
				}

				// set new bounds on offending axis
				if (successfulModification)
				{
					if (openAreas_[i]->GetMin ().y == openAreas_[j]->GetMax ().y)
						openAreas_[i]->SetMinY (openAreas_[j]->GetMin ().y);
					else if (openAreas_[i]->GetMax ().y == openAreas_[j]->GetMin ().y)
						openAreas_[i]->SetMaxY (openAreas_[j]->GetMax ().y);
					isModificationPerformed = true;
				}
			}
			else if ((int32)openAreas_[i]->GetLengthX () < ((int32)openAreas_[j]->GetLengthY () - (int32)openAreas_[i]->GetLengthY ()) && 
				IsAreaFullyAttachedToOtherAreaByWall_ (openAreas_[i]->GetMin (), openAreas_[i]->GetMax (), openAreas_[j]->GetMin (), openAreas_[j]->GetMax (), false))
			{
				bool successfulModification = false;

				// modify other area bounds to allow room for extension of offending area
				if (openAreas_[i]->GetMin ().y == openAreas_[j]->GetMin ().y && openAreas_[i]->GetMax ().y <= openAreas_[j]->GetMax ().y - openAreas_[i]->GetLengthX ())
				{
					openAreas_[j]->SetMinY (openAreas_[i]->GetMax ().y);
					successfulModification = true;
				}
				else if (openAreas_[i]->GetMax ().y == openAreas_[j]->GetMax ().y && openAreas_[i]->GetMin ().y >= openAreas_[j]->GetMin ().y + openAreas_[i]->GetLengthX ())
				{
					openAreas_[j]->SetMaxY (openAreas_[i]->GetMin ().y);
					successfulModification = true;
				}
				else if (openAreas_[i]->GetMin ().y >= openAreas_[j]->GetMin ().y + openAreas_[i]->GetLengthX () && openAreas_[i]->GetMax ().y <= openAreas_[j]->GetMax ().y - openAreas_[i]->GetLengthX ())
				{
					// create new area and modify other area
					RoomPlacementSpace* newArea = new RoomPlacementSpace (*openAreas_[j]);
					openAreas_[j]->SetMinY (openAreas_[i]->GetMax ().y);
					newArea->SetMaxY (openAreas_[i]->GetMin ().y);
					successfulModification = true;
				}

				// set new bounds on offending axis
				if (successfulModification)
				{
					if (openAreas_[i]->GetMin ().x == openAreas_[j]->GetMax ().x)
						openAreas_[i]->SetMinX (openAreas_[j]->GetMin ().x);
					else if (openAreas_[i]->GetMax ().x == openAreas_[j]->GetMin ().x)
						openAreas_[i]->SetMaxX (openAreas_[j]->GetMax ().x);
					isModificationPerformed = true;
				}
			}
		}
	}
	return isModificationPerformed;
}

void
BaseBuilding::CreateOpenAreas_ (bool doCombineImaginaryAndNonImaginaryWalls, bool allowMaximumAreaCoverageForRoofs)
{
	const uint32 wallThickness = 1;
	RoomPlacementSpace* openArea;

	ImaginaryType mostImaginaryTypeAllowed;
	if (allowMaximumAreaCoverageForRoofs)
		mostImaginaryTypeAllowed = IMAGINARY_TRUE; // prevents somewhat rare gaps in roofs from forming. STODO: does this prevent that from happening in every case?
	else
		mostImaginaryTypeAllowed = IMAGINARY_FALSE; // first create areas out of non imaginary walls, then create areas out of only one non imaginary wall and one non, then allow all

	bool isNewAreaCreated = true;
	while (isNewAreaCreated || mostImaginaryTypeAllowed != IMAGINARY_TRUE)
	{
		if (!isNewAreaCreated)
		{
			if (mostImaginaryTypeAllowed == IMAGINARY_FALSE)
				mostImaginaryTypeAllowed = IMAGINARY_PARTIAL;
			else
				mostImaginaryTypeAllowed = IMAGINARY_TRUE;
		}

		isNewAreaCreated = false;
		bool isAreaPlaceable = false;

		// create areas with the intersection of two walls
		for (int32 i = 0; i < (int32)openWalls_.GetCount (); ++i)
		{
			if (mostImaginaryTypeAllowed == IMAGINARY_FALSE && openWalls_[i]->imaginaryType != IMAGINARY_FALSE)
				continue;

			for (int32 j = i + 1; j < (int32)openWalls_.GetCount (); ++j)
			{
				Assert (i >= 0 && j >= 0);

				if (mostImaginaryTypeAllowed == IMAGINARY_FALSE && openWalls_[j]->imaginaryType != IMAGINARY_FALSE)
					continue;
				else if (mostImaginaryTypeAllowed == IMAGINARY_PARTIAL && openWalls_[i]->imaginaryType == IMAGINARY_TRUE && openWalls_[j]->imaginaryType == IMAGINARY_TRUE)
					continue;

				bool createArea = false;

				if (openWalls_[i]->GetMin ().z != openWalls_[j]->GetMin ().z)
					continue;

				Direction attachmentSide;

				// STODO: export these four checks to a method
				// check if walls intersect and if the wall directions are appropriate
				// front left corner of area to be created
				if ((openWalls_[i]->GetMin ().x == openWalls_[j]->GetMin ().x && openWalls_[i]->GetMin ().y == openWalls_[j]->GetMin ().y) &&
					((openWalls_[i]->attachmentSide == DIRECTION_SOUTH && openWalls_[j]->attachmentSide == DIRECTION_WEST) || 
					(openWalls_[i]->attachmentSide == DIRECTION_WEST && openWalls_[j]->attachmentSide == DIRECTION_SOUTH)))
				{
					createArea = true;
					if (rand () % 2 == 0)
						attachmentSide = DIRECTION_SOUTH;
					else
						attachmentSide = DIRECTION_WEST;
				}
				// front right corner
				else if ((openWalls_[i]->GetMax ().x == openWalls_[j]->GetMax ().x && openWalls_[i]->GetMin ().y == openWalls_[j]->GetMin ().y) &&
					((openWalls_[i]->attachmentSide == DIRECTION_SOUTH && openWalls_[j]->attachmentSide == DIRECTION_EAST) || 
					(openWalls_[i]->attachmentSide == DIRECTION_EAST && openWalls_[j]->attachmentSide == DIRECTION_SOUTH)))
				{
					createArea = true;
					if (rand () % 2 == 0)
						attachmentSide = DIRECTION_SOUTH;
					else
						attachmentSide = DIRECTION_EAST;
				}
				// back left corner
				else if ((openWalls_[i]->GetMin ().x == openWalls_[j]->GetMin ().x && openWalls_[i]->GetMax ().y == openWalls_[j]->GetMax ().y) &&
					((openWalls_[i]->attachmentSide == DIRECTION_NORTH && openWalls_[j]->attachmentSide == DIRECTION_WEST) || 
					(openWalls_[i]->attachmentSide == DIRECTION_WEST && openWalls_[j]->attachmentSide == DIRECTION_NORTH)))
				{
					createArea = true;
					if (rand () % 2 == 0)
						attachmentSide = DIRECTION_NORTH;
					else
						attachmentSide = DIRECTION_WEST;
				}
				// back right corner
				else if ((openWalls_[i]->GetMax ().x == openWalls_[j]->GetMax ().x && openWalls_[i]->GetMax ().y == openWalls_[j]->GetMax ().y) &&
					((openWalls_[i]->attachmentSide == DIRECTION_NORTH && openWalls_[j]->attachmentSide == DIRECTION_EAST) || 
					(openWalls_[i]->attachmentSide == DIRECTION_EAST && openWalls_[j]->attachmentSide == DIRECTION_NORTH)))
				{
					createArea = true;
					if (rand () % 2 == 0)
						attachmentSide = DIRECTION_NORTH;
					else
						attachmentSide = DIRECTION_EAST;
				}

				if (createArea)
				{
					// create initial area bounds using the bounds of the intersecting walls
					openArea = new RoomPlacementSpace ();
					openArea->SetMin (Min (openWalls_[i]->GetMin ().x, openWalls_[j]->GetMin ().x), 
						Min (openWalls_[i]->GetMin ().y, openWalls_[j]->GetMin ().y), 
						Min (openWalls_[i]->GetMin ().z, openWalls_[j]->GetMin ().z));
					openArea->SetMax (Max (openWalls_[i]->GetMax ().x, openWalls_[j]->GetMax ().x), 
						Max (openWalls_[i]->GetMax ().y, openWalls_[j]->GetMax ().y), 
						Max (openWalls_[i]->GetMax ().z, openWalls_[j]->GetMax ().z));

					// this isImaginary setting is used to give a preference to non imaginary areas over imaginary ones for room placement
					if (openWalls_[i]->imaginaryType != openWalls_[j]->imaginaryType)
						openArea->imaginaryType = IMAGINARY_PARTIAL;
					else
						openArea->imaginaryType = openWalls_[i]->imaginaryType;

					for (uint32 k = 0; k < openWalls_.GetCount (); ++k)
					{
						if (k == i || k == j)
							continue;

						// if the open area bounds intersect with a wall (not on wall ends), modify its bounds so they dont intersect
						if (IntersectAABoxWithAABoxFast (openWalls_[k]->GetMin (), openWalls_[k]->GetMax (), openArea->GetMin (), openArea->GetMax ()) &&
							!DoesBoundsIntersectOnlyOnBorderOfArea_ (openWalls_[k]->GetMin (), openWalls_[k]->GetMax (), openArea->GetMin (), openArea->GetMax ()))
						{
							RoomPlacementSpace tmpArea1 (*openArea);
							RoomPlacementSpace tmpArea2 (*openArea);
							delete openArea;

							tmpArea1.attachmentSide = openWalls_[i]->attachmentSide;
							tmpArea2.attachmentSide = openWalls_[j]->attachmentSide;
							
							ModifyAreaBoundsToAvoidWallIntersection_ (tmpArea1, *openWalls_[k]);
							ModifyAreaBoundsToAvoidWallIntersection_ (tmpArea2, *openWalls_[k]);

							// use the tmp area that allows the largest minimum length on any axis as the new open area 
							// STODO: this will not prevent all open areas that are too small to place a room from being created
							if (Min (tmpArea1.GetLengthX (), tmpArea1.GetLengthY ()) >= Min (tmpArea2.GetLengthX (), tmpArea2.GetLengthY ()))
								openArea = new RoomPlacementSpace (tmpArea1);
							else
								openArea = new RoomPlacementSpace (tmpArea2);

							// SNOW: if the areas are not the same and the second smallest length space is adequately large, 
								// create two areas instead of one. use a vector of areas and have all further operations deal with the vector
						}
					}
					
					// check if open area intersects with any other open areas and remove it if so
					bool removeOpenArea = false;
					for (uint32 k = 0; k < openAreas_.GetCount (); ++k)
					{
						if (IntersectAABoxWithAABoxFast (openAreas_[k]->GetMin (), openAreas_[k]->GetMax (), openArea->GetMin (), openArea->GetMax ()) &&
							!DoesBoundsIntersectOnlyOnBorderOfArea_ (openAreas_[k]->GetMin (), openAreas_[k]->GetMax (), openArea->GetMin (), openArea->GetMax ()))
						{
							// dont remove area if the intersection is just on the border
							if (openArea->GetMin ().x != openAreas_[k]->GetMax ().x && openArea->GetMax ().x != openAreas_[k]->GetMin ().x &&
								openArea->GetMin ().y != openAreas_[k]->GetMax ().y && openArea->GetMax ().y != openAreas_[k]->GetMin ().y)
							{
								removeOpenArea = true;
								break;
							}
						}
					}

					openArea->attachmentSide = attachmentSide;

					if (!removeOpenArea)
					{
						Assert (openArea->GetLengthX () > 1 && openArea->GetLengthY () > 1);

						if (openArea->GetLengthX () == 1 || openArea->GetLengthY () == 1)
							removeOpenArea = true;
					}

					// place open area if it does not intersect with rooms
					if (!removeOpenArea && !IntersectAreaWithRoomsFast (openArea->GetMin (), openArea->GetMax (), true))
					{
						AddAndModifyOpenWallsForNewArea_ (openArea, i, j, doCombineImaginaryAndNonImaginaryWalls);
						
						openAreas_.InsertBack (openArea);
					
						isNewAreaCreated = true;

						i = -1; // will be incremented to zero
						break;
					}
					else
					{
						delete openArea;
					}
				}
			}
		}
	}
}

void
BaseBuilding::ModifyAreaBoundsToAvoidWallIntersection_ (RoomPlacementSpace& area, const RoomPlacementSpace& wall)
{
	// DEBUG_BASEBUILDING:
	RoomPlacementSpace DebugArea(area);

	switch (area.attachmentSide)
	{
		case DIRECTION_SOUTH:
			area.SetMaxY (wall.GetMin ().y);
			break;
		case DIRECTION_NORTH:
			area.SetMinY (wall.GetMax ().y);
			break;
		case DIRECTION_WEST:
			area.SetMaxX (wall.GetMin ().x);
			break;
		case DIRECTION_EAST:
			area.SetMinX (wall.GetMax ().x);
			break;
	}

	Assert( area.GetMin().x <= area.GetMax().x );
	Assert( area.GetMin().y <= area.GetMax().y );
	Assert( area.GetMin().z <= area.GetMax().z );
}

void
BaseBuilding::AddAndModifyOpenWallsForNewArea_ (const RoomPlacementSpace* openArea, uint32 i, uint32 j, bool doCombineImaginaryAndNonImaginaryWalls)
{
	// creates two walls opposite from the open walls that created the area. delete or modify these walls if they intersect with other walls or areas
	Vector<RoomPlacementSpace*> newXWalls;
	Vector<RoomPlacementSpace*> newYWalls;

	{
		RoomPlacementSpace* newXWall = new RoomPlacementSpace (*openArea);
		RoomPlacementSpace* newYWall = new RoomPlacementSpace (*openArea);
		newXWall->imaginaryType = IMAGINARY_TRUE;
		newYWall->imaginaryType = IMAGINARY_TRUE;

		if (openWalls_[i]->attachmentSide == DIRECTION_SOUTH || openWalls_[j]->attachmentSide == DIRECTION_SOUTH)
		{
			newXWall->SetMinY (openArea->GetMax ().y);
			newXWall->attachmentSide = DIRECTION_SOUTH;
		}
		else
		{
			newXWall->SetMaxY (openArea->GetMin ().y);
			newXWall->attachmentSide = DIRECTION_NORTH;
		}

		if (openWalls_[i]->attachmentSide == DIRECTION_WEST || openWalls_[j]->attachmentSide == DIRECTION_WEST)
		{
			newYWall->SetMinX (openArea->GetMax ().x);
			newYWall->attachmentSide = DIRECTION_WEST;
		}
		else
		{
			newYWall->SetMaxX (openArea->GetMin ().x);
			newYWall->attachmentSide = DIRECTION_EAST;
		}

		newXWalls.InsertBack (newXWall);
		newYWalls.InsertBack (newYWall);
	}

	for (uint32 k = 0; k < openWalls_.GetCount (); ++k)
	{
		if (k == i || k == j)
			continue;

		AddAndModifyOpenWallsForNewAreaLoop_( newXWalls, newYWalls, openWalls_[k] );
	}

	if(rooms_.GetCount() == 7 && openArea->GetMin() == UPoint3D(200, 212, 315) && openArea->GetMax() == UPoint3D(217, 223, 320))
		int32 breakHere = 3;

	DeleteAndModifyOpenWallsThatIntersectWithOpenArea_ (openArea);

	// this is used to check if the new walls that are added intersect with any existing open areas and modify/delete them if so
	uint32 openWallsStartIndex = openWalls_.GetCount();

	// add to openWalls after shrinking and removing them so these walls arent affected
	for (uint32 k = 0; k < newXWalls.GetCount (); ++k)
		openWalls_.InsertBack (newXWalls[k]);

	for (uint32 k = 0; k < newYWalls.GetCount (); ++k)
		openWalls_.InsertBack (newYWalls[k]);

	if (newXWalls.GetCount () + newYWalls.GetCount () >= 1)
	{
		for( uint32 k = 0; k < openAreas_.GetCount(); ++k )
		{
			// make sure walls dont intersect with any existing areas
			DeleteAndModifyOpenWallsThatIntersectWithOpenArea_( openAreas_[k], openWallsStartIndex );
		}

		// combine new walls with others
		CombineOpenWalls_( doCombineImaginaryAndNonImaginaryWalls );
	}

	Assert (openWalls_.GetCount () < 100000);
}

void
BaseBuilding::AddAndModifyOpenWallsForNewAreaLoop_( Vector<RoomPlacementSpace*>& newXWalls, Vector<RoomPlacementSpace*>& newYWalls, const RoomPlacementSpace* openSpace )
{
	RoomPlacementSpace* newWall;

	for (int32 i = 0; i < (int32)newXWalls.GetCount (); ++i)
	{
		Assert (i >= 0);
		Assert (newXWalls.GetCount () < 100);

		// STODO: fix this and others like it when rooms span multiple floors
		if (openSpace->GetMin ().z != newXWalls[i]->GetMin ().z)
			continue;

		if (IsAABoxContainedInAABox (newXWalls[i]->GetMin (), newXWalls[i]->GetMax (), openSpace->GetMin (), openSpace->GetMax ()))
		{
			newXWalls.RemoveAndDelete (i);
			--i;
		}
		else if (IntersectAABoxWithAABoxFast (newXWalls[i]->GetMin (), newXWalls[i]->GetMax (), openSpace->GetMin (), openSpace->GetMax ()) &&
			!DoesWallIntersectWithAreaOnlyOnEndOfWall_ (newXWalls[i]->GetMin (), newXWalls[i]->GetMax (), openSpace->GetMin (), openSpace->GetMax ()))
		{
			newWall = new RoomPlacementSpace (*newXWalls[i]);
			if (newXWalls[i]->GetMin ().x < openSpace->GetMin ().x)
			{
				newWall->SetMinX (openSpace->GetMin ().x);
				newXWalls[i]->SetMaxX (openSpace->GetMin ().x);
			}
			else if (newXWalls[i]->GetMax ().x > openSpace->GetMax ().x)
			{
				newWall->SetMaxX (openSpace->GetMax ().x);
				newXWalls[i]->SetMinX (openSpace->GetMax ().x);
			}
			else
			{
				Assert (0);
			}

			if (newWall->GetLengthX () > 1)
				newXWalls.InsertBack (newWall);
			else
				delete newWall;
		}
	}

	for (int32 i = 0; i < (int32)newYWalls.GetCount (); ++i)
	{
		Assert (i >= 0);
		Assert (newYWalls.GetCount () < 100);

		// STODO: fix this and others like it when rooms span multiple floors
		if (openSpace->GetMin ().z != newYWalls[i]->GetMin ().z)
			continue;

		if (IsAABoxContainedInAABox (newYWalls[i]->GetMin (), newYWalls[i]->GetMax (), openSpace->GetMin (), openSpace->GetMax ()))
		{
			newYWalls.RemoveAndDelete (i);
			--i;
		}
		else if (IntersectAABoxWithAABoxFast (newYWalls[i]->GetMin (), newYWalls[i]->GetMax (), openSpace->GetMin (), openSpace->GetMax ()) &&
			!DoesWallIntersectWithAreaOnlyOnEndOfWall_ (newYWalls[i]->GetMin (), newYWalls[i]->GetMax (), openSpace->GetMin (), openSpace->GetMax ()))
		{
			newWall = new RoomPlacementSpace (*newYWalls[i]);
			if (newYWalls[i]->GetMin ().y < openSpace->GetMin ().y)
			{
				newWall->SetMinY (openSpace->GetMin ().y);
				newYWalls[i]->SetMaxY (openSpace->GetMin ().y);
			}
			else if (newYWalls[i]->GetMax ().y > openSpace->GetMax ().y)
			{
				newWall->SetMaxY (openSpace->GetMax ().y);
				newYWalls[i]->SetMinY (openSpace->GetMax ().y);
			}
			else
			{
				Assert (0);
			}

			if (newWall->GetLengthY () > 1)
				newYWalls.InsertBack (newWall);
			else
				delete newWall;
		}
	}
}

void
BaseBuilding::DeleteAndModifyOpenWallsThatIntersectWithOpenArea_( const RoomPlacementSpace* openArea, uint32 startingWallIndex )
{
	if(startingWallIndex > 0)
		int breakHere = 2;
	// Note- all walls that are modified must be at the border of the open area
	for (int32 i = startingWallIndex; i < (int32)openWalls_.GetCount (); ++i)
	{
		Assert (i >= 0);

		// STODO: fix this and others like it when rooms span multiple floors
		if (openArea->GetMin ().z != openWalls_[i]->GetMin ().z)
			continue;

		if (IsAABoxContainedInAABox (openWalls_[i]->GetMin (), openWalls_[i]->GetMax (), openArea->GetMin (), openArea->GetMax ()))
		{
			if(startingWallIndex > 0)
				int breakHere1 = 2;
			// delete walls that are contained in the open area
			openWalls_.RemoveAndDelete (i);
			--i; 
		}
		else if (IntersectAABoxWithAABoxFast (openWalls_[i]->GetMin (), openWalls_[i]->GetMax (), openArea->GetMin (), openArea->GetMax ()))
		{
			Assert (DoesBoundsIntersectOnlyOnBorderOfArea_ (openWalls_[i]->GetMin (), openWalls_[i]->GetMax (), openArea->GetMin (), openArea->GetMax ()));

			if(startingWallIndex > 0)
				int breakHer2e = 2;
			
			// STODO: double check the correctness of these cases
			// reduce size of walls that partially intersect with the new open area. split into two walls if intersecting wall stretches past both sides of the area
			if (openWalls_[i]->GetMin ().x < openArea->GetMin ().x && openWalls_[i]->GetMax ().x > openArea->GetMax ().x)
			{
				RoomPlacementSpace* newWall = new RoomPlacementSpace (*openWalls_[i]);
				openWalls_[i]->SetMaxX (openArea->GetMin ().x);
				newWall->SetMinX (openArea->GetMax ().x);
				openWalls_.InsertBack (newWall);
			}
			else if (openWalls_[i]->GetMin ().y < openArea->GetMin ().y && openWalls_[i]->GetMax ().y > openArea->GetMax ().y)
			{
				RoomPlacementSpace* newWall = new RoomPlacementSpace (*openWalls_[i]);
				openWalls_[i]->SetMaxY (openArea->GetMin ().y);
				newWall->SetMinY (openArea->GetMax ().y);
				openWalls_.InsertBack (newWall);
			}
			else if (openWalls_[i]->GetMin ().x < openArea->GetMin ().x && openWalls_[i]->GetMax ().x <= openArea->GetMax ().x && openWalls_[i]->GetMax ().x != openArea->GetMin ().x)
			{
				openWalls_[i]->SetMaxX (openArea->GetMin ().x);
			}
			else if (openWalls_[i]->GetMin ().y < openArea->GetMin ().y && openWalls_[i]->GetMax ().y <= openArea->GetMax ().y && openWalls_[i]->GetMax ().y != openArea->GetMin ().y)
			{
				openWalls_[i]->SetMaxY (openArea->GetMin ().y);
			}
			else if (openWalls_[i]->GetMax ().x > openArea->GetMax ().x && openWalls_[i]->GetMin ().x >= openArea->GetMin ().x && openWalls_[i]->GetMin ().x != openArea->GetMax ().x)
			{
				openWalls_[i]->SetMinX (openArea->GetMax ().x);
			}
			else if (openWalls_[i]->GetMax ().y > openArea->GetMax ().y && openWalls_[i]->GetMin ().y >= openArea->GetMin ().y && openWalls_[i]->GetMin ().y != openArea->GetMax ().y)
			{
				openWalls_[i]->SetMinY (openArea->GetMax ().y);
			}
			else
			{
				int32 breakHere = 3;
				//Assert(0);
			}
		}

		// DEBUG_BASEBUILDING:
		if(i != -1)
		{
			if( rooms_.GetCount() >= 7 )
			{
				if(openWalls_[i]->GetMin() == UPoint3D(210, 212, 315) && openWalls_[i]->GetMax() == UPoint3D(236, 212, 320)) // area forming wall north
					int breakHere1 = 1;
				if(openWalls_[i]->GetMin() == UPoint3D(210, 205, 315) && openWalls_[i]->GetMax() == UPoint3D(210, 212, 320)) // area forming wall west
					int breakHere1 = 1;
				if(openWalls_[i]->GetMin() == UPoint3D(217, 205, 315) && openWalls_[i]->GetMax() == UPoint3D(217, 223, 320)) // intersecting wall east
					int breakHere1 = 1;
			}
		}
	}

	
}

bool
BaseBuilding::IsPointOnWallConnectable_ (BaseRoom* room, Direction attachmentSide, uint32 x, uint32 y) const
{
	// always return true for first floor rooms
	if (room->GetFloorLevel () == 1)
		return true;

	// check if floor adjacent to wall is placable
	switch (attachmentSide)
	{
	case DIRECTION_SOUTH:
		return IntersectPointWithRooms (x, y + 1, room->GetMin ().z - 1, false);
		break;
	case DIRECTION_NORTH:
		return IntersectPointWithRooms (x, y - 1, room->GetMin ().z - 1, false);
		break;
	case DIRECTION_WEST:
		return IntersectPointWithRooms (x + 1, y, room->GetMin ().z - 1, false);
		break;
	case DIRECTION_EAST:
		return IntersectPointWithRooms (x - 1, y, room->GetMin ().z - 1, false);
		break;
	}

	Assert (0);
	return false;
}

bool
BaseBuilding::IsAreaFullyAttachedToOtherAreaByWall_ (const UPoint3D& areaMin, const UPoint3D& areaMax, const UPoint3D& otherMin, const UPoint3D& otherMax, bool isAlongXAxis) const
{
	// Note- this method checks if the first area has a wall that is shared entirely with the second, not vice versa

	if (areaMin.z < otherMin.z || areaMax.z > otherMax.z)
		return false;

	if (((areaMin.x == otherMax.x || areaMax.x == otherMin.x) && areaMin.y >= otherMin.y && areaMax.y <= otherMax.y && !isAlongXAxis) ||
		((areaMin.y == otherMax.y || areaMax.y == otherMin.y) && areaMin.x >= otherMin.x && areaMax.x <= otherMax.x && isAlongXAxis))
		return true;

	return false;
}

bool
BaseBuilding::DoesBoundsIntersectOnlyOnBorderOfArea_ (const UPoint3D& boundsMin, const UPoint3D& boundsMax, const UPoint3D& areaMin, const UPoint3D& areaMax) const
{
	Assert (IntersectAABoxWithAABoxFast (boundsMin, boundsMax, areaMin, areaMax));
	
	if (boundsMin.x == areaMax.x || boundsMax.x == areaMin.x || 
		boundsMin.y == areaMax.y || boundsMax.y == areaMin.y ||
		boundsMin.z == areaMax.z || boundsMax.z == areaMin.z)
		return true;

	return false;
}

bool
BaseBuilding::DoesWallIntersectWithAreaOnlyOnEndOfWall_ (const UPoint3D& wallMin, const UPoint3D& wallMax, const UPoint3D& areaMin, const UPoint3D& areaMax) const
{
	Assert (wallMax.x - wallMin.x == 0 || wallMax.y - wallMin.y == 0);
	Assert (IntersectAABoxWithAABoxFast (wallMin, wallMax, areaMin, areaMax));
	Assert (wallMin != areaMin || wallMax != areaMax);

	if (wallMin.z == areaMax.z || wallMax.z == areaMin.z)
		return true;

	if ((wallMax.x - wallMin.x == 0 && (wallMin.y == areaMax.y || wallMax.y == areaMin.y)) ||
		(wallMax.y - wallMin.y == 0 && (wallMin.x == areaMax.x || wallMax.x == areaMin.x)))
		return true;

	return false;
}

void
BaseBuilding::DetermineDistFromPriorityPointForWallsAndAreas_ ()
{
	UPoint3D centerOfMainRoom = rooms_[0]->GetCenterPoint ();
	centerOfMainRoom.z = groundFloorLevel_;

	for (uint32 i = 0; i < openWalls_.GetCount (); ++i)
		openWalls_[i]->distFromPriorityPoint = centerOfMainRoom.GetDistanceFast (openWalls_[i]->GetCenterPoint ());

	for (uint32 i = 0; i < openAreas_.GetCount (); ++i)
		openAreas_[i]->distFromPriorityPoint = centerOfMainRoom.GetDistanceFast (openAreas_[i]->GetCenterPoint ());
}

bool
BaseBuilding::BaseLinkRooms_ (RoomLinkerInitData& initData)
{
	RoomLinker linker = RoomLinker (initData);
	return linker.Link ();
}

void
BaseBuilding::BasePlaceWindows_ (uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance)
{
	for (uint32 i = 0; i < rooms_.GetCount (); ++i)
		rooms_[i]->PlaceWindows (heightAboveFloor, height, width, windowSpacing, placementChance);
}

void
BaseBuilding::LevelTerrain_ (uint32 maxLevelingDist, uint32 levelingStartDist)
{
	// maxLevelingDist: distance from each section that terrain will taper off completely from the ground floor level to the original terrain height
	// levelingStartDist: distance from each section that terrain will be set to the ground floor level of the building

	//const uint32 maxSectionDist = 6; default value for levelingStartDist should be 1
	// STODO: smooth entire town, further smooth building bounds, then taper smooth up to building section
	// This three pronged smoothing effect should look optimal
	// TODO smooth using scubes

	Assert (maxLevelingDist > 0);
	Assert (maxLevelingDist > levelingStartDist);
	
	for (uint32 z = min_.z - 4; z <= Min (min_.z + 30, max_.z + 15); ++z)
	{
		// STODO: in these for loops consider changing parent town to parent building. for now parent town will give better results
		for (uint32 y = min_.y; y <= max_.y; ++y)
		{
			for (uint32 x = min_.x; x <= max_.x; ++x)
			{
				int32 distFromRoom = (int32)maxLevelingDist + 1;
				for (uint32 i = 0; i < rooms_.GetCount (); ++i)
				{
					if (rooms_[i]->GetFloorLevel () == 1)
					{
						distFromRoom = Min (
											Max (
												Max (
													Max (int32(rooms_[i]->GetMin ().x) - int32(x), int32(rooms_[i]->GetMin ().y) - int32(y)), 
													Max (int32(x) - int32(rooms_[i]->GetMax ().x), int32(y) - int32(rooms_[i]->GetMax ().y))), 
												0),
											distFromRoom);
					}
				}
				if (distFromRoom <= (int32)maxLevelingDist)
				{
					uint32 smoothHeight = uint32(CosineInterpolate (
						float(groundFloorLevel_), 
						float(parentZone_->GetTerraHeightAtLoc (x, y)), 
						float(Max (distFromRoom - (int32)levelingStartDist, 0)) / float(maxLevelingDist - (int32)levelingStartDist)) + 0.5f);

					uint32 sCubeBuf = GetWorld()->GetSCube (x, y, z);

					// clear terrain with taper above ground floor level
					if (GetSCubeTerrainType (sCubeBuf) != TERRA_AIR && 
						z > smoothHeight) 
					{
						sCubeBuf = 0;
						SetSCubeTerrainType (&sCubeBuf, TERRA_AIR);
						GetWorld()->SetSCube (x, y, z, sCubeBuf);
					}
					// fill terrain with taper below ground floor level
					else if ((GetSCubeTerrainType (sCubeBuf) == TERRA_AIR || GetSCubeType (sCubeBuf) != SCUBE_CUBE) && 
						z <= smoothHeight)
					{
						sCubeBuf = 0;
						if (z == smoothHeight)
							SetSCubeTerrainType (&sCubeBuf, TERRA_GRASS); // STODO: modify this setting based on surrounding terrain and/or climate type and elevation
						else
							SetSCubeTerrainType (&sCubeBuf, TERRA_DIRT);
						GetWorld()->SetSCube (x, y, z, sCubeBuf);
					}

					if (GetSCubeTerrainType (sCubeBuf) == TERRA_DIRT && z == smoothHeight)
					{
						sCubeBuf = 0;
						SetSCubeTerrainType (&sCubeBuf, TERRA_GRASS); // STODO: modify this setting based on surrounding terrain and/or climate type and elevation
						GetWorld()->SetSCube (x, y, z, sCubeBuf);
					}
				}
			}
		}
	}
}

bool
BaseBuilding::IsRoomPlaceable_ (const UPoint3D& roomMin, const UPoint3D& roomMax, bool doChangeBuildingBoundsIfTrue)
{
	bool successful = true;
	bool buildingExtendCheck = false;
	int32 xOffset = 0;
	int32 yOffset = 0;

	// these values may need to be tweaked if we throw in some really large rooms.
	Assert (roomMin.x > min_.x - 40);
	Assert (roomMin.x < max_.x + 20);
	Assert (roomMin.y > min_.y - 40);
	Assert (roomMin.y < max_.y + 20);
	Assert (roomMax.x < max_.x + 40);
	Assert (roomMax.x > min_.x - 20);
	Assert (roomMax.y < max_.y + 40);
	Assert (roomMax.y > min_.y - 20);

	// check if room mins and maxs exceeds the bounds of the building. If so, extend building bounds.
	// these checks are offset by two to provide extra padding room between room and building bounds
	if (roomMin.x < min_.x + minRoomAndBuildingBoundSeparation_)
	{
		xOffset = int32(roomMin.x - minRoomAndBuildingBoundSeparation_) - (int32)min_.x; // negative offset
		min_.x += xOffset;
		buildingExtendCheck = true;
	}
	if (roomMax.x > max_.x - minRoomAndBuildingBoundSeparation_)
	{
		xOffset = (roomMax.x + minRoomAndBuildingBoundSeparation_) - max_.x; // positive offset
		max_.x += xOffset;
		buildingExtendCheck = true;
	}
	if (roomMin.y < min_.y + minRoomAndBuildingBoundSeparation_)
	{
		yOffset = int32(roomMin.y - minRoomAndBuildingBoundSeparation_) - (int32)min_.y; // negative offset
		min_.y += yOffset;
		buildingExtendCheck = true;
	}
	if (roomMax.y > max_.y - minRoomAndBuildingBoundSeparation_)
	{
		yOffset = (roomMax.y + minRoomAndBuildingBoundSeparation_) - max_.y; // positive offset
		max_.y += yOffset;
		buildingExtendCheck = true;
	}

	// check if new building extents intersects with any existing buildings, hazards, roads, or exceeds zone bounds
	if (buildingExtendCheck)
	{
		if( !parentZone_->IsBuildingBoundsPlaceable( min_, max_ ) )
			successful = false;
	}

	// check if room bounds interesects with any existing rooms.
	if (successful)
		successful = !IntersectAreaWithRoomsFast (roomMin, roomMax, true);

	// make sure room will not block a front door
	if (successful)
		successful = !DoesRoomBlockFrontDoor_ (roomMin, roomMax);

	// catch any rooms placed that are not touching any other rooms on borders
	if (successful)
		successful = rooms_.GetCount () == 0 || IntersectAreaWithRoomsFast (roomMin, roomMax, false);

	if (!successful || !doChangeBuildingBoundsIfTrue) // room cannot be placed
	{
		// reset building bounds to previous values
		if (xOffset < 0)
			min_.x -= xOffset;
		else
			max_.x -= xOffset;
		if (yOffset < 0)
			min_.y -= yOffset;
		else
			max_.y -= yOffset;		
	}

	if (!successful)
		return false; // room cannot be added

	Assert (roomMin.x >= min_.x + minRoomAndBuildingBoundSeparation_);
	Assert (roomMin.y >= min_.y + minRoomAndBuildingBoundSeparation_);
	Assert (roomMax.x <= max_.x - minRoomAndBuildingBoundSeparation_);
	Assert (roomMax.y <= max_.y - minRoomAndBuildingBoundSeparation_);

	return true; // section can be added
}

bool
BaseBuilding::DoesRoomBlockFrontDoor_ (const UPoint3D& roomMin, const UPoint3D& roomMax) const
{
	if (rooms_.GetCount () == 0)
		return false;

	BaseRoom* mainRoom = rooms_[0];

	Assert (mainRoom->GetFloorLevel () == 1);
	Assert (mainRoom->GetIsMainRoom () == 1);

	if (roomMin == mainRoom->GetMin ())
		return false;

	switch (doorSide_)
	{
	case DIRECTION_SOUTH:
		if (roomMax.y <= mainRoom->GetMin ().y && (roomMax.x > mainRoom->GetMin ().x && roomMin.x < mainRoom->GetMax ().x))
			return true;
		break;
	case DIRECTION_NORTH:
		if (roomMin.y >= mainRoom->GetMax ().y && (roomMax.x > mainRoom->GetMin ().x && roomMin.x < mainRoom->GetMax ().x))
			return true;
		break;
	case DIRECTION_WEST:
		if (roomMax.x <= mainRoom->GetMin ().x && (roomMax.y > mainRoom->GetMin ().y && roomMin.y < mainRoom->GetMax ().y))
			return true;
		break;
	case DIRECTION_EAST:
		if (roomMin.x >= mainRoom->GetMax ().x && (roomMax.y > mainRoom->GetMin ().y && roomMin.y < mainRoom->GetMax ().y))
			return true;
		break;
	}

	return false;
}