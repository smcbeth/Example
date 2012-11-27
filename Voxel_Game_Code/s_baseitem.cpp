//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "s_baseitem.h"

S_BaseItem::S_BaseItem( )
{
	owner_			= NULL;
	isInInventory_	= false;
	inventoryPosition_.Init();
}

S_BaseItem::~S_BaseItem( )
{
	;
}

void
S_BaseItem::SetOwner( IServerEntity* owner )
{
	owner_ = owner;
}

IServerEntity*
S_BaseItem::GetOwner( )
{
	return owner_;
}

bool
S_BaseItem::GetIsInInventory( )
{
	return isInInventory_;
}

void
S_BaseItem::SetIsInInventory( bool isInInventory )
{
	isInInventory_ = isInInventory;
}

void
S_BaseItem::GetInventoryPosition( Point2D& out )
{
	out.Init( inventoryPosition_ );
}

void
S_BaseItem::SetInventoryPosition( Point2D& position )
{
	inventoryPosition_.Init( position );
}