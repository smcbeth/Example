#include "dungeontower.h"

DungeonTower::DungeonTower (BaseZone* zone, BuildingSize size, uint32 numResidents)
	: BaseBuilding (zone, size, numResidents)
{
	numBasements_ = 0;
	ceilingHeight_ = 8;
	stairWidth_ = 3;
	minHallwayWidth_ = 6;
	maxHallwayWidth_ = 6;
	minRoomAndBuildingBoundSeparation_ = 13;
	maxRoomAndBuildingBoundSeparation_ = 15;

	innerWallMaterial_ = TERRA_STONEBRICK;
	outerWallMaterial_ = TERRA_STONEBRICK;
	floorMaterial_ = TERRA_STONE;
	roofMaterial_ = TERRA_STONEBRICK;
	stairMaterial_ = TERRA_STONEBRICK;
	roofType_ = ROOF_OPENPARAPETS;
	roofFillType_ = ROOFFILL_SOLID;
	maxRoofHeight_ = 1;
}

DungeonTower::~DungeonTower ()
{
	;
}

void
DungeonTower::Build ()
{
	const uint32 maxLevelingDist = 13;
	const uint32 levelingStartDist = 1;
	const uint32 foundationThickness = 3;
	BaseBuild_ (maxLevelingDist, levelingStartDist, foundationThickness);
}

