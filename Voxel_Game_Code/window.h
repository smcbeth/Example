//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\gmath.h"

#include "world.h"

enum WindowType
{
	WINDOW_NORMAL = 0,
	WINDOW_ARROWSLIT,
	WINDOW_BROKEN,
	WINDOW_STAINED
};

class Window
{
public:
	Window (UPoint3D& min, UPoint3D& max, WindowType type);
	~Window ();

	void Build ();

	UPoint3D& GetMin ();
	UPoint3D& GetMax ();
	WindowType GetWindowType ();

private:
	UPoint3D min_;
	UPoint3D max_;
	WindowType type_;
};