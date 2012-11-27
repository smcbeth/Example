//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\gmath.h"

enum Direction
{
	DIRECTION_SOUTH = 0,
	DIRECTION_NORTH,
	DIRECTION_WEST,
	DIRECTION_EAST
};

class BaseArea
{
public:
	BaseArea( );
	BaseArea( const UPoint3D& min, const UPoint3D& max );
	BaseArea( const BaseArea& otherArea );
	virtual	~BaseArea ();

	bool operator==( const BaseArea& otherArea ) const;
	bool operator!=( const BaseArea& otherArea ) const;

	const UPoint3D& GetMin( ) const;
	const UPoint3D& GetMax( ) const;
	UPoint3D		GetRandPointInArea( ) const;
	UPoint3D		GetRandBorderPoint( bool doIgnoreXAxis, bool doIgnoreYAxis, bool doIgnoreZAxis, float32 centeringWeight, bool doZeroIgnoredAxes = true ) const;
	UPoint3D		GetClosestPointInAreaToPoint( const UPoint3D& point ) const;
	UPoint3D		GetCenterPoint( ) const;
	Direction		GetSideOfArea2DBorderPointIsOn( const UPoint3D& borderPoint ) const;
	float32			GetCenterPointDistance( const BaseArea& other ) const;
	float32			GetCenterPointDistanceFast( const BaseArea& other ) const;
	uint32			GetLengthX( ) const;
	uint32			GetLengthY( ) const;
	uint32			GetLengthZ( ) const;
	uint32			GetCenterX( ) const;
	uint32			GetCenterY( ) const;
	uint32			GetCenterZ( ) const;
	uint32			GetRandX( ) const;
	uint32			GetRandY( ) const;
	uint32			GetRandZ( ) const;

	void			SetMin (const UPoint3D& point);
	void			SetMin (uint32 x, uint32 y, uint32 z);
	void			SetMax (const UPoint3D& point);
	void			SetMax (uint32 x, uint32 y, uint32 z);
	void			SetMinX (uint32 x);
	void			SetMinY (uint32 y);
	void			SetMinZ (uint32 z);
	void			SetMaxX (uint32 x);
	void			SetMaxY (uint32 y);
	void			SetMaxZ (uint32 z);

protected:
	UPoint3D min_;
	UPoint3D max_;
};

#include "basearea.inl"