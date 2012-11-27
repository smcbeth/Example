//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "public\nstdlib\platform.h"
#include "public\imaterialsystem.h"
#include "c_baseentity.h"
#include "cliententitylist.h"

class C_Chair: public C_BaseEntity
{
public:
	C_Chair( );
	~C_Chair( );

	const char*	GetEntityClassName( );

	void		Update( );

	void		Spawn( );
	void		Despawn( );
};

LINK_CLIENTENTITY_TO_CLASS( "chair", C_Chair );

C_Chair::C_Chair( )
{
	;
}

C_Chair::~C_Chair( )
{
	;
}

const char*
C_Chair::GetEntityClassName( )
{
	return "chair";
}

void
C_Chair::Update( )
{
	C_BaseEntity::Update();
}

void
C_Chair::Spawn( )
{
	C_BaseEntity::Spawn();

	SetMesh( MaterialAPI()->GetOrLoadMesh( "chair" ) );
}

void
C_Chair::Despawn( )
{
	C_BaseEntity::Despawn();
}