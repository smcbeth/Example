//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\_math.h"
#include "public\nstdlib\vector.h"
#include "public\nstdlib\list.h"
#include "public\nstdlib\upoint3d.h"

#include "world.h"
#include "terrainhelper.h"
#include "scubegeom.h"
#include "s_sector.h"
#include "basezone.h"
#include "basebuilding.h"
#include "stair.h"
#include "door.h"
#include "window.h"

class BaseZone;
class BaseBuilding;
class Town;
class Door;
class Stair;

enum RoomType
{
	ROOM_UNKNOWN = 0,
	// STODO: ROOM_ATTIC, // there will be some small attics that wont count as rooms
	// STODO: ROOM_ARMORY,
	ROOM_BEDROOM,
	ROOM_DISPLAYROOM,
	ROOM_DUNGEONROOM,
	ROOM_GREATHALL,
	ROOM_HALLWAY,
	ROOM_KITCHEN,
	ROOM_LIBRARY,
	ROOM_LIVINGROOM,
	ROOM_OFFICE,
	ROOM_ROOFTOP,
	ROOM_STAIRHALL,
	ROOM_STORAGEROOM,
	ROOM_TREASUREROOM,
	ROOM_TREASURYROOM
};

struct RoomInitData
{
	BaseZone* zone;
	BaseBuilding* building;
	uint32 minLength;
	uint32 maxLength;
	uint32 floorLevel;
	uint32 numFloorsSpanned;
	bool isMainRoom;
	bool canContainStairs;
	Vector<RoomType> linkableChildRoomTypes;
};


//STODO: add a list of additional Areas that the room can occupy in addition to its main Area. this will allow for the
 // building of rooms that do not take a simple rectangular shape

class BaseRoom: public BaseArea
{
public:
	explicit BaseRoom (RoomInitData& initData);
	virtual ~BaseRoom ();

	virtual void BuildFoundation (uint32 thickness);
	virtual void BuildFloor ();
	virtual void BuildWalls ();

	virtual void PlaceObjects ();

	virtual void PlaceWindows (uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance);

	// STODO: void Furnish ();
	bool LinkToRoomByDoor (BaseRoom* childRoom);
	bool LinkToRoomByStair (BaseRoom* childRoom, uint32 stairWidth, bool isSpiralStair, uint32 spiralStairLength = 1);

	// STODO: add door type to parameters
	bool AddDoorToRoom (DoorType dType, Direction doorSide, bool centerDoor, bool isMainDoor, BaseRoom* adjacentRoom = NULL);

	// STODO: improve random stair placement
	// STODO: allow stairs/ramps to be placed without linking to a room
	// STODO: allow a position and direction to be designated
	// STODO: make adjacent room an optional parameter
	bool AddStairToRoom (bool useSpiralStaircase, uint32 stairWidth, uint32 spiralStairLength, BaseRoom* adjacentRoom); // allow specification of a position, allowing stairs and ramps to be contained in a room and not link to another room
	
	bool IsPointOnExteriorWall (uint32 x, uint32 y, uint32 z, uint32 roomMinZ, bool isPointFullyExposedToExterior);
	bool IsRoomHorizontallyLinkableToOtherRoom (BaseRoom* other, uint32 doorWidth);
	// STODO: use an enum return type for this method.. see cpp
	bool IsRoomVerticallyLinkableByWallEdgeToOtherRoom (BaseRoom* other, uint32 ceilingHeight, uint32 stairWidth, bool isSpiralStair, uint32 spiralStairLength = 1);

	BaseRoom* GetParentRoom (uint32 idx);
	BaseRoom* GetChildRoom (uint32 idx);
	BaseBuilding* GetParentBuilding ();
	Window* GetWindow (uint32 idx);
	void RemoveWindow (uint32 idx);
	bool GetIsMainRoom ();
	bool GetCanContainStairs ();
	bool GetIsLinkableToChildRoom (BaseRoom* childRoom);
	int32 GetFloorLevel ();
	uint32 GetNumFloorsSpanned ();
	uint32 GetMinLength ();
	uint32 GetMaxLength ();
	uint32 GetRandLength ();
	uint32 GetNumParentRooms ();
	uint32 GetNumChildRooms ();
	uint32 GetNumStairs ();
	uint32 GetNumDoors ();
	uint32 GetNumWindows ();
	DoorType GetExteriorDoorType ();
	virtual DoorType GetConnectingDoorType( BaseRoom* childRoom );
	virtual RoomType GetType ();

	// DEBUG_BASEROOM: indicator for order of placement
	uint32 debugRoomsIndex_;

protected:

	bool DoesExtentsIntersectWithWindows_ (UPoint3D extentsMin, UPoint3D extentsMax, uint32 minSeparationDistance);
	bool DoesExtentsIntersectWithDoors_ (UPoint3D extentsMin, UPoint3D extentsMax, uint32 minSeparationDistance, bool checkLinkedRooms);
	bool DoesExtentsIntersectWithStairs_ (UPoint3D extentsMin, UPoint3D extentsMax, uint32 minSeparationDistance, bool checkLinkedRooms);

	// Vector<BaseArea*> roomSubsections_; // STODO: list of room bounds in addition to main bounds
	Vector<BaseRoom*> parentRooms_; // direction of parent rooms should lead back to main room
	Vector<BaseRoom*> childRooms_;
	BaseZone* parentZone_;
	BaseBuilding* parentBuilding_;
	bool isMainRoom_;
	bool canContainStairs_; // if true stairs can be placed in the room and stairs can link to the room provided it is a linkable child room type to the parent room type
	int32 floorLevel_; // 1 for first floor, negative numbers for basement levels. 0 not used.
	uint32 numFloorsSpanned_; // greater than one values used for unconventional rooms like rooms with pits, or stairwells, etc...
	uint32 minLength_;
	uint32 maxLength_;
	uint32 doorHeight_;
	DoorType exteriorDoorType_; // doors between inside and outside
	Vector<Window*> windows_;
	Vector<Door*> doors_; // doors are contained in child room
	Vector<Stair*> stairs_; // stairs are contained in lower elevation room
	Vector<RoomType> linkableChildRoomTypes_; // creates non-imaginary placement areas nearby. more likely to link to other rooms as a parent room
};