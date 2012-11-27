//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "roadnode.h"
#include "public\nstdlib\_math.h"
#include "public\nstdlib\upoint3d.h"
#include "public\nstdlib\vector.h"
#include "world.h"
#include "basearea.h"
#include "terrainhelper.h"


RoadNode::RoadNode( const UPoint3D& point, RoadType type )
	: UPoint3D( point ), type_( type )
{
	// TODO: refine type specifications more in the future
	switch( type )
	{
	case ROAD_BRIDGE:
		width_ = 4;
		break;
	case ROAD_HIGHWAY:
		width_ = 4;
		break;
	case ROAD_PRIMARY:
		width_ = 3;
		break;
	case ROAD_SECONDARY:
		width_ = 2;
		break;
	case ROAD_TRAIL:
		width_ = 2;
		break;
	}
}

RoadNode::RoadNode( const RoadNode& node )
	: UPoint3D( node ), width_( node.width_ ), type_( node.type_ )
{
	for( uint32 i = 0; i < node.parentNodes_.GetCount(); ++i )
	{
		parentNodes_.InsertBack( node.parentNodes_[i] );
	}
	for( uint32 i = 0; i < node.childNodes_.GetCount(); ++i )
	{
		childNodes_.InsertBack( node.childNodes_[i] );
	}
}

RoadNode::~RoadNode()
{
	;
}

