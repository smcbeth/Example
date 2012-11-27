//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "baseroom.h"

class BaseRoom;

enum STAIRDIRECTION 
{
	// directions rotate clockwise as direction increases
	STAIR_SOUTH = 0,
	STAIR_WEST,
	STAIR_NORTH,
	STAIR_EAST
};

// STODO: implement this
enum STAIRPLACEMENT
{
	STAIR_WALLMIN = 0,
	STAIR_WALLMAX,
	STAIR_WALLCENTER,
	STAIR_ROOMCENTER
};

class Stair
{
public:
	Stair (TERRATYPE stairMaterial);
	virtual ~Stair ();
	virtual void Build ();
	virtual bool SetBoundsForLinkedRooms (BaseRoom* parentRoom, BaseRoom* childRoom, bool isWallPerpendicularToX, bool isWallMin);

protected:
	void GetMinAndMaxPlacementPositions_ (BaseRoom* parentRoom, BaseRoom* childRoom, bool isWallPerpendicularToX, uint32& minPos, uint32& maxPos);
	
public:
	UPoint3D min;
	UPoint3D max;
	bool extendsToFloor; // STODO: make sure this works for stair and spiral stair
	bool isRamp; // STODO: add this functionality. when true, replaces stairs with a smooth incline
	uint32 length;
	uint32 width;
	STAIRDIRECTION direction; // direction traveled when going up the start of a set of stairs
	TERRATYPE material;
};

// STODO: allow for interlocking of multiple spiral stairs placed vertically over each other
// STODO: place spiral stairs that intersect with doors as long as its rotated in such a way that the door is not blocked
class SpiralStair: public Stair
{
public:
	SpiralStair (TERRATYPE stairMaterial, TERRATYPE floorMaterial);
	~SpiralStair ();
	void Build ();
	bool SetBoundsForLinkedRooms (BaseRoom* parentRoom, BaseRoom* childRoom, bool isWallPerpendicularToX, bool isWallMin);

	bool isClockwise;
	TERRATYPE floorMaterial_;
};