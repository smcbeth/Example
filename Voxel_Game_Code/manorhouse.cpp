#include "manorhouse.h"

ManorHouse::ManorHouse (BaseZone* zone, BuildingSize size, uint32 numResidents)
	: BaseBuilding (zone, size, numResidents)
{
	numBasements_ = 0;
	ceilingHeight_ = 5;
	stairWidth_ = 2;
	minHallwayWidth_ = 5;
	maxHallwayWidth_ = 5;
	minRoomAndBuildingBoundSeparation_ = 9;
	maxRoomAndBuildingBoundSeparation_ = 12;

	innerWallMaterial_ = TERRA_STONEBRICK;
	outerWallMaterial_ = TERRA_STONEBRICK;
	floorMaterial_ = TERRA_BOARDS;
	roofMaterial_ = TERRA_THATCH;
	stairMaterial_ = TERRA_BOARDS;
	roofType_ = ROOF_NORMAL;
	roofFillType_ = ROOFFILL_VAULTED;
	maxRoofHeight_ = 3 + rand () % 2;
}

ManorHouse::~ManorHouse ()
{
	;
}

void
ManorHouse::Build ()
{
	const uint32 maxLevelingDist = 10;
	const uint32 levelingStartDist = 1;
	const uint32 foundationThickness = 2;
	BaseBuild_ (maxLevelingDist, levelingStartDist, foundationThickness);
}

