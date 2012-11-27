//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "scubegeom.h"
#include "public\nstdlib\vector3d.h"
#include "definitions.h"
#include "terrainhelper.h"

SCubeGeom::SCubeGeom( )
{
	;
}

SCubeGeom::~SCubeGeom( )
{
	;
}

//-----------------------------------------------------------------------------
// Compute the number of vertices and indices for each terrain type
//-----------------------------------------------------------------------------
void
SCubeGeom::ComputeVertexAndIndexCounts(
	uint32* compressedData,
	uint32* vertexCounts,
	uint32* indexCounts )
{
	// variables for decompressing 'compressedData'
	uint32 rleOffset = 0;						// offset in 'compressedData'
	uint32 rleCubeData = compressedData[ 0 ];	// scube
	uint32 rleCubeCount = compressedData[ 1 ];	// count of scube

	for( uint32 i = 0; i < SECTOR_BVOLUME; ++i )
	{
		// decompress next scube?
		if( rleCubeCount == 0 )
		{
			rleOffset += 2;
			rleCubeData = compressedData[ rleOffset ];
			rleCubeCount = compressedData[ rleOffset + 1 ];
		}
		--rleCubeCount;

		// get visibility
		unsigned char vis = GetSCubeVisibility( rleCubeData );

		// invisible?
		if( !vis )
		{
			continue;
		}

		// get terrain type
		unsigned char terraType = GetSCubeTerrainType( rleCubeData );

		// air?
		if( terraType == TERRA_AIR )
		{
			continue;
		}

		// get spin
		uint32 spin = GetSCubeSpin( rleCubeData );

		uint32 front;
		uint32 back;
		uint32 left;
		uint32 right;
		uint32 top;
		uint32 bottom;
		switch( GetSCubeRotation( rleCubeData ) )
		{
		case 0: //( default orientation ) bottom side faces bottom in viewport
			bottom = VIS_BOTTOM;
			top = VIS_TOP;
			switch( spin ) // each consecutive spin is a clockwise rotation with respect to the bottom-top axis.
			{
			case 0:
				back = VIS_BACK; front = VIS_FRONT; right = VIS_RIGHT; left = VIS_LEFT;
				break;

			case 1:
				back = VIS_RIGHT; front = VIS_LEFT; right = VIS_FRONT; left = VIS_BACK;
				break;

			case 2:
				back = VIS_FRONT; front = VIS_BACK; right = VIS_LEFT; left = VIS_RIGHT;
				break;

			case 3:
				back = VIS_LEFT; front = VIS_RIGHT; right = VIS_BACK; left = VIS_FRONT;
				break;
			}
			break;

		case 1: // bottom faces top
			bottom = VIS_TOP;
			top = VIS_BOTTOM;
			switch( spin )
			{
			case 0:
				back = VIS_BACK; front = VIS_FRONT; right = VIS_LEFT; left = VIS_RIGHT;
				break;

			case 1:
				back = VIS_LEFT; front = VIS_RIGHT; right = VIS_FRONT; left = VIS_BACK;
				break;

			case 2:
				back = VIS_FRONT; front = VIS_BACK; right = VIS_RIGHT; left = VIS_LEFT;
				break;

			case 3:
				back = VIS_RIGHT; front = VIS_LEFT; right = VIS_BACK; left = VIS_FRONT;
				break;
			}
			break;

		case 2: // bottom faces left
			bottom = VIS_LEFT;
			top = VIS_RIGHT;
			switch( spin )
			{
			case 0:
				back = VIS_BACK; front = VIS_FRONT; right = VIS_BOTTOM; left = VIS_TOP;
				break;

			case 1:
				back = VIS_BOTTOM; front = VIS_TOP; right = VIS_FRONT; left = VIS_BACK;
				break;

			case 2:
				back = VIS_FRONT; front = VIS_BACK; right = VIS_TOP; left = VIS_BOTTOM;
				break;

			case 3:
				back = VIS_TOP; front = VIS_BOTTOM; right = VIS_BACK; left = VIS_FRONT;
				break;
			}
			break;

		case 3: // bottom faces right
			bottom = VIS_RIGHT;
			top = VIS_LEFT;
			switch( spin )
			{
			case 0:
				back = VIS_BACK; front = VIS_FRONT; right = VIS_TOP; left = VIS_BOTTOM;
				break;

			case 1:
				back = VIS_TOP; front = VIS_BOTTOM; right = VIS_FRONT; left = VIS_BACK;
				break;

			case 2:
				back = VIS_FRONT; front = VIS_BACK; right = VIS_BOTTOM; left = VIS_TOP;
				break;

			case 3:
				back = VIS_BOTTOM; front = VIS_TOP; right = VIS_BACK; left = VIS_FRONT;
				break;
			}
			break;

		case 4: // bottom faces front
			bottom = VIS_FRONT;
			top = VIS_BACK;
			switch( spin )
			{
			case 0:
				back = VIS_BOTTOM; front = VIS_TOP; right = VIS_RIGHT; left = VIS_LEFT;
				break;

			case 1:
				back = VIS_RIGHT; front = VIS_LEFT; right = VIS_TOP; left = VIS_BOTTOM;
				break;

			case 2:
				back = VIS_TOP; front = VIS_BOTTOM; right = VIS_LEFT; left = VIS_RIGHT;
				break;

			case 3:
				back = VIS_LEFT; front = VIS_RIGHT; right = VIS_BOTTOM; left = VIS_TOP;
				break;
			}
			break;

		case 5: // bottom faces back
			bottom = VIS_BACK;
			top = VIS_FRONT;
			switch( spin )
			{
			case 0:
				back = VIS_TOP; front = VIS_BOTTOM; right = VIS_RIGHT; left = VIS_LEFT;
				break;

			case 1:
				back = VIS_RIGHT; front = VIS_LEFT; right = VIS_BOTTOM; left = VIS_TOP;
				break;

			case 2:
				back = VIS_BOTTOM; front = VIS_TOP; right = VIS_LEFT; left = VIS_RIGHT;
				break;

			case 3:
				back = VIS_LEFT; front = VIS_RIGHT; right = VIS_TOP; left = VIS_BOTTOM;
				break;
			}
			break;
		}

		// get current counts
		uint32 vertexCount = vertexCounts[ terraType ];
		uint32 indexCount = indexCounts[ terraType ];

		switch( GetSCubeType( rleCubeData ) )
		{
		case SCUBE_CUBE:
			if( vis & front )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & back )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & left )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & right )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & top )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & bottom )
			{
				vertexCount += 4;
				indexCount += 6;
			}
		break;

		case SCUBE_THREEQUARTERCUBE:
			if( vis & front || vis & top || vis & right )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & front )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & back )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & left )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & right )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & top )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & bottom )
			{
				vertexCount += 4;
				indexCount += 6;
			}
		break;
	
		case SCUBE_HALFCUBE:
			if( vis & front )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & back )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & left )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & right )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & top )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & bottom )
			{
				vertexCount += 4;
				indexCount += 6;
			}
		break;

		case SCUBE_HALFCUBETRIANGLE:
			if( vis & front || vis & top )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & back )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & left )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & right )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & bottom )
			{
				vertexCount += 4;
				indexCount += 6;
			}
		break;

		case SCUBE_HALFCUBEWEDGE:
			if( (vis & front ) || ( vis & top ) ||( vis & right ) )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( (vis & back ) || ( vis & top ) ||( vis & left ) )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & front )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & back )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & left )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & right )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & bottom )
			{
				vertexCount += 4;
				indexCount += 6;
			}
		break;

		case SCUBE_QUARTERCUBEPYRAMID:
			if( (vis & front ) ||( vis & top ) )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( (vis & back ) ||( vis & top ) )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( (vis & left ) ||( vis & top ) )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( (vis & right ) ||( vis & top ) )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & bottom )
			{
				vertexCount += 4;
				indexCount += 6;
			}
		break;

		case SCUBE_QUARTERCUBECORNERPYRAMID:
			if( vis & front )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( (vis & back ) && !(vis & top ) )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & left )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( (vis & right ) && !(vis & top ) )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & top )
			{
				vertexCount += 4;
				indexCount += 6;
			}
			if( vis & bottom )
			{
				vertexCount += 4;
				indexCount += 6;
			}
		break;

		case SCUBE_QUARTERCUBETRIANGLE:
			if( (vis & front ) ||( vis & top ) ||( vis & right ) )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & back )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & left )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & bottom )
			{
				vertexCount += 3;
				indexCount += 3;
			}
		break;

		case SCUBE_QUARTERCUBESTRANGETRIANGLE:
			if( (vis & front ) ||( vis & top ) ) // front perspective used
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & back )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( vis & left )
			{
				vertexCount += 3;
				indexCount += 3;
			}
			if( (vis & right ) ||( vis & bottom ) ) // right perspective used
			{
				vertexCount += 3;
				indexCount += 3;
			}
		break;

		NO_DEFAULT
		}

		// update counts
		vertexCounts[ terraType ] = vertexCount;
		indexCounts[ terraType ] = indexCount;
	}
}

