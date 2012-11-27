//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "public\nstdlib\platform.h"
#include "public\newport lib\force.h"
#include "public\newport lib\aabox.h"
#include "public\iengine.h"
#include "public\imaterialsystem.h"
#include "public\imesh.h"
#include "serverentitylist.h"
#include "s_basemobile.h"

#define CHICKEN_WIDTH_DEPTH	2.2f
#define CHICKEN_HEIGHT		1.4f

class S_Chicken: public S_BaseMobile
{
public:
	S_Chicken( );
	~S_Chicken( );

	const char*	GetEntityClassName( );

	void		Think( );
};

LINK_SERVERENTITY_TO_CLASS( "chicken", S_Chicken );

S_Chicken::S_Chicken( )
{
	AABox* aabox = new AABox;
	aabox->SetSize( CHICKEN_WIDTH_DEPTH, CHICKEN_HEIGHT, CHICKEN_WIDTH_DEPTH );
	SetCollide( aabox );

	Force* force = new Force;
	force->SetMaxVelocity( 3.0f );
	SetMoveForce( force );
}

S_Chicken::~S_Chicken( )
{
	;
}

const char*
S_Chicken::GetEntityClassName( )
{
	return "chicken";
}

void
S_Chicken::Think( )
{
	S_BaseMobile::Think();

	SetMovingForward( true );

	// do chicken stuff
	SetNextThink( FRandom( 0.5f, 1.0f ) );
}