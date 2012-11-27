//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "roomlinker.h"

RoomLinker::RoomLinker( RoomLinkerInitData& initData )
	:  building_( initData.building ), maxStairsPerFloor_(initData.maxStairsPerFloor ), 
	   maxChildRooms_(initData.maxChildRooms ), maxParentRooms_(initData.maxParentRooms ), doorPlaceChance_(initData.doorPlaceChance ), 
	   stairPlaceChance_( initData.stairPlaceChance ), spiralStairTypeChance_( initData.spiralStairTypeChance )
{
	likelyLinking_ = false;
	forcedLinkingLevel1_ = false; // when likelyLinking_ fails to link rooms, this will modify various parameters to make linking more likely 
	forcedLinkingLevel2_ = false;

	unlinkedRoomsPerFloor_.SetCapacity( building_->GetNumFloors() + 1 );
	unlinkedRoomsPerFloor_.OverrideCount( building_->GetNumFloors() + 1 );
	ZeroMemory( unlinkedRoomsPerFloor_.GetMemory(), (building_->GetNumFloors() + 1) * sizeof( uint32 ) );
	stairsPerFloor_.SetCapacity( building_->GetNumFloors() + 1 );
	stairsPerFloor_.OverrideCount( building_->GetNumFloors() + 1 );
	ZeroMemory( stairsPerFloor_.GetMemory(), (building_->GetNumFloors() + 1) * sizeof( uint32 ) );

	parentRooms_.SetCapacity( building_->GetNumFloors() + 1 );
	parentRooms_.OverrideCount( building_->GetNumFloors() + 1 );
	childRooms_.SetCapacity( building_->GetNumFloors() + 1 );
	childRooms_.OverrideCount( building_->GetNumFloors() + 1 );
	for( uint32 i = 1; i <= building_->GetNumFloors(); ++i )
	{
		parentRooms_[i] = new Vector<BaseRoom*>();
		childRooms_[i] = new Vector<BaseRoom*>();
	}

	for( uint32 i = 0; i < building_->GetNumRooms(); ++i )
	{
		BaseRoom* room = building_->GetRoom( i );
		uint32 floorLevel = room->GetFloorLevel();

		if( room->GetIsMainRoom() )
		{
			// place main room in parent rooms
			parentRooms_[floorLevel]->InsertBack( room );
		}
		else
		{
			// fill child rooms
			childRooms_[floorLevel]->InsertBack( room );
			++unlinkedRoomsPerFloor_[floorLevel];
		}
	}
}

RoomLinker::~RoomLinker()
{
	parentRooms_.Purge();
	childRooms_.Purge();
}

