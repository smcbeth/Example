//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

FORCE_INLINE void
SetSCubeType( uint32* sCube, SCUBETYPE type )
{
	Assert( GetSCubeType( *sCube ) == SCUBE_CUBE );	// prevent setting twice without clearning
	*sCube |= uint32(type) << 28;					// 16 possibilities
}

FORCE_INLINE void
SetSCubeTerrainType( uint32* sCube, TERRATYPE terraType )
{
	Assert( GetSCubeTerrainType( *sCube ) == TERRA_AIR );	// prevent setting twice without clearning
	Assert( terraType < TERRATYPE_COUNT );
	*sCube |= uint32(terraType) << 20;						// 256 possibilities
}

FORCE_INLINE void
SetSCubeRotation( uint32* sCube, uint32 rot )
{
	*sCube |= rot << 17; // 6 possibilities
}

FORCE_INLINE void
SetSCubeSpin( uint32* sCube, uint32 spin )
{
	*sCube |= spin << 15; // 4 possibilities
}

FORCE_INLINE void
SetSCubeVisibility( uint32* sCube, uint32 vis )
{
	*sCube |= vis << 9; // 6 bools
}

FORCE_INLINE void 
SetSCubeHealth( uint32* sCube, uint32 health )
{
	Assert( health <= 31 );
	*sCube |= health << 4; // 0-31 points
}

FORCE_INLINE SCUBETYPE
GetSCubeType( uint32 sCube )
{
	sCube >>= 28;
	return SCUBETYPE(sCube & 0xF);
}

FORCE_INLINE TERRATYPE
GetSCubeTerrainType( uint32 sCube )
{
	SCUBETYPE sCubeType = GetSCubeType( sCube ); // DEBUG_TERRAINHELPER:
	sCube >>= 20;
	TERRATYPE sCubeTType = TERRATYPE(sCube & 0xFF); // DEBUG_TERRAINHELPER:
	Assert ( TERRATYPE(sCube & 0xFF) < TERRATYPE_COUNT );
	return TERRATYPE(sCube & 0xFF);
}

FORCE_INLINE uint8
GetSCubeRotation( uint32 sCube )
{
	sCube >>= 17;
	return sCube & 0x7;
}

FORCE_INLINE uint8
GetSCubeSpin( uint32 sCube )
{
	sCube >>= 15;
	return sCube & 0x3;
}

FORCE_INLINE uint8
GetSCubeVisibility( uint32 sCube )
{
	sCube >>= 9;
	return sCube & 0x3F;
}

FORCE_INLINE uint8
GetSCubeHealth( uint32 sCube )
{
	sCube >>= 4;
	return sCube & 0x1F;
}

FORCE_INLINE void
ClearSCubeType( uint32* sCube )
{
	*sCube &= 0x0FFFFFFF;
	Assert( GetSCubeType( *sCube ) == SCUBE_CUBE );
}

FORCE_INLINE void
ClearSCubeTerrainType( uint32* sCube )
{
	*sCube &= 0xF00FFFFF;
	Assert( GetSCubeTerrainType( *sCube ) == TERRA_AIR );
}

FORCE_INLINE void
ClearSCubeRotation( uint32* sCube )
{
	*sCube &= 0xFFF1FFFF;
	Assert( GetSCubeRotation( *sCube ) == 0 );
}

FORCE_INLINE void
ClearSCubeSpin( uint32* sCube )
{
	*sCube &= 0xFFFE7FFF;
	Assert( GetSCubeSpin( *sCube ) == 0 );
}

FORCE_INLINE void
ClearSCubeVisibility( uint32* sCube )
{
	*sCube &= 0xFFFF81FF;
	Assert( GetSCubeVisibility( *sCube ) == 0 );
}

FORCE_INLINE void
ClearSCubeHealth( uint32* sCube )
{
	*sCube &= 0xFFFFFE0F;
	Assert( GetSCubeHealth( *sCube ) == 0 );
}

FORCEINLINE bool
IsTerrainTypeABuildingMaterial( TERRATYPE type )
{
	if( type == TERRA_BOARDS || type == TERRA_BRICK || type == TERRA_STONEBRICK || type == TERRA_THATCH )
		return true;
	return false;
}

// Note- This must run as quickly as possible
FORCE_INLINE uint8
CalcTerraFlavor( int32 x, int32 y, int32 z )
{
	// STODO: find out how we want to do this, possibly a seperate random stream
	return uint8(rand() % 4);
}