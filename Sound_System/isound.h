//---------- Copyright � 2012, Daedal Games, All rights reserved. -------------

#pragma once

class ISound
{
public:
	virtual ~ISound () { ; }

	virtual const char* GetName () = 0;
	virtual void        Play () = 0;
	virtual void        Resume () = 0;
    virtual void        Pause () = 0;
	virtual void        Stop () = 0;

	virtual void        SetPositionalVolume (float x, float y, float z) = 0;
	virtual void        SetPositionalVolumeFast (float x, float y, float z, float distance) = 0;

	virtual void        SendNextBlock () = 0;

	// volume must be between or equal to 0 and 512
    // TODO: add float overloads for these for ease of use
	virtual void        SetLeftVolume (int32 volume) = 0;
	virtual void        SetRightVolume (int32 volume) = 0;
	virtual void        SetLeftAndRightVolume (int32 volume) = 0;

	virtual int32       GetLeftVolume () = 0;
	virtual int32       GetRightVolume () = 0;

	virtual bool        GetIsPlaying () = 0;

	virtual void        SetDoConserveMem (bool b) = 0;

	virtual void        Release () = 0;
	virtual bool        GetIsWaitingOnRelease () = 0;
};