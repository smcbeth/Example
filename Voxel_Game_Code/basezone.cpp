//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "basezone.h"
#include "roadnode.h"
#include "zoneroadgenerator.h"
#include "worldgenerator.h"
#include "basebuilding.h"

BaseZone::BaseZone (uint32 xMin, uint32 yMin)
	: BaseArea(UPoint3D(xMin, yMin, 0), UPoint3D(xMin + SECTOR_BLENGTH - 1, yMin + SECTOR_BLENGTH - 1, 0))
{
	// STODO: clean up this constructor since it is doing too much work
	// STODO: expand bounding box beyond sector area if neighboring areas can support a town
	// STODO: make town bounding box not conform exactly to sector lines

	roadGenerator_ = new ZoneRoadGenerator(this);

	heightMap_			= (uint16*)malloc (GetLengthX () * GetLengthY () * sizeof (uint16));
	revisedHeightMap_	= (uint16*)malloc (GetLengthX () * GetLengthY () * sizeof (uint16));
	slopeMap_			= (float32*)malloc (GetLengthX () * GetLengthY () * sizeof (float32));

	// generate height map requires noise to be prepared.
	WorldGeneratorAPI ()->PrepTerraNoise (xMin, yMin, 0, GetLengthX (), GetLengthY (), false);
			
	WorldGeneratorAPI ()->GenerateHeightMap (xMin, yMin, heightMap_, GetLengthX (), GetLengthY ());

	GenerateRevisedHeightMap_ (xMin, yMin);
}

BaseZone::~BaseZone ()
{
	buildings_.RemoveAndDeleteAll ();
	roadNodes_.RemoveAndDeleteAll ();
	delete roadGenerator_;
	free (heightMap_);
	free (revisedHeightMap_);
	free (slopeMap_);
}

void
BaseZone::Build ()
{
	// build all the buildings_ in town
	for (uint32 i = 0; i < buildings_.GetCount (); ++i)
	{
		buildings_.GetElement (i)->Build ();
	}

	GenerateRevisedHeightMap_( min_.x, min_.y, 5 );

	if( connectingRoadNodeForBuildings_.GetCount() > 0 && roadNodes_.GetCount() > 0 )
	{
		// connect a road to the front doors of buildings. This is done here because it must be done after terrain is leveled around the buildings
		for (uint32 i = 0; i < buildings_.GetCount (); ++i)
		{
			GetWorld()->BeginModify();
			roadGenerator_->ConnectRoadToFrontDoor( connectingRoadNodeForBuildings_[i], buildings_.GetElement (i) );
			GetWorld()->EndModify();
		}
	}

	GetWorld()->BeginModify();
	for (uint32 i = 0; i < roadNodes_.GetCount (); ++i)
	{
		for (uint32 j = 0; j < roadNodes_[i]->GetNumChildNodes (); ++j)
		{
			roadNodes_[i]->BuildOrDoIntersection (*roadNodes_[i]->GetChildNode (j), false);
		}
	}
		
	GetWorld()->EndModify();
}

uint32
BaseZone::GetHeightAtPoint (uint32 x, uint32 y)
{
	return (uint32)revisedHeightMap_[(x - min_.x) + (y - min_.y) * GetLengthX ()];
}

uint32
BaseZone::GetAverageHeightWithinBounds (const UPoint3D& min, const UPoint3D& max)
{
	// averages the height within the building bounds
	uint32 sum = 0;
	for (uint32 y = min.y; y <= max.y; ++y)
	{
		for (uint32 x = min.x; x <= max.x; ++x)
		{
			sum += revisedHeightMap_[(x - min_.x) + (y - min_.y) * GetLengthX ()];
		}
	}
	return sum / ((max.x - min.x + 1) * (max.y - min.y + 1));
}

bool
BaseZone::IsBuildingBoundsPlaceable (const UPoint3D& bMin, const UPoint3D& bMax)
{
	// check if building exceeds zone bounds
	if (bMin.x <= min_.x || bMax.x >= max_.x ||
		bMin.y <= min_.y || bMax.y >= max_.y)
	{
		return false;
	}

	for (uint32 i = 0; i < hazards_.GetCount (); ++i)
	{
		// check if building extents intersect with a hazard zone
		if (IntersectAABoxWithAABoxFast (bMin, bMax, hazards_[i].min, hazards_[i].max))
			return false;
	}

	for (uint32 i = 0; i < buildings_.GetCount (); ++i)
	{
		// check if building extents intersect with an existing building
		if (IntersectAABoxWithAABoxFast (bMin, bMax, buildings_[i]->GetMin (), buildings_[i]->GetMax ()))
			return false;
	}

	if( roadNodes_.GetCount() > 0 && roadGenerator_->Does3DAreaIntersectWithRoads( BaseArea( bMin, bMax ) ) )
		return false;

	return true;
}

