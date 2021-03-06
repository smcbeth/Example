//---------- Copyright � 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "baseroom.h"

struct RoomInitData;

class TreasuryRoom: public BaseRoom
{
public:
	explicit TreasuryRoom( RoomInitData& initData );
	~TreasuryRoom( );

	void BuildFoundation( uint32 thickness );
	void BuildFloor( );
	void BuildWalls( );
	void PlaceObjects( );

	void PlaceWindows( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance );

	DoorType GetConnectingDoorType( BaseRoom* childRoom );
	RoomType GetType( );
};