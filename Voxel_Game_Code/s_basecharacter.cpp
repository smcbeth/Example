//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "s_basecharacter.h"
#include "public\nstdlib\stools.h"
#include "public\istreamfile.h"
#include "serverentitylist.h"
#include "s_baseitem.h"
#include "s_basearmor.h"
#include "s_baseweapon.h"
#include "s_basemobile.h"

S_BaseCharacter::S_BaseCharacter( )
{
	head_			= NULL;
	chest_			= NULL;
	leftHand_		= NULL;
	rightHand_		= NULL;

	vitality_		= 100;
	stamina_		= 100;
	mana_			= 100;
	maxVitality_	= 100;
	maxStamina_		= 100;
	maxMana_		= 100;
}

S_BaseCharacter::~S_BaseCharacter( )
{
	SAFE_DELETE( head_ );
	SAFE_DELETE( chest_ );
	SAFE_DELETE( leftHand_ );
	SAFE_DELETE( rightHand_ );
}

//-----------------------------------------------------------------------------
//
// Inherited from BaseEntity
//
//-----------------------------------------------------------------------------
void
S_BaseCharacter::Think( )
{
	S_BaseMobile::Think();
}

void
S_BaseCharacter::SerializeToHDD( IStreamFile* file )
{
	S_BaseMobile::SerializeToHDD( file );

	// serialize equipment
	if( head_ )
	{
		head_->SerializeToHDD( file );
	}
	else
	{
		file->Write( "Nothing" );
	}

	if( chest_ )
	{
		chest_->SerializeToHDD( file );
	}
	else
	{
		file->Write( "Nothing" );
	}

	if( leftHand_ )
	{
		leftHand_->SerializeToHDD( file );
	}
	else
	{
		file->Write( "Nothing" );
	}

	if( rightHand_ )
	{
		rightHand_->SerializeToHDD( file );
	}
	else
	{
		file->Write( "Nothing" );
	}

	// serialize vitals
	file->Write( vitality_ );
	file->Write( maxVitality_ );
	file->Write( stamina_ );
	file->Write( maxStamina_ );
	file->Write( mana_ );
	file->Write( maxMana_ );
}

void
S_BaseCharacter::UnserializeFromHDD( IStreamFile* file )
{
	S_BaseMobile::UnserializeFromHDD( file );

	// unserialize equipment
	char stringBuffer[ MAX_ITEMNAME ];

	file->Read( stringBuffer );
	if( !stools::IsEqual( "Nothing", stringBuffer ) )
	{
		head_ = (S_BaseArmor*)GetServerEntityList()->Create( stringBuffer );
		head_->UnserializeFromHDD( file );
		head_->SetOwner( this );
	}

	file->Read( stringBuffer );
	if( !stools::IsEqual( "Nothing", stringBuffer ) )
	{
		chest_ = (S_BaseArmor*)GetServerEntityList()->Create( stringBuffer );
		chest_->UnserializeFromHDD( file );
		chest_->SetOwner( this );
	}

	file->Read( stringBuffer );
	if( !stools::IsEqual( "Nothing", stringBuffer ) )
	{
		leftHand_ = (S_BaseWeapon*)GetServerEntityList()->Create( stringBuffer );
		leftHand_->UnserializeFromHDD( file );
		leftHand_->SetOwner( this );
	}

	file->Read( stringBuffer );
	if( !stools::IsEqual( "Nothing", stringBuffer ) )
	{
		rightHand_ = (S_BaseWeapon*)GetServerEntityList()->Create( stringBuffer );
		rightHand_->UnserializeFromHDD( file );
		rightHand_->SetOwner( this );
	}

	// unserialize vitals
	file->Read( vitality_ );
	file->Read( maxVitality_ );
	file->Read( stamina_ );
	file->Read( maxStamina_ );
	file->Read( mana_ );
	file->Read( maxMana_ );
}

void
S_BaseCharacter::Spawn( )
{
	S_BaseMobile::Spawn();
}

void
S_BaseCharacter::Despawn( )
{
	S_BaseMobile::Despawn();
}

void
S_BaseCharacter::Attack( )
{
	if( rightHand_ )
	{
		rightHand_->PrimaryAttack();
	}
}

// Will place the item 'somewhere' in the players inventory
void
S_BaseCharacter::AddInventoryItem( S_BaseItem* item )
{
	FAssert();
}

S_BaseItem*
S_BaseCharacter::GetInventoryItem( const Point2D& pos )
{
	FAssert();
	return NULL;
}

void
S_BaseCharacter::EquipWeapon( S_BaseWeapon* weapon, EquipLocation location )
{
	weapon->SetOwner( this );

	switch( location )
	{
	case EQUIP_LEFTHAND:
		leftHand_ = weapon;
		break;

	case EQUIP_RIGHTHAND:
		rightHand_ = weapon;
		break;

	NO_DEFAULT
	}
}

S_BaseItem*
S_BaseCharacter::GetEquippedItem( EquipLocation location )
{
	FAssert();
	return NULL;
}

//-----------------------------------------------------------------------------
// Vitals
//-----------------------------------------------------------------------------
void
S_BaseCharacter::SetVitality( int32 vitality )
{
	vitality_ = vitality;
}

int32
S_BaseCharacter::GetVitality( )
{
	return vitality_;
}

void
S_BaseCharacter::SetMaxVitality( int32 maxVitality )
{
	maxVitality_ = maxVitality;
}

int32
S_BaseCharacter::GetMaxVitality( )
{
	return maxVitality_;
}

void
S_BaseCharacter::SetStamina( int32 stamina )
{
	stamina_ = stamina;
}

int32
S_BaseCharacter::GetStamina( )
{
	return stamina_;
}

void
S_BaseCharacter::SetMaxStamina( int32 maxStamina )
{
	maxStamina_ = maxStamina;
}

int32
S_BaseCharacter::GetMaxStamina( )
{
	return maxStamina_;
}

void
S_BaseCharacter::SetMana( int32 mana )
{
	mana_ = mana;
}

int32
S_BaseCharacter::GetMana( )
{
	return mana_;
}

void
S_BaseCharacter::SetMaxMana( int32 maxMana )
{
	maxMana_ = maxMana;
}

int32
S_BaseCharacter::GetMaxMana( )
{
	return maxMana_;
}