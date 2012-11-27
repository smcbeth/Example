//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "basearea.h"

UPoint3D
BaseArea::GetRandBorderPoint( bool doIgnoreXAxis, bool doIgnoreYAxis, bool doIgnoreZAxis, float32 centeringWeight, bool doZeroIgnoredAxes ) const
{
	Assert( min_.x <= max_.x );
	Assert( min_.y <= max_.y );
	Assert( min_.z <= max_.z );

	// centeringWeight when less than 0 will prevent the center of borders from being returned. when -1.0 will return a point on the corner of a wall.
	// centeringWeight when greater than 0 will prevent edges of borders from being returned. when 1.0 will return a point in the exact center of a wall.
	Assert( centeringWeight >= -1.0f );
	Assert( centeringWeight <= 1.0f );

	// one or more axes must be static to get a border point
	Assert( !doIgnoreXAxis || !doIgnoreYAxis || !doIgnoreZAxis );

	bool doVaryXAxis = doIgnoreXAxis;
	bool doVaryYAxis = doIgnoreYAxis;
	bool doVaryZAxis = doIgnoreZAxis;

	// make one axis vary if all do not vary
	if( !doVaryXAxis && !doVaryYAxis && !doVaryZAxis )
	{
		switch( rand() % 3 )
		{
		case 0:
			doVaryXAxis = true;
			break;
		case 1:
			doVaryYAxis = true;
			break;
		case 2:
			doVaryZAxis = true;
			break;
		}
	}
	
	// make sure two axes vary if only one varies
	if( !doVaryXAxis && !doVaryYAxis )
	{
		rand() % 2 ? 
			doVaryXAxis = true : 
			doVaryYAxis = true;
	}
	else if( !doVaryXAxis && !doVaryZAxis )
	{
		rand() % 2 ? 
			doVaryXAxis = true : 
			doVaryZAxis = true;
	}
	else if( !doVaryYAxis && !doVaryZAxis )
	{
		rand() % 2 ? 
			doVaryYAxis = true : 
			doVaryZAxis = true;
	}

	UPoint3D retPoint;

	if( doVaryXAxis )
	{
		if( centeringWeight == 0.0f )
		{
			retPoint.x = GetRandX();
		}
		else if( centeringWeight < 0.0f )
		{
			rand() % 2 ?
				retPoint.x = GetMin().x + rand() % uint32(float32(GetLengthX()) * (-centeringWeight * 0.5f)) :
				retPoint.x = GetMax().x - rand() % uint32(float32(GetLengthX()) * (-centeringWeight * 0.5f));
		}
		else if( centeringWeight > 0.0f )
		{
			rand() % 2 ?
				retPoint.x = GetCenterX() + rand() % uint32(float32(GetLengthX()) * (centeringWeight * 0.5f)) :
				retPoint.x = GetCenterX() - rand() % uint32(float32(GetLengthX()) * (centeringWeight * 0.5f));
		}
	}
	else
	{
		rand() % 2 ? 
			retPoint.x = GetMin().x : 
			retPoint.x = GetMax().x;
	}

	if( doVaryYAxis )
	{
		if( centeringWeight == 0.0f )
		{
			retPoint.y = GetRandY();
		}
		else if( centeringWeight < 0.0f )
		{
			rand() % 2 ?
				retPoint.y = GetMin().y + rand() % uint32(float32(GetLengthY()) * (-centeringWeight * 0.5f)) :
				retPoint.y = GetMax().y - rand() % uint32(float32(GetLengthY()) * (-centeringWeight * 0.5f));
		}
		else if( centeringWeight > 0.0f )
		{
			rand() % 2 ?
				retPoint.y = GetCenterY() + rand() % uint32(float32(GetLengthY()) * (centeringWeight * 0.5f)) :
				retPoint.y = GetCenterY() - rand() % uint32(float32(GetLengthY()) * (centeringWeight * 0.5f));
		}
	}
	else
	{
		rand() % 2 ? 
			retPoint.y = GetMin().y : 
			retPoint.y = GetMax().y;
	}

	if( doVaryZAxis )
	{
		if( centeringWeight == 0.0f )
		{
			retPoint.z = GetRandZ();
		}
		else if( centeringWeight < 0.0f )
		{
			rand() % 2 ?
				retPoint.z = GetMin().z + rand() % uint32(float32(GetLengthZ()) * (-centeringWeight * 0.5f)) :
				retPoint.z = GetMax().z - rand() % uint32(float32(GetLengthZ()) * (-centeringWeight * 0.5f));
		}
		else if( centeringWeight > 0.0f )
		{
			rand() % 2 ?
				retPoint.z = GetCenterZ() + rand() % uint32(float32(GetLengthZ()) * (centeringWeight * 0.5f)) :
				retPoint.z = GetCenterZ() - rand() % uint32(float32(GetLengthZ()) * (centeringWeight * 0.5f));
		}
	}
	else
	{
		rand() % 2 ? 
			retPoint.z = GetMin().z : 
			retPoint.z = GetMax().z;
	}

	if( doZeroIgnoredAxes )
	{
		if( doIgnoreXAxis )
			retPoint.x = 0;
		if( doIgnoreYAxis )
			retPoint.y = 0;
		if( doIgnoreZAxis )
			retPoint.z = 0;
	}

	return retPoint;
}

