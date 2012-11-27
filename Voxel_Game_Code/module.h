//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: Client module.
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"

#include "public\newport lib\oswindow.h"
#include "public\newport lib\imodule.h"

#include "public\interface.h"
#include "public\iengine.h"
#include "public\itimesystem.h"
#include "public\inetworksystem.h"
#include "public\ifilesystem.h"
#include "public\iinputsystem.h"
#include "public\isoundsystem.h"
#include "public\iphysicssystem.h"
#include "public\imaterialsystem.h"
#include "public\iguisystem.h"

// singletons
#include "world.h"
#include "scubegeom.h"
#include "aimanager.h"

// initial state
#include "clientstatemainmenu.h"

class Module: public IModule
{
public:
	Module( );
	~Module( );

	void Call( );

	void Shutdown( );
	bool GetIsShutdown( );

private:
	void Initialize_( );
	void Run_( );
	void Shutdown_( );


	OSWindow*	osWindow_;
	HMODULE		engineDll_;

	bool isInitialized_;
	bool doShutdown_;
	bool isShutdown_;
};