// CTODO: put in physics code for stuff other than the tops of blocks
//-----------------------------------------------------------------------------
// Fill 'TriangleSoup' memory with geometry
//-----------------------------------------------------------------------------
void
SCubeGeom::GenerateCollide(
	float32* collideMemory,
	uint32* compressedData,
	const Vector3D& origin )
{
	// compute 'offset' origin, at back bottom left corner
	// this is done because it is faster than doing the computations with a central origin
	Vector3D offsetOrigin(
		origin.x - (SECTOR_LENGTH / 2.0f),
		origin.y - (SECTOR_LENGTH / 2.0f),
		origin.z - (SECTOR_LENGTH / 2.0f) );

	// offset in the collide memory
	uint32 memoryOffset = 0;

	// variables for decompressing 'compressedData'
	uint32 rleOffset = 0;						// offset in 'compressedData'
	uint32 rleCubeData = compressedData[ 0 ];	// scube
	uint32 rleCubeCount = compressedData[ 1 ];	// count of scube

	// PERFORMANCE NOTE: changing these loop variables to floats and removing the float casts below resulted in this method running almost twice as slow
	for( int32 z = 0; z < SECTOR_BLENGTH; ++z )
	{
		for( int32 y = 0; y < SECTOR_BLENGTH; ++y )
		{
			for( int32 x = 0; x < SECTOR_BLENGTH; ++x )
			{
				// decompress next scube?
				if( rleCubeCount == 0 )
				{
					rleOffset += 2;
					rleCubeData = compressedData[ rleOffset ];
					rleCubeCount = compressedData[ rleOffset + 1 ];
				}
				--rleCubeCount;

				// get visibility
				unsigned char vis = GetSCubeVisibility( rleCubeData );
				// invisible?
				if( !vis )
				{
					continue;
				}

				// get terrain type
				TERRATYPE terraType = GetSCubeTerrainType( rleCubeData );
				// air?
				if( terraType == TERRA_AIR )
				{
					continue;
				}

				// get spin
				uint32 spin = GetSCubeSpin( rleCubeData );

				// handle rotations (shitloads of variables for this)
				float32	cX;
				float32	cY;
				float32	cZ;
				uint32	xIndex;
				uint32	yIndex;
				uint32	zIndex;
				uint32	front;
				uint32	back;
				uint32	left;
				uint32	right;
				uint32	top;
				uint32	bottom;
				float32	xOffA;
				float32	xOffB;
				float32	yOffA;
				float32	yOffB;
				float32	zOffA;
				float32	zOffB;
				switch( GetSCubeRotation( rleCubeData ) )
				{
				case 0: // (default orientation) bottom side faces bottom in viewport
					bottom = VIS_BOTTOM;
					top = VIS_TOP;
					switch( spin ) // each consecutive spin is a clockwise rotation with respect to the original bottom-top o
					{
					case 0:
						back = VIS_BACK; front = VIS_FRONT; right = VIS_RIGHT; left = VIS_LEFT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = 0.0f;			xOffB = SCUBE_LENGTH; // last two variables are in a positive configuration
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 1; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 1:
						back = VIS_RIGHT; front = VIS_LEFT; right = VIS_FRONT; left = VIS_BACK;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = SCUBE_LENGTH;	xOffB = 0.0f;
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 2:
						back = VIS_FRONT; front = VIS_BACK; right = VIS_LEFT; left = VIS_RIGHT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = SCUBE_LENGTH;	xOffB = 0.0f;
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 1; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 3:
						back = VIS_LEFT; front = VIS_RIGHT; right = VIS_BACK; left = VIS_FRONT;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;
					}
					break;

				case 1: // bottom faces top 
					bottom = VIS_TOP;
					top = VIS_BOTTOM;
					switch( spin )
					{
					case 0:
						back = VIS_BACK; front = VIS_FRONT; right = VIS_LEFT; left = VIS_RIGHT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = SCUBE_LENGTH;	xOffB = 0.0f;
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 1; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 1:
						back = VIS_LEFT; front = VIS_RIGHT; right = VIS_FRONT; left = VIS_BACK;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = SCUBE_LENGTH;	xOffB = 0.0f;
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 2:
						back = VIS_FRONT; front = VIS_BACK; right = VIS_RIGHT; left = VIS_LEFT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 1; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 3:
						back = VIS_RIGHT; front = VIS_LEFT; right = VIS_BACK; left = VIS_FRONT;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;
					}
					break;

				case 2: // bottom faces left
					bottom = VIS_LEFT;
					top = VIS_RIGHT;
					switch( spin )
					{
					case 0:
						back = VIS_BACK; front = VIS_FRONT; right = VIS_BOTTOM; left = VIS_TOP;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 2; xOffA = SCUBE_LENGTH;	xOffB = 0.0f;
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 1; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.x + float32(x) * SCUBE_LENGTH; zIndex = 0; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 1:
						back = VIS_BOTTOM; front = VIS_TOP; right = VIS_FRONT; left = VIS_BACK;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = SCUBE_LENGTH; xOffB = 0.0f; 
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = SCUBE_LENGTH; yOffB = 0.0f;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = 0.0f; zOffB = SCUBE_LENGTH;
						break;

					case 2:
						back = VIS_FRONT; front = VIS_BACK; right = VIS_TOP; left = VIS_BOTTOM;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 2; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 1; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.x + float32(x) * SCUBE_LENGTH; zIndex = 0; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 3:
						back = VIS_TOP; front = VIS_BOTTOM; right = VIS_BACK; left = VIS_FRONT;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.z + float32(z) * SCUBE_LENGTH; yIndex = 2; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.x + float32(x) * SCUBE_LENGTH; zIndex = 0; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;
					}
					break;

				case 3: // bottom faces right
					bottom = VIS_RIGHT;
					top = VIS_LEFT;
					switch( spin )
					{
					case 0:
						back = VIS_BACK; front = VIS_FRONT; right = VIS_TOP; left = VIS_BOTTOM;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 2; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 1; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.x + float32(x) * SCUBE_LENGTH; zIndex = 0; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 1:
						back = VIS_TOP; front = VIS_BOTTOM; right = VIS_FRONT; left = VIS_BACK;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = SCUBE_LENGTH; xOffB = 0.0f; 
						cY = offsetOrigin.z + float32(z) * SCUBE_LENGTH; yIndex = 2; yOffA = 0.0f; yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.x + float32(x) * SCUBE_LENGTH; zIndex = 0; zOffA = SCUBE_LENGTH; zOffB = 0.0f;
						break;

					case 2:
						back = VIS_FRONT; front = VIS_BACK; right = VIS_BOTTOM; left = VIS_TOP;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 1; xOffA = SCUBE_LENGTH;	xOffB = 0.0f; 
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 0; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.x + float32(x) * SCUBE_LENGTH; zIndex = 2; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 3:
						back = VIS_BOTTOM; front = VIS_TOP; right = VIS_BACK; left = VIS_FRONT;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.z + float32(z) * SCUBE_LENGTH; yIndex = 2; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.x + float32(x) * SCUBE_LENGTH; zIndex = 0; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;
					}
					break;

				case 4: // bottom faces front
					bottom = VIS_FRONT;
					top = VIS_BACK;
					switch( spin )
					{
					case 0:
						back = VIS_BOTTOM; front = VIS_TOP; right = VIS_RIGHT; left = VIS_LEFT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.z + float32(z) * SCUBE_LENGTH; yIndex = 2; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 1:
						back = VIS_RIGHT; front = VIS_LEFT; right = VIS_TOP; left = VIS_BOTTOM;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 2; xOffA = SCUBE_LENGTH;	xOffB = 0.0f; 
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 2:
						back = VIS_TOP; front = VIS_BOTTOM; right = VIS_LEFT; left = VIS_RIGHT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = SCUBE_LENGTH;	xOffB = 0.0f; 
						cY = offsetOrigin.z + float32(z) * SCUBE_LENGTH; yIndex = 2; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 3:
						back = VIS_LEFT; front = VIS_RIGHT; right = VIS_BOTTOM; left = VIS_TOP;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 2; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;
					}
					break;

				case 5: // bottom faces back
					bottom = VIS_BACK;
					top = VIS_FRONT;
					switch( spin )
					{
					case 0:
						back = VIS_TOP; front = VIS_BOTTOM; right = VIS_RIGHT; left = VIS_LEFT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.z + float32(z) * SCUBE_LENGTH; yIndex = 2; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 1:
						back = VIS_RIGHT; front = VIS_LEFT; right = VIS_BOTTOM; left = VIS_TOP;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 2; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 2:
						back = VIS_BOTTOM; front = VIS_TOP; right = VIS_LEFT; left = VIS_RIGHT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = SCUBE_LENGTH;	xOffB = 0.0f; 
						cY = offsetOrigin.z + float32(z) * SCUBE_LENGTH; yIndex = 2; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 3:
						back = VIS_LEFT; front = VIS_RIGHT; right = VIS_TOP; left = VIS_BOTTOM;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 2; xOffA = SCUBE_LENGTH;	xOffB = 0.0f; 
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;
					}
					break;
				}

				// STODO: double check that all the code in this switch is correct
				// write in data
				switch( GetSCubeType( rleCubeData ) )
				{
				case SCUBE_CUBE:
					if( vis & front )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & back )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
					}
					if( vis & left )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
					}
					if( vis & right )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & top )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
					}
					if( vis & bottom )
					{
						
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						
						/*
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						*/
					}
				break;
				case SCUBE_THREEQUARTERCUBE:
					if( (vis & front) || (vis & top) || (vis & right) )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & front )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;	
					}
					if( vis & back )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
					}
					if( vis & left )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;		
					}
					if( vis & right )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & top )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
					}
					if( vis & bottom )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
				break;
				case SCUBE_HALFCUBE:
					if( vis & front )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA;				memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA;				memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA;				memoryOffset += 3;
					}
					if( vis & back )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA;				memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA;				memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA;				memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	memoryOffset += 3;
					}
					if( vis & left )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA;				memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA;				memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA;				memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	memoryOffset += 3;
					}
					if( vis & right )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA;				memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA;				memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA;				memoryOffset += 3;
					}
					if( vis & top )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + SCUBE_HALFLENGTH; memoryOffset += 3;
					}
					if( vis & bottom )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
				break;
				case SCUBE_HALFCUBETRIANGLE:
					if( (vis & front) || (vis & top) )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & back )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
					}
					if( vis & left )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;		
					}
					if( vis & right )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;	
					}
					if( vis & bottom )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
				break;
				case SCUBE_HALFCUBEWEDGE:
					if( (vis & front) || (vis & top) || (vis & right) )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( (vis & back) || (vis & top) || (vis & left) )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & front )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;			
					}
					if( vis & back )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;		
					}
					if( vis & left )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;					
					}
					if( vis & right )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;	
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & bottom )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
				break;
				case SCUBE_QUARTERCUBEPYRAMID:
					if( (vis & front) || (vis & top) )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + SCUBE_HALFLENGTH; collideMemory[ memoryOffset + yIndex ] = cY + SCUBE_HALFLENGTH; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB;			collideMemory[ memoryOffset + yIndex ] = cY + yOffA;			collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA;			collideMemory[ memoryOffset + yIndex ] = cY + yOffA;			collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;	
					}
					if( (vis & back) || (vis & top) )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + SCUBE_HALFLENGTH; collideMemory[ memoryOffset + yIndex ] = cY + SCUBE_HALFLENGTH; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA;			collideMemory[ memoryOffset + yIndex ] = cY + yOffB;			collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB;			collideMemory[ memoryOffset + yIndex ] = cY + yOffB;			collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( (vis & left) || (vis & top) )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + SCUBE_HALFLENGTH; collideMemory[ memoryOffset + yIndex ] = cY + SCUBE_HALFLENGTH; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA;			collideMemory[ memoryOffset + yIndex ] = cY + yOffA;			collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA;			collideMemory[ memoryOffset + yIndex ] = cY + yOffB;			collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;		
					}
					if( (vis & right) || (vis & top) )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + SCUBE_HALFLENGTH; collideMemory[ memoryOffset + yIndex ] = cY + SCUBE_HALFLENGTH; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB;			collideMemory[ memoryOffset + yIndex ] = cY + yOffB;			collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB;			collideMemory[ memoryOffset + yIndex ] = cY + yOffA;			collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & bottom )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
				break;
				case SCUBE_QUARTERCUBECORNERPYRAMID:
					if( vis & front )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( (vis & back) && !(vis & top) )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & left )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( (vis & right) && !(vis & top) )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & top )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & bottom )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
				break;
				case SCUBE_QUARTERCUBETRIANGLE:
					if( (vis & front) || (vis & top) || (vis & right) )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & back )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & left )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & bottom )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
				break;
				case SCUBE_QUARTERCUBESTRANGETRIANGLE:
					if( (vis & front) || (vis & top) ) // front perspective used
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & back )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( vis & left )
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
					if( (vis & right) || (vis & bottom) ) // right perspective used
					{
						collideMemory[ memoryOffset + xIndex ] = cX + xOffB; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffB; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffB; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
						collideMemory[ memoryOffset + xIndex ] = cX + xOffA; collideMemory[ memoryOffset + yIndex ] = cY + yOffA; collideMemory[ memoryOffset + zIndex ] = cZ + zOffA; memoryOffset += 3;
					}
				break;
				// CTODO: put this back once we implement all types here
				//NO_DEFAULT
				}
			}
		}
	}