bool
BaseZone::DoesPointIntersectWithBuildingBounds( const UPoint3D& point )
{
	for (uint32 i = 0; i < buildings_.GetCount (); ++i)
	{
		BaseBuilding* building = buildings_[i];

		if (IntersectBoxWithPoint (point, building->GetMin (), building->GetMax ()))
			return true;
	}

	return false;
}

bool
BaseZone::PlaceBuildings ()
{
	return false;
}

void
BaseZone::LevelTerrain_ ()
{
	const float32 maxSmoothingFactor = 0.0f;
	const uint32 aboveGroundHeightLimit = 1;
	BaseLevelTerrain_ (maxSmoothingFactor, aboveGroundHeightLimit);
}

void
BaseZone::GenerateHazardExtents_ ()
{
	const float slopeThreshold = 0.30f;
	BaseGenerateHazardExtents_ (slopeThreshold);
}

bool
BaseZone::AttemptToPlaceBuilding_ (BaseBuilding* building, BuildingSize size, uint32 numResidents, uint32 extentsRadius, bool attemptToCenterBuilding)
{
	uint32 xStart;
	uint32 yStart;
	uint32 xEnd;
	uint32 yEnd;

	Direction doorSide = Direction(rand () % 4);

	bool doFirstPass = true;

	for(uint32 i = 0; i < 2; ++i)
	{
		// STODO: have proximity to roads determine door side
		// STODO: have options for placing a front door outside of the center of the building extents
		// determine bounds of where the building entrance can be potentially placed, based on doorSide
		switch (doorSide)
		{
		case DIRECTION_SOUTH:
			xStart = min_.x + extentsRadius + 1;
			yStart = min_.y + 1;
			xEnd = max_.x - extentsRadius - 1;
			yEnd = max_.y - extentsRadius * 2 - 1;
			break;
		case DIRECTION_NORTH:
			xStart = min_.x + extentsRadius + 1;
			yStart = min_.y + extentsRadius * 2 + 1;
			xEnd = max_.x - extentsRadius - 1;
			yEnd = max_.y - 1;
			break;
		case DIRECTION_WEST:
			xStart = min_.x + 1;
			yStart = min_.y + extentsRadius + 1;
			xEnd = max_.x - extentsRadius * 2 - 1;
			yEnd = max_.y - extentsRadius - 1;
			break;
		case DIRECTION_EAST:
			xStart = min_.x + extentsRadius * 2 + 1;
			yStart = min_.y + extentsRadius + 1;
			xEnd = max_.x - 1;
			yEnd = max_.y - extentsRadius - 1;
			break;
		}
		
		// Note- while this code could be modified for each doorside case, this code and some of the other code in this method will no longer be needed
		//	when buildings are placed along roads
		if (i == 0)
		{
			if (attemptToCenterBuilding)
			{
				xStart += (GetLengthX () - (extentsRadius + 1) * 2) / 2;
				yStart += (GetLengthY () - (extentsRadius + 1) * 2) / 2;
			}
			else
			{
				xStart += rand () % (GetLengthX () - (extentsRadius + 1) * 2);
				yStart += rand () % (GetLengthY () - (extentsRadius + 1) * 2);
			}
		}

		// STODO: make this more random by making the iteration go forward or backward
		for (uint32 entranceY = yStart; entranceY <= yEnd; entranceY += 2)
		{
			for (uint32 entranceX = xStart; entranceX <= xEnd; entranceX += 2)
			{
				UPoint3D buildingMin;
				UPoint3D buildingMax;
				UPoint3D entrance;

				switch (doorSide)
				{
				case DIRECTION_SOUTH:
					buildingMin.x = entranceX - extentsRadius;
					buildingMin.y = entranceY;
					break;
				case DIRECTION_NORTH:
					buildingMin.x = entranceX - extentsRadius;
					buildingMin.y = entranceY - extentsRadius * 2;
					break;
				case DIRECTION_WEST:
					buildingMin.x = entranceX;
					buildingMin.y = entranceY - extentsRadius;
					break;
				case DIRECTION_EAST:
					buildingMin.x = entranceX - extentsRadius * 2;
					buildingMin.y = entranceY - extentsRadius;
					break;
				}
				buildingMax.x = buildingMin.x + extentsRadius * 2;
				buildingMax.y = buildingMin.y + extentsRadius * 2;
				buildingMin.z = revisedHeightMap_[entranceX - min_.x + (entranceY - min_.y) * GetLengthX ()] - 5;
				buildingMax.z = revisedHeightMap_[entranceX - min_.x + (entranceY - min_.y) * GetLengthX ()] + 8; // STODO: figure out proper defining technique for z
	
				entrance.x = entranceX;
				entrance.y = entranceY;
				entrance.z = revisedHeightMap_[(entrance.x - min_.x) + (entrance.y - min_.y) * GetLengthX ()];

				building->SetBounds (entrance, buildingMin, buildingMax, size, doorSide);

				if (IsBuildingBoundsPlaceable (buildingMin, buildingMax))
				{
					if (building->PlaceRooms (doorSide))
					{
						buildings_.InsertBack (building);
						return true;
					}
					return false; // Note- must delete building data after unsuccessful PlaceRooms
				}
			}
		}
	}
	return false;
}

