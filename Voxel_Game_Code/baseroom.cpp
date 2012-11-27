//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "baseroom.h"

BaseRoom::BaseRoom (RoomInitData& initData)
	: parentZone_ (initData.zone), parentBuilding_ (initData.building), floorLevel_ (initData.floorLevel),
	  minLength_ (initData.minLength), maxLength_ (initData.maxLength), numFloorsSpanned_ (initData.numFloorsSpanned), 
	  isMainRoom_ (initData.isMainRoom), canContainStairs_ (initData.canContainStairs)
{
	for (uint32 i = 0; i < initData.linkableChildRoomTypes.GetCount (); ++i)
		linkableChildRoomTypes_.InsertBack (initData.linkableChildRoomTypes[i]);

	doorHeight_ = 2;
	exteriorDoorType_ = DOOR_SINGLE;
	Assert (minLength_ >= 3);
	Assert (minLength_ <= maxLength_);
	Assert (minLength_ >= parentBuilding_->GetMinHallWayWidth ()); // Note- parentBuilding_->minHallwayWidth_ determines minimum allowable open area length on the x and y axes
}

BaseRoom::~BaseRoom ()
{
	windows_.RemoveAndDeleteAll ();
	doors_.RemoveAndDeleteAll ();
	stairs_.RemoveAndDeleteAll ();
}

void BaseRoom::BuildFoundation (uint32 thickness)
{
	Assert( GetWorld()->GetIsModifying() );
	// STODO: modify when basements are implemented.
	if (floorLevel_ == 1)
	{
		for (uint32 y = min_.y; y <= max_.y; ++y)
		{
			for (uint32 x = min_.x; x <= max_.x; ++x)
			{
				int32 zOff = -((int32)thickness);
				for (zOff; zOff < 0; ++zOff)
				{
					uint32 sCubeBuf = 0;
					SetSCubeTerrainType (&sCubeBuf, TERRA_STONE);
				
					GetWorld()->SetSCube(x, y, min_.z + zOff, sCubeBuf);
				}
			}
		}
	}
}

void BaseRoom::BuildFloor ()
{
	Assert( GetWorld()->GetIsModifying() );
	uint32 sCubeBuf;
	for (uint32 y = min_.y; y <= max_.y; ++y)
	{
		for (uint32 x = min_.x; x <= max_.x; ++x)
		{
			sCubeBuf = 0;

			if (x == min_.x || x == max_.x || y == min_.y || y == max_.y)
			{
				// this sets the type to a wall type when a wall is present directly below
				if ((floorLevel_ == 1 && IsPointOnExteriorWall (x, y, min_.z + 1, min_.z, false)) ||
					(floorLevel_ > 1 && IsPointOnExteriorWall (x, y, min_.z - 1, min_.z - parentBuilding_->GetCeilingHeight (), false)))
				{
					SetSCubeTerrainType (&sCubeBuf, parentBuilding_->GetOuterWallMaterial ());
				}
				else if (parentBuilding_->IntersectPointWithRooms (x, y, min_.z - 1, false) && 
					!parentBuilding_->IntersectPointWithRooms (x, y, min_.z - 1, true))
				{
					SetSCubeTerrainType (&sCubeBuf, parentBuilding_->GetInnerWallMaterial ());
				}
			}

			if (sCubeBuf == 0)
				SetSCubeTerrainType (&sCubeBuf, parentBuilding_->GetFloorMaterial ()); // interior floor

			GetWorld()->SetSCube(x, y, min_.z, sCubeBuf);
		}
	}
}

void BaseRoom::BuildWalls ()
{
	Assert( GetWorld()->GetIsModifying() );
	uint32 sCubeBuf;

	for (uint32 y = min_.y; y <= max_.y; ++y)
	{
		for (uint32 x = min_.x; x <= max_.x; ++x)
		{
			for (int32 zOff = 1; zOff < (int32)parentBuilding_->GetCeilingHeight (); ++zOff)
			{
				sCubeBuf = 0;

				if (IsPointOnExteriorWall (x, y, (int32)min_.z + zOff, min_.z, false))
					SetSCubeTerrainType (&sCubeBuf, parentBuilding_->GetOuterWallMaterial ());
				else if (x == min_.x || x == max_.x || y == min_.y || y == max_.y)
					SetSCubeTerrainType (&sCubeBuf, parentBuilding_->GetInnerWallMaterial ());

				GetWorld()->SetSCube(x, y, min_.z + zOff, sCubeBuf);
			}
		}
	}
}

