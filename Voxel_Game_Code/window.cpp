//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "window.h"
#include "terrainhelper.h"

Window::Window (UPoint3D& min, UPoint3D& max, WindowType type)
	: min_ (min), max_ (max), type_ (type)
{
	Assert (min_.x <= max_.x);
	Assert (min_.y <= max_.y);
	Assert (min_.z <= max_.z);
}

Window::~Window ()
{
	;
}

void
Window::Build ()
{
	for (uint32 z = min_.z; z <= max_.z; ++z)
	{
		for (uint32 y = min_.y; y <= max_.y; ++y)
		{
			for (uint32 x = min_.x; x <= max_.x; ++x)
			{
				uint32 sCubeBuf = 0;
				SetSCubeTerrainType (&sCubeBuf, TERRA_AIR);
				GetWorld()->SetSCube(x, y, z, sCubeBuf);
			}
		}
	}
}

UPoint3D&
Window::GetMin ()
{
	return min_;
}

UPoint3D&
Window::GetMax ()
{
	return max_;
}

WindowType
Window::GetWindowType ()
{
	return type_;
}