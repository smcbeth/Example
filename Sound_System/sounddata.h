//---------- Copyright � 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: Handle to raw sound data.
//
//-----------------------------------------------------------------------------
#pragma once

#include "..\public\nstdlib\platform.h"
#include "..\public\nstdlib\referenced.h"
#include "..\public\nstdlib\list.h"

class SoundData
{
	DEFINE_REFERENCED

public:
	SoundData( );
	~SoundData( );

	char	name[ MAX_SOUNDNAME ];
	uint8*	oggBuffer;
	uint32	dataSize;
};