bool
BaseZone::AttemptToPlaceBuildingConnectedToRoad_( BaseBuilding* building, BuildingSize size, uint32 numResidents, uint32 extentsRadius, bool attemptToCenterBuilding, uint32 minDistFromRoad, uint32 maxDistFromRoad )
{
	// SNOW: do something with attemptToCenterBuilding

	// Note- there will be a performance increase here if road nodes that are surrounded by buildings are not included in this search
	Assert( roadNodes_.GetCount() > 0 );

	uint32 indexStart = rand() % roadNodes_.GetCount();
	bool isFirstIteration = true;
	int32 i = indexStart;
	int32 iInc = (rand() % 2) * 2 - 1;

	while( true )
	{
		if( i < 0 || i == roadNodes_.GetCount() )
		{
			// switch iteration direction and reset to starting point
			if( isFirstIteration )
			{
				iInc = -iInc;
				i = indexStart + iInc;
				isFirstIteration = false;
				continue;
			}
			break;
		}

		RoadNode* randomRoadNode = roadNodes_[i];

		BaseArea buildingArea;

		if( roadGenerator_->GetAreaAdjacentToRoadNode( randomRoadNode, buildingArea, extentsRadius * 2, extentsRadius * 2, 1, 5 ) )
		{
			// init entrance.
			UPoint3D entrance( buildingArea.GetClosestPointInAreaToPoint( *randomRoadNode ) );
			//UPoint3D entrance( buildingArea.GetRandBorderPoint( false, false, true, 0.4f ) );
			entrance.z = GetHeightAtPoint( entrance.x, entrance.y );

			Direction doorSide = buildingArea.GetSideOfArea2DBorderPointIsOn( entrance );

			building->SetBounds( entrance, buildingArea.GetMin(), buildingArea.GetMax(), size, doorSide );

			if( IsBuildingBoundsPlaceable( buildingArea.GetMin(), buildingArea.GetMax() ) )
			{
				if( building->PlaceRooms( doorSide ) )
				{
					// SNOW: implement this
					// TODO: add customization options for this
					Assert( buildings_.GetCount() == connectingRoadNodeForBuildings_.GetCount() );

					buildings_.InsertBack( building );
					connectingRoadNodeForBuildings_.InsertBack( randomRoadNode );
					return true;
				}
				return false; // Note- must delete building data after unsuccessful PlaceRooms
			}
		}

		i += iInc;
	}

	return false;
}

