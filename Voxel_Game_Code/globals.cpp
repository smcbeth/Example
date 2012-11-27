//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "public\nstdlib\platform.h"

class IModule;
class IEngine;
class ITimeSystem;
class INetworkSystem;
class IFileSystem;
class IInputSystem;
class ISoundSystem;
class IPhysicsSystem;
class IMaterialSystem;
class IGuiSystem;
class IGameSystem;
class SCubeGeom;
class WorldGenerator;
class World;
class AIManager;

// module global
IModule*			_g_ModuleInstance			= NULL;

// engine global
IEngine*			_g_EngineInstance			= NULL;

// systems this module uses
ITimeSystem*		_g_TimeSystemInstance		= NULL;
INetworkSystem*		_g_NetworkSystemInstance	= NULL;
IFileSystem*		_g_FileSystemInstance		= NULL;
IInputSystem*		_g_InputSystemInstance		= NULL;
ISoundSystem*		_g_SoundSystemInstance		= NULL;
IPhysicsSystem*		_g_PhysicsSystemInstance	= NULL;
IMaterialSystem*	_g_MaterialSystemInstance	= NULL;
IGuiSystem*			_g_GuiSystemInstance		= NULL;

// scube geometry
SCubeGeom*			_g_SCubeGeomInstance		= NULL;

// CTODO: make this not global if possible
WorldGenerator*		_g_WorldGeneratorInstance	= NULL;

// world
World*				_g_WorldInstance			= NULL;

// AI manager
AIManager*			_g_AIManagerInstance		= NULL;