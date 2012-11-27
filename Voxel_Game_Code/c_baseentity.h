//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\nstdlib\matrix.h"
#include "public\nstdlib\gmath.h"
#include "public\icliententity.h"

class IMesh;

class C_BaseEntity: public IClientEntity
{
public:
	C_BaseEntity( );
	~C_BaseEntity( );

	void				SetMesh( IMesh* mesh );
	IMesh*				GetMesh( );

	const Matrix&		GetTransform( ) const;

	//-------------------------------------------------------------------------
	// Inherited from IClientEntity
	//-------------------------------------------------------------------------
	void				SetUID( uint32 uid );
	uint32				GetUID( );

	virtual bool		ShouldSerializeToNetwork( );
	virtual void		SerializeToNetwork( SocketMessage* socketMessage );
	virtual void		UnserializeFromNetwork( SocketMessage* socketMessage );

	const Extents3D&	GetExtents( );

	void				AddBacklink( ClientEntityNode* node );
	ClientEntityNode*	GetBacklink( uint32 idx );
	uint32				GetBacklinkCount( );
	void				Unlink( );

	virtual void		Spawn( );
	virtual void		Update( );
	virtual void		Despawn( );

	// Warning- This method may be called more than once.
	virtual void		Draw( );

protected:
	bool				doSerializeToNetwork_;
	Extents3D			extents_;				// extnets of a bounding box that completely encloses our mesh

private:
	IMesh*				mesh_;					// visual representation
	Matrix				transform_;				// position matrix, used to transform our model
	uint32				uid_;					// unique identifier

	ClientEntityLinks	backlinks_;				// octree back-links

	bool				isDrawn_;				// drawn this frame?
};