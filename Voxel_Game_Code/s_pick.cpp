//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "public\nstdlib\platform.h"
#include "public\nstdlib\_math.h"
#include "public\nstdlib\gmath.h"
#include "public\newport lib\ray.h"
#include "public\newport lib\trianglesoup.h"
#include "public\iphysicssystem.h"
#include "serverentitylist.h"
#include "s_baseweapon.h"

class S_Pick: public S_BaseWeapon
{
public:
	S_Pick( );
	~S_Pick( );

	const char*	GetEntityClassName( );

	void		PrimaryAttack( );
	void		SecondaryAttack( );
};

LINK_SERVERENTITY_TO_CLASS( "pick", S_Pick );

S_Pick::S_Pick( )
{
	;
}

S_Pick::~S_Pick( )
{
	;
}

const char*
S_Pick::GetEntityClassName( )
{
	return "pick";
}

// CTODO: make it find out what nodes the ray intersects
void
S_Pick::PrimaryAttack( )
{
	/*
	//
	// set up our ray
	//
	Vector3D origin;
	GetOwner ()->GetPos (origin);

	Vector3D direction;
	direction.Init (0.0f, -1.0f, 0.0f);
	//direction.Normalize ();

	Ray ray( origin, direction, 6.0f );

	//
	// do the trace
	//
	TraceData data;
	PhysicsAPI ()->Trace (ray, data);

	//
	// handle the intersects
	//
	for (uint32 i = 0; i < data.collidesHit.GetCount(); ++i)
	{
		; // dont care atm
	}

	float32 closestTriangle[9];
	float32 closestDistance = 99999.0f;
	for (uint32 i = 0; i < data.trianglesHit.GetCount(); i += 9)
	{
		// CTODO: this is a shitty method but w/e
		float32 distance = Sqrt (Square (data.trianglesHit[ i ] - origin.x) + Square (data.trianglesHit[ i + 1 ] - origin.y) + Square (data.trianglesHit[ i + 2 ] - origin.z));

		if (distance < closestDistance)
		{
			for (uint32 j = 0; j < 9; ++j)
			{
				closestTriangle[ j ] = data.trianglesHit[ i + j ];
			}

			closestDistance = distance;
		}
	}
	*/
}

void
S_Pick::SecondaryAttack( )
{
	;
}