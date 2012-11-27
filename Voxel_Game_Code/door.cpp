//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "door.h"

Door::Door (UPoint3D& min, UPoint3D& max, Direction side, DoorType type, TERRATYPE floorMaterial, uint32 ceilingZLevel)
	: min_ (min), max_ (max), side_ (side), type_ (type), floorMaterial_ (floorMaterial), ceilingZLevel_ (ceilingZLevel)
{
	Assert (min_.x <= max_.x);
	Assert (min_.y <= max_.y);
	Assert (min_.z <= max_.z);
}

Door::~Door ()
{
	;
}

void
Door::Build ()
{
	uint32 z = min_.z - 1;
	for (uint32 y = min_.y; y <= max_.y; ++y)
	{
		for (uint32 x = min_.x; x <= max_.x; ++x)
		{
			uint32 sCubeBuf = 0;
			SetSCubeTerrainType (&sCubeBuf, floorMaterial_);
			GetWorld()->SetSCube(x, y, z, sCubeBuf);
		}
	}
	for (z = min_.z; z <= max_.z; ++z)
	{
		for (uint32 y = min_.y; y <= max_.y; ++y)
		{
			for (uint32 x = min_.x; x <= max_.x; ++x)
			{
				uint32 sCubeBuf = 0;
				
				SetSCubeTerrainType (&sCubeBuf, TERRA_AIR);
				GetWorld()->SetSCube(x, y, z, sCubeBuf);

				if (z == ceilingZLevel_ - 1)
				{
					// prevents texture clash on doorways that extend to the ceiling
					sCubeBuf = 0;
					SetSCubeTerrainType (&sCubeBuf, floorMaterial_);
					GetWorld()->SetSCube(x, y, z + 1, sCubeBuf);
				}
			}
		}
	}
}

UPoint3D&
Door::GetMin ()
{
	return min_;
}

UPoint3D&
Door::GetMax ()
{
	return max_;
}

Direction
Door::GetSide ()
{
	return side_;
}

DoorType
Door::GetType ()
{
	return type_;
}

TERRATYPE
Door::GetFloorMaterial ()
{
	return floorMaterial_;
}