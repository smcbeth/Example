//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "public\newport lib\aabox.h"
#include "serverentitylist.h"
#include "s_basefurniture.h"

class S_Table: public S_BaseFurniture
{
public:
	S_Table( );
	~S_Table( );

	const char*	GetEntityClassName( );
};

LINK_SERVERENTITY_TO_CLASS( "table", S_Table );

S_Table::S_Table( )
{
	AABox* aaBox = new AABox;
	aaBox->SetSize( 3.0f, 3.0f, 3.0f ); // CTODO: make better approximation
	aaBox->SetMass( 0.0f );
	SetCollide( aaBox );
}

S_Table::~S_Table( )
{
	;
}

const char*
S_Table::GetEntityClassName( )
{
	return "table";
}