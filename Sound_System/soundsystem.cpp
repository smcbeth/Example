//---------- Copyright � 2012, Daedal Games, All rights reserved. -------------

#include "soundsystem.h"
#include "..\public\nstdlib\stools.h"
#include "..\public\nstdlib\_string.h"
#include "..\public\interface.h"
#include "..\public\iengine.h"
#include "..\public\ifilesystem.h"
#include "..\public\istreamfile.h"
#include "..\public\isound.h"
#include "sound.h"
#include "mixer.h"

static void CALLBACK 
WaveOutProc( HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2  )
{
	// ignore calls that occur due to openining and closing the device. 
	if(uMsg != WOM_DONE )
	{
		return;
	}
 
	SoundAPI()->FinishedBlock();
}

SoundSystem::SoundSystem( )
{
	;
}

SoundSystem::~SoundSystem( )
{
	;
}

ISound*
SoundSystem::GetOrLoadSound( const char* name )
{
	// check cache
	for( uint32 i = 0; i < soundDatas_.GetCount(); ++i )
	{
		if( stools::IsEqual( soundDatas_[ i ]->name, name ) )
		{
			soundDatas_[ i ]->AddReference();
			return new Sound( soundDatas_[ i ] );
		}
	}

	// not found, create
	SoundData* soundData = new SoundData;
	char* relPath = new char[ MAX_PATH ];
	int dataSize;

	// read in data & get data size from file
	FileAPI()->GetResourceRelPath( name, RESOURCETYPE_SOUND, relPath );
	IStreamFile* file = FileAPI()->GetStreamFile( relPath, "rb" );
	if( !file )
	{
		EngineAPI()->Error( String( "Failed to load sound ", "'name'", "." ).CString( ), "SoundSystem::GetOrLoadSound(~)" );
		// TODO: use debug sound, or 'blank' sound
		FAssert();
	}
	dataSize = file->GetSize();
	soundData->oggBuffer = new unsigned char[ dataSize ];
	file->Read( soundData->oggBuffer, 1, dataSize );
	file->Release();

	stools::Copy( name, soundData->name );
	soundData->dataSize = dataSize;

	soundDatas_.InsertBack( soundData );
	soundData->AddReference();
	return new Sound( soundDatas_.GetBack() );
}

void
SoundSystem::StopAllSound( )
{
	isPlaying_ = false;

	while( soundPlayQueue_.Count() > 0 )
	{
		ISound* sound = soundPlayQueue_.PopFront();

		sound->Stop();

		if( sound->GetIsWaitingOnRelease() )
		{
			sound->Release();
		}
	}

	waveOutReset( device_ );
}

void
SoundSystem::PauseAllSound( )
{
	isPlaying_ = false;

	while( soundPlayQueue_.Count() > 0 )
	{
		ISound* soundInstance = soundPlayQueue_.PopFront();
		soundInstance->Pause();
		awaitingSoundPlayQueue_.InsertBack( soundInstance );
	}

	waveOutPause( device_ );
}

void
SoundSystem::ResumeAllSound( )
{
	waveOutRestart( device_ );

	while( awaitingSoundPlayQueue_.Count() > 0 )
	{
		ISound* soundInstance = awaitingSoundPlayQueue_.PopFront();
		soundInstance->Play();
	}

	if( soundPlayQueue_.Count() > 0 )
	{
		isPlaying_ = true;
	}
}

void
SoundSystem::SetDynamicRange( DynamicRange range )
{
	range_ = range;
}

DynamicRange
SoundSystem::GetDynamicRange( )
{
	return range_;
}

HWAVEOUT
SoundSystem::GetDevice( )
{
	return device_;
}

void 
SoundSystem::QueueSound( ISound* sound )
{
	Assert( sound != NULL );
	soundPlayQueue_.InsertBack( sound );
}

void
SoundSystem::FinishedBlock( )
{
	if( playingBlockCount_ > 0 )
	{
		--playingBlockCount_;
	}
	else if( playingBlockCount_ < 0 )
	{
		playingBlockCount_ = 0;
	}

	isPlaying_ = true;
}

void
SoundSystem::IncrementPlayingBlockCount( )
{
	++playingBlockCount_;
}

bool
SoundSystem::GetIsSoundEnabled( )
{
	return isSoundEnabled_;
}

Mixer* 
SoundSystem::GetMixer( )
{
	return mixer_;
}

int32
SoundSystem::GetMaxDevices( )
{
	return waveOutGetNumDevs();
}

void 
SoundSystem::SetDeviceVolume( int32 leftPercentOfMax, int32 rightPercentOfMax )
{
	Clamp( leftPercentOfMax, 0, 100 );
	Clamp( rightPercentOfMax, 0, 100 );
	
	int32 max = 0xFFFF;
	int32 leftTotal = (int32)((float64)max *( (float64)leftPercentOfMax / 100.0 ) );
	int32 rightTotal = (int32)((float64)max *( (float64)rightPercentOfMax / 100.0 ) );

	/*
	dwVolume

	Specifies a new volume setting. The low-order word contains the left-channel volume setting, 
	and the high-order word contains the right-channel setting. A value of 0xFFFF represents full volume, 
	and a value of 0x0000 is silence. If a device does not support both left and right volume control, 
	the low-order word of dwVolume specifies the volume level, and the high-order word is ignored.
	*/
	DWORD dwVolume = leftTotal;
	dwVolume <<= 32;
	dwVolume += rightTotal;

	waveOutSetVolume( device_, dwVolume );
}

