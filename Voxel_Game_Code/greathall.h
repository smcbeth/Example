//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "baseroom.h"

struct RoomInitData;

// STODO: make the room approximately 1.5-2 times as long as it is wide
class GreatHall: public BaseRoom
{
public:
	explicit GreatHall( RoomInitData& initData );
	~GreatHall( );

	void BuildFoundation( uint32 thickness );
	void BuildFloor( );
	void BuildWalls( );
	void PlaceObjects( );

	void PlaceWindows( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance );

	DoorType GetConnectingDoorType( BaseRoom* childRoom );
	RoomType GetType( );
};