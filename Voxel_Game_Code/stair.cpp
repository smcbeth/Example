//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "stair.h"

Stair::Stair (TERRATYPE stairMaterial)
	: material (stairMaterial)
{
	;
}

Stair::~Stair ()
{
	;
}

void
Stair::Build ()
{
	for (uint32 z = min.z; z <= max.z; ++z)
	{
		for (uint32 y = min.y; y <= max.y; ++y)
		{
			for (uint32 x = min.x; x <= max.x; ++x)
			{
				bool isStair = false;
				bool isUnderside = false;
				bool isSupportStructure = false;
				uint32 sCubeBuf = 0;

				switch (direction)
				{
				case STAIR_SOUTH:
					if (z - min.z == max.y - y)
						isStair = true;
					else if (!extendsToFloor && z - min.z + 1 == max.y - y)
						isUnderside = true;
					else if (extendsToFloor && z - min.z + 1 <= max.y - y)
						isSupportStructure = true;
					break;
				case STAIR_WEST:
					if (z - min.z == max.x - x) // max x - 0
						isStair = true;
					else if (!extendsToFloor && z - min.z + 1 == max.x - x)
						isUnderside = true;
					else if (extendsToFloor && z - min.z + 1 <= max.x - x)
						isSupportStructure = true;
					break;
				case STAIR_NORTH:
					if (z - min.z == y - min.y)
						isStair = true;
					else if (!extendsToFloor && z - min.z + 1 == y - min.y)
						isUnderside = true;
					else if (extendsToFloor && z - min.z + 1 <= y - min.y)
						isSupportStructure = true;
					break;
				case STAIR_EAST:
					if (z - min.z == x - min.x)
						isStair = true;
					else if (!extendsToFloor && z - min.z + 1 == x - min.x)
						isUnderside = true;
					else if (extendsToFloor && z - min.z + 1 <= x - min.x)
						isSupportStructure = true;
					break;
				}

				if (isStair)
				{
					SetSCubeTerrainType (&sCubeBuf, material);
					SetSCubeType (&sCubeBuf, SCUBE_HALFCUBETRIANGLE); // STODO: change to SCUBE_STAIR
					SetSCubeSpin (&sCubeBuf, ((uint32)direction + 2) % 4);
				}
				else if (isUnderside)
				{
					SetSCubeTerrainType (&sCubeBuf, material);
					SetSCubeType (&sCubeBuf, SCUBE_HALFCUBETRIANGLE);
					SetSCubeRotation (&sCubeBuf, 1);
					if (direction == STAIR_WEST || direction == STAIR_EAST)
						SetSCubeSpin (&sCubeBuf, ((uint32)direction + 2) % 4);
					else
						SetSCubeSpin (&sCubeBuf, (uint32)direction % 4);
				}
				else if (isSupportStructure)
				{
					SetSCubeTerrainType (&sCubeBuf, material);
				}
				else
				{
					SetSCubeTerrainType (&sCubeBuf, TERRA_AIR);
				}
							
				GetWorld()->SetSCube(x, y, z, sCubeBuf);
			}
		}
	}
}

