#include "house.h"

House::House (BaseZone* zone, BuildingSize size, uint32 numResidents)
	: BaseBuilding (zone, size, numResidents)
{
	numBasements_ = 0;
	ceilingHeight_ = 4;
	stairWidth_ = 1;
	minHallwayWidth_ = 4;
	maxHallwayWidth_ = 4;
	minRoomAndBuildingBoundSeparation_ = 4;
	maxRoomAndBuildingBoundSeparation_ = 8;

	floorMaterial_ = TERRA_BOARDS;
	roofMaterial_ = TERRA_THATCH;
	stairMaterial_ = TERRA_BOARDS;
	roofType_ = ROOF_NORMAL;
	
	if (size == BUILDING_SMALL)
	{
		// these values reflect poorer homes
		outerWallMaterial_ = TERRA_BOARDS;
		maxRoofHeight_ = 2;
		roofFillType_ = ROOFFILL_SOLID;
	}
	else if (size == BUILDING_MEDIUM)
	{
		if (rand () % 2)
			outerWallMaterial_ = TERRA_BOARDS;
		else
			outerWallMaterial_ = TERRA_BRICK;
		maxRoofHeight_ = 2 + rand () % 3;
		roofFillType_ = ROOFFILL_ATTIC;
	}
	else
	{
		// these values reflect richer homes
		outerWallMaterial_ = TERRA_BRICK;
		maxRoofHeight_ = 3 + rand () % 3;
		roofFillType_ = ROOFFILL_ATTIC;
	}
	innerWallMaterial_ = outerWallMaterial_;
}

House::~House ()
{
	;
}

void
House::Build ()
{
	const uint32 maxLevelingDist = 6;
	const uint32 levelingStartDist = 1;
	const uint32 foundationThickness = 1;
	BaseBuild_ (maxLevelingDist, levelingStartDist, foundationThickness);
}

bool
House::PlaceRooms (Direction doorSide)
{
	Assert (rooms_.GetCount () == 0);

	const uint32 maxAllowableFloors = 2;

	float32 doAttemptToPreserveBoxShapeOnFloorOneChance = FRandom (0.80f, 0.90f);
	float32 chanceOfSkippingPlacementSpaceOnFloorOne = FRandom (0.00f, 0.10f);
	float32 chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne = FRandom (0.20f, 0.30f);

	uint32 numFloorOneRooms;
	uint32 numRooms;
	
	doorSide_ = doorSide;
	
	switch (size_)
	{
	case BUILDING_SMALL:
		numFloorOneRooms = 1;
		numRooms = 1;
		break;
	case BUILDING_MEDIUM:
		numFloorOneRooms = 2;
		numRooms = 2;
		break;
	case BUILDING_LARGE:
		numFloorOneRooms = 2 + rand () % 3;
		numRooms = 3 + rand () % 2;
		break;
	}

	BaseRoom* room;
	uint32 floorLevel = 1;
	uint32 roomsPlacedPerFloor = 0;
	uint32 numRoomsPlaced = 0;
	bool doIncreaseFloorLevel = false;

	while (numRoomsPlaced < numRooms)
	{
		RoomInitData initData;

		// STODO: improve room type determining factors
		if (numRoomsPlaced == 0)
		{
			if (numRooms >= 2)
			{
				SetLivingRoomInitData_ (initData, floorLevel, true);
				room = new LivingRoom (initData);
			}
			else
			{
				SetBedroomInitData_ (initData, floorLevel, true);
				room = new Bedroom (initData);
			}
		}
		else if (numRoomsPlaced == 1 && numRooms >= 3)
		{
			SetKitchenInitData_ (initData, floorLevel);
			room = new Kitchen (initData);
		}
		else
		{
			SetBedroomInitData_ (initData, floorLevel, false);
			room = new Bedroom (initData);
		}
		bool doPlaceFirstRoomOnNewFloor = roomsPlacedPerFloor == 0 && numRoomsPlaced > 0;
		if (PlaceRoom_ (room, true, !doPlaceFirstRoomOnNewFloor, doPlaceFirstRoomOnNewFloor, doAttemptToPreserveBoxShapeOnFloorOneChance, chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne, chanceOfSkippingPlacementSpaceOnFloorOne))
		{
			++numRoomsPlaced;
			++roomsPlacedPerFloor;
		}
		else
		{
			delete room;

			if (roomsPlacedPerFloor >= 1)
				doIncreaseFloorLevel = true;
			else
				return false;
		}

		if (roomsPlacedPerFloor == numFloorOneRooms || doIncreaseFloorLevel)
		{
			++floorLevel;
			roomsPlacedPerFloor = 0;
			doIncreaseFloorLevel = false;

			if (floorLevel > maxAllowableFloors)
				return false;
		}
	}

	FinalizeRoomPlacement_ ();
	if (!LinkRooms_ ())
		return false;
	PlaceWindows_ ();
	return true;
}

BuildingType
House::GetType () const
{
	return BUILDING_HOUSE;
}

bool
House::LinkRooms_ ()
{
	RoomLinkerInitData initData;
	initData.building = this;
	initData.maxStairsPerFloor = 1;
	initData.maxParentRooms = 2;
	initData.maxChildRooms = 3;
	initData.doorPlaceChance = 0.5f;
	initData.stairPlaceChance = 1.0f;
	initData.spiralStairTypeChance = 0.13f;
	return BaseLinkRooms_ (initData);
}

void
House::PlaceWindows_ ()
{
	const uint32 heightAboveFloor = 2;
	const uint32 height = 1;
	const uint32 width = 1;
	const uint32 windowSpacing = 2;
	const float32 placementChance = 0.35f;
	BasePlaceWindows_ (heightAboveFloor, height, width, windowSpacing, placementChance);
}

void
House::SetBedroomInitData_ (RoomInitData& initData, uint32 floorLevel, bool isMainRoom)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 6;
	initData.maxLength = 10;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = isMainRoom;
	initData.canContainStairs = true; // STODO: change this to false when houses have an additional room that can handle stairs on the second floor

	if (isMainRoom)
		initData.minLength = 8;
}

void
House::SetKitchenInitData_ (RoomInitData& initData, uint32 floorLevel)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 6;
	initData.maxLength = 9;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = false;
	initData.canContainStairs = false;
}

void
House::SetLivingRoomInitData_ (RoomInitData& initData, uint32 floorLevel, bool isMainRoom)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 7;
	initData.maxLength = 10;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = isMainRoom;
	initData.canContainStairs = true;
	initData.linkableChildRoomTypes.InsertBack (ROOM_BEDROOM);
	initData.linkableChildRoomTypes.InsertBack (ROOM_KITCHEN);

	if (isMainRoom)
		initData.minLength = 9;
}