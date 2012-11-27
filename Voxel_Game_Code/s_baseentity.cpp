//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "s_baseentity.h"
#include "public\nstdlib\stools.h"
#include "public\nstdlib\vector3d.h"
#include "public\nstdlib\matrix.h"
#include "public\nstdlib\gmath.h"
#include "public\nstdlib\octree.h"
#include "public\newport lib\aabox.h"
#include "public\newport lib\box.h"
#include "public\newport lib\trianglesoup.h"
#include "public\icollide.h"
#include "public\iengine.h"
#include "public\isocket.h"
#include "public\istreamfile.h"
#include "public\iengine.h"
#include "public\imaterialsystem.h"
#include "public\imesh.h"

S_BaseEntity::S_BaseEntity( )
{
	doSerializeToNetwork_	= false;
	nextThink_				= THINK_NEVER;
	collide_				= NULL;
	uid_					= EngineAPI()->NewUID();
}

S_BaseEntity::~S_BaseEntity( )
{
	Unlink();
	SetCollide( NULL );
	EngineAPI()->FreeUID( uid_ );
}

uint32
S_BaseEntity::GetUID( )
{
	return uid_;
}

void
S_BaseEntity::Update( )
{
	;
}

void
S_BaseEntity::Think( )
{
	;
}

void
S_BaseEntity::SetNextThink( float32 deltaTime )
{
	nextThink_ = deltaTime;
}

float32
S_BaseEntity::GetNextThink( )
{
	return nextThink_;
}

bool
S_BaseEntity::ShouldSerializeToNetwork( )
{
	return doSerializeToNetwork_;
}

void
S_BaseEntity::SerializeToNetwork( SocketMessage* socketMessage )
{
	socketMessage->Write( GetUID() );

	if( collide_ )
	{
		// send transform matrix
		socketMessage->Write( collide_->GetTransform() );

		// send extents
		socketMessage->Write( collide_->GetExtents().mins );
		socketMessage->Write( collide_->GetExtents().maxs );
	}
	else
	{
		Matrix matrix;
		matrix.Identity();
		socketMessage->Write( matrix );

		socketMessage->Write( Vector3D( 0.0f, 0.0f, 0.0f ) );
		socketMessage->Write( Vector3D( 0.0f, 0.0f, 0.0f ) );
	}

	doSerializeToNetwork_ = false;
}

void
S_BaseEntity::UnserializeFromNetwork( SocketMessage* socketMessage )
{
	;
}

void
S_BaseEntity::SerializeToHDD( IStreamFile* file )
{
	// serialize entity name
	file->Write( GetEntityClassName() );

	// serialize collide
	if( collide_ )
	{
		file->Write( (int32)collide_->GetType() );
		collide_->SerializeToHDD( file );
	}
	else
	{
		file->Write( (int32)COLLIDETYPE_NONE );
	}
}

void
S_BaseEntity::UnserializeFromHDD( IStreamFile* file )
{
	// unserialize collide
	int32 collideType;
	file->Read( collideType );
	if( collideType == COLLIDETYPE_AABOX )
	{
		ICollide* collide = new AABox;
		collide->UnserializeFromHDD( file );
		SetCollide( collide );
	}
	else if( collideType == COLLIDETYPE_BOX )
	{
		ICollide* collide = new Box;
		collide->UnserializeFromHDD( file );
		SetCollide( collide );
	}
	else if( collideType == COLLIDETYPE_TRIANGLESOUP )
	{
		ICollide* collide = new TriangleSoup;
		collide->UnserializeFromHDD( file );
		SetCollide( collide );
	}
	else if( collideType != COLLIDETYPE_NONE )
	{
		EngineAPI()->Error( "Unrecognised collide type.", "S_BaseEntity::UnserializeFromHDD(~)" );
	}
}

void
S_BaseEntity::Spawn( )
{
	if( GetCollide() )
	{
		GetCollide()->SetIsSimulated( true );
	}
}

void
S_BaseEntity::Despawn( )
{
	if( GetCollide() )
	{
		GetCollide()->SetIsSimulated( false );
	}
}

void
S_BaseEntity::SetTranslation( const Vector3D& origin )
{
	if( GetCollide() )
	{
		GetCollide()->SetTranslation( origin );
	}
}

Vector3D
S_BaseEntity::GetTranslation( )
{
	if( GetCollide() )
	{
		return GetCollide()->GetTranslation( );
	}

	return Vector3D( 0.0f, 0.0f, 0.0f );
}

float32
S_BaseEntity::GetTranslationX( )
{
	if( GetCollide() )
	{
		return GetCollide()->GetTranslationX();
	}

	return 0.0f;
}

float32
S_BaseEntity::GetTranslationY( )
{
	if( GetCollide() )
	{
		return GetCollide()->GetTranslationY();
	}

	return 0.0f;
}

float32
S_BaseEntity::GetTranslationZ( )
{
	if( GetCollide() )
	{
		return GetCollide()->GetTranslationZ();
	}

	return 0.0f;
}

void
S_BaseEntity::SetRotationYawPitchRoll( float32 yaw, float32 pitch, float32 roll )
{
	if( collide_ )
	{
		collide_->SetRotationYawPitchRoll( yaw, pitch, roll );
	}
}

void
S_BaseEntity::RotateX( float32 x )
{
	if( collide_ )
	{
		collide_->RotateX( x );
	}
}

void
S_BaseEntity::RotateY( float32 y )
{
	if( collide_ )
	{
		collide_->RotateY( y );
	}
}

void
S_BaseEntity::RotateZ( float32 z )
{
	if( collide_ )
	{
		collide_->RotateZ( z );
	}
}

void
S_BaseEntity::AddBacklink( ServerEntityNode* node )
{
	backlinks_.InsertBack( node );
}

ServerEntityNode*
S_BaseEntity::GetBacklink( uint32 idx )
{
	return backlinks_[ idx ];
}

uint32
S_BaseEntity::GetBacklinkCount( )
{
	return backlinks_.GetCount();
}

void
S_BaseEntity::Unlink( )
{
	while( backlinks_.GetCount() )
	{
		backlinks_.PopBack()->RemoveObject( this );
	}
}

void
S_BaseEntity::SetCollide( ICollide* collide )
{
	if( collide_ == collide )
	{
		return;
	}

	SAFE_RELEASE( collide_ );
	collide_ = collide;
}

ICollide*
S_BaseEntity::GetCollide( )
{
	return collide_;
}