UPoint3D
BaseArea::GetClosestPointInAreaToPoint( const UPoint3D& point ) const
{
	Assert( min_.x <= max_.x );
	Assert( min_.y <= max_.y );
	Assert( min_.z <= max_.z );

	UPoint3D retPoint;

	if( point.x <= min_.x )
		retPoint.x = min_.x;
	else if( point.x >= max_.x )
		retPoint.x = max_.x;
	else
		retPoint.x = point.x;

	if( point.y <= min_.y )
		retPoint.y = min_.y;
	else if( point.y >= max_.y )
		retPoint.y = max_.y;
	else
		retPoint.y = point.y;

	if( point.z <= min_.z )
		retPoint.z = min_.z;
	else if( point.z >= max_.z )
		retPoint.z = max_.z;
	else
		retPoint.z = point.z;

	return retPoint;
}

Direction
BaseArea::GetSideOfArea2DBorderPointIsOn( const UPoint3D& borderPoint ) const
{
	// Note- if the point is at a corner of the area the direction returned will be random

	// make sure point is on the border
	Assert( borderPoint.x == min_.x || borderPoint.x == max_.x || borderPoint.y == min_.y || borderPoint.y == max_.y );

	Direction retDirection;

	if( borderPoint.x == min_.x )
	{
		if( borderPoint.y == min_.y )
		{
			rand() % 2 ?
				retDirection = DIRECTION_SOUTH :
				retDirection = DIRECTION_WEST;
		}
		else if( borderPoint.y == max_.y )
		{
			rand() % 2 ?
				retDirection = DIRECTION_NORTH :
				retDirection = DIRECTION_WEST;
		}
		else
		{
			retDirection = DIRECTION_WEST;
		}
			
	}
	else if( borderPoint.x == max_.x )
	{
		if( borderPoint.y == min_.y )
		{
			rand() % 2 ?
				retDirection = DIRECTION_SOUTH :
				retDirection = DIRECTION_EAST;
		}
		else if( borderPoint.y == max_.y )
		{
			rand() % 2 ?
				retDirection = DIRECTION_NORTH :
				retDirection = DIRECTION_EAST;
		}
		else
		{
			retDirection = DIRECTION_EAST;
		}
	}
	else if( borderPoint.y == min_.y )
	{
		if( borderPoint.x == min_.x )
		{
			rand() % 2 ?
				retDirection = DIRECTION_SOUTH :
				retDirection = DIRECTION_WEST;
		}
		else if( borderPoint.x == max_.x )
		{
			rand() % 2 ?
				retDirection = DIRECTION_SOUTH :
				retDirection = DIRECTION_EAST;
		}
		else
		{
			retDirection = DIRECTION_SOUTH;
		}
	}
	else if( borderPoint.y == max_.y )
	{
		if( borderPoint.x == min_.x )
		{
			rand() % 2 ?
				retDirection = DIRECTION_NORTH :
				retDirection = DIRECTION_WEST;
		}
		else if( borderPoint.x == max_.x )
		{
			rand() % 2 ?
				retDirection = DIRECTION_NORTH :
				retDirection = DIRECTION_EAST;
		}
		else
		{
			retDirection = DIRECTION_NORTH;
		}
	}
	else
	{
		Assert( 0 );
	}

	return retDirection;
}