bool
DungeonTower::PlaceRooms (Direction doorSide)
{
	Assert (rooms_.GetCount () == 0);

	const float32 reduceNumRoomsPerFloorChance = 1.00f;
	const uint32 numRoomsPerFloorReduceAmount = 1;
	const uint32 maxFailuresOfRandomRoomPlacement = 15;

	float32 doAttemptToPreserveBoxShapeOnFloorOneChance = FRandom (0.60f, 0.80f);
	float32 chanceOfSkippingPlacementSpaceOnFloorOne = FRandom (0.55f, 0.70f);
	float32 chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne = FRandom (0.20f, 0.30f);
	
	uint32 numFloorOneRooms;
	uint32 maxNumFloorTwoRooms;
	uint32 numRooms;

	doorSide_ = doorSide;

	switch (size_)
	{
	case BUILDING_SMALL:
		numFloorOneRooms = 4;
		maxNumFloorTwoRooms = numFloorOneRooms + 1;
		numRooms = 15 + rand () % 5;
		break;
	case BUILDING_MEDIUM:
		numFloorOneRooms = 6;
		maxNumFloorTwoRooms = numFloorOneRooms + 2;
		numRooms = 25 + rand () % 10;
		break;
	case BUILDING_LARGE:
		numFloorOneRooms = 8;
		maxNumFloorTwoRooms = numFloorOneRooms + 3;
		numRooms = 40 + rand () % 15;
		break;
	}

	Vector<uint32> numRoomsPlacedPerFloors;
	numRoomsPlacedPerFloors.InsertBack (0);

	uint32 floorLevel = 1;
	uint32 numRoomsPlaced = 0;
	uint32 maxNumRoomsOnFloor = numFloorOneRooms;
	bool doIncreaseFloorLevel = false;
	bool doPlaceOnRandomFloor = false;
	uint32 failureCount = 0;
	
	BaseRoom* room;

	// STODO: extract a method(s) from below code and put it in BaseBuilding, since much of this code is bound to be repeated. this will also allow
		// for better polymorphic control since any added method can take in useful parameters.
	while (numRoomsPlaced < numRooms)
	{
		RoomInitData initData;

		// in this case the original maxNumRoomsOnFloor will be ignored to allow numRoomsPlaced == numRooms to be possible
		if (doPlaceOnRandomFloor)
		{
			if (failureCount == maxFailuresOfRandomRoomPlacement - 1)
				floorLevel = 1; // increases likleyhood of not failing
			else
				floorLevel = (rand () % numFloors_) + 1;
		}

		// STODO: make sure treasure rooms are placed suitably
		// place a treasure room as the last room placed on every floor and as the last room placed in the building
		if (failureCount <= maxFailuresOfRandomRoomPlacement / 2 && 
			((numRoomsPlaced > 1 && numRoomsPlacedPerFloors[floorLevel - 1] == maxNumRoomsOnFloor - 1) || numRoomsPlaced == numRooms - 1))
		{
			SetTreasureRoomInitData_ (initData, floorLevel);
			room = new TreasureRoom (initData);
		}
		else if (!doPlaceOnRandomFloor && 
			numRoomsPlacedPerFloors[floorLevel - 1] < maxNumRoomsOnFloor &&
			((numRoomsPlacedPerFloors[floorLevel - 1] == 1 && rand () % 2 && maxNumRoomsOnFloor > 4) ||
				(numRoomsPlacedPerFloors[floorLevel - 1] == 3 && rand () % 4 && maxNumRoomsOnFloor > 6) ||
				(numRoomsPlacedPerFloors[floorLevel - 1] == 5 && rand () % 6 && maxNumRoomsOnFloor > 8))) // STODO: refine this and make it more configurable
		{
			SetHallwayInitData_ (initData, floorLevel);
			room = new Hallway (initData);
		}
		else
		{
			SetDungeonRoomInitData_ (initData, floorLevel, rooms_.GetCount () == 0);
			room = new DungeonRoom (initData);
		}
		bool doPlaceFirstRoomOnNewFloor = numRoomsPlacedPerFloors[floorLevel - 1] == 0 && numRoomsPlaced > 0;
		if (PlaceRoom_ (room, true, !doPlaceFirstRoomOnNewFloor, doPlaceFirstRoomOnNewFloor, doAttemptToPreserveBoxShapeOnFloorOneChance, chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne, chanceOfSkippingPlacementSpaceOnFloorOne))
		{
			++numRoomsPlaced;
			++numRoomsPlacedPerFloors[floorLevel - 1];
			failureCount = 0;
		}
		else
		{
			delete room;

			++failureCount;
			if (failureCount == maxFailuresOfRandomRoomPlacement)
			{
				rooms_.RemoveAndDeleteAll ();
				return false;
			}

			if (numRoomsPlacedPerFloors[floorLevel - 1] >= 1)
			{
				doIncreaseFloorLevel = true;
			}
			else if (!doPlaceOnRandomFloor)
			{
				rooms_.RemoveAndDeleteAll ();
				return false;
			}
	
			continue;
		}

		if (doPlaceOnRandomFloor)
			continue;

		if (numRoomsPlacedPerFloors[floorLevel - 1] == maxNumRoomsOnFloor || doIncreaseFloorLevel)
		{
			++floorLevel;
			numRoomsPlacedPerFloors.InsertBack (0);
			doIncreaseFloorLevel = false;

			maxNumRoomsOnFloor = GetMaxNumRoomsOnFloor_ (floorLevel, maxNumRoomsOnFloor, 
				maxNumFloorTwoRooms, numRoomsPerFloorReduceAmount, reduceNumRoomsPerFloorChance);
		}

		if (maxNumRoomsOnFloor == 1 && rooms_.GetCount () > 0)
			doPlaceOnRandomFloor = true;
	}

	FinalizeRoomPlacement_ ();
	if (!LinkRooms_ ())
		return false;
	PlaceWindows_ ();
	return true;
}

BuildingType
DungeonTower::GetType () const
{
	return BUILDING_DUNGEONTOWER;
}

