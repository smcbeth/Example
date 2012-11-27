//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: Countermine Client
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\nstdlib\_windows.h"
#include "public\nstdlib\_math.h"

#include "public\newport lib\imodule.h"

#include "public\interface.h"

#include "module.h"

extern IModule* _g_ModuleInstance;

DLLEXPORT void*
GetModule( )
{
	return (void*)_g_ModuleInstance;
}

BOOL WINAPI
DllMain( HINSTANCE hinstDLL, DWORD fwdReason, LPVOID lpvReserved )
{
	static uint32 referenceCount = 0;

	if( !referenceCount )
	{
		// init the math part of the support lib
		InitMathLib();

		// create module instance
		_g_ModuleInstance = new Module;
	}

	switch( fwdReason )
	{
	case DLL_PROCESS_ATTACH:
		++referenceCount;
		break;

	case DLL_PROCESS_DETACH:
		--referenceCount;
		break;
	}

	if( !referenceCount )
	{
		// destroy module instance
		delete _g_ModuleInstance;
	}

	return TRUE;
}