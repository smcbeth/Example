//---------- Copyright � 2012, Daedal Games, All rights reserved. -------------

#include "sound.h"

Sound::Sound (SoundData* data, bool doConserveMem)
{
	data_ = data;

	oggDecoderIsInitialized_ = false;
	isPlaying_ = false;
	doConserveMem_ = doConserveMem;
	isWaitingOnRelease_ = false;

	leftVolumeMult_ = 1.0;
	rightVolumeMult_ = 1.0;
	
	pcmBufferOffset_ = 0;

	mixer_ = SoundAPI ()->GetMixer ();

	DynamicRange range = SoundAPI ()->GetDynamicRange ();

	switch (range)
	{
	case DYNAMICRANGE_NONE:
		leftVolume_ = 512;
		rightVolume_ = 512;
		break;
	case DYNAMICRANGE_LOW:
		leftVolume_ = 256;
		rightVolume_ = 256;
		break;
	case DYNAMICRANGE_MEDIUM:
		leftVolume_ = 128;
		rightVolume_ = 128;
		break;
	case DYNAMICRANGE_HIGH:
		leftVolume_ = 64;
		rightVolume_ = 64;
		break;
	case DYNAMICRANGE_EXTREME:
		leftVolume_ = 32;
		rightVolume_ = 32;
		break;
	}

	if (!doConserveMem)
		InitOggDecoder_ ();
}

Sound::~Sound ()
{
	Stop();

	if (!doConserveMem_)
		CloseOggDecoder_ ();
}

const char*
Sound::GetName ()
{
	return data_->name;
}

void 
Sound::Play ()
{
	// initialize ogg decoder if not already initialized
	InitOggDecoder_ ();
		
	// play from the start of the sound data
	stb_vorbis_seek_start(oggDecoderHandle_);
	pcmBufferOffset_ = 0;

	isPlaying_ = true;
	
	// send first block
	SendNextBlock ();

	Resume ();
}

void
Sound::Resume ()
{
	isPlaying_ = true;

	Mixer* mixer_ = SoundAPI ()->GetMixer ();

	// get mixer_ started up if not currently playing
	if (!mixer_->IsPlaying () && isPlaying_)
	{
		mixer_->PrepareAndPlayNextBlock ();

		// send and play second block
		SendNextBlock ();
		if (isPlaying_)
			mixer_->PrepareAndPlayNextBlock ();
	}
	if (isPlaying_)
		SoundAPI ()->QueueSound (this);
}

void
Sound::Pause ()
{
	isPlaying_ = false;
}

void
Sound::Stop ()
{
	isPlaying_ = false;

	if (doConserveMem_)
	{
		// close ogg decoder if already initialized
		CloseOggDecoder_ ();
	}

	leftVolumeMult_ = 1.0;
	rightVolumeMult_ = 1.0;
	
	pcmBufferOffset_ = 0;
}

void
Sound::SendNextBlock ()
{
	if (isPlaying_)
	{
		// stop playing sound if the decoder offset has reached the end of the ogg file
		if (stb_vorbis_get_offset(oggDecoderHandle_) >= (data_->dataSize))
		{
			Stop ();
		}
		else
		{
			Decode_ ();
		
			if (pcmBufferOffset_ < BLOCK_SIZE)
				mixer_->AddUnmixedBlock (pcmBuffer_, int((float)leftVolume_ * leftVolumeMult_), int((float)rightVolume_ * rightVolumeMult_));
			else
				mixer_->AddUnmixedBlock (pcmBuffer_ + BLOCK_SIZE / 2, int((float)leftVolume_ * leftVolumeMult_), int((float)rightVolume_ * rightVolumeMult_));
		}
	}
}

void
Sound::SetPositionalVolume (float x, float y, float z)
{
	float distance;
	
	distance = sqrtf((x * x) + (y * y) + (z * z));

	SetPositionalVolumeFast (x, y, z, distance);
}

void Sound::SetPositionalVolumeFast (float x, float y, float z, float distance)
{
	// TODO: some of these values may need tweaking to improve the feel of positional sound modification
	// x y z are in feet
	// positive y is up
	// positive x is right
	// positive z is front
	
    // sounds equal to or closer than this will be at maximal loudness. TODO: experiment with other values for this
	const int distanceThreshold = 5; 
	const float twoOverPi = .63661977f;
	
	float distanceDivisor;
	float leftMult;
	float rightMult;
	float minimumMult;
	float minMultDivisor;
	float absX;
	float absY;
	float absZ;
	float ratio;
	float xyAngle;
	float xzAngle;
	float avgAngle;
	
    // must be between 0 and ~.5, determines maximum difference in volume between left and right speakers
	minimumMult = .3f; 

	if (x < 0)
		absX = -x;
	else
		absX = x;
	if (y < 0)
		absY = -y;
	else
		absY = y;
	if (z < 0)
		absZ = -z;
	else
		absZ = z;

	// determine left-right triangulation of sound
	if (absX != 0 || distance > 1000)
	{
		// calculate angle on xy axis and xz axis
		xyAngle = atan2f (absY, absX); // TODO: check speed
		xzAngle = atan2f (absZ, absX);

		// find weighted average of both angles
		if (absY > absZ)
		{
			ratio = (absZ / absY);
			avgAngle = ((ratio * xzAngle) + xyAngle) / (ratio + 1);
		}
		else
		{
			ratio = (absY / absZ);
			avgAngle = ((ratio * xyAngle) + xzAngle) / (ratio + 1);
		}
		
		// find multipliers for left and right volume based on the average angle
		minimumMult += distance / 50; //makes far away sounds become more centered
		minMultDivisor = minimumMult + 1;
		if (x < 0)
		{
			leftMult = 1;
			rightMult = avgAngle * twoOverPi;
			rightMult = (rightMult + minimumMult) / minMultDivisor;
		}
		else
		{
			rightMult = 1;
			leftMult = avgAngle * twoOverPi;
			leftMult = (leftMult + minimumMult) / minMultDivisor;
		}	
	}
	else
	{
		leftMult = 1;
		rightMult = 1;
	}

	if (distance < distanceThreshold)
		distance = (float32)distanceThreshold;

    // sound has inverse proportional behavior with distance
	distanceDivisor = distance / distanceThreshold; 

	leftVolumeMult_ = (1.0f / distanceDivisor) * leftMult;
	rightVolumeMult_ = (1.0f / distanceDivisor) * rightMult;
}