bool 
RoomLinker::Link()
{
	// add door to main room
	parentRooms_[1]->GetElement( 0 )->AddDoorToRoom( parentRooms_[1]->GetElement( 0 )->GetExteriorDoorType(),
		parentRooms_[1]->GetElement( 0 )->GetParentBuilding()->GetDoorSide(), true, true );

	// description: loop through potential parent rooms, linking them by doors or stairs to child rooms, until no potential child rooms are left
	// linking scheme: except for floor 1, link rooms on floor only if all stair wells have been placed in the floor below

	for( uint32 floorLevel = 1; floorLevel <= building_->GetNumFloors(); ++floorLevel )
	{
		uint32 failedLinkCount = 0;
		while( childRooms_[floorLevel]->GetCount() > 0 ||( floorLevel != building_->GetNumFloors() && parentRooms_[floorLevel + 1]->GetCount() == 0 ) )
		{
			uint32 linkingCount = 0;
		
			for( uint32 i = 0; i < parentRooms_[floorLevel]->GetCount(); ++i )
			{
				BaseRoom* parentRoom = parentRooms_[floorLevel]->GetElement( i );

				if( likelyLinking_ || parentRooms_[floorLevel]->GetElement( i )->GetType() == ROOM_HALLWAY || 
					parentRooms_[floorLevel]->GetElement( i )->GetNumChildRooms() < maxChildRooms_ )
				{
					for( uint32 j = 0; j < childRooms_[floorLevel]->GetCount(); ++j )
					{
						BaseRoom* childRoom = childRooms_[floorLevel]->GetElement( j );

						if( !likelyLinking_ && !parentRoom->GetIsLinkableToChildRoom ( childRoom ) )
							continue;
						
						// attempt to link rooms on same floor based on conditions
						linkingCount += AttemptLink_( parentRoom, childRoom, floorLevel, floorLevel );
						UpdateLinkingFactors_( linkingCount, failedLinkCount );
						ConditionallyRemoveChildRoom_( j, floorLevel );
					}

					if( floorLevel < building_->GetNumFloors() )
					{
						for( uint32 j = 0; j < childRooms_[floorLevel + 1]->GetCount(); ++j )
						{
							BaseRoom* childRoom = childRooms_[floorLevel + 1]->GetElement( j );

							if( !likelyLinking_ && !parentRoom->GetIsLinkableToChildRoom ( childRoom ) )
								continue;
							if( !forcedLinkingLevel2_ && (!parentRoom->GetCanContainStairs() || !childRoom->GetCanContainStairs())) // STODO: make this forcedLinkingLevel2_ if necessary
								continue;

							// attempt to link rooms on different floors based on conditions
							linkingCount += AttemptLink_( parentRoom, childRoom, floorLevel, floorLevel + 1 );
							UpdateLinkingFactors_( linkingCount, failedLinkCount );
						}
					}
				}
			}
			if( floorLevel > 1 && likelyLinking_ && childRooms_[floorLevel]->GetCount() > 0 )
			{
				// reattempt to link rooms on the floor below to this floor
				for( uint32 i = 0; i < parentRooms_[floorLevel - 1]->GetCount() && linkingCount == 0; ++i )
				{
					BaseRoom* parentRoom = parentRooms_[floorLevel - 1]->GetElement( i );

					for( uint32 j = 0; j < childRooms_[floorLevel]->GetCount() && linkingCount == 0; ++j )
					{
						BaseRoom* childRoom = childRooms_[floorLevel]->GetElement( j );

						if( !forcedLinkingLevel2_ && (!parentRoom->GetCanContainStairs() || !childRoom->GetCanContainStairs())) // STODO: make this forcedLinkingLevel2_ if necessary
							continue;

						linkingCount += AttemptVerticalLink_( parentRoom, childRoom, floorLevel - 1, floorLevel, true );
						UpdateLinkingFactors_( linkingCount, failedLinkCount );
					}
				}
			}
			
			if( linkingCount == 0 )
			{
				++failedLinkCount;
				if( failedLinkCount > 10 )
					likelyLinking_ = true;
				if( failedLinkCount > 20 )
					forcedLinkingLevel1_ = true; // STODO: make sure forced Linking never has to be used
				if( failedLinkCount > 30 )
					forcedLinkingLevel2_ = true; // last ditch effort to link rooms: will produce some very undesirable results
				
				if( failedLinkCount > 100 )
				{
					return false; // STODO: make this configurable

					// linker failure recovery

					// add parent room to next floor if there isnt one already to allow linking to continue on the floors above
					if( floorLevel < building_->GetNumFloors() && parentRooms_[floorLevel + 1]->GetCount() == 0 )
					{
						parentRooms_[floorLevel + 1]->InsertBack( childRooms_[floorLevel + 1]->GetElement( rand() % childRooms_[floorLevel + 1]->GetCount() ) );
						continue;
					}
					break; // prevents infinite loop from occuring
				}
			}
		}
	}
	return true;
}

uint32
RoomLinker::AttemptLink_( BaseRoom* parentRoom, BaseRoom* childRoom, uint32 pRoomFloorLevel, uint32 cRoomFloorLevel )
{
	if( parentRoom->GetType() == ROOM_HALLWAY && 
		childRoom->GetType() == ROOM_HALLWAY )
	{
		likelyLinking_ = true; // increases connections between hallways and rooftops
	}

	uint32 linkingCount = 0;
	if( likelyLinking_ || 
		(parentRoom->GetType() == ROOM_HALLWAY && parentRoom->GetNumChildRooms() < maxChildRooms_ * 2 ) ||
		childRoom->GetNumParentRooms() < maxParentRooms_ )
	{
		// check to make sure rooms are not the same
		if( parentRoom->GetMin() != childRoom->GetMin() )
		{
			if( pRoomFloorLevel == cRoomFloorLevel )
				linkingCount += AttemptHorizontalLink_(parentRoom, childRoom, pRoomFloorLevel );
			else if( pRoomFloorLevel == cRoomFloorLevel - 1 )
				linkingCount += AttemptVerticalLink_(parentRoom, childRoom, pRoomFloorLevel, cRoomFloorLevel, false );
		}
	}
	return linkingCount;
}

