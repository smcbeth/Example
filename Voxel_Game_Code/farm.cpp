#include "farm.h"

Farm::Farm (uint32 xMin, uint32 yMin)
	: BaseZone (xMin, yMin)
{
	LevelTerrain_ ();

	// generate slope map
	GenerateSlopeMap_ (slopeMap_, GetLengthX (), GetLengthY ());

	// define areas where buildings_ cannot be placed
	GenerateHazardExtents_ ();
}

Farm::~Farm ()
{
	;
}

void
Farm::LevelTerrain_ ()
{
	const float32 maxSmoothingFactor = 0.5f;
	const uint32 aboveGroundHeightLimit = 1;
	BaseLevelTerrain_ (maxSmoothingFactor, aboveGroundHeightLimit);
}

void
Farm::GenerateHazardExtents_ ()
{
	const float slopeThreshold = 0.25f;
	BaseGenerateHazardExtents_ (slopeThreshold);
}