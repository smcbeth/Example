//---------- Copyright � 2012, Daedal Games, All rights reserved. -------------

#include "mixer.h"

Mixer::Mixer ()
{
    blockData1_ = (short*)new char[BLOCK_SIZE];
    blockData2_ = (short*)new char[BLOCK_SIZE];
	doUseOne_ = true;
	isPlaying_ = false;
}

Mixer::~Mixer ()
{
	if (blockData1_)
    {
		delete blockData1_;
        blockData1_ = 0;
    }
	if (blockData2_)
    {
		delete blockData2_;
        blockData2_ = 0;
    }
}

void
Mixer::PrepareAndPlayNextBlock ()
{
	if (unmixedBlocks_.Count () > 0)
	{
		isPlaying_ = true;
		Mix ();

		if (doUseOne_)
			{
				waveOutUnprepareHeader (SoundAPI ()->GetDevice (), &blockHdr1_, sizeof (WAVEHDR));

				//prepare header
				ZeroMemory (&blockHdr1_, sizeof (WAVEHDR));
				blockHdr1_.lpData = (char*)blockData1_;
				blockHdr1_.dwBytesRecorded = 0;
				blockHdr1_.dwUser = 0;
				blockHdr1_.dwFlags = 0;
				blockHdr1_.dwLoops = 0;
				blockHdr1_.dwBufferLength = BLOCK_SIZE;
				waveOutPrepareHeader (SoundAPI ()->GetDevice (), &blockHdr1_, sizeof (WAVEHDR));
				waveOutWrite (SoundAPI ()->GetDevice (), &blockHdr1_, sizeof (WAVEHDR));
			}
		else
			{
				waveOutUnprepareHeader (SoundAPI ()->GetDevice (), &blockHdr2_, sizeof (WAVEHDR));

				//prepare header
				ZeroMemory (&blockHdr2_, sizeof (WAVEHDR));
				blockHdr2_.lpData = (char*)blockData2_;
				blockHdr2_.dwBytesRecorded = 0;
				blockHdr2_.dwUser = 0;
				blockHdr2_.dwFlags = 0;
				blockHdr2_.dwLoops = 0;
				blockHdr2_.dwBufferLength = BLOCK_SIZE;
				waveOutPrepareHeader (SoundAPI ()->GetDevice (), &blockHdr2_, sizeof (WAVEHDR));
				waveOutWrite (SoundAPI ()->GetDevice (), &blockHdr2_, sizeof (WAVEHDR));
			}
		SoundAPI ()->IncrementPlayingBlockCount ();
		doUseOne_ = !doUseOne_;
	}
	else
		isPlaying_ = false;
}

void
Mixer::AddUnmixedBlock (short* blockData, int percentLeftVolume, int percentRightVolume)
{
	unmixedBlocks_.InsertBack (blockData);
	unmixedBlocksLeftVolume_.InsertBack (percentLeftVolume);
	unmixedBlocksRightVolume_.InsertBack (percentRightVolume);
}

bool
Mixer::IsPlaying ()
{
	return isPlaying_;
}


//Private Methods