bool
RoadNode::BuildOrDoIntersection( RoadNode& childNode, bool doPointIntersection, UPoint3D* point )
{
	// SNOW: consolidate this with FillRoadMap in a private method. have Build(), IntersectWithPoint(), and FillRoadMap() as three separate public methods that call this private method.

	// Note- some of this code is repeated in FillRoadMap. See note in FillRoadMap.
	// STODO: smooth terrain up to road for best looks

	// STODO: possibly separate this method into build and intersection. do this after building has been more finalized
	Assert( TERRA_AIR == 0 );
	Assert( GetWorld()->GetIsModifying() );

	RoadDirection direction = DetermineRoadDirectionBetweenPoints_( *this, childNode );
	UPoint3D* min;
	UPoint3D* max;
	DetermineRoadBoundsBetweenPoints_( min, max, this, &childNode );

	// STODO: check connecting roads and fill in gaps created when grid aligned roads connect to diagonal roads of large enough size
	// SNOW: clear two+ blocks above road with air

	switch( direction )
	{
	case RDIRECTION_SOUTHTONORTH:
		if( min->z == max->z )
		{
			for( uint32 y = min->y; y <= max->y; ++y )
			{
				for( uint32 x = min->x - width_ / 2; x <= (min->x - (width_ / 2) + width_); ++x )
				{
					if( IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( x, y, min->z ) ) ) )
						continue;

					if( doPointIntersection && *point == UPoint3D(x, y, min->z) )
						return true;

					uint32 sCubeBuf = 0;
					SetSCubeTerrainType( &sCubeBuf, TERRA_DIRTROAD ); // STODO: make this a different road-only terrain type(s)
					GetWorld()->SetSCube( x, y, min->z, sCubeBuf );
					GetWorld()->SetSCube( x, y, min->z + 1, TERRA_AIR );
					GetWorld()->SetSCube( x, y, min->z + 2, TERRA_AIR );
				}
			}
		}
		else if( min->y < max->y )
		{
			uint32 z = min->z;
			for( uint32 y = min->y; y <= max->y; ++y )
			{
				for( uint32 x = min->x - width_ / 2; x <= (min->x - (width_ / 2) + width_); ++x )
				{
					if( IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( x, y, z ) ) ) )
						continue;

					if( doPointIntersection && *point == UPoint3D(x, y, z) )
						return true;

					uint32 sCubeBuf = 0;
					SetSCubeTerrainType( &sCubeBuf, TERRA_DIRTROAD ); // STODO: make this a different road-only terrain type(s)
					GetWorld()->SetSCube( x, y, z, sCubeBuf );
					GetWorld()->SetSCube( x, y, z + 1, TERRA_AIR );
					GetWorld()->SetSCube( x, y, z + 2, TERRA_AIR );
				}
				++z;
			}
		}
		else
		{
			uint32 z = max->z;
			for( uint32 y = max->y; y <= min->y; ++y )
			{
				for( uint32 x = min->x - width_ / 2; x <= (min->x - (width_ / 2) + width_); ++x )
				{
					if( IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( x, y, z ) ) ) )
						continue;

					if( doPointIntersection && *point == UPoint3D(x, y, z) )
						return true;

					uint32 sCubeBuf = 0;
					SetSCubeTerrainType( &sCubeBuf, TERRA_DIRTROAD ); // STODO: make this a different road-only terrain type(s)
					GetWorld()->SetSCube( x, y, z, sCubeBuf );
					GetWorld()->SetSCube( x, y, z + 1, TERRA_AIR );
					GetWorld()->SetSCube( x, y, z + 2, TERRA_AIR );
				}
				--z;
			}
		}
		break;
	case RDIRECTION_WESTTOEAST:
		if( min->z == max->z )
		{
			for( uint32 y = min->y - width_ / 2; y <= (min->y - (width_ / 2) + width_); ++y )
			{
				for( uint32 x = min->x; x <= max->x; ++x )
				{
					if( IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( x, y, min->z ) ) ) )
						continue;

					if( doPointIntersection && *point == UPoint3D(x, y, min->z) )
						return true;

					uint32 sCubeBuf = 0;
					SetSCubeTerrainType( &sCubeBuf, TERRA_DIRTROAD ); // STODO: make this a different road-only terrain type(s)
					GetWorld()->SetSCube( x, y, min->z, sCubeBuf );
					GetWorld()->SetSCube( x, y, min->z + 1, TERRA_AIR );
					GetWorld()->SetSCube( x, y, min->z + 2, TERRA_AIR );
				}
			}
		}
		else if( min->x < max->x )
		{
			for( uint32 y = min->y - width_ / 2; y <= (min->y - (width_ / 2) + width_); ++y )
			{
				uint32 z = min->z;
				for( uint32 x = min->x; x <= max->x; ++x )
				{
					if( IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( x, y, z ) ) ) )
						continue;

					if( doPointIntersection && *point == UPoint3D(x, y, z) )
						return true;

					uint32 sCubeBuf = 0;
					SetSCubeTerrainType( &sCubeBuf, TERRA_DIRTROAD ); // STODO: make this a different road-only terrain type(s)
					GetWorld()->SetSCube( x, y, z, sCubeBuf );
					GetWorld()->SetSCube( x, y, z + 1, TERRA_AIR );
					GetWorld()->SetSCube( x, y, z + 2, TERRA_AIR );
					++z;
				}
			}
		}
		else
		{
			for( uint32 y = min->y - width_ / 2; y <= (min->y - (width_ / 2) + width_); ++y )
			{
				uint32 z = max->z;
				for( uint32 x = max->x, z = max->z; x <= min->x; ++x )
				{
					if( IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( x, y, z ) ) ) )
						continue;

					if( doPointIntersection && *point == UPoint3D(x, y, z) )
						return true;

					uint32 sCubeBuf = 0;
					SetSCubeTerrainType( &sCubeBuf, TERRA_DIRTROAD ); // STODO: make this a different road-only terrain type(s)
					GetWorld()->SetSCube( x, y, z, sCubeBuf );
					GetWorld()->SetSCube( x, y, z + 1, TERRA_AIR );
					GetWorld()->SetSCube( x, y, z + 2, TERRA_AIR );
					--z;
				}
			}
		}
		break;
	case RDIRECTION_SOUTHWESTTONORTHEAST:
		{
			uint32 z;
			int32 zInc;
			uint32 x = Min(min->x, max->x);
			if( min->z == max->z )
			{
				z = min->z;
				zInc = 0;
			}
			else if( min->y < max->y )
			{
				Assert( min->x < max->x );
				z = min->z;
				zInc = 1;
			}
			else
			{
				Assert( min->x > max->x );
				z = max->z;
				zInc = -1;
			}

			for( uint32 y = Min(min->y, max->y); y <= Max(min->y, max->y); ++x, ++y )
			{
				bool doSetLowerYBlock = false; // this avoids setting the next block down on y on the first iteration for correct appearance
				// 
				for( uint32 xMod = x - (width_ / 2) + width_, yMod = y - (width_ / 2); yMod <= y - width_ / 2 + width_; --xMod, ++yMod )
				{
					uint32 sCubeBuf = 0;
					SetSCubeTerrainType( &sCubeBuf, TERRA_DIRTROAD ); // STODO: make this a different road-only terrain type(s)

					if( !IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( xMod, yMod, z ) ) ) )
					{
						if( doPointIntersection && *point == UPoint3D(xMod, yMod, z) )
						return true;

						GetWorld()->SetSCube( uint32(xMod), uint32(yMod), z, sCubeBuf );
						GetWorld()->SetSCube( uint32(xMod), uint32(yMod), z + 1, TERRA_AIR );
						GetWorld()->SetSCube( uint32(xMod), uint32(yMod), z + 2, TERRA_AIR );
					}

					if( doSetLowerYBlock )
					{
						if( IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( xMod, yMod - 1, z ) ) ) )
							continue;

						if( doPointIntersection && *point == UPoint3D(xMod, yMod - 1, z) )
							return true;

						GetWorld()->SetSCube( uint32(xMod), uint32(yMod - 1), z, sCubeBuf );
						GetWorld()->SetSCube( uint32(xMod), uint32(yMod - 1), z + 1, TERRA_AIR );
						GetWorld()->SetSCube( uint32(xMod), uint32(yMod - 1), z + 2, TERRA_AIR );
					}
					else
					{
						doSetLowerYBlock = true;
					}
				}
				z = uint32(int32(z) + zInc);
			}
			break;
		}
	case RDIRECTION_SOUTHEASTTONORTHWEST:
		{
			uint32 z;
			int32 zInc;
			
			if( min->z == max->z )
			{
				z = min->z;
				zInc = 0;
			}
			else if( min->x < max->x )
			{
				Assert( min->y > max->y );
				z = max->z;
				zInc = -1;
			}
			else
			{
				Assert( min->y < max->y );
				z = min->z;
				zInc = 1;
			}
			// uint32 x = Min(min->x, max->x)
			for( uint32 x = Max(min->x, max->x), y = Min(min->y, max->y); y <= Max(min->y, max->y); --x, ++y )
			{
				bool doSetLowerYBlock = false; // this avoids setting the next block down on y on the first iteration for correct appearance
				
				for( uint32 xMod = x - (width_ / 2), yMod = y - (width_ / 2); yMod <= y - (width_ / 2) + width_; ++xMod, ++yMod )
				{
					uint32 sCubeBuf = 0;
					SetSCubeTerrainType( &sCubeBuf, TERRA_DIRTROAD ); // STODO: make this a different road-only terrain type(s)

					if( !IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( xMod, yMod, z ) ) ) )
					{
						if( doPointIntersection && *point == UPoint3D(xMod, yMod, z) )
							return true;
					
						GetWorld()->SetSCube( uint32(xMod), uint32(yMod), z, sCubeBuf );
						GetWorld()->SetSCube( uint32(xMod), uint32(yMod), z + 1, TERRA_AIR );
						GetWorld()->SetSCube( uint32(xMod), uint32(yMod), z + 2, TERRA_AIR );
					}

					if( doSetLowerYBlock )
					{
						if( IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( xMod, yMod - 1, z ) ) ) )
							continue;

						if( doPointIntersection && *point == UPoint3D(xMod, yMod - 1, z) )
							return true;

						GetWorld()->SetSCube( uint32(xMod), uint32(yMod - 1), z, sCubeBuf );
						GetWorld()->SetSCube( uint32(xMod), uint32(yMod - 1), z + 1, TERRA_AIR );
						GetWorld()->SetSCube( uint32(xMod), uint32(yMod - 1), z + 2, TERRA_AIR );
					}
					else
					{
						doSetLowerYBlock = true;
					}
				}
				z = uint32(int32(z) + zInc);
			}
			break;
		}
	}

	return false;
}

