//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "public\newport lib\aabox.h"
#include "serverentitylist.h"
#include "s_basefurniture.h"

class S_Chair: public S_BaseFurniture
{
public:
	S_Chair( );
	~S_Chair( );

	const char*	GetEntityClassName( );
};

LINK_SERVERENTITY_TO_CLASS( "chair", S_Chair );

S_Chair::S_Chair( )
{
	AABox* aaBox = new AABox;
	aaBox->SetSize( 2.0f, 2.0f, 2.0f ); // CTODO: make better approximation
	aaBox->SetMass( 0.0f );
	SetCollide( aaBox );
}

S_Chair::~S_Chair( )
{
	;
}

const char*
S_Chair::GetEntityClassName( )
{
	return "chair";
}