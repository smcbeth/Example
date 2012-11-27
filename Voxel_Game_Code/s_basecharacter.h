//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: Anything that has vitals, stats, an inventory and skills inherits
//          from this.
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "s_basemobile.h"

class S_BaseItem;
class S_BaseArmor;
class S_BaseWeapon;

enum EquipLocation
{
	EQUIP_UNKNOWN,
	EQUIP_HEAD,
	EQUIP_CHEST,
	EQUIP_LEFTHAND,
	EQUIP_RIGHTHAND
};

class S_BaseCharacter: public S_BaseMobile
{
public:
	S_BaseCharacter( );
	virtual ~S_BaseCharacter( );

	void			AddInventoryItem( S_BaseItem* item );
	S_BaseItem*		GetInventoryItem( const Point2D& pos );

	void			EquipWeapon( S_BaseWeapon* weapon, EquipLocation location );
	S_BaseItem*		GetEquippedItem( EquipLocation location );

	// Vitals
	void			SetVitality( int32 vitality );
	int32			GetVitality( );
	void			SetMaxVitality( int32 maxVitality );
	int32			GetMaxVitality( );

	void			SetStamina( int32 stamina );
	int32			GetStamina( );
	void			SetMaxStamina( int32 maxStamina );
	int32			GetMaxStamina( );

	void			SetMana( int32 mana );
	int32			GetMana( );
	void			SetMaxMana( int32 maxMana );
	int32			GetMaxMana( );

	//-------------------------------------------------------------------------
	// Inherited from S_BaseMobile
	//-------------------------------------------------------------------------
	virtual void	Think( );

	virtual void	SerializeToHDD( IStreamFile* file );
	virtual void	UnserializeFromHDD( IStreamFile* file );

	virtual void	Spawn( );
	virtual void	Despawn( );

	void			Attack( );

private:
	Vector< S_BaseItem* > inventory_;
	
	// equipment slots
	S_BaseArmor*	head_;
	S_BaseArmor*	chest_;
	S_BaseWeapon*	leftHand_;
	S_BaseWeapon*	rightHand_;

	// vitals
	int32			vitality_;
	int32			maxVitality_;
	int32			stamina_;
	int32			maxStamina_;
	int32			mana_;
	int32			maxMana_;

	Point2D			inventorySize_;
};