//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "module.h"

Module::Module( )
{
	isInitialized_	= false;
	doShutdown_		= false;
	isShutdown_		= false;
}

Module::~Module( )
{
	;
}

void
Module::Call( )
{
	if ( !isInitialized_ )
	{
		Initialize_();
	}
	else if( doShutdown_ )
	{
		Shutdown_();
	}
	else
	{
		Run_();
	}
}

void
Module::Shutdown( )
{
	doShutdown_ = true;
}

bool
Module::GetIsShutdown( )
{
	return isShutdown_;
}

void
Module::Initialize_( )
{
	// load engine dll
	engineDll_ = LoadLibrary( ENGINEDLL_RELPATH );

	// get engine instance
	extern IEngine* _g_EngineInstance;
	_g_EngineInstance = (IEngine*)GetSystemFromDLL( engineDll_ );

	// create window
	osWindow_ = new OSWindow;
	osWindow_->SetTitle( "Countermine Pre-Alpha" );
	osWindow_->SetIsVisible( true );
	
	// attach to engine
	EngineAPI()->AttachToWindow( osWindow_ );

	// get systems
	extern ITimeSystem*		_g_TimeSystemInstance;
	extern INetworkSystem*	_g_NetworkSystemInstance;
	extern IFileSystem*		_g_FileSystemInstance;
	extern IInputSystem*	_g_InputSystemInstance;
	extern ISoundSystem*	_g_SoundSystemInstance;
	extern IPhysicsSystem*	_g_PhysicsSystemInstance;
	extern IMaterialSystem*	_g_MaterialSystemInstance;
	extern IGuiSystem*		_g_GuiSystemInstance;

	_g_TimeSystemInstance		= (ITimeSystem*)EngineAPI()->GetSystem( "timesystem" );
	_g_NetworkSystemInstance	= (INetworkSystem*)EngineAPI()->GetSystem( "networksystem" );
	_g_FileSystemInstance		= (IFileSystem*)EngineAPI()->GetSystem( "filesystem" );
	_g_InputSystemInstance		= (IInputSystem*)EngineAPI()->GetSystem( "inputsystem" );
	_g_SoundSystemInstance		= (ISoundSystem*)EngineAPI()->GetSystem( "soundsystem" );
	_g_PhysicsSystemInstance	= (IPhysicsSystem*)EngineAPI()->GetSystem( "physicssystem" );
	_g_MaterialSystemInstance	= (IMaterialSystem*)EngineAPI()->GetSystem( "materialsystem" );
	_g_GuiSystemInstance		= (IGuiSystem*)EngineAPI()->GetSystem( "guisystem" );

	// create singletons
	extern World*		_g_WorldInstance;
	extern SCubeGeom*	_g_SCubeGeomInstance;
	extern AIManager*	_g_AIManagerInstance;

	_g_WorldInstance		= new World;
	_g_SCubeGeomInstance	= new SCubeGeom;
	_g_AIManagerInstance	= new AIManager;

	// start initial state
	EngineAPI()->RunState( new ClientStateMainMenu );

	isInitialized_ = true;
}

void
Module::Run_( )
{
	;
}

void
Module::Shutdown_( )
{
	// destroy singletons
	extern World*		_g_WorldInstance;
	extern SCubeGeom*	_g_SCubeGeomInstance;
	extern AIManager*	_g_AIManagerInstance;

	delete _g_WorldInstance;
	delete _g_SCubeGeomInstance;
	delete _g_AIManagerInstance;

	// release systems (reverse order)
	GuiAPI()->Release();
	MaterialAPI()->Release();
	PhysicsAPI()->Release();
	SoundAPI()->Release();
	InputAPI()->Release();
	FileAPI()->Release();
	NetworkAPI()->Release();
	TimeAPI()->Release();

	// free engine dll
	FreeLibrary( engineDll_ );

	// destroy OSWindow
	delete osWindow_;

	isShutdown_ = true;
}