void
BaseZone::BaseLevelTerrain_ (float32 maxSmoothingFactor, uint32 aboveGroundHeightLimit)
{
	// SNOW: store sector locations that have had level terrain called on them. if this method is called many times over the same area, various problems in the terrain will result
	uint32 averageTerraHeight;
	uint32 minHeight = revisedHeightMap_[0];
	uint32 maxHeight = minHeight;
	{
		uint32 sum = 0;
		for (uint32 y = 0; y < GetLengthY (); ++y)
		{
			for (uint32 x = 0; x < GetLengthX (); ++x)
			{
				uint32 height = revisedHeightMap_[x + y * GetLengthX ()];
				if (height < minHeight)
					minHeight = height;
				if (height > maxHeight)
					maxHeight = height;
				sum += height;
			}
		}
		averageTerraHeight = sum / (GetLengthX () * GetLengthY ());
	}

	UPoint3D centerPoint;
	centerPoint.x = min_.x + GetLengthX () / 2;
	centerPoint.y = min_.y + GetLengthY () / 2;
	GetWorld()->BeginModify ();
	for (uint32 y = min_.y; y <= max_.y; ++y)
	{
		for (uint32 x = min_.x; x <= max_.x; ++x)
		{
			uint32 distFromCenter = Max (Abs (int32(x) - int32(min_.x + GetLengthX () / 2)), Abs (int32(y) - int32(min_.y + GetLengthY () / 2)));
			uint32 height = (uint16)revisedHeightMap_[(x - min_.x) + (y - min_.y) * GetLengthX ()];

			uint32 smoothHeight = uint32(LinearInterpolate(
											CosineInterpolate (float(averageTerraHeight), float(height), float(distFromCenter) / float(Max (GetLengthX () / 2, GetLengthY () / 2))), 
											float(height), maxSmoothingFactor) + 0.5f);
			
			// slide blocks up or down. some compression or expansion of terrain layers will occur.
			if (height < smoothHeight)
			{
				uint32 difference = smoothHeight - height;
				Vector<uint32> blockStack;
				Vector<uint32> aboveGroundBStack;
				for (uint32 z = height - difference; z <= height; ++z)
					blockStack.InsertBack (GetWorld()->GetSCube (x, y, z));
				for (uint32 z = height + 1; z <= height + aboveGroundHeightLimit; ++z)
					aboveGroundBStack.InsertBack (GetWorld()->GetSCube (x, y, z));

				// expand terrain stack
				bool incrementI = false;
				uint32 i = 0;
				for (uint32 z = height - difference; z <= smoothHeight; ++z)
				{
					GetWorld()->SetSCube (x, y, z, blockStack[i]);
					if (incrementI)
						++i;
					incrementI = !incrementI;
				}
				i = 0;
				for (uint32 z = smoothHeight + 1; z <= smoothHeight + aboveGroundHeightLimit; ++z)
				{
					GetWorld()->SetSCube (x, y, z, aboveGroundBStack[i]);
					++i;
				}
			}
			else if (height > smoothHeight)
			{
				uint32 difference = height - smoothHeight;
				Vector<uint32> blockStack;
				Vector<uint32> aboveGroundBStack;
				for (uint32 z = smoothHeight - difference; z <= height; ++z)
					blockStack.InsertBack (GetWorld()->GetSCube (x, y, z));
				for (uint32 z = height + 1; z <= height + aboveGroundHeightLimit; ++z)
					aboveGroundBStack.InsertBack (GetWorld()->GetSCube (x, y, z));

				// compress terrain stack
				uint32 i = 0;
				for (uint32 z = smoothHeight - difference; z <= height; ++z)
				{
					if (z <= smoothHeight)
					{
						if (z == height || i >= blockStack.GetCount ())
							GetWorld()->SetSCube (x, y, z, blockStack[blockStack.GetCount () - 1]);
						else
							GetWorld()->SetSCube (x, y, z, blockStack[i]);
						i += 2;
					}
					else
					{
						GetWorld()->SetSCube (x, y, z, 0);
					}
				}
				i = 0;
				for (uint32 z = smoothHeight + 1; z <= smoothHeight + aboveGroundHeightLimit; ++z)
				{
					GetWorld()->SetSCube (x, y, z, aboveGroundBStack[i]);
					++i;
				}
			}

			// correct revised height map values
			revisedHeightMap_[(x - min_.x) + (y - min_.y) * GetLengthX ()] = (uint16)smoothHeight;
		}
	}
	GetWorld()->EndModify ();
}

