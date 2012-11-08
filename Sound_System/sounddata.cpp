//---------- Copyright � 2012, Daedal Games, All rights reserved. -------------

#include "sounddata.h"

SoundData::SoundData ()
{
	referenceCount_ = 0;

	oggBuffer = NULL;
}

SoundData::~SoundData ()
{
	if (oggBuffer)
	{
		delete oggBuffer;
	}
}