uint32
RoomLinker::AttemptHorizontalLink_( BaseRoom* parentRoom, BaseRoom* childRoom, uint32 floorLevel )
{
	if( likelyLinking_ || doorPlaceChance_ >= FRandom() || floorLevel <= 1 ||
		childRoom->GetNumParentRooms() == 0 ||
		(childRoom->GetNumParentRooms() > 0 &&
			(rand() % 8 == 0 || (parentRoom->GetType() == ROOM_HALLWAY && childRoom->GetType() == ROOM_HALLWAY))) )
	{
		// attempt to link rooms by door
		if( parentRoom->LinkToRoomByDoor( childRoom ) )
		{
			// successful link: make child room a parent room
			parentRooms_[floorLevel]->InsertBack( childRoom );
			return 1;
		}
	}
	return 0;
}

uint32
RoomLinker::AttemptVerticalLink_( BaseRoom* parentRoom, BaseRoom* childRoom, uint32 pRoomFloorLevel, uint32 cRoomFloorLevel, bool doIgnoreMaxStairsPerFloor )
{
	if( (stairsPerFloor_[pRoomFloorLevel] < maxStairsPerFloor_ || doIgnoreMaxStairsPerFloor) && 
		(likelyLinking_ || stairPlaceChance_ >= FRandom()) &&
		(forcedLinkingLevel2_ || (parentRoom->GetType() != ROOM_HALLWAY && 
		childRoom->GetType() != ROOM_HALLWAY)) )
	{
		// stair placement for random stair buildings:( ie. certain dungeons )
		if( stairPlaceChance_ < 1.0f && !forcedLinkingLevel1_ )
		{
			// makes sure stairs aren't placed in the main room
			if( parentRoom->GetIsMainRoom() )
				return 0;

			// makes sure stairs aren't placed in rooms connected to by stairs
			for( uint32 k = 0; k < parentRoom->GetNumParentRooms(); ++k )
			{
				if( parentRoom->GetParentRoom( k )->GetNumStairs() > 0 )
					return 0;
			}
		}

		bool isSpiralStair = false;
		uint32 spiralStairLength = 2;
		uint32 stairWidth = building_->GetStairWidth();

		// chance for spiral stair to be placed
		if( spiralStairTypeChance_ >= FRandom() )
			isSpiralStair = true;

		if( forcedLinkingLevel2_ )
		{
			// Note- this code will make room linking by stairs much more likely, at the cost of odd looking spiral stairs 
			spiralStairLength = 1;
			stairWidth = 1;
			isSpiralStair = true;
		}

		bool successfulLink = false;

		// attempt to link rooms by stair. if first attempt fails it will try again with the other stair type
		if( parentRoom->LinkToRoomByStair( 
			childRoom, stairWidth, isSpiralStair, spiralStairLength ) )
		{
			successfulLink = true;
		}
		else if( parentRoom->LinkToRoomByStair(
			childRoom, stairWidth, !isSpiralStair, spiralStairLength ) )
		{
			isSpiralStair = !isSpiralStair;
			successfulLink = true;
		}

		if( successfulLink )
		{
			++stairsPerFloor_[pRoomFloorLevel];

			// make child room a parent room
			parentRooms_[cRoomFloorLevel]->InsertBack( childRoom );

			return 1;
		}
	}
	return 0;
}

void
RoomLinker::ConditionallyRemoveChildRoom_( uint32 childRoomIndex, uint32 floorLevel )
{
	BaseRoom* childRoom = childRooms_[floorLevel]->GetElement( childRoomIndex );

	// potentially remove child room if it has a parent
	if( (childRoom->GetNumParentRooms() > 0 && rand() % 3 == 0 ) || childRoom->GetNumParentRooms() >= maxParentRooms_ )
	{
		if( childRoom->GetType() != ROOM_HALLWAY || rand() % 4 == 0 )
		{
			--unlinkedRoomsPerFloor_[floorLevel]; // STODO: refine this
			childRooms_[floorLevel]->Remove( childRoomIndex );
		}
	}
}

void
RoomLinker::UpdateLinkingFactors_( uint32 linkingCount, uint32& failedLinkCount )
{
	if( linkingCount > 0 )
	{
		likelyLinking_ = false; // turn back on rand checks if a room has been successfully linked.
		forcedLinkingLevel1_ = false;
		forcedLinkingLevel2_ = false;
		failedLinkCount = 0;
	}
}