bool
ManorHouse::PlaceRooms (Direction doorSide)
{
	Assert (rooms_.GetCount () == 0);

	const uint32 maxAllowableFloors = 3;

	float32 doAttemptToPreserveBoxShapeOnFloorOneChance = FRandom (0.60f, 0.70f);
	float32 chanceOfSkippingPlacementSpaceOnFloorOne = FRandom (0.30f, 0.40f);
	float32 chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne = FRandom (0.20f, 0.30f);

	uint32 maxFloorRooms;
	uint32 numRooms;
	
	doorSide_ = doorSide;
	
	switch (size_)
	{
	case BUILDING_SMALL:
		maxFloorRooms = 5;
		numRooms = 8;
		break;
	case BUILDING_MEDIUM:
		maxFloorRooms = 6;
		numRooms = 10;
		break;
	case BUILDING_LARGE:
		maxFloorRooms = 7;
		numRooms = 12;
		break;
	}

	uint32 maxFloorRoomsOffset = 0;
	uint32 floorLevel = 1;
	uint32 roomsPlacedPerFloor = 0;
	uint32 numRoomsPlaced = 0;
	uint32 numBedroomsPlaced = 0;
	bool doIncreaseFloorLevel = false;
	bool isStairHallPlacedOnFloor = false; // STODO: in the future, make number of stairhalls on floor depend on the number of stairs allowed per floor
	
	BaseRoom* room;

	bool doPlaceStairHall = false;
	bool doPlaceHallway = true;
	bool doPlaceKitchen = true;
	bool doPlaceOffice = true;
	bool doPlaceTreasuryRoom = true;
	bool doPlaceLibrary = true;

	while (numRoomsPlaced < numRooms)
	{
		// STODO: refactor everything in this loop as much as possible. this code will become the basis for the most complex to design buildings in the game (castles)

		RoomInitData initData;

		if (floorLevel == 1 && roomsPlacedPerFloor == maxFloorRooms + maxFloorRoomsOffset || floorLevel > 1 && roomsPlacedPerFloor == 0)
			doPlaceStairHall = true;
		else if (roomsPlacedPerFloor == 1)
			doPlaceHallway = true;

		// STODO: improve the determination of polymorphic room type
		Assert (numRooms >= 7);
		if (numRoomsPlaced == 0)
		{
			SetGreatHallInitData_ (initData, floorLevel);
			room = new GreatHall (initData);
		}
		else if (doPlaceStairHall)
		{
			SetStairHallInitData_ (initData, floorLevel);
			room = new StairHall (initData);
		}
		else if (doPlaceHallway)
		{
			bool doAllowLongHallway = numRoomsPlaced < numRooms - 2 && roomsPlacedPerFloor < maxFloorRooms + maxFloorRoomsOffset - 2;
			SetHallwayInitData_ (initData, floorLevel, doAllowLongHallway);
			room = new Hallway (initData);
		}
		else if (doPlaceKitchen)
		{
			SetKitchenInitData_ (initData, floorLevel);
			room = new Kitchen (initData);
		}
		else if (doPlaceOffice)
		{
			SetOfficeInitData_ (initData, floorLevel);
			room = new Office (initData);
		}
		else if (doPlaceTreasuryRoom)
		{
			SetTreasuryRoomInitData_ (initData, floorLevel);
			room = new TreasuryRoom (initData);
		}
		else if (doPlaceLibrary)
		{
			SetLibraryInitData_ (initData, floorLevel);
			room = new Library (initData);
		}
		else
		{
			SetBedroomInitData_ (initData, floorLevel);
			room = new Bedroom (initData);
		}

		bool doPlaceFirstRoomOnNewFloor = roomsPlacedPerFloor == 0 && numRoomsPlaced > 0;
		if (PlaceRoom_ (room, false, !doPlaceFirstRoomOnNewFloor, doPlaceFirstRoomOnNewFloor, doAttemptToPreserveBoxShapeOnFloorOneChance, chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne, chanceOfSkippingPlacementSpaceOnFloorOne))
		{
			Assert (floorLevel == 1 || roomsPlacedPerFloor == 0 || room->GetType () != ROOM_STAIRHALL);

			if (room->GetType () == ROOM_STAIRHALL)
			{
				//Assert (floorLevel > 1 || roomsPlacedPerFloor > 4);

				doPlaceStairHall = false;
				isStairHallPlacedOnFloor = true;

				if (floorLevel == 1)
					doIncreaseFloorLevel = true;

				++numRooms; 
				++maxFloorRoomsOffset; // stairhalls should not count toward the number of rooms needed to be placed
			}
			else if (room->GetType () == ROOM_HALLWAY)
			{
				doPlaceHallway = false;
				++numRooms; 
				++maxFloorRoomsOffset; // hallways should not count toward the number of rooms needed to be placed
			}
			else if (room->GetType () == ROOM_KITCHEN)
			{
				doPlaceKitchen = false;
			}
			else if (room->GetType () == ROOM_OFFICE)
			{
				doPlaceOffice = false;
			}
			else if (room->GetType () == ROOM_TREASURYROOM)
			{
				doPlaceTreasuryRoom = false;
			}
			else if (room->GetType () == ROOM_LIBRARY)
			{
				doPlaceLibrary = false;
			}
			else if (room->GetType () == ROOM_BEDROOM)
			{
				++numBedroomsPlaced;
			}

			++numRoomsPlaced;
			++roomsPlacedPerFloor;
		}
		else
		{
			RoomType type = room->GetType ();
			delete room;

			if (type == ROOM_STAIRHALL)
			{
				return false;
			}
			else if (roomsPlacedPerFloor > 0 && type != ROOM_HALLWAY && numRooms - numRoomsPlaced > 1 && (floorLevel != 1 || roomsPlacedPerFloor < maxFloorRooms + maxFloorRoomsOffset - 1))
			{
				doPlaceHallway = true; // attempt to place a hallway in order to place additional rooms that connect to said hallway
			}
			else if (roomsPlacedPerFloor >= 3) // allows for stairhall, hallway and another room on every floor
			{
				if (isStairHallPlacedOnFloor)
					doIncreaseFloorLevel = true;
				else
					doPlaceStairHall = true;
			}
			else
			{
				return false;
			}
		}

		if ((floorLevel > 1 && roomsPlacedPerFloor == maxFloorRooms + maxFloorRoomsOffset) || doIncreaseFloorLevel)
		{
			Assert (isStairHallPlacedOnFloor);

			++floorLevel;
			maxFloorRoomsOffset = 0;
			roomsPlacedPerFloor = 0;
			doIncreaseFloorLevel = false;
			isStairHallPlacedOnFloor = false;

			if (floorLevel > maxAllowableFloors)
				return false;
		}
	}

	if (numBedroomsPlaced == 0)
		return false;
	
	FinalizeRoomPlacement_ ();
	if (!LinkRooms_ ())
		return false;
	PlaceWindows_ ();
	return true;
}