bool
Stair::SetBoundsForLinkedRooms (BaseRoom* parentRoom, BaseRoom* childRoom, bool isWallPerpendicularToX, bool isWallMin)
{
	const uint32 wallThickness = 1;
	uint32 minPos;
	uint32 maxPos;

	GetMinAndMaxPlacementPositions_ (parentRoom, childRoom, isWallPerpendicularToX, minPos, maxPos);

	if (minPos + length + 1 <= maxPos - 1)
	{
		if (isWallPerpendicularToX)
		{
			if (rand () % 2 == 0)
				direction = STAIR_SOUTH;
			else
				direction = STAIR_NORTH;

			if (isWallMin)
			{
				min.x = parentRoom->GetMin ().x + wallThickness;
				max.x = min.x + width - 1;
			}
			else
			{		
				max.x = parentRoom->GetMax ().x - wallThickness;
				min.x = max.x - width + 1;
			}

			min.y = (minPos + 1) + rand () % (((maxPos - 1) - length) - (minPos + 1) + 1);
			max.y = min.y + length - 1;
		}
		else
		{
			if (rand () % 2 == 0)
				direction = STAIR_WEST;
			else
				direction = STAIR_EAST;
			
			min.x = (minPos + 1) + rand () % (((maxPos - 1) - length) - (minPos + 1) + 1);
			max.x = min.x + length - 1;

			if (isWallMin)
			{
				min.y = parentRoom->GetMin ().y + wallThickness;
				max.y = min.y + width - 1;
			}
			else
			{
				max.y = parentRoom->GetMax ().y - wallThickness;
				min.y = max.y - width + 1;
			}
		}

		if (childRoom->GetFloorLevel () < parentRoom->GetFloorLevel ())
		{
			min.z = childRoom->GetMin ().z + 1;
			max.z = parentRoom->GetMin ().z;
		}
		else
		{
			min.z = parentRoom->GetMin ().z + 1;
			max.z = childRoom->GetMin ().z;
		}

		Assert (min.x > 0);
		Assert (min.y > 0);
		Assert (min.z > 0);
		Assert (max.x > 0);
		Assert (max.y > 0);
		Assert (max.z > 0);
		Assert (max.z - min.z > 1);

		return true;
	}
	return false;
}

void
Stair::GetMinAndMaxPlacementPositions_ (BaseRoom* parentRoom, BaseRoom* childRoom, bool isWallPerpendicularToX, uint32& minPos, uint32& maxPos)
{
	const uint32 wallThickness = 1;

	if (isWallPerpendicularToX)
	{
		if (parentRoom->GetMin ().y > childRoom->GetMin ().y)
			minPos = parentRoom->GetMin ().y + wallThickness;
		else
			minPos = childRoom->GetMin ().y + wallThickness;

		if (parentRoom->GetMax ().y < childRoom->GetMax ().y)
			maxPos = parentRoom->GetMax ().y - wallThickness;
		else
			maxPos = childRoom->GetMax ().y - wallThickness;
	}
	else
	{
		if (parentRoom->GetMin ().x > childRoom->GetMin ().x)
			minPos = parentRoom->GetMin ().x + wallThickness;
		else
			minPos = childRoom->GetMin ().x + wallThickness;

		if (parentRoom->GetMax ().x < childRoom->GetMax ().x)
			maxPos = parentRoom->GetMax ().x - wallThickness;
		else
			maxPos = childRoom->GetMax ().x - wallThickness;	
	}
}

// Spiral Stair methods

SpiralStair::SpiralStair (TERRATYPE stairMaterial, TERRATYPE floorMaterial)
	: Stair (stairMaterial), floorMaterial_ (floorMaterial)
{
	;
}

SpiralStair::~SpiralStair ()
{
	;
}

