//---------- Copyright � 2012, Daedal Games, All rights reserved. -------------

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>			// 'Windows.h' must be included be 'mmsystem.h'
#include <mmsystem.h>
#include "nstdlib\platform.h"
#include "isystem.h"

#define BLOCK_SIZE 8820 // must be divisible by 4, 176400 equals 1 second of playback 8820 is 1/20th of a second

enum DynamicRange
{
	DYNAMICRANGE_NONE,
	DYNAMICRANGE_LOW,		// >= 2 simultaneous sounds before division takes place (instead of bit shifting)
	DYNAMICRANGE_MEDIUM,	// >= 4
	DYNAMICRANGE_HIGH,		// >= 8
	DYNAMICRANGE_EXTREME,	// >= 16
};

class ISound;
class Mixer;

class ISoundSystem: public ISystem
{
public:
	virtual ~ISoundSystem () { ; }

	virtual ISound*			GetOrLoadSound( const char* name ) = 0;

	virtual void			SetDeviceVolume( int32 leftPercentOfMax, int32 rightPercentOfMax ) = 0;
	virtual void			SetDevicePitch( int32 pitch ) = 0;
	virtual void			SetDevicePlaybackRate( int32 rate ) = 0;

	virtual void			StopAllSound( ) = 0;
	virtual void			PauseAllSound( ) = 0;
	virtual void			ResumeAllSound( ) = 0;

	virtual void			SetDynamicRange( DynamicRange dynamicRange ) = 0;
	virtual DynamicRange	GetDynamicRange( ) = 0;

	virtual void			QueueSound( ISound* sound ) = 0;
	virtual void			FinishedBlock( ) = 0;
	virtual void			IncrementPlayingBlockCount( ) = 0;

	virtual bool			GetIsSoundEnabled( ) = 0;
	virtual int32			GetMaxDevices( ) = 0;

	virtual HWAVEOUT		GetDevice( ) = 0;
	virtual Mixer*			GetMixer( ) = 0;
};

// Accessor
FORCE_INLINE ISoundSystem* SoundAPI( )
{
	extern ISoundSystem* _g_SoundSystemInstance;
	Assert( _g_SoundSystemInstance );
	return _g_SoundSystemInstance;
}