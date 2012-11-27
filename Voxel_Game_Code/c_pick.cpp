//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "public\nstdlib\platform.h"
#include "c_baseentity.h"
#include "cliententitylist.h"

class C_Pick: public C_BaseEntity
{
public:
	C_Pick( );
	~C_Pick( );

	const char*	GetEntityClassName( );

	void		Update( );

	void		Spawn( );
	void		Despawn( );
};

LINK_CLIENTENTITY_TO_CLASS( "pick", C_Pick );

C_Pick::C_Pick( )
{
	;
}

C_Pick::~C_Pick( )
{
	;
}

const char*
C_Pick::GetEntityClassName( )
{
	return "pick";
}

void
C_Pick::Update( )
{
	C_BaseEntity::Update();
}

void
C_Pick::Spawn( )
{
	C_BaseEntity::Spawn();

	// CTODO:
	// SetMesh( MaterialAPI()->GetOrLoadMesh( "pick" ) );
}

void
C_Pick::Despawn( )
{
	C_BaseEntity::Despawn();
}