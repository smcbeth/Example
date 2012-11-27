//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------

#pragma once

#include "public\nstdlib\platform.h"
#include "public\nstdlib\vector.h"
#include "public\nstdlib\sort.h"

#include "basearea.h"
#include "basezone.h"
#include "baseroom.h"
#include "iworldgenerator.h"
#include "roomlinker.h"

class BaseZone;
class RoomLinker;
class BaseRoom;

struct RoomLinkerInitData;
struct RoomInfo;
struct RoomInitData;

enum Direction;
enum IntersectType;

enum BuildingSize
{
	BUILDING_SMALL = 0,
	BUILDING_MEDIUM,
	BUILDING_LARGE
};

enum BuildingType
{
	BUILDING_UNKNOWN = 0,
	BUILDING_DUNGEONTOWER,
	BUILDING_GRANARY,
	BUILDING_HOUSE,
	BUILDING_MANORHOUSE,
	BUILDING_SHOP
};

enum RoofType
{
	ROOF_NONE = 0,
	ROOF_NORMAL, // flat or pointed depending on maxRoofHeight_
	ROOF_OPEN,
	ROOF_OPENPARAPETS,
	//STODO: ROOF_ANGLED_X,
	//STODO: ROOF_ANGLED_Z,
};

enum RoofFillType
{
	ROOFFILL_VAULTED = 0,
	ROOFFILL_SOLID,
	ROOFFILL_ATTIC
};

enum ImaginaryType // used in RoomPlacementSpace class
{
	IMAGINARY_FALSE = 0, // wall space or area space is defined by existing walls
	IMAGINARY_PARTIAL, // wall or area space is defined by a mix of existing walls and imaginary wall spaces
	IMAGINARY_TRUE // wall or area space does not correspond to existing walls
};

class RoomPlacementSpace: public BaseArea
{
public:
	RoomPlacementSpace( ) {;};
	RoomPlacementSpace( const UPoint3D& min, const UPoint3D& max ) : BaseArea( min, max ) {;};

	// these operators allow room placement spaces to be sorted by priority
	FORCE_INLINE bool operator<( const RoomPlacementSpace& other ) const { return distFromPriorityPoint < other.distFromPriorityPoint; };
	FORCE_INLINE bool operator>( const RoomPlacementSpace& other ) const { return distFromPriorityPoint > other.distFromPriorityPoint; };

	float32 distFromPriorityPoint; // orders room placement spaces: lower = higher priority, higher = lower priority
	ImaginaryType imaginaryType; // used in determining priority of spaces chosen for room placement, with IMAGINARY_FALSE having the highest. see enum ImaginaryType
	Direction attachmentSide; // normal direction of a wall or area. for walls this is the opposite of the side of the room they enclose. for areas this is the side of the area that is guaranteed to be connected to another room.
};

// STODO: for buildings with parapets, have the option of placing either parapets or roofs in rooms that are only partially covered by a ceiling

// STODO: to get specific building shapes, allow for open areas to be supplied to room placement independent of the rooms that already have been placed

class BaseBuilding: public BaseArea
{
public:
	// STODO: add basements to some buildings
	BaseBuilding( BaseZone* zone, BuildingSize size, uint32 numResidents );
	virtual	~BaseBuilding( );

	virtual void	Build( );

	// Room Placement // STODO: add a polymorphic placeRoom method to place rooms one at a time with various parameters passed in.
	virtual bool	PlaceRooms( Direction doorSide );

	// Public Helpers
	void			SetBounds( const UPoint3D& entrance, const UPoint3D& buildingMin, const UPoint3D& buildingMax, BuildingSize size, Direction doorSide );

	// Building Specific GMath
	uint32			GetNumRoomsContainingPoint( uint32 x, uint32 y, uint32 z, uint32 roomMinZ ) const;
	bool			IntersectPointWithRoomsFast( uint32 x, uint32 y, uint32 z, uint32 roomMinZ ) const;
	bool			IntersectPointWithRooms( uint32 x, uint32 y, uint32 z, bool doExcludeRoomBorders, BaseRoom** intersectedRoomPtr = NULL ) const;
	void			RemoveWindowsThatIntersectWithPoint( uint32 x, uint32 y, uint32 z, uint32 roomMinZ ) const;
	IntersectType	IntersectAreaWithRooms( const UPoint3D& areaMin, const UPoint3D& areaMax, bool doExcludeRoomBorders, BaseRoom** intersectedRoomPtr = NULL ) const;
	bool			IntersectAreaWithRoomsFast( const UPoint3D& areaMin, const UPoint3D& areaMax, bool doExcludeRoomBorders, BaseRoom** intersectedRoomPtr = NULL ) const;
	

