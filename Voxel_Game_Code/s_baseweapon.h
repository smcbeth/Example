//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: All weapons inherit from this.
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\point2d.h"

#include "s_baseitem.h"

class S_BaseWeapon: public S_BaseItem
{
public:
	S_BaseWeapon( );
	virtual ~S_BaseWeapon( );

	virtual void PrimaryAttack( ) = 0;
	virtual void SecondaryAttack( ) = 0;
};