void 
Mixer::Mix ()
{
	short* memPtr;
	const int halfOfBlockSize = BLOCK_SIZE >> 1;
	const int quarterOfBlockSize = BLOCK_SIZE >> 2;
	int unmixedBlockCount;
	int leftVolume;
	int rightVolume;
	int leftVolumeSum;
	int rightVolumeSum;
    
    //each 4 byte block represents the sum of 2 byte data
	int leftChannelSum[quarterOfBlockSize]; 
	int rightChannelSum[quarterOfBlockSize];

	for (int i = 0; i < quarterOfBlockSize; ++i)
	{
		leftChannelSum[i] = 0;
		rightChannelSum[i] = 0;
	}

	leftVolumeSum = 0;
	rightVolumeSum = 0;

	//sum sound data
	for (unmixedBlockCount = unmixedBlocks_.Count (); unmixedBlockCount > 0; --unmixedBlockCount)
	{
		memPtr = unmixedBlocks_.PopFront ();
		leftVolume = unmixedBlocksLeftVolume_.PopFront ();
		rightVolume = unmixedBlocksRightVolume_.PopFront ();
		for (int i = 0; i < halfOfBlockSize; i += 2)
		{
			//Note- these two lines will be called 44100 times a second multiplied by the number of sounds simultaneously playing
			*(leftChannelSum + (i >> 1)) += (*(memPtr + i) * leftVolume);
			*(rightChannelSum + (i >> 1)) += (*(memPtr + i + 1) * rightVolume);
		}
		leftVolumeSum += leftVolume;
		rightVolumeSum += rightVolume;
		if (leftVolumeSum >= 65536 || rightVolumeSum >= 65536)
			break; //prevents clipping
	}
	
	//average sound data and write to block data
	if (doUseOne_)
	{
		if (leftVolumeSum <= 512 || rightVolumeSum <= 512 )
		{
			//fastest
			for (int i = 0; i < halfOfBlockSize; i += 2)
			{
				//these two lines are called 44100 times a second
				*(blockData1_ + i) = short(*(leftChannelSum + (i >> 1)) >> 9);
				*(blockData1_ + i + 1) = short(*(rightChannelSum + (i >> 1)) >> 9);
			}
		}
		else if (leftVolumeSum <= 512 || rightVolumeSum > 512 )
		{
			for (int i = 0; i < halfOfBlockSize; i += 2)
			{
				*(blockData1_ + i) = short(*(leftChannelSum + (i >> 1)) >> 9);
				*(blockData1_ + i + 1) = short(*(rightChannelSum + (i >> 1)) / rightVolumeSum);
			}
		}
		else if (leftVolumeSum > 512 || rightVolumeSum <= 512 )
		{
			for (int i = 0; i < halfOfBlockSize; i += 2)
			{
				*(blockData1_ + i) = short(*(leftChannelSum + (i >> 1)) / leftVolumeSum);
				*(blockData1_ + i + 1) = short(*(rightChannelSum + (i >> 1)) >> 9);
			}
		}
		else
		{
			for (int i = 0; i < halfOfBlockSize; i += 2)
			{
				*(blockData1_ + i) = short(*(leftChannelSum + (i >> 1)) / leftVolumeSum);
				*(blockData1_ + i + 1) = short(*(rightChannelSum + (i >> 1)) / rightVolumeSum);
			}
		}
	}
	else
	{
		if (leftVolumeSum <= 512 || rightVolumeSum <= 512 )
		{
			//fastest
			for (int i = 0; i < halfOfBlockSize; i += 2)
			{
				//these two lines are called 44100 times a second
				*(blockData2_ + i) = short(*(leftChannelSum + (i >> 1)) >> 9);
				*(blockData2_ + i + 1) = short(*(rightChannelSum + (i >> 1)) >> 9);
			}
		}
		else if (leftVolumeSum <= 512 || rightVolumeSum > 512 )
		{
			for (int i = 0; i < halfOfBlockSize; i += 2)
			{
				*(blockData2_ + i) = short(*(leftChannelSum + (i >> 1)) >> 9);
				*(blockData2_ + i + 1) = short(*(rightChannelSum + (i >> 1)) / rightVolumeSum);
			}
		}
		else if (leftVolumeSum > 512 || rightVolumeSum <= 512 )
		{
			for (int i = 0; i < halfOfBlockSize; i += 2)
			{
				*(blockData2_ + i) = short(*(leftChannelSum + (i >> 1)) / leftVolumeSum);
				*(blockData2_ + i + 1) = short(*(rightChannelSum + (i >> 1)) >> 9);
			}
		}
		else
		{
			for (int i = 0; i < halfOfBlockSize; i += 2)
			{
				*(blockData2_ + i) = short(*(leftChannelSum + (i >> 1)) / leftVolumeSum);
				*(blockData2_ + i + 1) = short(*(rightChannelSum + (i >> 1)) / rightVolumeSum);
			}
		}
	}
}