void
BaseRoom::PlaceObjects ()
{
	Assert( GetWorld()->GetIsModifying() );
	for (uint32 i = 0; i < windows_.GetCount (); ++i)
		windows_[i]->Build ();

	for (uint32 i = 0; i < doors_.GetCount (); ++i)
		doors_[i]->Build ();

	for (uint32 i = 0; i < stairs_.GetCount (); ++i)
		stairs_[i]->Build ();
}

void
BaseRoom::PlaceWindows (uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance)
{
	Assert (placementChance >= 0.0f && placementChance <= 1.0f);

	const uint32 wallThickness = 1;

	for (uint32 y = min_.y; y <= max_.y; ++y)
	{
		for (uint32 x = min_.x; x <= max_.x; ++x)
		{
			if (IsPointOnExteriorWall (x, y, min_.z + heightAboveFloor, min_.z, true) && FRandom () <= placementChance)
			{
				UPoint3D windowMin;
				windowMin.x = x;
				windowMin.y = y;
				windowMin.z = min_.z + heightAboveFloor;
				if (parentBuilding_->IntersectPointWithRoomsFast (windowMin.x, windowMin.y, windowMin.z, min_.z))
				{
					bool canWindowBePlaced = true;
					UPoint3D windowMax;
					if (x == min_.x || x == max_.x)
					{
						if (y <= min_.y + 1 || y >= max_.y - 1)
							continue; // prevent windows from being placed in wall corners
						windowMax.x = windowMin.x + wallThickness - 1;
						windowMax.y = windowMin.y + width - 1;
					}
					else
					{
						if (x <= min_.x + 1 || x >= max_.x - 1)
							continue; // prevent windows from being placed in wall corners
						windowMax.x = windowMin.x + width - 1;
						windowMax.y = windowMin.y + wallThickness - 1;
					}
					windowMax.z = windowMin.z + height - 1;

					if (!DoesExtentsIntersectWithWindows_ (windowMin, windowMax, windowSpacing) && 
						!DoesExtentsIntersectWithDoors_ (windowMin, windowMax, 1, true) &&
						!DoesExtentsIntersectWithStairs_ (windowMin, windowMax, 1, false))
					{
						Window* window = new Window (windowMin, windowMax, WINDOW_NORMAL);
						windows_.InsertBack (window);
					}
				}
			}
		}
	}
}

bool 
BaseRoom::LinkToRoomByDoor (BaseRoom* childRoom)
{
	DoorType doorType = GetConnectingDoorType (childRoom);

	uint32 doorWidth = 1;
	if (doorType == DOOR_DOUBLE)
		doorWidth += 1;

	if (!IsRoomHorizontallyLinkableToOtherRoom (childRoom, doorWidth))
		return false;

	// loop through parent's children to check if the rooms are already linked
	for (uint32 i = 0; i < childRooms_.GetCount (); ++i)
	{
		if (childRooms_[i]->min_ == childRoom->min_)
			return false;
	}
	// loop through parent's parents to check if the rooms are already linked
	for (uint32 i = 0; i < parentRooms_.GetCount (); ++i)
	{
		if (parentRooms_[i]->min_ == childRoom->min_)
			return false;
	}

	// link rooms
	childRooms_.InsertBack (childRoom);
	childRoom->parentRooms_.InsertBack (this);

	// determine door side
	Direction doorSide;
	if (childRoom->min_.y == max_.y)
		doorSide = DIRECTION_SOUTH;
	else if (childRoom->max_.y == min_.y)
		doorSide = DIRECTION_NORTH;
	else if (childRoom->min_.x == max_.x)
		doorSide = DIRECTION_WEST;
	else
		doorSide = DIRECTION_EAST;

	// add door
	childRoom->AddDoorToRoom (doorType, doorSide, false, false, this);

	// successful link
	return true;
}

