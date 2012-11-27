//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: Solves cross-include with "towngenerator.h"
//
//-----------------------------------------------------------------------------
#pragma once



class IWorldGenerator
{
public:
	~IWorldGenerator () {	}

	virtual void Generate (uint32 size, uint32 depth) = 0;

	virtual bool GenerateHeightMap	(uint32 oX, uint32 oZ, uint16* heightMap, uint32 lengthX, uint32 lengthY) = 0;
	virtual void GenerateSlopeMap	(float32* slopeMap, uint16* heightMaps[], uint32 lengthX, uint32 lengthY) = 0;

	virtual void PrepTerraNoise (uint32 oX, uint32 oY, uint32 oZ, uint32 lengthX, uint32 lengthY, bool prep3DNoise) = 0;

	virtual uint32 GetWorldDepth () = 0;
	virtual uint32 GetSeaLevel () = 0;
	virtual uint32 GetWorldSize () = 0;
	virtual Vector<uint32>* GetTownSupportingAreas () = 0;
	virtual char* GetTownSupportingAreasSparseArray () = 0;
};

