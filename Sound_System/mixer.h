//---------- Copyright � 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: Allows multiple sounds to play with a single device handle.
//
//-----------------------------------------------------------------------------
#pragma once

#include "..\public\isoundsystem.h"

#include "..\public\nstdlib\list.h"
#include "..\public\nstdlib\platform.h"

#include "sounddata.h"

class Mixer
{
public:
	Mixer ();
	~Mixer ();

	void PrepareAndPlayNextBlock ();
	void AddUnmixedBlock (short* blockData, int percentLeftVolume, int percentRightVolume);

	bool IsPlaying ();

private:
	void Mix ();

	List<short*> unmixedBlocks_;
	List<int> unmixedBlocksLeftVolume_;
	List<int> unmixedBlocksRightVolume_;

	bool doUseOne_;
	bool isPlaying_;

	short* blockData1_;
	short* blockData2_;
	WAVEHDR blockHdr1_;
	WAVEHDR blockHdr2_;
};