bool
BaseRoom::LinkToRoomByStair (BaseRoom* childRoom, uint32 stairWidth, bool isSpiralStair, uint32 spiralStairLength)
{
	uint32 ceilingHeight = max_.z - min_.z;

	if (!IsRoomVerticallyLinkableByWallEdgeToOtherRoom (childRoom, ceilingHeight, stairWidth, isSpiralStair, spiralStairLength))
		return false;

	// loop through parent's children to check if the rooms are already linked
	for (uint32 i = 0; i < childRooms_.GetCount (); ++i)
	{
		if (childRooms_[i]->min_ == childRoom->min_)
			return false;
	}

	if (AddStairToRoom (isSpiralStair, stairWidth, spiralStairLength, childRoom))
	{
		// link rooms
		childRooms_.InsertBack (childRoom);
		childRoom->parentRooms_.InsertBack (this);
		return true;
	}
	else
	{
		return false;
	}
}

bool
BaseRoom::AddDoorToRoom (DoorType dType, Direction doorSide, bool centerDoor, bool isMainDoor, BaseRoom* adjacentRoom)
{
	// Note- adjacentRoom is optional and will cause the door to be placed on the adjacentRoom border
	const uint32 floorThickness = 1;
	const uint32 wallThickness = 1;

	uint32 doorWidth = 1;
	if (dType == DOOR_DOUBLE)
		doorWidth += 1;

	Assert (doorSide == DIRECTION_SOUTH || doorSide == DIRECTION_NORTH || doorSide == DIRECTION_WEST || doorSide == DIRECTION_EAST);

	Assert (max_.x - min_.x >= wallThickness * 2);
	Assert (max_.y - min_.y >= wallThickness * 2);
	if (doorSide == DIRECTION_SOUTH || doorSide == DIRECTION_NORTH)
		Assert (doorWidth <= max_.x - min_.x - wallThickness * 2 + 1);
	else
		Assert (doorWidth <= max_.y - min_.y - wallThickness * 2 + 1);
	
	Assert (max_.z - min_.z >= floorThickness * 2);
	Assert (doorHeight_ <= GetLengthZ () - floorThickness * 2);

	UPoint3D doorMin;
	UPoint3D doorMax;
	Direction dSide;

	dSide = doorSide;
	doorMin.z = min_.z + floorThickness;
	if (dType == DOOR_NONE)
		doorMax.z = max_.z - floorThickness;
	else
		doorMax.z = doorMin.z + doorHeight_ - 1;

	// set position values that align with wall
	switch (dSide)
	{
	case DIRECTION_SOUTH:
		doorMin.y = min_.y;
		doorMax.y = min_.y + wallThickness - 1;
		break;
	case DIRECTION_NORTH:
		doorMin.y = max_.y - wallThickness + 1;
		doorMax.y = max_.y;
		break;
	case DIRECTION_WEST:
		doorMin.x = min_.x;
		doorMax.x = min_.x + wallThickness - 1;
		break;
	case DIRECTION_EAST:
		doorMin.x = max_.x - wallThickness + 1;
		doorMax.x = max_.x;
		break;
	}

	// set position along wall
	if (dSide == DIRECTION_SOUTH || dSide == DIRECTION_NORTH)
	{
		if (centerDoor && !dType == DOOR_NONE)
		{
			doorMin.x = min_.x + GetLengthX () / 2;
			doorMax.x = doorMin.x + doorWidth - 1;
		}
		else
		{
			uint32 minVal;
			uint32 maxVal;

			if (adjacentRoom != NULL && adjacentRoom->min_.x > min_.x)
				minVal = adjacentRoom->min_.x + wallThickness;
			else
				minVal = min_.x + wallThickness;

			if (adjacentRoom != NULL && adjacentRoom->max_.x < max_.x)
				maxVal = adjacentRoom->max_.x - wallThickness;
			else
				maxVal = max_.x - wallThickness;

			// reduce likelihood of door touching wall
			if (dType != DOOR_NONE && maxVal - minVal >= 2)
			{
				--maxVal;
				++minVal;
			}

			Assert (maxVal >= minVal);

			if (dType == DOOR_NONE)
			{
				doorMin.x = minVal;
				doorMax.x = maxVal;
			}
			else
			{
				doorMin.x = minVal + rand () % (maxVal - minVal + 1);
				doorMax.x = doorMin.x + doorWidth - 1;
			}
		}
	}
	else
	{
		if (centerDoor && !dType == DOOR_NONE)
		{
			doorMin.y = min_.y + GetLengthY () / 2;
			doorMax.y = doorMin.y + doorWidth - 1;
		}
		else
		{
			uint32 minVal;
			uint32 maxVal;

			if (adjacentRoom != NULL && adjacentRoom->min_.y > min_.y)
				minVal = adjacentRoom->min_.y + wallThickness;
			else
				minVal = min_.y + wallThickness;

			if (adjacentRoom != NULL && adjacentRoom->max_.y < max_.y)
				maxVal = adjacentRoom->max_.y - wallThickness;
			else
				maxVal = max_.y - wallThickness;

			// reduce likelihood of door touching wall
			if (dType != DOOR_NONE && maxVal - minVal >= 2)
			{
				--maxVal;
				++minVal;
			}

			Assert (maxVal >= minVal);

			if (dType == DOOR_NONE)
			{
				doorMin.y = minVal;
				doorMax.y = maxVal;
			}
			else
			{
 				doorMin.y = minVal + rand () % (maxVal - minVal + 1);
				doorMax.y = doorMin.y + doorWidth - 1;
			}
		}
	}

	Assert (doorMin.x <= doorMax.x);
	Assert (doorMin.y <= doorMax.y);
	Assert (doorMin.z <= doorMax.z);
	Assert (doorMin.x >= min_.x);
	Assert (doorMin.y >= min_.y);
	Assert (doorMin.z >= min_.z);
	Assert (doorMax.x <= max_.x);
	Assert (doorMax.y <= max_.y);
	Assert (doorMax.z <= max_.z);

	if (DoesExtentsIntersectWithDoors_ (doorMin, doorMax, 1, true) || 
		DoesExtentsIntersectWithStairs_ (doorMin, doorMax, 2, true))
	{
		return false;
	}

	// SHACK: STODO: use polymorphism here instead of type info
	uint32 maxZ;
	if (GetType () == ROOM_ROOFTOP)
		maxZ = max_.z + 1; // avoids creating hovering floor material for rooftop-rooftop "doors"
	else
		maxZ = max_.z;

	Door* door = new Door (doorMin, doorMax, dSide, dType, parentBuilding_->GetFloorMaterial (), maxZ);

	if (isMainDoor)
		parentBuilding_->SetFDoorPos (doorMin);

	doors_.InsertBack (door);

	// successful link
	return true;
}

