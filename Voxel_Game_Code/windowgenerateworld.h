//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\stools.h"

#include "public\newport lib\basewindow.h"
#include "public\newport lib\textbox.h"
#include "public\newport lib\button.h"

#include "public\iengine.h"

#include "worldgenerator.h"

// this should not change without putting some thought into it
#define STANDARD_WORLDDEPTH 8

class WindowGenerateWorld: public BaseWindow
{
public:
	enum
	{
		MSG_GW_GENERATE = 10000,
	};

	WindowGenerateWorld ();
	~WindowGenerateWorld ();

	//-------------------------------------------------------------------------
	// Inherited from 'IPanel'
	//-------------------------------------------------------------------------
	void ProcessMsg( KeyValues* msg, IPanel* sender );
};