	// Accessors
	BaseRoom*				GetRoom( uint32 idx ) const;
	const UPoint3D&			GetEntrance( ) const;
	const UPoint3D&			GetFrontDoorPos( ) const;
	uint32					GetNumRooms( ) const;
	uint32					GetNumFloors( ) const;
	uint32					GetGroundFloorLevel( ) const;
	uint32					GetCeilingHeight( ) const;
	uint32					GetMaxRoofHeight( ) const;
	uint32					GetStairWidth( ) const;
	uint32					GetMinHallWayWidth( ) const;
	uint32					GetMaxHallwayWidth( ) const;
	uint32					GetRandHallwayWidth( ) const;
	//uint32					GetHasSquareMainSection( ) const;
	//uint32					GetBuildingHasSymmetry( ) const;
	BuildingSize			GetSize( ) const;
	TERRATYPE				GetFloorMaterial( ) const;
	TERRATYPE				GetInnerWallMaterial( ) const;
	TERRATYPE				GetOuterWallMaterial( ) const;
	TERRATYPE				GetRoofMaterial( ) const;
	TERRATYPE				GetStairMaterial( ) const;
	RoofType				GetRoofType( ) const;
	RoofFillType			GetRoofFillType( ) const;
	Direction				GetDoorSide( ) const;
	virtual BuildingType	GetType( ) const;

	// Mutators
	void					SetFDoorPos( UPoint3D& point );

protected:
	virtual bool		LinkRooms_( );
	virtual void		PlaceWindows_( );
	
	void				BaseBuild_( uint32 maxLevelingDist, uint32 levelingStartDist, uint32 foundationThickness );
	void				BuildRoofs_( );

	void				FinalizeBuildingBounds_( bool doReduceBoundsSizeOnly );

	// STODO: possibly make room placement its own class: has instance vars not used elsewhere, can cut down on parameters passed around, code is completely separate from rest of basebuilding (except BaseBuild_ () which calls it)
	// Room Placement (high level)
	bool				PlaceRoom_( BaseRoom* room, bool doAllowImaginaryAreasToBeUsed, bool doAllowImaginaryTypeCombination, bool doPlaceFirstRoomOnNewFloor, 
							float32 doAttemptToPreserveBoxShapeOnFloorOneChance, float32 chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne, 
							float32 chanceOfSkipPlacementSpaceOnFloorOne, float32 chanceOfSkipPlacementSpaceOnOtherFloors = 0.00f );
	uint32				GetMaxNumRoomsOnFloor_( uint32 floorLevel, uint32 maxNumRoomsOnFloor, uint32 maxNumFloorTwoRooms, uint32 reduceAmount, float32 reduceNumRoomsPerFloorChance );
	virtual void		FinalizeRoomPlacement_( );
	bool				SetRoomBounds_( BaseRoom* room, float32 chanceOfSkippingSpace, float32 chanceOfExpandingRoomBoundsBeyondAreaAxisOnFloorOne, bool doAttemptToPreserveBoxShapeOnFloorOne, bool doAllowImaginaryAreasToBeUsed );
	bool				SetRoomBoundsInArea_( BaseRoom* room, BaseArea& roomBounds, RoomPlacementSpace* area, bool doForceSetting, bool doAllowPerpendicularExpansion, bool doAllowParallelExpansion );
	void				AddRoomToBuilding_( BaseRoom* room );
	