void
SpiralStair::Build ()
{
	// height increase per turn and center hole size is equivalent to length
	const uint32 floorThickness = 1;
	bool doAdditionalDirectionTurnAtEnd = true; // when true this will reduce the floor space used on the floor the stair connects to. STODO: make this configurable
	bool doPlacePlatformOnly = false; // used in conjunction with doAdditionalDirectionTurnAtEnd

	uint32 numDirectionTurns = 0; // if this number is greater than 2, extendsToFloor will be treated as though its false

	uint32 z = min.z + length - 1; // z is set to the maximum z value for the first flight of the staircase
	if (z > max.z)
		z = max.z;

	uint32 minZValue = min.z - 1; // this is initially set to min.z - 1 to allow for undersides to be constructed in all appropriate places

	while (true)
	{
		uint32 negZOff = 0; // this variable allows for building stair structure below the point where z is incremented to.

		STAIRDIRECTION currentDir;

		// adjust calculations for clockwise rotation when numDirectionTurns is odd
		if (!isClockwise && numDirectionTurns % 2 != 0)
			currentDir = STAIRDIRECTION((direction + numDirectionTurns + 2) % 4);
		else
			currentDir = STAIRDIRECTION((direction + numDirectionTurns) % 4);

		while (z - negZOff >= minZValue && z - negZOff >= min.z)
		{
			for (uint32 y = min.y; y <= max.y; ++y)
			{
				for (uint32 x = min.x; x <= max.x; ++x)
				{
					uint32 sCubeBuf = 0;
					// these bools arent necessarily needed
					bool isStair = false;
					bool isPlatform = false;
					bool isUnderside = false;
					bool isSupportStructure = false;

					switch (currentDir)
					{
					case STAIR_SOUTH:
						if ((isClockwise && x >= max.x - width + 1) ||
							(!isClockwise && x <= min.x + width - 1))
						{
							if ((z - negZOff) - minZValue + width - 1 == max.y - y && (z - negZOff) != minZValue)
							{
								isStair = true;
							}
							else if (((z - negZOff) - minZValue + width - 1 < max.y - y && z - negZOff == max.z && y < max.y - width + 1) || 
									((z - negZOff) == minZValue + length && y <= min.y + width - 1))
							{
								isPlatform = true;
							}
							else if (extendsToFloor && numDirectionTurns <= 2 && y <= min.y + width - 1)
							{
								isSupportStructure = true;
							}
							else if ((!extendsToFloor || numDirectionTurns > 2) && (z - negZOff) - minZValue + width == max.y - y)
							{
								isUnderside = true;
							}
							else if (extendsToFloor && numDirectionTurns <= 2 && (z - negZOff) - minZValue + width <= max.y - y)
							{
								isSupportStructure = true;
							}
						}
						break;
					case STAIR_WEST:
						if ((isClockwise && y <= min.y + width - 1) ||
							(!isClockwise && y >= max.y - width + 1))
						{
							if ((z - negZOff) - minZValue + width - 1 == max.x - x && (z - negZOff) != minZValue)
							{
								isStair = true;
							}
							else if (((z - negZOff) - minZValue + width - 1 < max.x - x && z - negZOff == max.z && x < max.x - width + 1)|| 
									((z - negZOff) == minZValue + length && x <= min.x + width - 1))
							{
								isPlatform = true;
							}
							else if (extendsToFloor && numDirectionTurns <= 2 && x <= min.x + width - 1)
							{
								isSupportStructure = true;
							}
							else if ((!extendsToFloor || numDirectionTurns > 2) && (z - negZOff) - minZValue + width == max.x - x)
							{
								isUnderside = true;
							}
							else if (extendsToFloor && numDirectionTurns <= 2 && (z - negZOff) - minZValue + width <= max.x - x)
							{
								isSupportStructure = true;
							}
						}
						break;
					case STAIR_NORTH:
						if ((isClockwise && x <= min.x + width - 1) ||
							(!isClockwise && x >= max.x - width + 1))	
						{	
							if ((z - negZOff) - minZValue + width - 1 == y - min.y && (z - negZOff) != minZValue)
							{
								isStair = true;
							}
							else if (((z - negZOff) - minZValue + width - 1 < y - min.y && z - negZOff == max.z && y > min.y + width - 1) || 
									((z - negZOff) == minZValue + length && y >= max.y - width + 1))
							{
								isPlatform = true;
							}
							else if (extendsToFloor && numDirectionTurns <= 2 && y >= max.y - width + 1)
							{
								isSupportStructure = true;
							}
							else if ((!extendsToFloor || numDirectionTurns > 2) && (z - negZOff) - minZValue + width == y - min.y)
							{
								isUnderside = true;
							}
							else if (extendsToFloor && numDirectionTurns <= 2 && (z - negZOff) - minZValue + width <= y - min.y)
							{
								isSupportStructure = true;
							}
						}
						break;
					case STAIR_EAST:
						if ((isClockwise && y >= max.y - width + 1) ||
							(!isClockwise && y <= min.y + width - 1))	
						{		
							if ((z - negZOff) - minZValue + width - 1 == x - min.x && (z - negZOff) != minZValue)
							{
								isStair = true;
							}
							else if (((z - negZOff) - minZValue + width - 1 < x - min.x && z - negZOff == max.z && x > min.x + width - 1) || 
									((z - negZOff) == minZValue + length && x >= max.x - width + 1))
							{
								isPlatform = true;
							}
							else if (extendsToFloor && numDirectionTurns <= 2 && x >= max.x - width + 1)
							{
								isSupportStructure = true;
							}
							else if ((!extendsToFloor || numDirectionTurns > 2) && (z - negZOff) - minZValue + width == x - min.x)
							{
								isUnderside = true;
							}
							else if (extendsToFloor && numDirectionTurns <= 2 && (z - negZOff) - minZValue + width <= x - min.x)
							{
								isSupportStructure = true;
							}
						}
						break;
					}

					// STODO: these settings are temporary, differentiate between types in the future and possibly remove some or all of these bool values
					if (isStair && !doPlacePlatformOnly)
					{
						SetSCubeTerrainType (&sCubeBuf, material);
						SetSCubeType (&sCubeBuf, SCUBE_HALFCUBETRIANGLE); // STODO: change to SCUBE_STAIR
						SetSCubeSpin (&sCubeBuf, ((uint32)currentDir + 2) % 4);
					}
					else if (isPlatform || (doPlacePlatformOnly && (isStair || isUnderside || isSupportStructure)))
					{
						SetSCubeTerrainType (&sCubeBuf, floorMaterial_); // STODO: make the types of these materials more configurable
					}
					else if (isUnderside)
					{
						SetSCubeTerrainType (&sCubeBuf, material);
						SetSCubeType (&sCubeBuf, SCUBE_HALFCUBETRIANGLE);
						SetSCubeRotation (&sCubeBuf, 1);
						if (currentDir == STAIR_WEST || currentDir == STAIR_EAST)
							SetSCubeSpin (&sCubeBuf, ((uint32)currentDir + 2) % 4);
						else
							SetSCubeSpin (&sCubeBuf, (uint32)currentDir % 4);
					}
					else if (isSupportStructure)
					{
						SetSCubeTerrainType (&sCubeBuf, material);
					}
					else
					{
						SetSCubeTerrainType (&sCubeBuf, TERRA_AIR);
					}
					
					 // air clears a floor (STODO: make this optional and settable in building settings)
					if (GetSCubeTerrainType (sCubeBuf) != TERRA_AIR || (z - negZOff >= max.z - floorThickness + 1 && !doPlacePlatformOnly))
						GetWorld()->SetSCube(x, y, z - negZOff, sCubeBuf);
				}
			}
			++negZOff;
		}

		++numDirectionTurns;

		// conditional incrementaion of z.
		if (z + 1 > max.z)
		{
			if (doAdditionalDirectionTurnAtEnd)
			{
				doAdditionalDirectionTurnAtEnd = false;
				doPlacePlatformOnly = true;
				minZValue = z;
				continue; // avoid modification of minZValue below
			}
			else
			{
				break;
			}
		}
		else if (z + length > max.z)
		{
			z = max.z;
		}
		else
		{
			z += length;
		}

		// for stairs that extend to floor, it is infeasible to extend down when the number of turns is greater than or equal to 3 
			// because the supports will interfere with the lower stair portions
		if (extendsToFloor && numDirectionTurns == 3)
			minZValue += length * 3;
		else if (!extendsToFloor || numDirectionTurns > 3)
			minZValue += length;

		Assert (z >= minZValue);
	}
}

