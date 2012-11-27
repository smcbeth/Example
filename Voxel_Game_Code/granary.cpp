#include "granary.h"

Granary::Granary (BaseZone* zone, BuildingSize size, uint32 numResidents)
	: BaseBuilding (zone, size, numResidents)
{
	numBasements_ = 0; // STODO: add basements to some buildings.
	ceilingHeight_ = 4; // STODO: ceiling height should vary in some rooms.
	stairWidth_ = 1;
	minHallwayWidth_ = 4;
	maxHallwayWidth_ = 4;
	minRoomAndBuildingBoundSeparation_ = 5;
	maxRoomAndBuildingBoundSeparation_ = 10;

	innerWallMaterial_ = TERRA_BOARDS;
	outerWallMaterial_ = TERRA_STONEBRICK;
	floorMaterial_ = TERRA_BOARDS;
	roofMaterial_ = TERRA_THATCH;
	stairMaterial_ = TERRA_BOARDS;
	roofType_ = ROOF_NORMAL;
	roofFillType_ = ROOFFILL_VAULTED;
	maxRoofHeight_ = 5;
}

Granary::~Granary ()
{
	;
}

void
Granary::Build ()
{
	const uint32 maxLevelingDist = 7;
	const uint32 levelingStartDist = 1;
	const uint32 foundationThickness = 1;
	BaseBuild_ (maxLevelingDist, levelingStartDist, foundationThickness);
}

bool
Granary::PlaceRooms (Direction doorSide)
{
	Assert (rooms_.GetCount () == 0);

	const uint32 floorLevel = 1;

	float32 doAttemptToPreserveBoxShapeOnFloorOneChance = 1.00f;
	float32 chanceOfSkippingPlacementSpaceOnFloorOne = 0.00f;
	float32 chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne = 0.00f;
	bool doAttemptToPreserveBoxShapeOnFloorOne = true;

	doorSide_ = doorSide;

	RoomInitData initData;
	SetStorageRoomInitData_ (initData, floorLevel);
	BaseRoom* room = new StorageRoom (initData);

	if (!PlaceRoom_ (room, true, true, false, doAttemptToPreserveBoxShapeOnFloorOneChance, chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne, chanceOfSkippingPlacementSpaceOnFloorOne))
	{
		delete room;
		return false;
	}

	FinalizeRoomPlacement_ ();
	if (!LinkRooms_ ())
		return false;
	PlaceWindows_ ();
	return true;
}

BuildingType
Granary::GetType () const
{
	return BUILDING_GRANARY;
}

bool
Granary::LinkRooms_ ()
{
	RoomLinkerInitData initData;
	initData.building = this;
	initData.maxStairsPerFloor = 1;
	initData.maxParentRooms = 2;
	initData.maxChildRooms = 3;
	initData.doorPlaceChance = 0.4f;
	initData.stairPlaceChance = 1.0f;
	initData.spiralStairTypeChance = 0.33f;
	return BaseLinkRooms_ (initData);
}

void
Granary::PlaceWindows_ ()
{
	const uint32 heightAboveFloor = 2;
	const uint32 height = 1;
	const uint32 width = 1;
	const uint32 windowSpacing = 3;
	const float32 placementChance = 0.15f;
	BasePlaceWindows_ (heightAboveFloor, height, width, windowSpacing, placementChance);
}

void
Granary::SetStorageRoomInitData_ (RoomInitData& initData, uint32 floorLevel)
{
	Assert (initData.linkableChildRoomTypes.GetCount () == 0);

	initData.zone = parentZone_;
	initData.building = this;
	initData.minLength = 13;
	initData.maxLength = 15;
	initData.floorLevel = floorLevel;
	initData.numFloorsSpanned = 1;
	initData.isMainRoom = true;
	initData.canContainStairs = false;
	initData.linkableChildRoomTypes.InsertBack (ROOM_STORAGEROOM);
}