void 
Sound::SetLeftVolume (int32 volume)
{
	// volume must be between or equal to 0 and 512
    Assert( volume >= 0 && volume <= 512 );
    
	leftVolume_ = volume;
}

void
Sound::SetRightVolume (int32 volume)
{
	// volume must be between or equal to 0 and 512
    Assert( volume >= 0 && volume <= 512 );
    
	rightVolume_ = volume;
}

void
Sound::SetLeftAndRightVolume (int32 volume)
{
	// volume must be between or equal to 0 and 512
    Assert( volume >= 0 && volume <= 512 );
    
	leftVolume_ = volume;
	rightVolume_ = volume;
}

int32
Sound::GetLeftVolume ()
{
	return leftVolume_;
}

int32
Sound::GetRightVolume ()
{
	return rightVolume_;
}

bool
Sound::GetIsPlaying ()
{
	return isPlaying_;
}

void
Sound::SetDoConserveMem (bool b)
{
	// memory conservation when sound is not playing: "up to 150KB" + BLOCK_SIZE * 3 bytes
	doConserveMem_ = b;

	if (!isPlaying_)
	{
		if (doConserveMem_)
			CloseOggDecoder_ ();
		else
			InitOggDecoder_ ();
	}
}

void
Sound::Release ()
{
	if (isPlaying_)
	{
        // allows sounds to playback in full before deletion
		isWaitingOnRelease_ = true; 
	}
	else
	{
		data_->RemoveReference();
		delete this;
	}
}

bool
Sound::GetIsWaitingOnRelease ()
{
	return isWaitingOnRelease_;
}

// Private Methods

void 
Sound::Decode_ ()
{
	Assert (oggDecoderIsInitialized_);

	const int halfOfBlockSize = BLOCK_SIZE >> 1;
	int pcmBufferOffsetLimit;
	int shortCount;
	int i;
	int j;

	shortCount = 0;

	if (pcmBufferOffset_ < halfOfBlockSize)
		pcmBufferOffsetLimit = halfOfBlockSize;
	else if (pcmBufferOffset_ < BLOCK_SIZE)
		pcmBufferOffsetLimit = BLOCK_SIZE;
	else
	{
		// move data from ending third of pcmBuffer to beginning third
		j = 0;
		for (i = BLOCK_SIZE; i <= pcmBufferOffset_; ++i)
		{
			*(pcmBuffer_ + j) = *(pcmBuffer_ + i);
			++j;
		}
		pcmBufferOffsetLimit = halfOfBlockSize;
		pcmBufferOffset_ -= BLOCK_SIZE;
	}

	// decode more data from ogg file
	while (pcmBufferOffset_ < pcmBufferOffsetLimit)
	{
		 shortCount = stb_vorbis_get_frame_short_interleaved(
                         oggDecoderHandle_, 2, pcmBuffer_ + pcmBufferOffset_, (int)((float)BLOCK_SIZE * 1.5f)) * 2;

		 if (shortCount == 0)
		 {
			 memset (pcmBuffer_ + pcmBufferOffset_, 0, pcmBufferOffsetLimit - pcmBufferOffset_);
			 pcmBufferOffset_ = pcmBufferOffsetLimit;
			 break;
		 }
		 pcmBufferOffset_ += shortCount;
	}

	Assert (oggDecoderIsInitialized_);
}

bool
Sound::InitOggDecoder_ ()
{
	if (!oggDecoderIsInitialized_)
	{
		int error = 0;
		oggDecoderHandle_ = stb_vorbis_open_memory(data_->oggBuffer, data_->dataSize, &error, NULL);
		oggDecoderIsInitialized_ = true;

		pcmBuffer_ = (short*)new char[BLOCK_SIZE * 3];
		
		return true;
	}
	return false;
}

bool
Sound::CloseOggDecoder_ ()
{
	if (oggDecoderIsInitialized_)
	{
		stb_vorbis_close(oggDecoderHandle_);
		if(pcmBuffer_)
        {
			delete pcmBuffer_;
            pcmBuffer_ = 0;
        }
		oggDecoderIsInitialized_ = false;
		return true;
	}
	return false;
}

