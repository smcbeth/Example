//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\stools.h"
#include "public\nstdlib\keyvalues.h"

#include "public\newport lib\ipanel.h"
#include "public\newport lib\basewindow.h"
#include "public\newport lib\textbox.h"
#include "public\newport lib\button.h"
#include "public\newport lib\imodule.h"

#include "public\imaterialsystem.h"
#include "public\iguisystem.h"

#include "windowgenerateworld.h"
#include "clientstateconnect.h"
#include "serverstateplay.h"
#include "module.h"

class WindowMainMenu: public BaseWindow
{
public:
	enum
	{
		MSG_MM_GENWORLD = 10000,
		MSG_MM_NEWGAME,
		MSG_MM_QUIT
	};


	WindowMainMenu( );
	~WindowMainMenu( );

	//-------------------------------------------------------------------------
	// Inherited from 'IPanel'
	//-------------------------------------------------------------------------
	void ProcessMsg( KeyValues* msg, IPanel* sender );

private:
	WindowGenerateWorld* windowGenerateWorld_;
};