	// Room Placement (low level)
	void				UpdateRoomPlacementSpaces_( int32 minFloorLevel, int32 floorLevel, bool createOnlyImaginaryWallsBelowFloorLevel, 
							bool doPlaceFirstRoomOnNewFloor, bool doAllowImaginaryTypeCombination, bool allowMaximumAreaCoverageForRoofs, BaseRoom* roomToBeAdded = NULL );
	bool				AddOpenWallsToRooms_( int32 minFloorLevel, int32 floorLevel, bool createOnlyImaginaryWallsBelowFloorLevel, bool doPlaceFirstRoomOnNewFloor, BaseRoom* roomToBeAdded );
	bool				AttemptToAddOpenWallToRoom_( BaseRoom* room, uint32 initialX, uint32 initialY, Direction attachmentSide, bool placeImaginaryWall, bool doPlaceFirstRoomOnNewFloor, BaseRoom* roomToBeAdded );
	RoomPlacementSpace* CreateOpenWall_( BaseRoom* room, uint32 x, uint32 y, uint32 minZ, uint32 maxZ, bool isPriorityHigh, Direction attachmentSide, BaseRoom* roomToBeAdded );
	bool				AddImaginaryWall_( RoomPlacementSpace* parentWall, bool raiseFloorLevelByOne, bool doMakeWallNotImaginary, uint32 recursiveDepth = 0 );
	void				CombineOpenWalls_( bool doCombineImaginaryWithNonImaginary );
	bool				CombineOpenAreas_( bool doCombineImaginaryWithNonImaginary );
	bool				RedefineSmallOpenAreas_( );
	void				CreateOpenAreas_( bool doCombineImaginaryAndNonImaginaryWalls, bool allowMaximumAreaCoverageForRoofs );
	void				ModifyAreaBoundsToAvoidWallIntersection_( RoomPlacementSpace& area, const RoomPlacementSpace& wall );
	void				AddAndModifyOpenWallsForNewArea_( const RoomPlacementSpace* openArea, uint32 i, uint32 j, bool doCombineImaginaryAndNonImaginaryWalls );
	void				AddAndModifyOpenWallsForNewAreaLoop_( Vector<RoomPlacementSpace*>& newXWalls, Vector<RoomPlacementSpace*>& newYWalls, const RoomPlacementSpace* openSpace );
	void				DeleteAndModifyOpenWallsThatIntersectWithOpenArea_( const RoomPlacementSpace* openArea, uint32 startingWallIndex = 0 );
	
	// Room Placement (gmath)
	bool				IsPointOnWallConnectable_( BaseRoom* room, Direction attachmentSide, uint32 x, uint32 y ) const;
	bool				IsAreaFullyAttachedToOtherAreaByWall_( const UPoint3D& areaMin, const UPoint3D& areaMax, const UPoint3D& otherMin, const UPoint3D& otherMax, bool isAlongXAxis ) const;
	bool				DoesBoundsIntersectOnlyOnBorderOfArea_( const UPoint3D& boundsMin, const UPoint3D& boundsMax, const UPoint3D& areaMin, const UPoint3D& areaMax ) const;
	bool				DoesWallIntersectWithAreaOnlyOnEndOfWall_( const UPoint3D& wallMin, const UPoint3D& wallMax, const UPoint3D& areaMin, const UPoint3D& areaMax ) const;
	void				DetermineDistFromPriorityPointForWallsAndAreas_( );

	// Room Linking
	bool				BaseLinkRooms_( RoomLinkerInitData& initData );
	void				BasePlaceWindows_( uint32 heightAboveFloor, uint32 height, uint32 width, uint32 windowSpacing, float32 placementChance );
	//uint32 DefineHallway_ (BaseBuilding* building, uint32 numConnectingRooms, int32 floorLevel, BaseRoom* parentRoom, Direction doorSide, DoorType doorType, uint32 hallwayLength = 0, uint32 hallwayWidth = 0); // remove this
		 // STODO: isMainSection can be removed since this can be determined by a null parent.
	void				LevelTerrain_( uint32 maxLevelingDist, uint32 levelingStartDist );
	bool				IsRoomPlaceable_( const UPoint3D& roomMin, const UPoint3D& roomMax, bool doChangeBuildingBoundsIfTrue );
	bool				DoesRoomBlockFrontDoor_( const UPoint3D& roomMin, const UPoint3D& roomMax ) const;

	// STODO: move some of these vars into methods and have the building type classes pass these in as parameters
	// Instance Vars
	BaseZone*			parentZone_;
	Vector<BaseRoom*>	rooms_;
	BuildingSize		size_;
	UPoint3D			entrance_; // Note- this is the position of where the building property connects to the outside. This is useful for connecting to roads.
	UPoint3D			frontDoorPos_;
	uint32				groundFloorLevel_;
	uint32				numFloors_;
	uint32				numResidents_;
	uint32				numBasements_;
	uint32				maxRoofHeight_;
	uint32				ceilingHeight_;
	uint32				stairWidth_;
	uint32				minHallwayWidth_; // Note- determines minimum allowable open area length on the x and y axes
	uint32				maxHallwayWidth_;
	uint32				minRoomAndBuildingBoundSeparation_;
	uint32				maxRoomAndBuildingBoundSeparation_;
	TERRATYPE			floorMaterial_;
	TERRATYPE			innerWallMaterial_;
	TERRATYPE			outerWallMaterial_;
	TERRATYPE			roofMaterial_;
	TERRATYPE			stairMaterial_;
	RoofType			roofType_;
	RoofFillType		roofFillType_;
	Direction			doorSide_;

	Vector<RoomPlacementSpace*> openWalls_; // 2D building boundaries where rooms can be placed
	Vector<RoomPlacementSpace*> openAreas_; // 3D building boundaries where rooms should be placed for proper shape
};