//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#include "public\nstdlib\platform.h"
#include "public\nstdlib\pair.h"
#include "public\nstdlib\vector.h"
#include "public\nstdlib\upoint3d.h"

// STODO: it may be necessary to add a road group class or destination info to speed travel calculations for npcs, since a curvy road stretch could be composed of hundreds of road sections
// STODO: REFACTOR: change to a node based system. much of the code shouldnt have to change much to do this. the road map would store pairs of nodes rather than pointers to segments
	// the build/intersect method would just take another node as an argument and build the segment 
	// use a parent child relationship between road nodes. the root node in a town would be the entrance. 
	// to build roads in a town iterate through each road node and call build with each of its children

enum RoadType
{
	ROAD_BRIDGE = 0, // STODO: this will have specialized building: dont fill (too much)space below and add support pillars that depend on the size of the road.
	ROAD_HIGHWAY, // largest roads (usually inter-town)
	ROAD_PRIMARY, // large roads
	ROAD_SECONDARY, // lesser roads
	ROAD_TRAIL
};

enum RoadDirection
{
	RDIRECTION_SOUTHTONORTH = 0,
	RDIRECTION_WESTTOEAST,
	RDIRECTION_SOUTHWESTTONORTHEAST,
	RDIRECTION_SOUTHEASTTONORTHWEST
};
// Note- max and min definitions for roads: max gets assigned to the highest z value. if the z values are equal then its the highest y value. 
		// if the y values are also equal, the highest x value determines the max point
class RoadNode: public UPoint3D
{
public:
	RoadNode( const UPoint3D& point, RoadType type );
	RoadNode( const RoadNode& node );
	virtual ~RoadNode( );

	bool			BuildOrDoIntersection( RoadNode& childNode, bool doPointIntersection, UPoint3D* point = NULL );
	void			FillRoadMap( RoadNode& connectedNode, Vector< Pair< RoadNode*, RoadNode* > >** roadMap_, uint32 xLength, uint32 yLength, uint32 zoneXMin, uint32 zoneYMin );

	void			AddParentNode( RoadNode& other, bool isLinkBidirectional );
	void			AddChildNode( RoadNode& other, bool isLinkBidirectional );

	uint32			GetNumParentNodes( );
	uint32			GetNumChildNodes( );
	RoadNode*		GetParentNode( uint32 index );
	RoadNode*		GetChildNode( uint32 index );

	uint32			GetWidth( );

private:

	RoadDirection	DetermineRoadDirectionBetweenPoints_( const UPoint3D& firstPoint, const UPoint3D& secondPoint );
	void			DetermineRoadBoundsBetweenPoints_( UPoint3D*& min, UPoint3D*& max, UPoint3D* firstPoint, UPoint3D* secondPoint );

	uint32				width_; // Note- width between nodes is determined by the node with the lowest width
					// Note- let width determine grid aligned width. diagonal width must be different for looks
					// 3 width = 3 diagonal. 4 width = 4 diagonal. 5 width = 4 diagonal. 6 width = 5 diagonal

	//bool isHidden_; // determines if road is built or not. npcs (that know about it) treat it the same as a normal road.
	RoadType			type_; // Note- type of road built between nodes is determined by highest road type
	//RoadDirection direction_;

	Vector<RoadNode*>	parentNodes_;
	Vector<RoadNode*>	childNodes_;

	// STODO: possibly store connections that road links to such as buildings, towns, etc
};