BuildingType
ManorHouse::GetType () const
{
	return BUILDING_MANORHOUSE;
}

bool
ManorHouse::LinkRooms_ ()
{
	RoomLinkerInitData initData;
	initData.building = this;
	initData.maxStairsPerFloor = 1;
	initData.maxParentRooms = 2;
	initData.maxChildRooms = 3;
	initData.doorPlaceChance = 0.4f;
	initData.stairPlaceChance = 1.0f;
	initData.spiralStairTypeChance = 0.15f;
	return BaseLinkRooms_ (initData);
}

void
ManorHouse::PlaceWindows_ ()
{
	const uint32 heightAboveFloor = 2;
	const uint32 height = 2;
	const uint32 width = 1;
	const uint32 windowSpacing = 2;
	const float32 placementChance = 0.38f;
	BasePlaceWindows_ (heightAboveFloor, height, width, windowSpacing, placementChance);
}

void
ManorHouse::SetBedroomInitData_ (RoomInitData& initData, uint32 floorLevel)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 7;
	initData.maxLength = 10;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = false;
	initData.canContainStairs = false;
}

void
ManorHouse::SetGreatHallInitData_ (RoomInitData& initData, uint32 floorLevel)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 13; // STODO: make the room approximately 1.5-2 times as long as it is wide
	initData.maxLength = 16;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1; // STODO: make this 2
	initData.isMainRoom = true;
	initData.canContainStairs = false;
	initData.linkableChildRoomTypes.InsertBack (ROOM_HALLWAY);
	initData.linkableChildRoomTypes.InsertBack (ROOM_STAIRHALL);
}

void
ManorHouse::SetHallwayInitData_ (RoomInitData& initData, uint32 floorLevel, bool doAllowLongHallway)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 8;
	if (doAllowLongHallway)
		initData.maxLength = 20;
	else
		initData.maxLength = 12;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = false;
	initData.canContainStairs = false;
	initData.linkableChildRoomTypes.InsertBack (ROOM_BEDROOM);
	initData.linkableChildRoomTypes.InsertBack (ROOM_GREATHALL);
	initData.linkableChildRoomTypes.InsertBack (ROOM_HALLWAY);
	initData.linkableChildRoomTypes.InsertBack (ROOM_KITCHEN);
	initData.linkableChildRoomTypes.InsertBack (ROOM_LIBRARY);
	initData.linkableChildRoomTypes.InsertBack (ROOM_OFFICE);
	initData.linkableChildRoomTypes.InsertBack (ROOM_STAIRHALL);
	initData.linkableChildRoomTypes.InsertBack (ROOM_TREASURYROOM);
}

void
ManorHouse::SetKitchenInitData_ (RoomInitData& initData, uint32 floorLevel)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 8;
	initData.maxLength = 10;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = false;
	initData.canContainStairs = false;
}

void
ManorHouse::SetLibraryInitData_ (RoomInitData& initData, uint32 floorLevel)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 9;
	initData.maxLength = 15;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = false;
	initData.canContainStairs = false;
	initData.linkableChildRoomTypes.InsertBack (ROOM_OFFICE);
}

void
ManorHouse::SetOfficeInitData_ (RoomInitData& initData, uint32 floorLevel)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 8;
	initData.maxLength = 11;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = false;
	initData.canContainStairs = false;
}

void
ManorHouse::SetStairHallInitData_ (RoomInitData& initData, uint32 floorLevel)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 11;
	initData.maxLength = 12;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = false;
	initData.canContainStairs = true;
	initData.linkableChildRoomTypes.InsertBack (ROOM_HALLWAY); // Note- this is acceptable since hallways are small enough for a stairhall to never be placed over one. 
																// STODO: add functionality for the first room placed on the floor that checks if the rooms below have stair placement capability
	initData.linkableChildRoomTypes.InsertBack (ROOM_STAIRHALL);
}

void
ManorHouse::SetTreasuryRoomInitData_ (RoomInitData& initData, uint32 floorLevel)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 9;
	initData.maxLength = 12;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = false;
	initData.canContainStairs = false;
}