void
SoundSystem::SetDevicePitch( long pitch )
{
	//check if device can set the pitch.
	WAVEOUTCAPS pwoc;
	waveOutGetDevCaps( (UINT)device_, &pwoc, sizeof( WAVEOUTCAPS ) );
	if( pwoc.dwSupport & WAVECAPS_PITCH ) 
	{
		/*
		dwPitch

		Specifies a new pitch multiplier setting. This setting indicates the current change in pitch from the original authored setting. 
		The pitch multiplier must be a positive value. The pitch multiplier is specified as a fixed-point value. 
		The high-order word contains the signed integer part of the number, and the low-order word contains the fractional part.
	
		x7FFF0000 = 32767.0
		x00018000 = 1.5
		x00010000 = 1.0
		x00008000 = 0.5
		x00000001 = ~0.000015258
		*/
		DWORD dwPitch = (DWORD) pitch;
		waveOutSetPitch( device_, dwPitch );
	}
}
void
SoundSystem::SetDevicePlaybackRate( long rate )
{
	//check if device can set the playback rate.
	WAVEOUTCAPS pwoc;
	waveOutGetDevCaps( (UINT)device_, &pwoc, sizeof( WAVEOUTCAPS ) );
	if( pwoc.dwSupport & WAVECAPS_PLAYBACKRATE ) 
	{
		/*
		dwRate

		Specifies a new playback rate setting. This setting is a multiplier indicating the current change in playback rate from the original authored setting. 
		The playback rate multiplier must be a positive value. The rate is specified as a fixed-point value. 
		The high-order word contains the signed integer part of the number, and the low-order word contains the fractional part. 
		*/
		DWORD dwRate = (DWORD) rate;
		waveOutSetPlaybackRate( device_, dwRate );
	}
}

//-----------------------------------------------------------------------------
//
// Inherited from ISystem
//
//-----------------------------------------------------------------------------
const char*
SoundSystem::GetName( )
{
	return "soundsystem";
}

uint32
SoundSystem::GetUpdateOrder( )
{
	return UPDATEORDER_SOUNDSYSTEM;
}

void
SoundSystem::Connect()
{
	// load engine dll
	engineDll_ = LoadLibrary( ENGINEDLL_RELPATH  );

	// get engine
	extern IEngine* _g_EngineInstance;
	_g_EngineInstance =( IEngine* )GetSystemFromDLL( engineDll_  );

	// get systems
	extern IFileSystem* _g_FileSystemInstance;
	_g_FileSystemInstance =( IFileSystem* )EngineAPI()->GetSystem( "filesystem" );
}

void
SoundSystem::Disconnect( )
{
	// release systems
	FileAPI()->Release( );

	// free engine dll
	FreeLibrary( engineDll_ );
}

void
SoundSystem::AttachToWindow( OSWindow* osWindow )
{
	;
}

void
SoundSystem::Initialize( )
{
	extern ISoundSystem* _g_SoundSystemInstance;
	_g_SoundSystemInstance = this;

	playingBlockCount_ = 0;

	range_ = DYNAMICRANGE_MEDIUM;

	isPlaying_ = false;
	isSoundEnabled_ = true;

	mixer_ = new Mixer;

	WAVEFORMATEX wfx;
	wfx.nChannels		= 2;
	wfx.nSamplesPerSec	= 44100;
	wfx.wBitsPerSample	= 16;
	wfx.nBlockAlign		= 4;
	wfx.cbSize			= 0;
	wfx.wFormatTag		= 1;
	wfx.nAvgBytesPerSec	= 176400;

	MMRESULT mmResult = waveOutOpen( &device_, WAVE_MAPPER, &wfx,( DWORD_PTR )WaveOutProc, NULL, CALLBACK_FUNCTION );
	if( mmResult != MMSYSERR_NOERROR )
	{
		isSoundEnabled_ = false;
		EngineAPI()->Error( "Unable to open wave mapper device, sound will be disabled.", "Initialize(~)" );
	}
}

void
SoundSystem::Shutdown( )
{
	StopAllSound();
	RunGarbageCollection();
	delete mixer_;
	waveOutClose( device_ );

	Assert( !soundDatas_.GetCount() );
}

void
SoundSystem::Update( )
{
	ISound* sound;
	int32 soundPlayQueueCount;

	if( !isPlaying_ || playingBlockCount_ == 2 )
	{
		return;
	}

	soundPlayQueueCount = soundPlayQueue_.Count();

	if( soundPlayQueueCount == 0 )
	{
		isPlaying_ = false;
		return;
	}

	for( int32 i = 0; i < soundPlayQueueCount; ++i )
	{
		sound = soundPlayQueue_.PopFront();
		sound->SendNextBlock();
        
        // this statement is true during a stutter in block playback
		if( playingBlockCount_ == 0 ) 
		{
			mixer_->PrepareAndPlayNextBlock();
			sound->SendNextBlock();
		}

		if( sound->GetIsPlaying() )
		{
			QueueSound( sound );
		}
		else if( sound->GetIsWaitingOnRelease() )
		{
            // allows sounds to playback in full before deletion
			sound->Release(); 
		}
	}

	mixer_->PrepareAndPlayNextBlock();
}

void
SoundSystem::RunGarbageCollection( )
{
	for( uint32 i = 0; i < soundDatas_.GetCount(); ++i )
	{
		if( !soundDatas_[ i ]->GetReferenceCount() )
		{
			soundDatas_.RemoveAndDelete( i );
		}
	}
}

void
SoundSystem::Release( )
{
	RemoveReference();
}