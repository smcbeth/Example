//---------- Copyright � 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: Representation of a single sound source.
//
//-----------------------------------------------------------------------------
#pragma once

#include "..\public\nstdlib\list.h"
#include "..\public\nstdlib\platform.h"
#include "..\public\nstdlib\_math.h"

#include "..\public\isoundsystem.h"
#include "..\public\isound.h"

#include "sounddata.h"
#include "mixer.h"
#include "oggdecoder.h"

class Sound: public ISound
{
public:
	Sound (SoundData* data, bool doConserveMem = false);
	~Sound ();

	const char*	GetName ();

	void    Play ();
	void    Resume ();
	void    Pause ();
	void    Stop ();

	void	SendNextBlock ();

	void	SetPositionalVolume (float x, float y, float z);
	void	SetPositionalVolumeFast (float x, float y, float z, float distance);

	// volume must be between or equal to 0 and 512
	void	SetLeftVolume (int32 volume);
	void	SetRightVolume (int32 volume);
	void	SetLeftAndRightVolume (int32 volume);

	int32	GetLeftVolume ();
	int32	GetRightVolume ();

	bool	GetIsPlaying ();

	void    SetDoConserveMem (bool b);

	void    Release ();
	bool    GetIsWaitingOnRelease ();

private:
	void Decode_ ();
	bool InitOggDecoder_ ();
	bool CloseOggDecoder_ ();

	SoundData* data_;

	stb_vorbis* oggDecoderHandle_;

	Mixer* mixer_;

	bool isPlaying_;
	bool oggDecoderIsInitialized_;
	bool doConserveMem_;
	bool isWaitingOnRelease_;

	int32 leftVolume_;
	int32 rightVolume_;
	int32 pcmBufferOffset_;

	float leftVolumeMult_;
	float rightVolumeMult_;

	int16* pcmBuffer_;
};