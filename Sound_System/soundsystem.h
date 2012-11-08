//---------- Copyright � 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: Manager for all sound sources.
//
//-----------------------------------------------------------------------------
#pragma once

#include "..\public\nstdlib\platform.h"
#include "..\public\nstdlib\vector.h"
#include "..\public\nstdlib\list.h"
#include "..\public\isoundsystem.h"

class SoundData;

// TODO: change the lists to vectors
class SoundSystem: public ISoundSystem
{
public:
	SoundSystem( );
	~SoundSystem( );

	ISound*			GetOrLoadSound( const char* name );

	void			SetDeviceVolume( int32 leftPercentOfMax, int32 rightPercentOfMax );
	void			SetDevicePitch( int32 pitch );
	void			SetDevicePlaybackRate( int32 rate );

	void			StopAllSound( );
	void			PauseAllSound( );
	void			ResumeAllSound( );

	void			SetDynamicRange( DynamicRange dynamicRange );
	DynamicRange	GetDynamicRange( );

	void			QueueSound( ISound* sound );
	void			FinishedBlock( );
	void			IncrementPlayingBlockCount( );

	bool			GetIsSoundEnabled( );
	int32			GetMaxDevices( );

	HWAVEOUT		GetDevice( );
	Mixer*			GetMixer( );

	//-------------------------------------------------------------------------
	// Inherited from ISystem
	//-------------------------------------------------------------------------
	const char*		GetName( );
	uint32			GetUpdateOrder( );
	void			Connect( );
	void			Disconnect( );
	void			AttachToWindow( OSWindow* osWindow );
	void			Initialize( );
	void			Shutdown( );
	void			Update( );
	void			RunGarbageCollection( );
	void			Release( );

private:
	HWAVEOUT				device_;

	Vector< SoundData* >	soundDatas_;

	List< ISound* >			soundPlayQueue_;
	List< ISound* >			awaitingSoundPlayQueue_;

	Mixer*					mixer_;

	int32					playingBlockCount_;
	DynamicRange			range_;

	bool					isPlaying_;
	bool					isSoundEnabled_;	

	HMODULE					engineDll_;
};