//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "c_baseentity.h"
#include "public\isocket.h"
#include "public\imaterialsystem.h"
#include "public\imesh.h"

C_BaseEntity::C_BaseEntity( )
{
	extents_.Init();
	doSerializeToNetwork_	= false;
	mesh_					= NULL;
	transform_.Identity();
	uid_					= 0;
	isDrawn_				= false;
}

C_BaseEntity::~C_BaseEntity( )
{
	;
}

void
C_BaseEntity::SetUID( uint32 uid )
{
	uid_ = uid;
}

uint32
C_BaseEntity::GetUID( )
{
	return uid_;
}

bool
C_BaseEntity::ShouldSerializeToNetwork( )
{
	return doSerializeToNetwork_;
}

void
C_BaseEntity::SerializeToNetwork( SocketMessage* socketMessage )
{
	socketMessage->Write( uid_ );
	doSerializeToNetwork_ = false;
}

void
C_BaseEntity::UnserializeFromNetwork( SocketMessage* socketMessage )
{
	socketMessage->Read( transform_ );

	socketMessage->Read( extents_.mins );
	socketMessage->Read( extents_.maxs );
}

const Extents3D&
C_BaseEntity::GetExtents( )
{
	return extents_;
}

void
C_BaseEntity::AddBacklink( ClientEntityNode* node )
{
	backlinks_.InsertBack( node );
}

ClientEntityNode*
C_BaseEntity::GetBacklink( uint32 idx )
{
	return backlinks_[ idx ];
}

uint32
C_BaseEntity::GetBacklinkCount( )
{
	return backlinks_.GetCount();
}

void
C_BaseEntity::Unlink( )
{
	while( backlinks_.GetCount() )
	{
		backlinks_.PopBack()->RemoveObject( this );
	}
}

void
C_BaseEntity::Spawn( )
{
	MaterialAPI()->AddEntity( this );
	// make sure we got linked to the material system octree
	AssertMsg( backlinks_.GetCount(), "Entity failed to link to Material System octree." );
}

void
C_BaseEntity::Update( )
{
	isDrawn_ = false;
}

void
C_BaseEntity::Despawn( )
{
	MaterialAPI()->RemoveEntity( this );
}

void
C_BaseEntity::SetMesh( IMesh* mesh )
{
	if( mesh_ != mesh )
	{
		SAFE_RELEASE( mesh_ );
		mesh_ = mesh;
	}
}

IMesh*
C_BaseEntity::GetMesh( )
{
	return mesh_;
}

const Matrix&
C_BaseEntity::GetTransform( ) const
{
	return transform_;
}

//-----------------------------------------------------------------------------
// Warning- This method may be called more than once.
//-----------------------------------------------------------------------------
void
C_BaseEntity::Draw( )
{
	// Note- This is the draw call for non-animated entities
	//       'BaseAnimated' contains the draw call for animated entities

	if( isDrawn_ )
	{
		return;
	}

	if( mesh_ )
	{
		// draw transformed mesh
		MaterialAPI()->PushMatrix( transform_ );
		mesh_->Draw();
		MaterialAPI()->PopMatrix();
	}

	isDrawn_ = true;
}