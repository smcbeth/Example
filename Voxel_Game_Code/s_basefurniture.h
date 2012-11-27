//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\point3d.h"
#include "s_baseentity.h"

class S_BaseFurniture: public S_BaseEntity
{
public:
	S_BaseFurniture( );
	~S_BaseFurniture( );

	void SetBOrigin( const UPoint3D& bOrigin );

	// Each value is equal to a 90 degree ccw rotation about that axis
	void BRotate( uint32 x, uint32 y, uint32 z );
	void BRotateX( uint32 x );
	void BRotateY( uint32 y );
	void BRotateZ( uint32 z );
};