void
BaseZone::GenerateSlopeMap_ (float32* slopeMap, uint32 lengthX, uint32 lengthY)
{
	// Note- this generate slope map method differs from the world gen generate slope map in that it does not sample neighboring height maps. In addition no max_ slope value needs to be enforced.
	const int32 townSlopeSampDist = 4;
	int32 x;
	int32 y;
	for (y = 0; y < (int32)lengthY; ++y)
	{
		for (x = 0; x < (int32)lengthX; ++x)
		{
			float32 divisor = 0.0f;
			float32 sum = 0.0f;
			for (int32 yOff = -townSlopeSampDist; yOff <= townSlopeSampDist; ++yOff)
			{
				for (int32 xOff = -townSlopeSampDist; xOff <= townSlopeSampDist; ++xOff)
				{
					if ((Square (xOff) + Square (yOff)) > Square(townSlopeSampDist + 1)) // keeps sample locations in a circular pattern rather than a square one.
						continue;
					if (x + xOff < 0 || x + xOff > (int32)lengthX || y + yOff < 0 || y + yOff > (int32)lengthY)
						continue;
					if (xOff == 0 && yOff == 0)
						continue;
					divisor += 1.0f;
					sum += Abs(heightMap_[x + y * lengthX] - heightMap_[(x + lengthX + xOff) % lengthX + ((y + lengthY + yOff) % lengthY) * lengthX]) 
						/ FastSqrt (float32 (Square (xOff) + Square (yOff))); // PERFORMANCE: use lookup table
				}
			}
			slopeMap[x + y * lengthX] = sum / divisor;
		}
	}
}

void
BaseZone::GenerateRevisedHeightMap_ (uint32 xMin, uint32 yMin, uint32 heightOffset)
{
	//OPTIMIZATION: load some/all relevant sectors into memory. dont switch back and forth between them all, this is too slow.
	uint32 oX;
	uint32 oY;
	uint32 oZ;
	uint32 x;
	uint32 y;
	uint32 minValue = 1000000000;
	uint32 maxValue = 0;
	uint32 height;
	S_Sector* sector;
	uint32* sectorData;

	oZ = WorldGeneratorAPI ()->GetSeaLevel ();
	oZ -= oZ % SECTOR_BLENGTH;
	sector = GetWorld()->GetSector (xMin, yMin, oZ); 
	oX = uint32(sector->GetTranslationX () / SCUBE_LENGTH) - SECTOR_BLENGTH / 2;
	oY = uint32(sector->GetTranslationY () / SCUBE_LENGTH) - SECTOR_BLENGTH / 2;
	sectorData = sector->BeginModify ();

	Assert (oX >= xMin);
	Assert (oY >= yMin);
	Assert (oX % SECTOR_BLENGTH == 0);
	Assert (oY % SECTOR_BLENGTH == 0);
	Assert (oZ % SECTOR_BLENGTH == 0);

	for (y = 0; y < GetLengthY (); ++y)
	{
		for (x = 0; x < GetLengthX (); ++x)
		{
			height = (uint32)heightMap_[x + y * GetLengthX ()] + heightOffset;
			while (true)
			{
				//navigate to appropriate sector
				while (oX + SECTOR_BLENGTH <= xMin + x)
				{
					sector->EndModify ();
					oX += SECTOR_BLENGTH;
					sector = GetWorld()->GetSector (oX, oY, oZ);
					sectorData = sector->BeginModify ();
				}
				while (oX > xMin + x)
				{
					sector->EndModify ();
					oX -= SECTOR_BLENGTH;
					sector = GetWorld()->GetSector (oX, oY, oZ);
					sectorData = sector->BeginModify ();
				}

				while (oY + SECTOR_BLENGTH <= yMin + y)
				{
					sector->EndModify ();
					oY += SECTOR_BLENGTH;
					sector = GetWorld()->GetSector (oX, oY, oZ);
					sectorData = sector->BeginModify ();
				}
				while (oY > yMin + y)
				{
					sector->EndModify ();
					oY -= SECTOR_BLENGTH;
					sector = GetWorld()->GetSector (oX, oY, oZ);
					sectorData = sector->BeginModify ();
				}

				while (oZ + SECTOR_BLENGTH <= height)
				{
					sector->EndModify ();
					oZ += SECTOR_BLENGTH;
					sector = GetWorld()->GetSector (oX, oY, oZ);
					sectorData = sector->BeginModify ();
				}
				while (oZ > height)
				{
					sector->EndModify ();
					oZ -= SECTOR_BLENGTH;
					sector = GetWorld()->GetSector (oX, oY, oZ);
					sectorData = sector->BeginModify ();
				}	

				// check if location in sector is an air/water/lava block, if it is keep checking downward till air is no longer found
				TERRATYPE terraType = GetSCubeTerrainType (sectorData[(x + xMin - oX) + (y + yMin - oY) * SECTOR_BLENGTH + (height - oZ) * SECTOR_BAREA]);
				if (terraType == TERRA_AIR || terraType == TERRA_OLDAIR || terraType == TERRA_WATER || terraType == TERRA_LAVA)
				{
					--height;
				}
				else
				{
					if (height < minValue)
						minValue = height;
					if (height > maxValue)
						maxValue = height;
					revisedHeightMap_[x + y * GetLengthX ()] = (uint16)height;
					break;
				}
			}
		}
	}
	sector->EndModify ();

	// set town bounding box for z axis
	min_.z = minValue;
	max_.z = maxValue + 30;
}

