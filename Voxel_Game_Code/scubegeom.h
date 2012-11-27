//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: Helper class for terrain SCube geometry.
//          The code herein is extremely performance critical.
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\nstdlib\_math.h"

class Vector3D;

// CTODO: clean up
// CTODO: for both generates, take in the origin as a copy so we can offset that instead of having two origins on the stack
// CTODO: for all, optimize the functions to take advantage of the data compression, doing less computations and skipping over things faster
// CTODO: change the Generate functions to work from bottom left back corner of block instead of central?
class SCubeGeom
{
public:
	SCubeGeom( );
	~SCubeGeom( );

	// Compute the number of vertices and indices for each terrain type
	void ComputeVertexAndIndexCounts(
		uint32* compressedData,
		uint32* vertexCounts,
		uint32* indexCounts );

	// Fill 'TriangleSoup' memory with geometry
	void GenerateCollide(
		float32*		collideMemory,
		uint32*			compressedData,
		const Vector3D&	origin );

	// Fill vertex and index buffers with geometry
	void GenerateSectorHull(
		float32*		vb,
		uint32*			ib,
		uint32*			compressedData,
		const Vector3D&	origin,
		uint32*			vertexCounts,
		uint32*			indexCounts );
};

// Accessor
FORCE_INLINE SCubeGeom*
GetSCubeGeom( )
{
	extern SCubeGeom* _g_SCubeGeomInstance;
	return _g_SCubeGeomInstance;
}