bool
SpiralStair::SetBoundsForLinkedRooms (BaseRoom* parentRoom, BaseRoom* childRoom, bool isWallPerpendicularToX, bool isWallMin)
{
	const uint32 wallThickness = 1;
	uint32 minPos;
	uint32 maxPos;
	uint32 numTurns;
	BaseRoom* roomWithStairs;

	GetMinAndMaxPlacementPositions_ (parentRoom, childRoom, isWallPerpendicularToX, minPos, maxPos);

	
	uint32 stairCaseDiameter = length + width * 2;
	if (minPos + stairCaseDiameter - 1 <= maxPos)
	{
		isClockwise = rand () % 2 == 0;

		if (childRoom->GetFloorLevel () < parentRoom->GetFloorLevel ())
		{
			min.z = childRoom->GetMin ().z + 1;
			max.z = parentRoom->GetMin ().z;
			roomWithStairs = childRoom;
		}
		else
		{
			min.z = parentRoom->GetMin ().z + 1;
			max.z = childRoom->GetMin ().z;
			roomWithStairs = parentRoom;
		}

		numTurns = (max.z - min.z) / length;

		
		if (isWallPerpendicularToX)
		{
			// these statements will place the staircase in the corner of a room if possible
			if (rand () % 2 == 0)
			{
				if (childRoom->GetMin ().y == parentRoom->GetMin ().y)
					min.y = roomWithStairs->GetMin ().y + wallThickness;
				else if (childRoom->GetMax ().y == parentRoom->GetMax ().y)
					min.y = (roomWithStairs->GetMax ().y - wallThickness) - stairCaseDiameter + 1;
				else
					min.y = minPos + rand () % ((maxPos - (stairCaseDiameter - 1)) - minPos + 1);
			}
			else
			{
				if (childRoom->GetMax ().y == parentRoom->GetMax ().y)
					min.y = (roomWithStairs->GetMax ().y - wallThickness) - stairCaseDiameter + 1;
				else if (childRoom->GetMin ().y == parentRoom->GetMin ().y)
					min.y = roomWithStairs->GetMin ().y + wallThickness;
				else
					min.y = minPos + rand () % ((maxPos - (stairCaseDiameter - 1)) - minPos + 1);
			}
			max.y = min.y + stairCaseDiameter - 1;

			if (isWallMin)
			{
				// STODO: add checks for when stairs are in corner. consider changing cw/ccw orientation to improve looks in certain corner cases
					// non corner cases allow for only one sensible outgoing direction, corner cases allow for two.
				if (numTurns % 4 == 0)
				{
					direction = STAIR_EAST;
				}
				else if (numTurns % 4 == 1)
				{
					if (min.y == roomWithStairs->GetMin ().y + wallThickness)
						isClockwise = false;
					else if (max.y == roomWithStairs->GetMax ().y - wallThickness)
						isClockwise = true;

					if (isClockwise)
						direction = STAIR_NORTH;
					else
						direction = STAIR_SOUTH;
				}
				else if (numTurns % 4 == 2)
				{
					direction = STAIR_WEST;
				}
				else
				{
					if (isClockwise)
						direction = STAIR_SOUTH;
					else
						direction = STAIR_NORTH;
				}
				min.x = roomWithStairs->GetMin ().x + wallThickness;
				max.x = min.x + stairCaseDiameter - 1;
			}
			else
			{
				if (numTurns % 4 == 0)
				{
					direction = STAIR_WEST;
				}
				else if (numTurns % 4 == 1)
				{
					if (min.y == roomWithStairs->GetMin ().y + wallThickness)
						isClockwise = true;
					else if (max.y == roomWithStairs->GetMax ().y - wallThickness)
						isClockwise = false;

					if (isClockwise)
						direction = STAIR_SOUTH;
					else
						direction = STAIR_NORTH;
				}
				else if (numTurns % 4 == 2)
				{
					direction = STAIR_EAST;
				}
				else
				{
					if (isClockwise)
						direction = STAIR_NORTH;
					else
						direction = STAIR_SOUTH;
				}
				max.x = roomWithStairs->GetMax ().x - wallThickness;
				min.x = max.x - stairCaseDiameter + 1;
			}

			// determine if staircase ascends in a clockwise or counterclockwise manner.
			/*
			if (childRoom->GetMin ().y == parentRoom->GetMin ().y && direction == STAIR_WEST)
				isClockwise = true;
			else if (childRoom->GetMin ().y == parentRoom->GetMin ().y && direction == STAIR_EAST)
				isClockwise = false;
			else if (childRoom->GetMax ().y == parentRoom->GetMax ().y && direction == STAIR_WEST)
				isClockwise = false;
			else if (childRoom->GetMax ().y == parentRoom->GetMax ().y && direction == STAIR_EAST)
				isClockwise = true;
			else if (rand () % 2 == 0)
				isClockwise = true;
			else
				isClockwise = false;
				*/		
		}
		else
		{
			// these statements will place the staircase in the corner of a room if possible
			if (rand () % 2 == 0)
			{
				if (childRoom->GetMin ().x == parentRoom->GetMin ().x)
					min.x = roomWithStairs->GetMin ().x + wallThickness;
				else if (childRoom->GetMax ().x == parentRoom->GetMax ().x)
					min.x = (roomWithStairs->GetMax ().x - wallThickness) - stairCaseDiameter + 1;
				else
					min.x = minPos + rand () % ((maxPos - (stairCaseDiameter - 1)) - minPos + 1);
			}
			else
			{
				if (childRoom->GetMax ().x == parentRoom->GetMax ().x)
					min.x = (roomWithStairs->GetMax ().x - wallThickness) - stairCaseDiameter + 1;
				else if (childRoom->GetMin ().x == parentRoom->GetMin ().x)
					min.x = roomWithStairs->GetMin ().x + wallThickness;
				else
					min.x = minPos + rand () % ((maxPos - (stairCaseDiameter - 1)) - minPos + 1);
			}
			max.x = min.x + stairCaseDiameter - 1;

			if (isWallMin)
			{
				if (numTurns % 4 == 0)
				{
					direction = STAIR_NORTH;
				}
				else if (numTurns % 4 == 1)
				{
					if (min.y == roomWithStairs->GetMin ().y + wallThickness)
						isClockwise = true;
					else if (max.y == roomWithStairs->GetMax ().y - wallThickness)
						isClockwise = false;

					if (isClockwise)
						direction = STAIR_WEST;
					else
						direction = STAIR_EAST;
				}
				else if (numTurns % 4 == 2)
				{
					direction = STAIR_SOUTH;
				}
				else
				{
					if (isClockwise)
						direction = STAIR_EAST;
					else
						direction = STAIR_WEST;
				}
				min.y = roomWithStairs->GetMin ().y + wallThickness;
				max.y = min.y + stairCaseDiameter - 1;
			}
			else
			{
				if (numTurns % 4 == 0)
				{
					direction = STAIR_SOUTH;
				}
				else if (numTurns % 4 == 1)
				{
					if (min.y == roomWithStairs->GetMin ().y + wallThickness)
						isClockwise = false;
					else if (max.y == roomWithStairs->GetMax ().y - wallThickness)
						isClockwise = true;

					if (isClockwise)
						direction = STAIR_EAST;
					else
						direction = STAIR_WEST;
				}
				else if (numTurns % 4 == 2)
				{
					direction = STAIR_NORTH;
				}
				else
				{
					if (isClockwise)
						direction = STAIR_WEST;
					else
						direction = STAIR_EAST;
				}
				max.y = roomWithStairs->GetMax ().y - wallThickness;
				min.y = max.y - stairCaseDiameter + 1;
			}

			/*
			// determine if staircase ascends in a clockwise or counterclockwise manner.
			if (childRoom->GetMin ().x == parentRoom->GetMin ().x && direction == STAIR_SOUTH)
				isClockwise = false;
			else if (childRoom->GetMin ().x == parentRoom->GetMin ().x && direction == STAIR_NORTH)
				isClockwise = true;
			else if (childRoom->GetMax ().x == parentRoom->GetMax ().x && direction == STAIR_SOUTH)
				isClockwise = true;
			else if (childRoom->GetMax ().x == parentRoom->GetMax ().x && direction == STAIR_NORTH)
				isClockwise = false;
			else if (rand () % 2 == 0)
				isClockwise = true;
			else
				isClockwise = false;
				*/		
		}
		return true;
	}
	return false;
}