void
BaseZone::BaseGenerateHazardExtents_ (float32 slopeThreshold)
{
	Hazard hazard;
	bool isWaterType;
	uint32 x;
	uint32 y;
	uint32 i;

	for (y = 0; y < GetLengthY (); ++y)
	{
		for (x = 0; x < GetLengthX (); ++x)
		{
			if (revisedHeightMap_[y * SECTOR_BLENGTH + x] <= WorldGeneratorAPI ()->GetSeaLevel ())
				isWaterType = true;
			else
				isWaterType = false;

			// DEBUG_BASEZONE:
			float32 debugSlope = slopeMap_[y * GetLengthX () + x];

			if (slopeMap_[y * GetLengthX () + x] > slopeThreshold || isWaterType)
			{
				bool isExtentsExtended = false;
				for (i = 0; i < hazards_.GetCount (); ++i)
				{
					// check if current point is in an existing hazard box.
					UPoint3D point;
					point.x = x + min_.x;
					point.y = y + min_.y;
					point.z = revisedHeightMap_[y * SECTOR_BLENGTH + x];
					
					if (IntersectBoxWithPoint (point, hazards_[i].min, hazards_[i].max))
					{
						hazards_[i].max.y = hazards_[i].max.y + 1; // hazard boxes are only elongated in the y axis to avoid excessively large boxes.
						isExtentsExtended = true;
						break;
					}
				}

				if (!isExtentsExtended)
				{
					hazard.min.x = x + min_.x - 4;
					hazard.max.x = x + min_.x + 4;
					hazard.min.y = y + min_.y - 4;
					hazard.max.y = y + min_.y + 4;
					hazard.min.z = revisedHeightMap_[y * GetLengthX () + x] - 5;
					hazard.max.z = revisedHeightMap_[y * GetLengthX () + x] + 8;
					if (isWaterType)
						hazard.hazardType = HAZARD_WATER;
					else
						hazard.hazardType = HAZARD_SLOPE;

					hazards_.InsertBack (hazard);

					Assert (hazard.min.x < 1000000 && hazard.min.y < 1000000 && hazard.min.z < 1000000);
					Assert (hazard.max.x < 1000000 && hazard.max.y < 1000000 && hazard.max.z < 1000000);
				}

				if (x < GetLengthX () - 5)
				{
					x += 4;
				}
				else
				{
					x += GetLengthX () - x - 1;
				}	
			}
		}
	}
}

const Hazard& 
BaseZone::GetHazard (uint32 idx)
{
	return hazards_[idx];
}

BaseBuilding* 
BaseZone::GetBuilding (uint32 idx)
{
	return buildings_[idx];
}

uint32 BaseZone::GetNumBuildings ()
{
	return buildings_.GetCount ();
}

uint32 BaseZone::GetNumHazards ()
{
	return hazards_.GetCount ();
}

uint32
BaseZone::GetTerraHeightAtLoc (uint32 x, uint32 y)
{
	x -= min_.x;
	y -= min_.y;
	Assert (x < GetLengthX ());
	Assert (y < GetLengthY ());
	return (uint32)revisedHeightMap_[x + y * GetLengthX ()];
}

void
BaseZone::AddEntrance (const UPoint3D& point)
{
	entrances_.InsertBack (point);
}

uint32
BaseZone::GetNumEntrances ()
{
	return entrances_.GetCount ();
}

const UPoint3D&
BaseZone::GetEntrance (uint32 index)
{
	return entrances_[index];
}

void
BaseZone::AddRoadNode (RoadNode* roadNode)
{
	roadNodes_.InsertBack (roadNode);
}

uint32
BaseZone::GetNumRoadNodes ()
{
	return roadNodes_.GetCount ();
}

RoadNode&
BaseZone::GetRoadNode (uint32 index)
{
	return *roadNodes_[index];
}