bool 
BaseRoom::AddStairToRoom (bool useSpiralStaircase, uint32 stairWidth, uint32 spiralStairLength, BaseRoom* adjacentRoom)
{
	// STODO: improve random stair placement
	// STODO: allow stairs_/ramps to be placed without linking to a room
	// STODO: allow a position and direction to be designated

	const int32 floorThickness = 1;
	uint32 ceilingHeight = parentBuilding_->GetCeilingHeight ();
	bool isStairDefined = false;
	Stair* stair;

	//--------------------------------------------------------------------------------------------------------------
	// code to set stair specifications (spiral or not, length, width, if the stair extends to the floor/ground)
	if (useSpiralStaircase)
	{
		stair = new SpiralStair (parentBuilding_->GetStairMaterial (), parentBuilding_->GetFloorMaterial ());
		stair->length = spiralStairLength;
	}
	else
	{
		stair = new Stair (parentBuilding_->GetStairMaterial ());
		stair->length = ceilingHeight + floorThickness - 1;
	}
	stair->width = stairWidth;
	stair->extendsToFloor = false;
	//--------------------------------------------------------------------------------------------------------------
	
	// determine what wall to place stairs_ on.
	if (rand () % 2 == 0)
	{
		if (min_.x == adjacentRoom->min_.x && !isStairDefined)
			isStairDefined = stair->SetBoundsForLinkedRooms (this, adjacentRoom, true, true);
		if (max_.x == adjacentRoom->max_.x && !isStairDefined)
			isStairDefined = stair->SetBoundsForLinkedRooms (this, adjacentRoom, true, false);
		if (min_.y == adjacentRoom->min_.y && !isStairDefined)
			isStairDefined = stair->SetBoundsForLinkedRooms (this, adjacentRoom, false, true);
		if (max_.y == adjacentRoom->max_.y && !isStairDefined)
			isStairDefined = stair->SetBoundsForLinkedRooms (this, adjacentRoom, false, false);
	}
	else
	{
		if (max_.y == adjacentRoom->max_.y && !isStairDefined)
			isStairDefined = stair->SetBoundsForLinkedRooms (this, adjacentRoom, false, false);
		if (min_.y == adjacentRoom->min_.y && !isStairDefined)
			isStairDefined = stair->SetBoundsForLinkedRooms (this, adjacentRoom, false, true);
		if (max_.x == adjacentRoom->max_.x && !isStairDefined)
			isStairDefined = stair->SetBoundsForLinkedRooms (this, adjacentRoom, true, false);
		if (min_.x == adjacentRoom->min_.x && !isStairDefined)
			isStairDefined = stair->SetBoundsForLinkedRooms (this, adjacentRoom, true, true);
	}

	// Assert (isStairDefined); 
	// STODO: make sure a successful IsRoomVerticallyLinkableByWallEdgeToOtherRoom_ call always results in a successful stair definition
	if (!isStairDefined)
		return false;

	// SNOW: if spiral stairs can link, allow intersection
	if (DoesExtentsIntersectWithDoors_ (stair->min, stair->max, 2, true) || 
		DoesExtentsIntersectWithStairs_ (stair->min, stair->max, 1, true))
	{
		delete stair;
		return false;
	}

	if (adjacentRoom->floorLevel_ < floorLevel_)
	{
		adjacentRoom->stairs_.InsertBack (stair);

		Assert (stair->min.x > adjacentRoom->min_.x);
		Assert (stair->min.y > adjacentRoom->min_.y);
		Assert (stair->min.z > adjacentRoom->min_.z);
		Assert (stair->max.x < adjacentRoom->max_.x);
		Assert (stair->max.y < adjacentRoom->max_.y);
		Assert (stair->max.z <= adjacentRoom->max_.z);
	}
	else
	{
		stairs_.InsertBack (stair);

		Assert (stair->min.x > min_.x);
		Assert (stair->min.y > min_.y);
		Assert (stair->min.z > min_.z);
		Assert (stair->max.x < max_.x);
		Assert (stair->max.y < max_.y);
		Assert (stair->max.z <= max_.z);
	}

	return true;
}