void
RoadNode::FillRoadMap( RoadNode& connectedNode, Vector< Pair< RoadNode*, RoadNode* > >** roadMap_, uint32 xLength, uint32 yLength, uint32 zoneXMin, uint32 zoneYMin )
{
	// Note- some of this code is a repeat of BuildOrDoIntersection. However this code is much simplier since z levels arent taken into account.
		// ANY APPLICABLE CHANGES IN BuildOrDoIntersection MUST BE REFLECTED HERE
	RoadDirection direction = DetermineRoadDirectionBetweenPoints_( *this, connectedNode );
	UPoint3D* min;
	UPoint3D* max;
	DetermineRoadBoundsBetweenPoints_( min, max, this, &connectedNode );

	switch( direction )
	{
	case RDIRECTION_SOUTHTONORTH:
		for( uint32 y = min->y; y <= max->y; ++y )
		{
			for( uint32 x = min->x - width_ / 2; x <= (min->x - (width_ / 2) + width_); ++x )
			{
				if( IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( x, y, min->z ) ) ) )
					continue;

				Vector< Pair< RoadNode*, RoadNode* > >*& mapLocation = roadMap_[(x - zoneXMin) + (y - zoneYMin) * xLength];
				if( mapLocation == NULL )
					mapLocation = new Vector< Pair< RoadNode*, RoadNode* > >();
				mapLocation->InsertBack( Pair< RoadNode*, RoadNode* >( this, &connectedNode ) );
			}
		}
		break;
	case RDIRECTION_WESTTOEAST:
		for( uint32 y = min->y - width_ / 2; y <= (min->y - (width_ / 2) + width_); ++y )
		{
			for( uint32 x = min->x; x <= max->x; ++x )
			{
				if( IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( x, y, min->z ) ) ) )
					continue;

				Vector< Pair< RoadNode*, RoadNode* > >*& mapLocation = roadMap_[(x - zoneXMin) + (y - zoneYMin) * xLength];
				if( mapLocation == NULL )
					mapLocation = new Vector< Pair< RoadNode*, RoadNode* > >();
				mapLocation->InsertBack( Pair< RoadNode*, RoadNode* >( this, &connectedNode ) );
				uint32 breakhere = 1;
			}
		}
	
		break;
	case RDIRECTION_SOUTHWESTTONORTHEAST:
		{
			for( uint32 x = Min(min->x, max->x), y = Min(min->y, max->y); y <= Max(min->y, max->y); ++x, ++y )
			{
				bool doSetLowerYBlock = false; // this avoids setting the next block down on y on the first iteration for correct appearance
				// uint32 xMod = x - (width_ / 2)
				for( uint32 xMod = x - (width_ / 2) + width_, yMod = y - (width_ / 2); yMod <= y - (width_ / 2) + width_; --xMod, ++yMod )
				{
					if( xMod < zoneXMin || xMod > zoneXMin + xLength )
						continue;
					if( yMod < zoneYMin || yMod > zoneYMin + yLength )
						continue;

					if( !IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( xMod, yMod, z ) ) ) )
					{
						Vector< Pair< RoadNode*, RoadNode* > >*& mapLocation = roadMap_[(uint32(xMod) - zoneXMin) + (uint32(yMod) - zoneYMin) * xLength];
						if( mapLocation == NULL )
							mapLocation = new Vector< Pair< RoadNode*, RoadNode* > >();
						mapLocation->InsertBack( Pair< RoadNode*, RoadNode* >( this, &connectedNode ) );
					}

					if( doSetLowerYBlock )
					{
						if( IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( xMod, yMod - 1, z ) ) ) )
							continue;

						Vector< Pair< RoadNode*, RoadNode* > >*& mapLocationLower = roadMap_[(uint32(xMod) - zoneXMin) + (uint32(yMod - 1) - zoneYMin) * xLength];
						if( mapLocationLower == NULL )
							mapLocationLower = new Vector< Pair< RoadNode*, RoadNode* > >();
						mapLocationLower->InsertBack( Pair< RoadNode*, RoadNode* >( this, &connectedNode ) );
					}
					else
					{
						doSetLowerYBlock = true;
					}
					
				}
			}
			break;
		}
	case RDIRECTION_SOUTHEASTTONORTHWEST:
		{
			 // SNOW: verify this
			// uint32 x = Min(min->x, max->x)
			for( uint32 x = Max(min->x, max->x), y = Min(min->y, max->y); y <= Max(min->y, max->y); --x, ++y )
			{
				bool doSetLowerYBlock = false; // this avoids setting the next block down on y on the first iteration for correct appearance
				for( uint32 xMod = x - (width_ / 2), yMod = y - (width_ / 2); yMod <= y - (width_ / 2) + width_; ++xMod, ++yMod )
				{
					if( xMod < zoneXMin || xMod > zoneXMin + xLength )
						continue;
					if( yMod < zoneYMin || yMod > zoneYMin + yLength )
						continue;

					if( !IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( xMod, yMod, z ) ) ) )
					{
						Vector< Pair< RoadNode*, RoadNode* > >*& mapLocation = roadMap_[(uint32(xMod) - zoneXMin) + (uint32(yMod) - zoneYMin) * xLength];
						if( mapLocation == NULL )
							mapLocation = new Vector< Pair< RoadNode*, RoadNode* > >();
						mapLocation->InsertBack( Pair< RoadNode*, RoadNode* >( this, &connectedNode ) );
					}

					if( doSetLowerYBlock )
					{
						if( IsTerrainTypeABuildingMaterial( GetSCubeTerrainType( GetWorld()->GetSCube( xMod, yMod - 1, z ) ) ) )
							continue;

						Vector< Pair< RoadNode*, RoadNode* > >*& mapLocationLower = roadMap_[(uint32(xMod) - zoneXMin) + (uint32(yMod - 1) - zoneYMin) * xLength];
						if( mapLocationLower == NULL )
							mapLocationLower = new Vector< Pair< RoadNode*, RoadNode* > >();
						mapLocationLower->InsertBack( Pair< RoadNode*, RoadNode* >( this, &connectedNode ) );
					}
					else
					{
						doSetLowerYBlock = true;
					}
				}
			}
			break;
		}
	}

}