void
DungeonTower::FinalizeRoomPlacement_ ()
{
	// add rooftop rooms if roofType_ allows it using openArea_ bounds,
	// add areas that can later be used for building roofs

	// generate open areas for all floors above floor 1. PERFORMANCE: this may be slow for large buildings, since the entire building is scanned
	UpdateRoomPlacementSpaces_ (0, numFloors_, false, false, true, true);

	if (roofType_ == ROOF_OPEN || roofType_ == ROOF_OPENPARAPETS)
	{
		// place rooftop rooms using open area bounds
		for (int32 i = 0; i < (int32)openAreas_.GetCount (); ++i)
		{
			if (openAreas_[i]->GetLengthX () < minHallwayWidth_ || openAreas_[i]->GetLengthY () < minHallwayWidth_)
				continue;

			// create rooftop room
			uint32 floorLevel = (openAreas_[i]->GetMin ().z - groundFloorLevel_) / ceilingHeight_ + 1;
			if (floorLevel == 1)
				continue;

			RoomInitData initData;
			SetRoofTopInitData_ (initData, floorLevel, 
				Min (openAreas_[i]->GetLengthX (), openAreas_[i]->GetLengthY ()), 
				Max (openAreas_[i]->GetLengthX (), openAreas_[i]->GetLengthY ()));

			BaseRoom* room = new RoofTop (initData);

			room->SetMin (openAreas_[i]->GetMin ());
			room->SetMax (openAreas_[i]->GetMax ());

			AddRoomToBuilding_ (room);

			// remove area so it isnt later used in roof building
			openAreas_.RemoveAndDelete (i);
			--i;
		}
	}

	FinalizeBuildingBounds_ (false);
}

bool
DungeonTower::LinkRooms_ ()
{
	RoomLinkerInitData initData;
	initData.building = this;
	initData.maxStairsPerFloor = 1;
	initData.maxParentRooms = 2;
	initData.maxChildRooms = 3;
	initData.doorPlaceChance = 0.4f;
	initData.stairPlaceChance = 0.25f;
	initData.spiralStairTypeChance = 0.33f;
	return BaseLinkRooms_ (initData);
}

void
DungeonTower::PlaceWindows_ ()
{
	const uint32 heightAboveFloor = 4;
	const uint32 height = 2;
	const uint32 width = 1;
	const uint32 windowSpacing = 4;
	const float32 placementChance = 0.07f;
	BasePlaceWindows_ (heightAboveFloor, height, width, windowSpacing, placementChance);
}

void
DungeonTower::SetDungeonRoomInitData_ (RoomInitData& initData, uint32 floorLevel, bool isMainRoom)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 14;
	initData.maxLength = 26;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = isMainRoom;
	initData.canContainStairs = true;
	initData.linkableChildRoomTypes.InsertBack (ROOM_DUNGEONROOM);
	initData.linkableChildRoomTypes.InsertBack (ROOM_HALLWAY);
	initData.linkableChildRoomTypes.InsertBack (ROOM_ROOFTOP);
	initData.linkableChildRoomTypes.InsertBack (ROOM_TREASUREROOM);

	if (isMainRoom)
		initData.minLength = 14;
}

void
DungeonTower::SetHallwayInitData_ (RoomInitData& initData, uint32 floorLevel)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 8;
	initData.maxLength = 28;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = false;
	initData.canContainStairs = false;
	initData.linkableChildRoomTypes.InsertBack (ROOM_DUNGEONROOM);
	initData.linkableChildRoomTypes.InsertBack (ROOM_HALLWAY);
	initData.linkableChildRoomTypes.InsertBack (ROOM_ROOFTOP);
	initData.linkableChildRoomTypes.InsertBack (ROOM_TREASUREROOM);
}

void
DungeonTower::SetRoofTopInitData_ (RoomInitData& initData, uint32 floorLevel, uint32 minLength, uint32 maxLength)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = minLength;
	initData.maxLength = maxLength;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = false;
	initData.canContainStairs = true;
	initData.linkableChildRoomTypes.InsertBack (ROOM_DUNGEONROOM);
	initData.linkableChildRoomTypes.InsertBack (ROOM_HALLWAY);
	initData.linkableChildRoomTypes.InsertBack (ROOM_ROOFTOP);
	initData.linkableChildRoomTypes.InsertBack (ROOM_TREASUREROOM);
}

void
DungeonTower::SetTreasureRoomInitData_ (RoomInitData& initData, uint32 floorLevel)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 14;
	initData.maxLength = 22;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = false;
	initData.canContainStairs = false;
}