#ifdef _DEBUG
	// check for boundary violations
	// if any of these ever fire triangles are being written outside the boundaries of the sector

	Vector3D minBound(
		origin.x - (SECTOR_LENGTH / 2.0f),
		origin.y - (SECTOR_LENGTH / 2.0f),
		origin.z - (SECTOR_LENGTH / 2.0f) );

	Vector3D maxBound(
		origin.x - (SECTOR_LENGTH / 2.0f) + SECTOR_LENGTH,
		origin.y - (SECTOR_LENGTH / 2.0f) + SECTOR_LENGTH,
		origin.z - (SECTOR_LENGTH / 2.0f) + SECTOR_LENGTH );

	for( uint32 i = 0; i < memoryOffset; i += 3 )
	{
		Assert( collideMemory[ i ] >= minBound.x );
		Assert( collideMemory[ i + 1 ] >= minBound.y );
		Assert( collideMemory[ i + 2 ] >= minBound.z );

		Assert( collideMemory[ i ] <= maxBound.x );
		Assert( collideMemory[ i + 1 ] <= maxBound.y );
		Assert( collideMemory[ i + 2 ] <= maxBound.z );
	}
#endif
}

//-----------------------------------------------------------------------------
// Fill vertex and index buffers with geometry
//-----------------------------------------------------------------------------
void
SCubeGeom::GenerateSectorHull(
	float32*		vb,
	uint32*			ib,
	uint32*			compressedData,
	const Vector3D&	origin,
	uint32*			vertexCounts,
	uint32*			indexCounts )
{
	// compute 'offset' origin, at back bottom left corner
	// this is done because it is faster than doing the computations with a central origin
	Vector3D offsetOrigin(
		origin.x - (SECTOR_LENGTH / 2.0f),
		origin.y - (SECTOR_LENGTH / 2.0f),
		origin.z - (SECTOR_LENGTH / 2.0f) );

	// variables for decompressing 'compressedData'
	uint32 rleOffset = 0;						// offset in 'compressedData'
	uint32 rleCubeData = compressedData[ 0 ];	// scube
	uint32 rleCubeCount = compressedData[ 1 ];	// count of scube

	// set up starting offsets
	uint32 vbOffsets[ TERRAINTYPE_COUNT ];
	uint32 ibOffsets[ TERRAINTYPE_COUNT ];
	ZeroMemory( vbOffsets, TERRAINTYPE_COUNT * sizeof( uint32 ) );
	ZeroMemory( ibOffsets, TERRAINTYPE_COUNT * sizeof( uint32 ) );
	for( uint32 i = 0; i < TERRAINTYPE_COUNT; ++i )
	{
		for( uint32 j = 0; j < i; ++j )
		{
			vbOffsets[ i ] += vertexCounts[ j ] * 5; // CTODO: assuming 5 floats per vertex here
			ibOffsets[ i ] += indexCounts[ j ];
		}
	}

	// CTODO: rename this shit
	uint32 _vertexCounts[ TERRAINTYPE_COUNT ];
	ZeroMemory( _vertexCounts, TERRAINTYPE_COUNT * sizeof( uint32 ) );
	
	// CTODO: CLEAN UP AND REMOVE CASTING
	// PERFORMANCE NOTE: making these for loop variables into floats and removing casts seems to slow this method down. the effect is much more pronounced in GenerateCollide()
	for( int32 z = 0; z < SECTOR_BLENGTH; ++z )
	{
		for( int32 y = 0; y < SECTOR_BLENGTH; ++y )
		{
			for( int32 x = 0; x < SECTOR_BLENGTH; ++x )
			{
				// decompress next scube?
				if( rleCubeCount == 0 )
				{
					rleOffset += 2;
					rleCubeData = compressedData[ rleOffset ];
					rleCubeCount = compressedData[ rleOffset + 1 ];
				}
				--rleCubeCount;

				// get visibility
				unsigned char vis = GetSCubeVisibility( rleCubeData );
				// invisible?
				if( !vis )
				{
					continue;
				}

				// get terrain type
				TERRATYPE terraType = GetSCubeTerrainType( rleCubeData );
				// air?
				if( terraType == TERRA_AIR )
				{
					continue;
				}

				// select vertex & index offsets
				uint32 vbOffset = vbOffsets[ terraType ];
				uint32 ibOffset = ibOffsets[ terraType ];

				// select vertex count
				uint32 vertexCount = _vertexCounts[ terraType ];

				// set uv offsets
				float32 uOffset;
				float32 vOffset;
				switch( CalcTerraFlavor( x, y, z ) )
				{
				case 0:
					uOffset = 0;
					vOffset = 0;
					break;

				case 1:
					uOffset = TERRATEXSIZE;
					vOffset = 0;
					break;

				case 2:
					uOffset = 0;
					vOffset = TERRATEXSIZE;
					break;

				case 3:
					uOffset = TERRATEXSIZE;
					vOffset = TERRATEXSIZE;
					break;

				NO_DEFAULT
				}

				// get spin
				uint32 spin = GetSCubeSpin( rleCubeData );

				// handle rotations (shitloads of variables for this)
				float32	cX;
				float32	cY;
				float32	cZ;
				uint32	xIndex;
				uint32	yIndex;
				uint32	zIndex;
				uint32	front;
				uint32	back;
				uint32	left;
				uint32	right;
				uint32	top;
				uint32	bottom;
				float32	xOffA;
				float32	xOffB;
				float32	yOffA;
				float32	yOffB;
				float32	zOffA;
				float32	zOffB;
				switch( GetSCubeRotation( rleCubeData ) )
				{
				case 0: // (default orientation) bottom side faces bottom in viewport
					bottom = VIS_BOTTOM;
					top = VIS_TOP;
					switch( spin ) // each consecutive spin is a clockwise rotation with respect to the original bottom-top o
					{
					case 0:
						back = VIS_BACK; front = VIS_FRONT; right = VIS_RIGHT; left = VIS_LEFT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = 0.0f;			xOffB = SCUBE_LENGTH; // last two variables are in a positive configuration
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 1; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 1:
						back = VIS_RIGHT; front = VIS_LEFT; right = VIS_FRONT; left = VIS_BACK;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = SCUBE_LENGTH;	xOffB = 0.0f;
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 2:
						back = VIS_FRONT; front = VIS_BACK; right = VIS_LEFT; left = VIS_RIGHT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = SCUBE_LENGTH;	xOffB = 0.0f;
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 1; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 3:
						back = VIS_LEFT; front = VIS_RIGHT; right = VIS_BACK; left = VIS_FRONT;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;
					}
					break;

				case 1: // bottom faces top 
					bottom = VIS_TOP;
					top = VIS_BOTTOM;
					switch( spin )
					{
					case 0:
						back = VIS_BACK; front = VIS_FRONT; right = VIS_LEFT; left = VIS_RIGHT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = SCUBE_LENGTH;	xOffB = 0.0f;
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 1; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 1:
						back = VIS_LEFT; front = VIS_RIGHT; right = VIS_FRONT; left = VIS_BACK;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = SCUBE_LENGTH;	xOffB = 0.0f;
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 2:
						back = VIS_FRONT; front = VIS_BACK; right = VIS_RIGHT; left = VIS_LEFT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 1; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 3:
						back = VIS_RIGHT; front = VIS_LEFT; right = VIS_BACK; left = VIS_FRONT;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;
					}
					break;

				case 2: // bottom faces left
					bottom = VIS_LEFT;
					top = VIS_RIGHT;
					switch( spin )
					{
					case 0:
						back = VIS_BACK; front = VIS_FRONT; right = VIS_BOTTOM; left = VIS_TOP;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 2; xOffA = SCUBE_LENGTH;	xOffB = 0.0f;
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 1; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.x + float32(x) * SCUBE_LENGTH; zIndex = 0; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 1:
						back = VIS_BOTTOM; front = VIS_TOP; right = VIS_FRONT; left = VIS_BACK;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = SCUBE_LENGTH;	xOffB = 0.0f; 
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.z + float32(z) * SCUBE_LENGTH; zIndex = 2; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 2:
						back = VIS_FRONT; front = VIS_BACK; right = VIS_TOP; left = VIS_BOTTOM;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 2; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 1; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.x + float32(x) * SCUBE_LENGTH; zIndex = 0; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 3:
						back = VIS_TOP; front = VIS_BOTTOM; right = VIS_BACK; left = VIS_FRONT;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.z + float32(z) * SCUBE_LENGTH; yIndex = 2; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.x + float32(x) * SCUBE_LENGTH; zIndex = 0; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;
					}
					break;

				case 3: // bottom faces right
					bottom = VIS_RIGHT;
					top = VIS_LEFT;
					switch( spin )
					{
					case 0:
						back = VIS_BACK; front = VIS_FRONT; right = VIS_TOP; left = VIS_BOTTOM;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 2; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 1; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.x + float32(x) * SCUBE_LENGTH; zIndex = 0; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 1:
						back = VIS_TOP; front = VIS_BOTTOM; right = VIS_FRONT; left = VIS_BACK;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = SCUBE_LENGTH;	xOffB = 0.0f; 
						cY = offsetOrigin.z + float32(z) * SCUBE_LENGTH; yIndex = 2; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.x + float32(x) * SCUBE_LENGTH; zIndex = 0; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 2:
						back = VIS_FRONT; front = VIS_BACK; right = VIS_BOTTOM; left = VIS_TOP;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 1; xOffA = SCUBE_LENGTH;	xOffB = 0.0f; 
						cY = offsetOrigin.y + float32(y) * SCUBE_LENGTH; yIndex = 0; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.x + float32(x) * SCUBE_LENGTH; zIndex = 2; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 3:
						back = VIS_BOTTOM; front = VIS_TOP; right = VIS_BACK; left = VIS_FRONT;
						cX = offsetOrigin.y + float32(y) * SCUBE_LENGTH; xIndex = 1; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.z + float32(z) * SCUBE_LENGTH; yIndex = 2; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.x + float32(x) * SCUBE_LENGTH; zIndex = 0; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;
					}
					break;

				case 4: // bottom faces front
					bottom = VIS_FRONT;
					top = VIS_BACK;
					switch( spin )
					{
					case 0:
						back = VIS_BOTTOM; front = VIS_TOP; right = VIS_RIGHT; left = VIS_LEFT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.z + float32(z) * SCUBE_LENGTH; yIndex = 2; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 1:
						back = VIS_RIGHT; front = VIS_LEFT; right = VIS_TOP; left = VIS_BOTTOM;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 2; xOffA = SCUBE_LENGTH;	xOffB = 0.0f; 
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;

					case 2:
						back = VIS_TOP; front = VIS_BOTTOM; right = VIS_LEFT; left = VIS_RIGHT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = SCUBE_LENGTH; xOffB = 0.0f; 
						cY = offsetOrigin.z + float32(z) * SCUBE_LENGTH; yIndex = 2; yOffA = SCUBE_LENGTH; yOffB = 0.0f;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = SCUBE_LENGTH; zOffB = 0.0f;
						break;

					case 3:
						back = VIS_LEFT; front = VIS_RIGHT; right = VIS_BOTTOM; left = VIS_TOP;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 2; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = SCUBE_LENGTH;	zOffB = 0.0f;
						break;
					}
					break;

				case 5: // bottom faces back
					bottom = VIS_BACK;
					top = VIS_FRONT;
					switch( spin )
					{
					case 0:
						back = VIS_TOP; front = VIS_BOTTOM; right = VIS_RIGHT; left = VIS_LEFT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.z + float32(z) * SCUBE_LENGTH; yIndex = 2; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 1:
						back = VIS_RIGHT; front = VIS_LEFT; right = VIS_BOTTOM; left = VIS_TOP;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 2; xOffA = 0.0f;			xOffB = SCUBE_LENGTH;
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 2:
						back = VIS_BOTTOM; front = VIS_TOP; right = VIS_LEFT; left = VIS_RIGHT;
						cX = offsetOrigin.x + float32(x) * SCUBE_LENGTH; xIndex = 0; xOffA = SCUBE_LENGTH;	xOffB = 0.0f; 
						cY = offsetOrigin.z + float32(z) * SCUBE_LENGTH; yIndex = 2; yOffA = 0.0f;			yOffB = SCUBE_LENGTH;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;

					case 3:
						back = VIS_LEFT; front = VIS_RIGHT; right = VIS_TOP; left = VIS_BOTTOM;
						cX = offsetOrigin.z + float32(z) * SCUBE_LENGTH; xIndex = 2; xOffA = SCUBE_LENGTH;	xOffB = 0.0f; 
						cY = offsetOrigin.x + float32(x) * SCUBE_LENGTH; yIndex = 0; yOffA = SCUBE_LENGTH;	yOffB = 0.0f;
						cZ = offsetOrigin.y + float32(y) * SCUBE_LENGTH; zIndex = 1; zOffA = 0.0f;			zOffB = SCUBE_LENGTH;
						break;
					}
					break;
				}

				// write in data
				switch( GetSCubeType( rleCubeData ) )
				{
				case SCUBE_CUBE:
					if( vis & front )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & back )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & left )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & right )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & top )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;			vb[ vbOffset + yIndex ] = cY + yOffB;			vb[ vbOffset + zIndex ] = cZ + zOffB;			vbOffset += 3;			vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;			vb[ vbOffset + yIndex ] = cY + yOffB;			vb[ vbOffset + zIndex ] = cZ + zOffB;			vbOffset += 3;			vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;			vb[ vbOffset + yIndex ] = cY + yOffA;			vb[ vbOffset + zIndex ] = cZ + zOffB;			vbOffset += 3;			vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;			vb[ vbOffset + yIndex ] = cY + yOffA;			vb[ vbOffset + zIndex ] = cZ + zOffB;			vbOffset += 3;			vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;				ib[ ibOffset++ ] = vertexCount + 1;				ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;				ib[ ibOffset++ ] = vertexCount + 3;				ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & bottom )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
				break;

				case SCUBE_THREEQUARTERCUBE:
					if( (vis & front ) ||( vis & top ) ||( vis & right ) )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 3;
					}
					if( vis & front )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( vis & back )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & left )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( vis & right )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & top )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 3;
					}
					if( vis & bottom )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
				break;

				case SCUBE_HALFCUBE:
					if( vis & front )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;				vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;				vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & back )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;				vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;				vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & left )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;				vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;				vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & right )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;				vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;				vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & top )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + SCUBE_HALFLENGTH;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & bottom )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
				break;

				case SCUBE_HALFCUBETRIANGLE:
					if( (vis & front ) ||( vis & top ) )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2; 
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & back )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & left )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( vis & right )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( vis & bottom )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
				break;

				case SCUBE_HALFCUBEWEDGE:
					if( (vis & front ) ||( vis & top ) ||( vis & right ) )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 3;
					}
					if( (vis & back ) ||( vis & top ) ||( vis & left ) )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 3;
					}
					if( vis & front )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( vis & back )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( vis & left )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( vis & right )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( vis & bottom )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
				break;

				case SCUBE_QUARTERCUBEPYRAMID:
					if( (vis & front ) ||( vis & top ) )
					{
						vb[ vbOffset + xIndex ] = cX + SCUBE_HALFLENGTH;	vb[ vbOffset + yIndex ] = cY + SCUBE_HALFLENGTH;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top
						vb[ vbOffset + xIndex ] = cX + xOffA;				vb[ vbOffset + yIndex ] = cY + yOffA;				vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;				vb[ vbOffset + yIndex ] = cY + yOffA;				vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( (vis & back ) ||( vis & top ) )
					{
						vb[ vbOffset + xIndex ] = cX + SCUBE_HALFLENGTH;	vb[ vbOffset + yIndex ] = cY + SCUBE_HALFLENGTH;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top
						vb[ vbOffset + xIndex ] = cX + xOffB;				vb[ vbOffset + yIndex ] = cY + yOffB;				vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;				vb[ vbOffset + yIndex ] = cY + yOffB;				vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( (vis & left ) ||( vis & top ) )
					{
						vb[ vbOffset + xIndex ] = cX + SCUBE_HALFLENGTH;	vb[ vbOffset + yIndex ] = cY + SCUBE_HALFLENGTH;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top
						vb[ vbOffset + xIndex ] = cX + xOffA;				vb[ vbOffset + yIndex ] = cY + yOffB;				vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;				vb[ vbOffset + yIndex ] = cY + yOffA;				vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( (vis & right ) ||( vis & top ) )
					{
						vb[ vbOffset + xIndex ] = cX + SCUBE_HALFLENGTH;	vb[ vbOffset + yIndex ] = cY + SCUBE_HALFLENGTH;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top
						vb[ vbOffset + xIndex ] = cX + xOffB;				vb[ vbOffset + yIndex ] = cY + yOffA;				vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;				vb[ vbOffset + yIndex ] = cY + yOffB;				vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( vis & bottom )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
				break;

				case SCUBE_QUARTERCUBECORNERPYRAMID:
					if( vis & front )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( (vis & back ) && !(vis & top ) )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( vis & left )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( (vis & right ) && !(vis & top ) )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 3;
					}
					if( vis & top )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
					if( vis & bottom )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 3;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 4;
					}
				break;

				case SCUBE_QUARTERCUBETRIANGLE:
					if( (vis & front ) ||( vis & top ) ||( vis & right ) )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( vis & back )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( vis & left )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( vis & bottom )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 3;
					}
				break;

				case SCUBE_QUARTERCUBESTRANGETRIANGLE:
					if( (vis & front ) ||( vis & top ) ) // front perspective used
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2; 
						vertexCount += 3;
					}
					if( vis & back )
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset + TERRATEXSIZE;	vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 1;	ib[ ibOffset++ ] = vertexCount + 2;
						vertexCount += 3;
					}
					if( vis & left )
					{
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
					if( (vis & right ) ||( vis & bottom ) ) // right perspective used
					{
						vb[ vbOffset + xIndex ] = cX + xOffB;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffB;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset;					// top right
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffA;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom left
						vb[ vbOffset + xIndex ] = cX + xOffA;	vb[ vbOffset + yIndex ] = cY + yOffB;	vb[ vbOffset + zIndex ] = cZ + zOffA;	vbOffset += 3;	vb[ vbOffset++ ] = uOffset;					vb[ vbOffset++ ] = vOffset + TERRATEXSIZE;	// bottom right
						ib[ ibOffset++ ] = vertexCount + 0;	ib[ ibOffset++ ] = vertexCount + 2;	ib[ ibOffset++ ] = vertexCount + 1;
						vertexCount += 3;
					}
				break;

				NO_DEFAULT
				}

				// update offsets
				vbOffsets[ terraType ] = vbOffset;
				ibOffsets[ terraType ] = ibOffset;

				// update vertex count
				_vertexCounts[ terraType ] = vertexCount;
			}
		}
	}
}