void
RoadNode::AddParentNode( RoadNode& other, bool isLinkBidirectional )
{
	parentNodes_.InsertBack( &other );
	if( isLinkBidirectional )
		other.AddChildNode( *this, false );
}

void
RoadNode::AddChildNode( RoadNode& other, bool isLinkBidirectional )
{
	childNodes_.InsertBack( &other );
	if( isLinkBidirectional )
		other.AddParentNode( *this, false );
}

uint32
RoadNode::GetNumParentNodes( )
{
	return parentNodes_.GetCount();
}

uint32
RoadNode::GetNumChildNodes( )
{
	return childNodes_.GetCount();
}

RoadNode*
RoadNode::GetParentNode( uint32 index )
{
	Assert( parentNodes_.GetCount() > index );
	return parentNodes_[ index ];
}

RoadNode*
RoadNode::GetChildNode( uint32 index )
{
	Assert( childNodes_.GetCount() > index );
	return childNodes_[ index ];
}

uint32
RoadNode::GetWidth( )
{
	return width_;
}

RoadDirection
RoadNode::DetermineRoadDirectionBetweenPoints_( const UPoint3D& firstPoint, const UPoint3D& secondPoint )
{
	if( firstPoint.x == secondPoint.x )
	{
		return RDIRECTION_SOUTHTONORTH;
	}
	else if( firstPoint.y == secondPoint.y )
	{
		return RDIRECTION_WESTTOEAST;
	}
	else if( (firstPoint.x > secondPoint.x && firstPoint.y > secondPoint.y) || 
		(firstPoint.x < secondPoint.x && firstPoint.y < secondPoint.y) )
	{
		return RDIRECTION_SOUTHWESTTONORTHEAST;
	}
	else
	{
		return RDIRECTION_SOUTHEASTTONORTHWEST;
	}
}

void
RoadNode::DetermineRoadBoundsBetweenPoints_( UPoint3D*& min, UPoint3D*& max, UPoint3D* firstPoint, UPoint3D* secondPoint )
{
	if( firstPoint->z < secondPoint->z )
	{
		min = firstPoint;
		max = secondPoint;
	}
	else if( firstPoint->z > secondPoint->z )
	{
		min = secondPoint;
		max = firstPoint;
	}
	else if( firstPoint->y < secondPoint->y )
	{
		min = firstPoint;
		max = secondPoint;
	}
	else if( firstPoint->y > secondPoint->y )
	{
		min = secondPoint;
		max = firstPoint;
	}
	else if( firstPoint->x < secondPoint->x )
	{
		min = firstPoint;
		max = secondPoint;
	}
	else
	{
		min = secondPoint;
		max = firstPoint;
	}

	Assert( min->x <= max->x || min->y < max->y || min->z < max->z );
	Assert( min->y <= max->y || min->z < max->z );
	Assert( min->z <= max->z );
}