bool
BaseRoom::IsPointOnExteriorWall (uint32 x, uint32 y, uint32 z, uint32 roomMinZ, bool isPointFullyExposedToExterior)
{
	// STODO: move this method to basebuilding
	if (x == min_.x || x == max_.x || y == min_.y || y == max_.y)
	{
		UPoint3D point = UPoint3D (x, y, z);

		if (x == min_.x || x == max_.x)
		{
			if (x == min_.x)
				point.x -= 1;
			else if (x == max_.x)
				point.x += 1;

			if (isPointFullyExposedToExterior)
			{
				if (parentBuilding_->IntersectPointWithRoomsFast (point.x, point.y - 1, point.z, roomMinZ) == false &&
					parentBuilding_->IntersectPointWithRoomsFast (point.x, point.y, point.z, roomMinZ) == false &&
					parentBuilding_->IntersectPointWithRoomsFast (point.x, point.y + 1, point.z, roomMinZ) == false)
				{
					return true;
				}
			}
			else
			{
				if (parentBuilding_->IntersectPointWithRoomsFast (point.x, point.y - 1, point.z, roomMinZ) == false ||
					parentBuilding_->IntersectPointWithRoomsFast (point.x, point.y, point.z, roomMinZ) == false ||
					parentBuilding_->IntersectPointWithRoomsFast (point.x, point.y + 1, point.z, roomMinZ) == false)
				{
					return true;
				}
			}
		}

		point = UPoint3D (x, y, z);

		if (y == min_.y || y == max_.y)
		{
			if (y == min_.y)
				point.y -= 1;
			else if (y == max_.y)
				point.y += 1;

			if (isPointFullyExposedToExterior)
			{
				if (parentBuilding_->IntersectPointWithRoomsFast (point.x - 1, point.y, point.z, roomMinZ) == false &&
					parentBuilding_->IntersectPointWithRoomsFast (point.x, point.y, point.z, roomMinZ) == false &&
					parentBuilding_->IntersectPointWithRoomsFast (point.x + 1, point.y, point.z, roomMinZ) == false)
				{
					return true;
				}
			}
			else
			{
				if (parentBuilding_->IntersectPointWithRoomsFast (point.x - 1, point.y, point.z, roomMinZ) == false ||
					parentBuilding_->IntersectPointWithRoomsFast (point.x, point.y, point.z, roomMinZ) == false ||
					parentBuilding_->IntersectPointWithRoomsFast (point.x + 1, point.y, point.z, roomMinZ) == false)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool
BaseRoom::IsRoomHorizontallyLinkableToOtherRoom (BaseRoom* other, uint32 doorWidth)
{
	// STODO: if diagonal passages are ever implemented, this will have to change.
	const int32 wallThickness = 1;
	int32 offset = (int32)doorWidth + wallThickness;

	// either min_ or max_ y must match. this allows for connections to rooms with higher ceilings or a lower floor level.
	if (min_.z == other->min_.z || max_.z == other->max_.z)
	{
		
		if ((min_.x == other->max_.x || max_.x == other->min_.x) &&

			// STODO: double check logic here. draw out some grids to verify
			((min_.y <= other->max_.y - offset && max_.y >= other->min_.y + offset) ||
				(other->min_.y <= max_.y - offset && other->max_.y >= min_.y + offset)))
		{
			return true;
		}

		
		if ((min_.y == other->max_.y || max_.y == other->min_.y) &&

			// STODO:
			((min_.x <= other->max_.x - offset && max_.x >= other->min_.x + offset) ||
			(other->max_.x <= max_.x - offset && other->max_.x >= min_.x + offset)))
		{
			return true;
		}
	}
	return false;
}

bool
BaseRoom::IsRoomVerticallyLinkableByWallEdgeToOtherRoom (BaseRoom* other, uint32 ceilingHeight, uint32 stairWidth, bool isSpiralStair, uint32 spiralStairLength)
{
	// STODO: use an enum return type_ for this method. 0 = false, 1 = x axis only, 2 = y axis only 3 = both axis
	// returns true if both rooms share a wall with enough length to place a stair there.
	// STODO: if diagonal passages are ever implemented, this will have to change.
	const int32 floorThickness = 1;
	const int32 wallThickness = 1;
	uint32 effectiveStairLength;

	if (other->max_.z != min_.z && other->min_.z != max_.z)
		return false;

	if (isSpiralStair)
	{

		effectiveStairLength = spiralStairLength + stairWidth * 2;

		if (effectiveStairLength > GetLengthX () - wallThickness * 2 || effectiveStairLength > GetLengthY () - wallThickness * 2)
			return false;

		if (min_.x == other->min_.x || max_.x == other->max_.x)
		{
			if ((min_.y + effectiveStairLength < other->max_.y && max_.y > other->min_.y + effectiveStairLength) &&
				(min_.y + effectiveStairLength < max_.y && other->min_.y + effectiveStairLength < other->max_.y))
				return true;
		}
		if (min_.y == other->min_.y || max_.y == other->max_.y)
		{ 
			if ((min_.x + effectiveStairLength < other->max_.x && max_.x > other->min_.x + effectiveStairLength) &&
				(min_.x + effectiveStairLength < max_.x && other->min_.x + effectiveStairLength < other->max_.x))
				return true;
		}
	}
	else
	{
		effectiveStairLength = ceilingHeight + floorThickness + 1;

		if ((min_.x == other->min_.x || max_.x == other->max_.x) &&
			(GetLengthX () - wallThickness * 2 >= effectiveStairLength && other->GetLengthX () - wallThickness * 2 >= effectiveStairLength))
		{
			if ((min_.y + stairWidth < other->max_.y && max_.y > other->min_.y + stairWidth) &&
				(min_.y + stairWidth < max_.y && other->min_.y + stairWidth < other->max_.y))
				return true;
		}
		if ((min_.y == other->min_.y || max_.y == other->max_.y) &&
			(GetLengthY () - wallThickness * 2 >= effectiveStairLength && other->GetLengthY () - wallThickness * 2 >= effectiveStairLength))
		{ 
			if ((min_.x + stairWidth < other->max_.x && max_.x > other->min_.x + stairWidth) &&
				(min_.x + stairWidth < max_.x && other->min_.x + stairWidth < other->max_.x))
				return true;
		}
	}

	/* OLD CODE
	if ((min_.x == other->min_.x || max_.x == other->max_.x) &&
		(min_.y + effectiveStairLength < other->max_.y && max_.y > other->min_.y + effectiveStairLength) &&
		(min_.y + effectiveStairLength < max_.y && other->min_.y + effectiveStairLength < other->max_.y))
		return true;

	if ((min_.y == other->min_.y || max_.y == other->max_.y) &&
		(min_.x + effectiveStairLength < other->max_.x && max_.x > other->min_.x + effectiveStairLength) &&
		(min_.x + effectiveStairLength < max_.x && other->min_.x + effectiveStairLength < other->max_.x))
		return true;
		*/

	return false;
}

/* ----------------------------------------------------
	Accessors
------------------------------------------------------- */

BaseRoom*
BaseRoom::GetParentRoom (uint32 idx)
{
	return parentRooms_[idx];
}

BaseRoom*
BaseRoom::GetChildRoom (uint32 idx)
{
	return childRooms_[idx];
}

BaseBuilding*
BaseRoom::GetParentBuilding ()
{
	return parentBuilding_;
}

Window*
BaseRoom::GetWindow (uint32 idx)
{
	return windows_[idx];
}

void
BaseRoom::RemoveWindow (uint32 idx)
{
	windows_.RemoveAndDelete (idx);
}

bool
BaseRoom::GetIsMainRoom ()
{
	return isMainRoom_;
}

bool
BaseRoom::GetCanContainStairs ()
{
	return canContainStairs_;
}

bool
BaseRoom::GetIsLinkableToChildRoom (BaseRoom* childRoom)
{
	for (uint32 i = 0; i < linkableChildRoomTypes_.GetCount (); ++i)
	{
		if (childRoom->GetType () == linkableChildRoomTypes_[i])
			return true;
	}
	return false;
}

int32
BaseRoom::GetFloorLevel ()
{
	return floorLevel_;
}

uint32
BaseRoom::GetNumFloorsSpanned ()
{
	return numFloorsSpanned_;
}

uint32
BaseRoom::GetMinLength ()
{
	return minLength_;
}

uint32
BaseRoom::GetMaxLength ()
{
	return maxLength_;
}

uint32
BaseRoom::GetRandLength ()
{
	Assert (maxLength_ >= minLength_);
	return minLength_ + rand () % (maxLength_ - minLength_ + 1);
}

uint32 
BaseRoom::GetNumParentRooms ()
{
	return parentRooms_.GetCount ();
}

uint32
BaseRoom::GetNumChildRooms ()
{
	return childRooms_.GetCount ();
}

uint32
BaseRoom::GetNumStairs ()
{
	return stairs_.GetCount ();
}

uint32
BaseRoom::GetNumDoors ()
{
	return doors_.GetCount ();
}

uint32
BaseRoom::GetNumWindows ()
{
	return windows_.GetCount ();
}

DoorType
BaseRoom::GetExteriorDoorType ()
{
	return exteriorDoorType_;
}

DoorType 
BaseRoom::GetConnectingDoorType (BaseRoom* otherRoom)
{
	return DOOR_SINGLE;
}

RoomType
BaseRoom::GetType ()
{
	Assert (0);
	return ROOM_UNKNOWN;
}

/* ----------------------------------------------------
	Private Methods
------------------------------------------------------- */

bool
BaseRoom::DoesExtentsIntersectWithWindows_ (UPoint3D extentsMin, UPoint3D extentsMax, uint32 minSeparationDistance)
{
	extentsMin.x -= minSeparationDistance; extentsMin.y -= minSeparationDistance; extentsMin.z -= minSeparationDistance;
	extentsMax.x += minSeparationDistance; extentsMax.y += minSeparationDistance; extentsMax.z += minSeparationDistance;
	for (uint32 i = 0; i < windows_.GetCount (); ++i)
	{
		if (IntersectAABoxWithAABoxFast (extentsMin, extentsMax, windows_[i]->GetMin (), windows_[i]->GetMax ()))
			return true;
	}
	return false;
}

bool
BaseRoom::DoesExtentsIntersectWithDoors_ (UPoint3D extentsMin, UPoint3D extentsMax, uint32 minSeparationDistance, bool checkLinkedRooms)
{
	extentsMin.x -= minSeparationDistance; extentsMin.y -= minSeparationDistance; extentsMin.z -= minSeparationDistance;
	extentsMax.x += minSeparationDistance; extentsMax.y += minSeparationDistance; extentsMax.z += minSeparationDistance;
	for (uint32 i = 0; i < doors_.GetCount (); ++i)
	{
		if (IntersectAABoxWithAABoxFast (extentsMin, extentsMax, doors_[i]->GetMin (), doors_[i]->GetMax ()))
			return true;
	}
	if (checkLinkedRooms)
	{
		for (uint32 i = 0; i < parentRooms_.GetCount (); ++i)
		{
			for (uint32 j = 0; j < parentRooms_[i]->doors_.GetCount (); ++j)
			{
				if (IntersectAABoxWithAABoxFast (extentsMin, extentsMax, parentRooms_[i]->doors_[j]->GetMin (), parentRooms_[i]->doors_[j]->GetMax ()))
					return true;
			}
		}
		for (uint32 i = 0; i < childRooms_.GetCount (); ++i)
		{
			for (uint32 j = 0; j < childRooms_[i]->doors_.GetCount (); ++j)
			{
				if (IntersectAABoxWithAABoxFast (extentsMin, extentsMax, childRooms_[i]->doors_[j]->GetMin (), childRooms_[i]->doors_[j]->GetMax ()))
					return true;
			}
		}
	}
	return false;
}

bool
BaseRoom::DoesExtentsIntersectWithStairs_ (UPoint3D extentsMin, UPoint3D extentsMax, uint32 minSeparationDistance, bool checkLinkedRooms)
{
	extentsMin.x -= minSeparationDistance; extentsMin.y -= minSeparationDistance; extentsMin.z -= minSeparationDistance;
	extentsMax.x += minSeparationDistance; extentsMax.y += minSeparationDistance; extentsMax.z += minSeparationDistance;
	for (uint32 i = 0; i < stairs_.GetCount (); ++i)
	{
		if (IntersectAABoxWithAABoxFast (extentsMin, extentsMax, stairs_[i]->min, stairs_[i]->max))
			return true;
	}
	if (checkLinkedRooms)
	{
		for (uint32 i = 0; i < parentRooms_.GetCount (); ++i)
		{
			for (uint32 j = 0; j < parentRooms_[i]->stairs_.GetCount (); ++j)
			{
				if (IntersectAABoxWithAABoxFast (extentsMin, extentsMax, parentRooms_[i]->stairs_[j]->min, parentRooms_[i]->stairs_[j]->max))
					return true;
			}
		}
		for (uint32 i = 0; i < childRooms_.GetCount (); ++i)
		{
			for (uint32 j = 0; j < childRooms_[i]->stairs_.GetCount (); ++j)
			{
				if (IntersectAABoxWithAABoxFast (extentsMin, extentsMax, childRooms_[i]->stairs_[j]->min, childRooms_[i]->stairs_[j]->max))
					return true;
			}
		}
	}
	return false;
}


