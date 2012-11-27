//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\point2d.h"

#include "iserverentity.h"
#include "s_baseentity.h"

#define MAX_ITEMNAME 32

class S_BaseItem: public S_BaseEntity
{
public:
	S_BaseItem( );
	~S_BaseItem( );

	void			SetOwner( IServerEntity* owner);
	IServerEntity*	GetOwner( );

	void			SetIsInInventory( bool isInInventory );
	bool			GetIsInInventory( );

	void			SetInventoryPosition( Point2D& position );
	void			GetInventoryPosition( Point2D& out );

private:
	IServerEntity*	owner_;

	bool			isInInventory_;
	Point2D			inventoryPosition_;
};