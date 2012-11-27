//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "s_basefurniture.h"
#include "definitions.h"

S_BaseFurniture::S_BaseFurniture( )
{
	;
}

S_BaseFurniture::~S_BaseFurniture( )
{
	;
}

void
S_BaseFurniture::SetBOrigin( const UPoint3D& bOrigin )
{
	/*
	SetOrigin( Vector3D(
		bOrigin.x * SCUBE_LENGTH,
		bOrigin.y * SCUBE_LENGTH,
		bOrigin.z * SCUBE_LENGTH
		) );
	*/

	// HACK: SNOW: temporary offset of bOrigin by 6 on x, y, and 7 on z to place in appropriate place.
	// STODO: even with this fix, z seems to need an additional offset of a few inches upward.

	SetTranslation(
		Vector3D(
			(bOrigin.x + 6) * SCUBE_LENGTH,
			(bOrigin.y + 6) * SCUBE_LENGTH,
			(bOrigin.z + 7) * SCUBE_LENGTH
		) );
}

//-----------------------------------------------------------------------------
// Each value is equal to a 90 degree ccw rotation about that axis
//-----------------------------------------------------------------------------
void
S_BaseFurniture::BRotate( uint32 x, uint32 y, uint32 z )
{
	;
}

void
S_BaseFurniture::BRotateX( uint32 x )
{
	;
}

void
S_BaseFurniture::BRotateY( uint32 y )
{
	;
}

void
S_BaseFurniture::BRotateZ( uint32 z )
{
	;
}