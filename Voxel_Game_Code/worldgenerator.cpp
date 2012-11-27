//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "worldgenerator.h"
#include "public\nstdlib\_math.h"
#include "public\nstdlib\noisegenerator2d.h"
#include "public\nstdlib\noisegenerator3d.h"
#include "public\nstdlib\list.h"
#include "world.h"
#include "iworldgenerator.h"
#include "s_sector.h"
#include "towngenerator.h"
#include "astar3d.h"
#include "serverentitylist.h"
#include "s_basefurniture.h"

WorldGenerator::WorldGenerator ()
{
	size_			= 0;
	depth_			= 0;
	heightMap_		= NULL;
	seaLevel_		= 0;
	upperLavaLevel_	= 0;

	sectorOffsets_	= NULL;
	sectorsWritten_	= 0;
}

WorldGenerator::~WorldGenerator ()
{
	;
}

#include "public\nstdlib\binaryheap.h"
void
WorldGenerator::Generate (uint32 size, uint32 depth)
{
	// DEBUG_ASTAR3D:
	struct IntObj
	{
		IntObj(int v)
		{
			val = v;
		}
		int val;
		bool operator<(const IntObj& obj) const
		{
			return val < obj.val;
		}
	};

	BinaryHeap<IntObj> testHeap(&IntObj::operator<);
	IntObj* testResort = new IntObj(16);
	testHeap.Insert(new IntObj(5));
	testHeap.DebugDisplay();
	testHeap.Insert(new IntObj(12));
	testHeap.DebugDisplay();
	testHeap.Insert(new IntObj(2));
	testHeap.DebugDisplay();
	testHeap.Insert(new IntObj(1));
	testHeap.DebugDisplay();
	testHeap.Insert(new IntObj(11));
	testHeap.DebugDisplay();
	testHeap.Insert(new IntObj(25));
	testHeap.DebugDisplay();
	testHeap.Insert(new IntObj(15));
	testHeap.DebugDisplay();
	testHeap.Insert(new IntObj(6));
	testHeap.DebugDisplay();
	testHeap.Insert(testResort);
	testHeap.DebugDisplay();
	testHeap.Insert(new IntObj(3));
	testHeap.DebugDisplay();
	testHeap.Insert(new IntObj(0));
	testHeap.DebugDisplay();
	testHeap.Insert(new IntObj(11));
	testHeap.DebugDisplay();
	testHeap.Insert(new IntObj(22));
	testHeap.DebugDisplay();
	testHeap.Insert(new IntObj(7));

	testHeap.DebugDisplay();

	IntObj* a = testHeap.PopFront();
	testHeap.DebugDisplay();
	IntObj* b = testHeap.PopFront();
	testHeap.DebugDisplay();
	IntObj* c = testHeap.PopFront();
	testHeap.DebugDisplay();
	IntObj* d = testHeap.PopFront();

	testResort->val = 8;
	testHeap.ResortExistingElement(testResort);
	testHeap.DebugDisplay();
	IntObj* e = testHeap.PopFront();
	IntObj* f = testHeap.PopFront();
	IntObj* g = testHeap.PopFront();
	IntObj* h = testHeap.PopFront();
	IntObj* i = testHeap.PopFront();
	IntObj* j = testHeap.PopFront();
	IntObj* k = testHeap.PopFront();
	IntObj* l = testHeap.PopFront();
	IntObj* m = testHeap.PopFront();
	IntObj* n = testHeap.PopFront();

	size *= SECTOR_BLENGTH;
	depth *= SECTOR_BLENGTH;

	Assert (depth < 100000);
	Assert (depth > 0);
	Assert (size < 1000000);
	Assert (size > 0);

	// tell the world api how large our world is going to be
	GetWorld()->SetSize (float32 (size) * SCUBE_LENGTH, float32 (depth) * SCUBE_LENGTH);

	// init vars
	size_			= size;
	depth_			= depth;
	depthMod_		= depth / 2; // STODO: change the ratio between depth and depth mod as the depth of the world changes.
	seaLevel_		= (depthMod_ / 2);
	upperLavaLevel_	= (depthMod_ / 4);
	lowerLavaLevel_	= (depthMod_ / 8);
	sectorsWritten_	= 0;
	townSupportingAreasSparseArray_ = (char*)malloc (Square(size_ / SECTOR_BLENGTH) * sizeof (char));

	seed_ = 33463213;//33463212; //good for town gen //rand () % 10000 + (10000 * rand () % 10000); // huge mountain: 24356675,24356676;//1118969;
	srand (seed_);

	heightMap_	= (uint16*)malloc (SECTOR_BAREA * sizeof (uint16));
	slopeMap_	= (float32*)malloc (SECTOR_BAREA * sizeof (float32));

	veryNarrow3DNoiseObj_ = new NoiseGenerator3D( SECTOR_BLENGTH, SECTOR_BLENGTH, VERYNARROW3D_DILATION, 1, seed_ );
	narrow3DNoiseObj_ = new NoiseGenerator3D( SECTOR_BLENGTH, SECTOR_BLENGTH, NARROW3D_DILATION, 1, seed_ + 55 );
	medium3DNoiseObj_ = new NoiseGenerator3D( SECTOR_BLENGTH, SECTOR_BLENGTH, MEDIUM3D_DILATION, 1, seed_ + 178 );
	broad3DNoiseObj_ = new NoiseGenerator3D( SECTOR_BLENGTH, SECTOR_BLENGTH, BROAD3D_DILATION, 1, seed_ + 2345 );

	aridty2DNoiseObj_ = new NoiseGenerator2D( SECTOR_BLENGTH, SECTOR_BLENGTH, ARIDTY_DILATION, 1, seed_ );
	smoothing2DNoiseObj_ = new NoiseGenerator2D( SECTOR_BLENGTH, SECTOR_BLENGTH, SMOOTHING_DILATION, 1, seed_ + 12 );
	highlands2DNoiseObj_ = new NoiseGenerator2D( SECTOR_BLENGTH, SECTOR_BLENGTH, HIGHLANDS_DILATION, 1, seed_ + 45 );
	hillPeak2DNoiseObj_  = new NoiseGenerator2D( SECTOR_BLENGTH, SECTOR_BLENGTH, HILLPEAK_DILATION, 1, seed_ + 241 );
	mountain2DNoiseObj_ = new NoiseGenerator2D( SECTOR_BLENGTH, SECTOR_BLENGTH, MOUNTAINPEAK_DILATION, 1, seed_ + 1421 );
	veryNarrow2DNoiseObj_ = new NoiseGenerator2D( SECTOR_BLENGTH, SECTOR_BLENGTH, VERYNARROW2D_DILATION, 1, seed_ );
	narrow2DNoiseObj_ = new NoiseGenerator2D( SECTOR_BLENGTH, SECTOR_BLENGTH, NARROW2D_DILATION, 1, seed_ + 24 );
	medium2DNoiseObj_ = new NoiseGenerator2D( SECTOR_BLENGTH, SECTOR_BLENGTH, MEDIUM2D_DILATION, 1, seed_ + 255 );
	broad2DNoiseObj_ = new NoiseGenerator2D( SECTOR_BLENGTH, SECTOR_BLENGTH, BROAD2D_DILATION, 1, seed_ + 1712 );
	quarterMile2DNoiseObj_ = new NoiseGenerator2D( SECTOR_BLENGTH, SECTOR_BLENGTH, QUARTERMILE2D_DILATION, 1, seed_  + 3523 );
	halfMile2DNoiseObj_ = new NoiseGenerator2D( SECTOR_BLENGTH, SECTOR_BLENGTH, HALFMILE2D_DILATION, 1, seed_ + 4235 );
	mile2DNoiseObj_ = new NoiseGenerator2D( SECTOR_BLENGTH, SECTOR_BLENGTH, MILE2D_DILATION, 1, seed_ + 5556 );

	aridty_				= (float32*)malloc (SECTOR_BAREA * sizeof (float32));
	veryNarrow2DNoise_	= (float32*)malloc (SECTOR_BAREA * sizeof (float32));
	narrow2DNoise_		= (float32*)malloc (SECTOR_BAREA * sizeof (float32));
	medium2DNoise_		= (float32*)malloc (SECTOR_BAREA * sizeof (float32));
	broad2DNoise_		= (float32*)malloc (SECTOR_BAREA * sizeof (float32));
	quarterMile2DNoise_	= (float32*)malloc (SECTOR_BAREA * sizeof (float32));
	halfMile2DNoise_	= (float32*)malloc (SECTOR_BAREA * sizeof (float32));
	mile2DNoise_		= (float32*)malloc (SECTOR_BAREA * sizeof (float32));

	veryNarrow3DNoise_	= (float32*)malloc (SECTOR_BVOLUME * sizeof (float32));
	narrow3DNoise_		= (float32*)malloc (SECTOR_BVOLUME * sizeof (float32));
	medium3DNoise_		= (float32*)malloc (SECTOR_BVOLUME * sizeof (float32));
	broad3DNoise_		= (float32*)malloc (SECTOR_BVOLUME * sizeof (float32));

	uint16* heightMaps[9];
	for (uint32 i = 0; i < 9; ++i)
		heightMaps[i] = (uint16*)malloc (SECTOR_BAREA * sizeof (uint16));

	// PERFORMANCE: generate 2D noise, heightmap, and slopemap only once for every y stack, rather than every sector
	//              just reordering the loop for this causes problems, check how sector data is saved and loaded
	for (uint32 oZ = 0; oZ < depth_; oZ += SECTOR_BLENGTH)
	{
		for (uint32 oY = 0; oY < size_; oY += SECTOR_BLENGTH)
		{
			for (uint32 oX = 0; oX < size_; oX += SECTOR_BLENGTH)
			{
				S_Sector* sector;
				bool isTownSupportingArea;

				PrepTerraNoise (oX, oY, oZ, SECTOR_BLENGTH, SECTOR_BLENGTH, false);
				isTownSupportingArea = GenerateHeightMap (oX, oY, heightMap_, SECTOR_BLENGTH, SECTOR_BLENGTH);
				
				if (oZ == 0) // to prevent repeats of suitable town sector coordinates
				{
					if (isTownSupportingArea)
					{
						townSupportingAreas_.InsertBack (oX / SECTOR_BLENGTH + (oY / SECTOR_BLENGTH) * (size_ / SECTOR_BLENGTH)); // ignores y coordinates since sector depth might change
						townSupportingAreasSparseArray_[oX / SECTOR_BLENGTH + (oY / SECTOR_BLENGTH) * (size_ / SECTOR_BLENGTH)] = 1;
					}
					else
					{
						townSupportingAreasSparseArray_[oX / SECTOR_BLENGTH + (oY / SECTOR_BLENGTH) * (size_ / SECTOR_BLENGTH)] = 0;
					}	
				}

				FillHeightMaps_ (oX, oY, oZ, heightMaps);
				
				GenerateSlopeMap (slopeMap_, heightMaps, SECTOR_BLENGTH, SECTOR_BLENGTH);

				PrepTerraNoise (oX, oY, oZ, SECTOR_BLENGTH, SECTOR_BLENGTH, true);

				// 2d xy noise interpolation loop
				for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
				{
					for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
					{
						// origin values are moduloed by the dilation to get noise to match up for each sector
						aridty_[x + y * SECTOR_BLENGTH] =				aridty2DNoiseObj_->GetNoiseFast(float32(x + oX % ARIDTY_DILATION), float32(y + oY % ARIDTY_DILATION), INTERPOLATE_COSINE);
						veryNarrow2DNoise_[x + y * SECTOR_BLENGTH] =	veryNarrow2DNoiseObj_->GetNoiseFast(float32(x + oX % VERYNARROW2D_DILATION), float32(y + oY % VERYNARROW2D_DILATION), INTERPOLATE_COSINE);
						narrow2DNoise_[x + y * SECTOR_BLENGTH] =		narrow2DNoiseObj_->GetNoiseFast(float32(x + oX % NARROW2D_DILATION), float32(y + oY % NARROW2D_DILATION), INTERPOLATE_COSINE);
						medium2DNoise_[x + y * SECTOR_BLENGTH] =		medium2DNoiseObj_->GetNoiseFast(float32(x + oX % MEDIUM2D_DILATION), float32(y + oY % MEDIUM2D_DILATION), INTERPOLATE_COSINE);
						broad2DNoise_[x + y * SECTOR_BLENGTH] =			broad2DNoiseObj_->GetNoiseFast(float32(x + oX % BROAD2D_DILATION), float32(y + oY % BROAD2D_DILATION), INTERPOLATE_COSINE);
						quarterMile2DNoise_[x + y * SECTOR_BLENGTH] =	quarterMile2DNoiseObj_->GetNoiseFast(float32(x + oX % QUARTERMILE2D_DILATION), float32(y + oY % QUARTERMILE2D_DILATION), INTERPOLATE_COSINE);
						halfMile2DNoise_[x + y * SECTOR_BLENGTH] =		halfMile2DNoiseObj_->GetNoiseFast(float32(x + oX % HALFMILE2D_DILATION), float32(y + oY % HALFMILE2D_DILATION), INTERPOLATE_COSINE);
						mile2DNoise_[x + y * SECTOR_BLENGTH] =			mile2DNoiseObj_->GetNoiseFast(float32(x + oX % MILE2D_DILATION), float32(y + oY % MILE2D_DILATION), INTERPOLATE_COSINE);
						
					}
				}

				sector = GenerateSector_ (oX, oY, oZ, heightMaps);

				// compress the sectors data for now
				//   (it will have to be uncompressed and recompressed during the compute vis stage)
				sector->PurgeData ();

				// add sector to world
				GetWorld()->AddSector (sector);

				// revise y-stack of sectors, adding trees, grass, snow.
				if (oZ == depth_ - SECTOR_BLENGTH)
				{
					ReviseHeightMap_ (oX, oY);
					GenerateSlopeMap (slopeMap_, heightMaps, SECTOR_BLENGTH, SECTOR_BLENGTH);

					ReviseSectors_ (oX, oY);
				}
			}
		}
	}

	// free local noise buffers
	free (aridty_);
	free (veryNarrow2DNoise_);
	free (narrow2DNoise_);
	free (medium2DNoise_);
	free (broad2DNoise_);
	free (quarterMile2DNoise_);
	free (halfMile2DNoise_);
	free (mile2DNoise_);
	free (veryNarrow3DNoise_);
	free (narrow3DNoise_);
	free (medium3DNoise_);
	free (broad3DNoise_);

	// cleanup memory
	for (uint32 i = 0; i < 9; ++i)
		free (heightMaps[i]);
	free (heightMap_);
	free (slopeMap_);
	free (townSupportingAreasSparseArray_);

	// generate towns
 	TownGenerator townGenerator;
	townGenerator.Generate();

	// delete noise objects: town generator is the last thing to utilize them
	delete veryNarrow3DNoiseObj_;
	delete narrow3DNoiseObj_;
	delete medium3DNoiseObj_;
	delete broad3DNoiseObj_;

	delete aridty2DNoiseObj_;
	delete smoothing2DNoiseObj_;
	delete highlands2DNoiseObj_;
	delete hillPeak2DNoiseObj_;
	delete mountain2DNoiseObj_;
	delete veryNarrow2DNoiseObj_;
	delete narrow2DNoiseObj_;
	delete medium2DNoiseObj_;
	delete broad2DNoiseObj_;
	delete quarterMile2DNoiseObj_;
	delete halfMile2DNoiseObj_;
	delete mile2DNoiseObj_;

	SmoothSectorsAndFindOldAir_();

	// now all of our sectors have been loaded up
	// compute visibility information
	GetWorld()->ComputeVis();

	// create the player
	IServerEntity* player = GetWorld()->CreateEntity( "player" );
	player->SetTranslation( Vector3D( 300.0f, 300.0f, 1000.0f ) ); // replace with non-shitty position plzing (middle of a town?)

	// save out and unload
	GetWorld()->Save();
	GetWorld()->Unload();

	// reseed srand based on time
	srand( (uint32)time( NULL ) );
}

#define DEBUG_HEIGHT	300

bool
WorldGenerator::GenerateHeightMap (uint32 oX, uint32 oY, uint16* heightMap, uint32 lengthX, uint32 lengthY)
{
	uint32 minHeight = 0;
	uint32 maxHeight = 0;
	uint32 waterCount = 0;
	float32 mileRangeHeight;
	float32 veryLongRangeHeight;
	float32 longRangeHeight;
	float32 mediumRangeHeight;
	float32 shortRangeHeight;
	float32 veryShortRangeHeight;
	float32 highlandsBonus;
	float32 hillPeakHeight;
	float32 mountainPeakHeight; // STODO: try using two sets of noise for this, one that has cliffs and one that doesnt
	const float32 smoothnessDivisor = 10.0f; //8.0f; //increasing this divisor will smooth the terrain as a whole and decreasing it will roughen the terrain.

	for (uint32 y = 0; y < lengthY; ++y)
	{
		for (uint32 x = 0; x < lengthX; ++x)
		{
			mileRangeHeight = mile2DNoiseObj_->GetNoiseFast (float32(x + oX % MILE2D_DILATION), float32(y + oY % MILE2D_DILATION), INTERPOLATE_COSINE);
			veryLongRangeHeight = halfMile2DNoiseObj_->GetNoiseFast (float32(x + oX % HALFMILE2D_DILATION), float32(y + oY % HALFMILE2D_DILATION), INTERPOLATE_COSINE);
			longRangeHeight = quarterMile2DNoiseObj_->GetNoiseFast (float32(x + oX % QUARTERMILE2D_DILATION), float32(y + oY % QUARTERMILE2D_DILATION), INTERPOLATE_COSINE);
			mediumRangeHeight = broad2DNoiseObj_->GetNoiseFast (float32(x + oX % BROAD2D_DILATION), float32(y + oY % BROAD2D_DILATION), INTERPOLATE_COSINE);
			shortRangeHeight = medium2DNoiseObj_->GetNoiseFast (float32(x + oX % MEDIUM2D_DILATION), float32(y + oY % MEDIUM2D_DILATION), INTERPOLATE_COSINE);
			veryShortRangeHeight = narrow2DNoiseObj_->GetNoiseFast (float32(x + oX % NARROW2D_DILATION), float32(y + oY % NARROW2D_DILATION), INTERPOLATE_COSINE);
			highlandsBonus = highlands2DNoiseObj_->GetNoiseFast (float32(x + oX % HIGHLANDS_DILATION), float32(y + oY % HIGHLANDS_DILATION), INTERPOLATE_COSINE);

			if (highlandsBonus < 0.0f)
				highlandsBonus = 0.0f;

			// generate cliffs. STODO: improve this. possibly have multiple cliff sources.
			if (longRangeHeight < -0.05f && mediumRangeHeight > 0.0f)
				highlandsBonus -= 0.3f * mediumRangeHeight;
			
			if (longRangeHeight + veryLongRangeHeight + mileRangeHeight > 0.0f)
			{
				hillPeakHeight = hillPeak2DNoiseObj_->GetNoiseFast (float32(x + oX % HILLPEAK_DILATION), float32(y + oY % HILLPEAK_DILATION), INTERPOLATE_COSINE);
				if (hillPeakHeight < -0.1f)
				{
					hillPeakHeight = 0.0f;
				}
				else
				{
					hillPeakHeight += 0.1001f;
					hillPeakHeight *= (longRangeHeight + veryLongRangeHeight + mileRangeHeight) * 0.9f * (highlandsBonus + 0.9f);

					mediumRangeHeight *= (hillPeakHeight + 1.0f);
					shortRangeHeight *= (hillPeakHeight + 1.0f);
					veryShortRangeHeight *= (hillPeakHeight + 1.0f);

					//if (longRangeHeight + highlandsBonus < -0.1f)
						//hillPeakHeight *= 0.666667f; 
				}
			}
			else
			{
				hillPeakHeight = 0.0f;
			}

			if (longRangeHeight + veryLongRangeHeight + mileRangeHeight > 0.6f)
			{
				mountainPeakHeight = mountain2DNoiseObj_->GetNoiseFast (float32(x + oX % MOUNTAINPEAK_DILATION), float32(y + oY % MOUNTAINPEAK_DILATION), INTERPOLATE_LINEAR);
				if (mountainPeakHeight < -0.1f)
				{
					mountainPeakHeight = 0.0f;
				}
				else
				{
					mountainPeakHeight += 0.1001f;
					mountainPeakHeight *= ((longRangeHeight + veryLongRangeHeight + mileRangeHeight) - 0.59999f) * 0.80f * (highlandsBonus + 1.0f);// * (highlandsBonus + 1.5f);

					hillPeakHeight *= (mountainPeakHeight + 0.2f) * 2.0f;
					mediumRangeHeight *= (mountainPeakHeight + 0.2f) * 2.5f;
					shortRangeHeight *= (mountainPeakHeight + 0.2f) * 3.0f;
					veryShortRangeHeight *= (mountainPeakHeight + 0.2f) * 2.5f;

					// taper excessive mountain peaks.
					if (mountainPeakHeight > 0.5f)
					{
						mountainPeakHeight = (mountainPeakHeight - 0.5f) * 0.5f + 0.5f;
						if (mountainPeakHeight > 0.75f)
						{
							mountainPeakHeight = (mountainPeakHeight - 0.75f) * 0.6f + 0.75f;
							if (mountainPeakHeight > 1.1f)
								mountainPeakHeight = (mountainPeakHeight - 1.1f) * 0.3f + 1.1f;
						}
					}

					//if (longRangeHeight + highlandsBonus < 0.1f)
						//mountainPeakHeight *= 0.666667f;
				}
			}
			else
			{
				mountainPeakHeight = 0.0f;
			}

			// noise generated smoothness gradient of 0-1. 0 being smooth, 1 being unsmooth
			float smoothingGradient = smoothing2DNoiseObj_->GetNoiseFast (float32(x + oX % SMOOTHING_DILATION), float32(y + oY % SMOOTHING_DILATION), INTERPOLATE_COSINE);
			smoothingGradient = (smoothingGradient + 1.0f) / 2.0f;

			uint32 height = uint32(int32(((mileRangeHeight * 1.5f + veryLongRangeHeight * 4.0f + longRangeHeight * 5.0f + mediumRangeHeight * 2.1f + shortRangeHeight * 1.1f + veryShortRangeHeight * 0.8f +
				+ highlandsBonus * 11.0f + hillPeakHeight * 12.0f + mountainPeakHeight * 24.0f) / smoothnessDivisor) * smoothingGradient * (float32)seaLevel_) + (int32)seaLevel_);

			// excessive height check
			if (height > depth_ / 2)
			{
				// taper excessive height
				height = (height - (depth_ / 2)) / 2 + (depth_ / 2);

				// place hard limit on excessive height
				if (height > depth_ - (depth_ / 10))
				{
					if (height > (depth_ - 3) - uint32(Abs (mediumRangeHeight * 1.8f - shortRangeHeight + veryShortRangeHeight * 0.8f) * (depth_ / 32)))
						height = (depth_ - 3) - uint32(Abs (mediumRangeHeight * 1.8f - shortRangeHeight + veryShortRangeHeight * 0.8f) * (depth_ / 32));
				}
			}
			else if (height < upperLavaLevel_ + 20)
			{
				height = upperLavaLevel_ + 20;
			}
			// STODO: add valleys and plateaus: a variation on the smoothness gradient, where smoothed areas are raised or lowered in elevation from surronding areas
			//			(can exclude mountain peak areas.)
			//STODO: (for large maps)
				// for mountain ranges generate some dilated 2D noise, then use a range of noise say, 0.00 to 0.20, as a positive modifer to height.
				// all other values should have no effect on height. have values in the middle of the range increase height more

				// also to avoid mountain ranges forming rings, have another dilated noise range act as a multiplier for the mountain range height
					// increaser. mountain ranges could be active from range 0.00+ and the higher the number, the higher the multiplier
					// for the mountain range.

			//height = DEBUG_HEIGHT; // DEBUG_WORLDGENERATOR:

			// DEBUG_WORLDGENERATOR: visualize positive x and y

			//BUG: probably need terrain debugger to find this one
			// notes: this is reproducable on a 3x3 sector map with terrain height all set to 300.
			// notes: this has nothing to do with cliff sculpting. (turning off cliff sculpting will make this bug more visible)
				// this will not be caught by accessing the heightmap from generate sector
			
			//if(x + oX == size_ - 1)
				//height = 305; // BUG: this creates only one wall at the edge of the world. the second wall is not present even with cliff sculpting turned off
			
			//if(x + oX == size_ - 2 && y % 2 == 0)
				//height = 305; // BUG: this creates one line of pillars near the end of the world
			//if(x + oX == size_ - 1 && y % 2 == 0)
				//height = 305; // BUG: this creates two lines of pillars. one at the edge of the world(which is mostly solid), one at the previous sector
			//if(x + oX == size_ - 129 && y % 2 == 0)
				//height = 305; // BUG: same as above but with two lines bordering the center row of sectors
			//if(x + oX == size_ - 129)
				//height = 305; // BUG: creates one solid wall

			// DEBUG_WORLDGENERATOR:

			heightMap[x + y * lengthX] = (uint16)height;
			
			if (x == 0 && y == 0)
			{
				minHeight = height;
				maxHeight = height;
			}
			else
			{
				if (height < minHeight)
					minHeight = height;
				else if (height > maxHeight)
					maxHeight = height;
			}
			if (height < seaLevel_)
				++waterCount;
		}
	}
	// the probability of this failing in the absence of a bug should be astronomically low.
	//assert (minHeight != maxHeight || minHeight == DEBUG_HEIGHT);

	if (oX == 0 || oY == 0 || oX == size_ - SECTOR_BLENGTH || oY == size_ - SECTOR_BLENGTH)
		return false; // dont allow suitable areas on edge of map
	else
		return (maxHeight - minHeight < 50 && waterCount < SECTOR_BAREA / 2); //returns true if heightmap area is suitable for town placement
}

void
WorldGenerator::GenerateSlopeMap (float32* slopeMap, uint16* heightMaps[], uint32 lengthX, uint32 lengthY)
{
	const int32 slopeSampDist = 5;

	// currently samples in a "circle" with a rough radius of 3
	for (uint32 y = 0; y < lengthY; ++y)
	{
		for (uint32 x = 0; x < lengthX; ++x)
		{
			float32 divisor = 0.0f;
			float32 sum = 0.0f;
			for (int32 yOff = -slopeSampDist; yOff <= slopeSampDist; ++yOff)
			{
				for (int32 xOff = -slopeSampDist; xOff <= slopeSampDist; ++xOff)
				{
					if ((Square (xOff) + Square (yOff)) > Square(slopeSampDist + 1)) // keeps sample locations in a circular pattern rather than a square one.
						continue;
					if (xOff == 0 && yOff == 0)
						continue;
					divisor += 1.0f;
					sum += Abs(heightMaps[4][x + y * lengthX] - heightMaps[((x + lengthX + xOff) / SECTOR_BLENGTH) + ((y + lengthY + yOff) / SECTOR_BLENGTH) * 3][(x + lengthX + xOff) % lengthX + ((y + lengthY + yOff) % lengthY) * lengthX]) 
						/ FastSqrt (float32 (Square (xOff) + Square (yOff))); // PERFORMANCE: use lookup table
				}
			}
			slopeMap[x + y * lengthX] = sum / divisor;
		}
	}
	
	// enforce max slope value
	for (uint32 y = 0; y < lengthY; ++y)
	{
		for (uint32 x = 0; x < lengthX; ++x)
		{
			if (slopeMap[x + y * lengthX] > 1.0f)
				slopeMap[x + y * lengthX] = 1.0;
		}
	}
}

void
WorldGenerator::PrepTerraNoise (uint32 oX, uint32 oY, uint32 oZ, uint32 lengthX, uint32 lengthY, bool prep3DNoise)
{
	// arbitary values are added to seed to differentiate noise types
	if (prep3DNoise)
	{
		veryNarrow3DNoiseObj_->PrepNoise (oX, oY, oZ, SMOOTH_LIGHT); 
		narrow3DNoiseObj_->PrepNoise (oX, oY, oZ, SMOOTH_LIGHT);
		medium3DNoiseObj_->PrepNoise (oX, oY, oZ, SMOOTH_NONE);
		broad3DNoiseObj_->PrepNoise (oX, oY, oZ, SMOOTH_LIGHT);
	}

	// x and y 2D noise
	aridty2DNoiseObj_->PrepNoise (oX, oY, SMOOTH_LIGHT);
	smoothing2DNoiseObj_->PrepNoise (oX, oY, SMOOTH_LIGHT);
	highlands2DNoiseObj_->PrepNoise (oX, oY, SMOOTH_NORMAL);
	hillPeak2DNoiseObj_->PrepNoise (oX, oY, SMOOTH_NONE);
	mountain2DNoiseObj_->PrepNoise (oX, oY, SMOOTH_NONE);
	veryNarrow2DNoiseObj_->PrepNoise (oX, oY, SMOOTH_NORMAL);
	narrow2DNoiseObj_->PrepNoise (oX, oY, SMOOTH_NORMAL);
	medium2DNoiseObj_->PrepNoise (oX, oY, SMOOTH_LIGHT);
	broad2DNoiseObj_->PrepNoise (oX, oY, SMOOTH_LIGHT);
	quarterMile2DNoiseObj_->PrepNoise (oX, oY, SMOOTH_LIGHT);
	halfMile2DNoiseObj_->PrepNoise (oX, oY, SMOOTH_LIGHT);
	mile2DNoiseObj_->PrepNoise (oX, oY, SMOOTH_LIGHT);
}

uint32 
WorldGenerator::GetWorldDepth ()
{
	return depth_;
}

uint32
WorldGenerator::GetSeaLevel ()
{
	return seaLevel_;
}

uint32
WorldGenerator::GetWorldSize ()
{
	return size_;
}

Vector<uint32>* 
WorldGenerator::GetTownSupportingAreas ()
{
	return &townSupportingAreas_;
}

char* 
WorldGenerator::GetTownSupportingAreasSparseArray ()
{
	return townSupportingAreasSparseArray_;
}



//-------------------- Private Methods------------------------------------------------------------------------------------

					
#define VERYNARROW3DNOISE		veryNarrow3DNoise_[x + y * SECTOR_BLENGTH + z * SECTOR_BAREA]
#define NARROW3DNOISE			narrow3DNoise_[x + y * SECTOR_BLENGTH + z * SECTOR_BAREA]
#define MEDIUM3DNOISE			medium3DNoise_[x + y * SECTOR_BLENGTH + z * SECTOR_BAREA]
#define BROAD3DNOISE			broad3DNoise_[x + y * SECTOR_BLENGTH + z * SECTOR_BAREA]
// combined 3D noise: intermediate 3D noise interpolates between narrow and broad 3D noise.
#define COMBINED3DNOISE1		((BROAD3DNOISE - 0.06f) * ((MEDIUM3DNOISE + 1.0f) * 0.5f) + (NARROW3DNOISE * 1.1f + 0.22f) * (1.0f - ((MEDIUM3DNOISE + 1.0f) * 0.5f))) // reduced broad, greatly increased narrow
#define COMBINED3DNOISE2		((BROAD3DNOISE + 0.06f) * ((MEDIUM3DNOISE + 1.0f) * 0.5f) + (NARROW3DNOISE * 1.1f - 0.22f) * (1.0f - ((MEDIUM3DNOISE + 1.0f) * 0.5f))) // increased broad, greatly decreased narrow
#define HEIGHTOFFSET			(((int32)oZ + (int32)z - (int32)heightMap_[y * SECTOR_BLENGTH + x]))
#define SEALEVELOFFSET			(((int32)oZ + (int32)z - (int32)seaLevel_))
#define DEPTHFACTOR				(float32(HEIGHTOFFSET) / 15.0f)
#define HEIGHTFACTOR			(COMBINED3DNOISE1 * 2.0f - DEPTHFACTOR + (float32(oZ + z) / float32(depthMod_ / 3)))
#define ARIDTY					aridty_[x + y * SECTOR_BLENGTH]
#define VERYNARROW2DNOISE		veryNarrow2DNoise_[x + y * SECTOR_BLENGTH]
#define NARROW2DNOISE			narrow2DNoise_[x + y * SECTOR_BLENGTH]
#define BROAD2DNOISE			broad2DNoise_[x + y * SECTOR_BLENGTH]
#define QUARTERMILE2DNOISE		quarterMile2DNoise_[x + y * SECTOR_BLENGTH]
#define HALFMILE2DNOISE			halfMile2DNoise_[x + y * SECTOR_BLENGTH]
#define MILE2DNOISE				mile2DNoise_[x + y * SECTOR_BLENGTH]

S_Sector*
WorldGenerator::GenerateSector_ (uint32 oX, uint32 oY, uint32 oZ, uint16* heightMaps[])
{
	uint32* sectorData;

	// set all blocks in sector to default values.
	sectorData = (uint32*)Calloc (size_t(Cube (SECTOR_BLENGTH)), sizeof (uint32));

	// generate sector in three passes
	GSSetTerrainShapeAnd3DNoisePass_ (oX, oY, oZ, sectorData);
	GSModifyTerrainWith3DNoisePass_ (oX, oY, oZ, sectorData, heightMaps);
	GSSetTerrainTypesPass_ (oX, oY, oZ, sectorData);
	
	// create new sector
	S_Sector* sector = new S_Sector;
	
	Vector3D origin (
		oX * SCUBE_LENGTH + (SECTOR_LENGTH / 2),
		oY * SCUBE_LENGTH + (SECTOR_LENGTH / 2),
		oZ * SCUBE_LENGTH + (SECTOR_LENGTH / 2));

	sector->Init (origin, sectorData);
	
	return sector;
}

void
WorldGenerator::GSSetTerrainShapeAnd3DNoisePass_ (uint32 oX, uint32 oY, uint32 oZ, uint32* sectorData)
{
	uint32 sCubeBuf;

	// define air, water, lava, land, 3D noise for land.
	for (uint32 z = 0; z < SECTOR_BLENGTH; ++z)
	{
		for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
		{
			for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
			{
				sCubeBuf = CENTER; // this data will usually be zeroed.

				// determine if block is air/water or solid type
				if (HEIGHTOFFSET > 0)
				{
					if (oZ + z < seaLevel_)
						SetSCubeTerrainType (&sCubeBuf, TERRA_WATER);
					// else: air by default
				}
				else if (z + oZ < 3)
				{
					TERRATYPE type = GetSCubeTerrainType (sCubeBuf);
					SetSCubeTerrainType (&sCubeBuf, TERRA_LAVA);
				}
				else
				{
					// PERFORMANCE: these calls are SLOW.
					
					// SNOW: see if this can be moved to the next gs call since cave noise does not use this (its best to make noisegenerator a class first)
					VERYNARROW3DNOISE = veryNarrow3DNoiseObj_->GetNoiseFast (float32(x + oX % VERYNARROW3D_DILATION), float32(y + oY % VERYNARROW3D_DILATION), float32(z + oZ % (VERYNARROW3D_DILATION / 2)));
					
					NARROW3DNOISE = narrow3DNoiseObj_->GetNoiseFast (float32(x + oX % NARROW3D_DILATION), float32(y + oY % NARROW3D_DILATION), float32(z + oZ % (NARROW3D_DILATION / 2)));
					MEDIUM3DNOISE = medium3DNoiseObj_->GetNoiseFast (float32(x + oX % MEDIUM3D_DILATION), float32(y + oY % MEDIUM3D_DILATION), float32(z + oZ % (MEDIUM3D_DILATION / 2)));
					BROAD3DNOISE = broad3DNoiseObj_->GetNoiseFast (float32(x + oX % BROAD3D_DILATION), float32(y + oY % BROAD3D_DILATION), float32(z + oZ % (BROAD3D_DILATION / 2)));

					// represents land types: for performance, 3D noise is only defined for land types. All other areas will be undefined.
					SetSCubeTerrainType (&sCubeBuf, TERRA_STONE); 
				}
				CENTER = sCubeBuf;
			}
		}
	}
}

void
WorldGenerator::GSModifyTerrainWith3DNoisePass_ (uint32 oX, uint32 oY, uint32 oZ, uint32* sectorData, uint16* heightMaps[])
{
	uint32 sCubeBuf;

	// define caves, aquifiers, lava pockets. // STODO: use cave noise to modify cliff faces and create overhangs. (check for high slope, close to surface (within ~3 blocks))
	for (uint32 z = 0; z < SECTOR_BLENGTH; ++z)
	{
		for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
		{
			for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
			{
				sCubeBuf = CENTER;

				if (GetSCubeTerrainType (sCubeBuf) == TERRA_STONE)
				{
					float32 minValue;
					float32 maxValue;
					 // STODO: consider having SOME caves open up when approaching surface. or just have opening up done within ~3 blocks away from surface.
					// Note- this old code does the opposite of opening up. 
					//if (HEIGHTOFFSET > -25)
					//{
					//	minValue = .40f + float32(HEIGHTOFFSET) / 500.0f; // range .40-.35
					//}
						
					// prevent air caves from reaching bodies of water
					if ((int16)heightMap_[y * SECTOR_BLENGTH + x] - (int16)seaLevel_ <= 10 && HEIGHTOFFSET > -20)
					{
						if ((int16)heightMap_[y * SECTOR_BLENGTH + x] - (int16)seaLevel_ > 0)
							minValue = .35f + (float32(20 + HEIGHTOFFSET) / 40.0f) / float32((int16)heightMap_[y * SECTOR_BLENGTH + x] - (int16)seaLevel_);
						else
							minValue = .35f + float32(20 + HEIGHTOFFSET) / 40.0f; //range .35-.85
					}
					else
					{
						minValue = .35f;
					}
					// prevent aquifer cave formation at upperLavaLevel_ + 20 to allow for air below this value
					if (Abs ((int32)z + (int32)oZ - int32(upperLavaLevel_ + 20)) < 10)
					{
						maxValue = -0.35f - (float32(10 - Abs ((int32)z + (int32)oZ - int32(upperLavaLevel_ + 20))) / 20.0f); //subtracts .50f from maxValue when y + oY == upperLavaLevel_ + 20
					}
					else
					{
						maxValue = -0.35f;
					}

					// reduce cave formation at flat surfaces and near the surface in general
					if (HEIGHTOFFSET > -10 && slopeMap_[x + y * SECTOR_BLENGTH] < 0.35f)
					{
						minValue += float32 (10 + HEIGHTOFFSET) * 0.1f * (0.401f - slopeMap_[x + y * SECTOR_BLENGTH]);
						maxValue -= float32 (10 + HEIGHTOFFSET) * 0.1f * (0.401f - slopeMap_[x + y * SECTOR_BLENGTH]);
					}
					
					
					// sculpt cliffs and steep surfaces.
					if (slopeMap_[x + y * SECTOR_BLENGTH] > 0.601f && HEIGHTOFFSET > -80)
					{
						// supersample heightmaps to find a posivite height offset
						if ((int32((int32)oZ + (int32)z - 
								(int32)heightMaps[((x + SECTOR_BLENGTH - 4) / SECTOR_BLENGTH) + ((y + SECTOR_BLENGTH) / SECTOR_BLENGTH) * 3]
									[(x + SECTOR_BLENGTH - 4) % SECTOR_BLENGTH	+ ((y + SECTOR_BLENGTH) % SECTOR_BLENGTH) * SECTOR_BLENGTH] >= 1)) ||
							(int32((int32)oZ + (int32)z - 
								(int32)heightMaps[((x + SECTOR_BLENGTH + 4) / SECTOR_BLENGTH) + ((y + SECTOR_BLENGTH) / SECTOR_BLENGTH) * 3]
									[(x + SECTOR_BLENGTH + 4) % SECTOR_BLENGTH	+ ((y + SECTOR_BLENGTH) % SECTOR_BLENGTH) * SECTOR_BLENGTH] >= 1)) ||
							(int32((int32)oZ + (int32)z - 
								(int32)heightMaps[((x + SECTOR_BLENGTH) / SECTOR_BLENGTH) + ((y + SECTOR_BLENGTH - 4) / SECTOR_BLENGTH) * 3]
									[(x + SECTOR_BLENGTH) % SECTOR_BLENGTH	+ ((y + SECTOR_BLENGTH - 4) % SECTOR_BLENGTH) * SECTOR_BLENGTH] >= 1)) ||
							(int32((int32)oZ + (int32)z - 
								(int32)heightMaps[((x + SECTOR_BLENGTH) / SECTOR_BLENGTH) + ((y + SECTOR_BLENGTH + 4) / SECTOR_BLENGTH) * 3]
									[(x + SECTOR_BLENGTH) % SECTOR_BLENGTH	+ ((y + SECTOR_BLENGTH + 4) % SECTOR_BLENGTH) * SECTOR_BLENGTH] >= 1)))
						{
							minValue -= (slopeMap_[x + y * SECTOR_BLENGTH] - 0.6f) * 0.5f;
							maxValue += (slopeMap_[x + y * SECTOR_BLENGTH] - 0.6f) * 0.5f;
						}
					}
					
					// cavern check
					if (COMBINED3DNOISE1 > minValue)
					{
						sCubeBuf = 0;	
						// STODO: add cube data for heated blocks, make all blocks within two spaces from lava heated(glowing) so miners know when lava is near.
							//having heated blocks two spaces away, not one, will tempt miners to mine heated blocks
							//for further temptation, have heated blocks be more likely to contain valuable resources. (use cave noise below magma table) 
						// STODO: also add wet blocks for aquifers, some blocks should transmit wetness, some should be immune and act as barriers
							
						// STODO: have globular areas where both cave systems combine outside of water and magma tables
					
						if (oZ + z < upperLavaLevel_)
							SetSCubeTerrainType (&sCubeBuf, TERRA_LAVA);
						else
							SetSCubeTerrainType (&sCubeBuf, TERRA_OLDAIR); // represents caverns
							
					}
					else if (COMBINED3DNOISE2 < maxValue)
					{
						sCubeBuf = 0;

						if (oZ + z < seaLevel_ && oZ + z > upperLavaLevel_ + 20)
							SetSCubeTerrainType (&sCubeBuf, TERRA_WATER); // represents aquifers
						else if (oZ + z < lowerLavaLevel_)
							SetSCubeTerrainType (&sCubeBuf, TERRA_LAVA);
						else
							SetSCubeTerrainType (&sCubeBuf, TERRA_OLDAIR); // represents caverns
					}
					
				}
				CENTER = sCubeBuf;
			}
		}
	}
}

void
WorldGenerator::GSSetTerrainTypesPass_ (uint32 oX, uint32 oY, uint32 oZ, uint32* sectorData)
{
	uint32 sCubeBuf;

	// third 3D pass: define most terrain types, resources.
	for (uint32 z = 0; z < SECTOR_BLENGTH; ++z)
	{
		for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
		{
			for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
			{		
				sCubeBuf = CENTER;

				if (GetSCubeTerrainType (sCubeBuf) == TERRA_STONE)
				{
					sCubeBuf = 0;
					// soil and rock type separator
					if (COMBINED3DNOISE1 + COMBINED3DNOISE2 + NARROW2DNOISE * 0.2f + QUARTERMILE2DNOISE * 0.3f - DEPTHFACTOR * 5.0f + (float32(oZ + z) / float32(depthMod_ / 3)) + slopeMap_[y * SECTOR_BLENGTH + x] < 2.80f) 
					{
						// determine soil type
						float32 sandFactor;
								
						if (oZ + z < seaLevel_ + 2)
						{
							sandFactor = 2.0f;
						}
						else
							sandFactor = 0.0f;

						if (ARIDTY < 0.0f)
							sandFactor -= ARIDTY * 3.0f;
						if (ARIDTY < -0.5f)
							sandFactor += 1.0f;
						// allocate clay
						if ((float32(oZ + z) / float32(depthMod_ / 3)) + BROAD3DNOISE + MEDIUM3DNOISE - DEPTHFACTOR * 4.0f + NARROW2DNOISE + QUARTERMILE2DNOISE * 1.5f > 3.35f)
						{
							SetSCubeTerrainType (&sCubeBuf, TERRA_CLAY);
						}
						// allocate sand
						else if (HEIGHTFACTOR + DEPTHFACTOR - BROAD3DNOISE + BROAD2DNOISE + ARIDTY * 2.0f - sandFactor + QUARTERMILE2DNOISE < 0.3f)
						{
							if (sandFactor > 3.0f)
								SetSCubeTerrainType (&sCubeBuf, TERRA_SAND);
							else if (NARROW2DNOISE + COMBINED3DNOISE1 >= 0.6f)
								SetSCubeTerrainType (&sCubeBuf, TERRA_GRAVEL);
							else if (QUARTERMILE2DNOISE + COMBINED3DNOISE1 <= -0.6f)
								SetSCubeTerrainType (&sCubeBuf, TERRA_CLAY);
							else
								SetSCubeTerrainType (&sCubeBuf, TERRA_SAND);
						}
						// allocate extraneous gravel, clay, and sand
						else if (BROAD2DNOISE + QUARTERMILE2DNOISE - NARROW2DNOISE > 0.9f)
						{
							if (BROAD3DNOISE < -0.3f)
								SetSCubeTerrainType (&sCubeBuf, TERRA_CLAY);
							else if (MEDIUM3DNOISE + BROAD3DNOISE > 0.3f)
								SetSCubeTerrainType (&sCubeBuf, TERRA_GRAVEL);
							else
								SetSCubeTerrainType (&sCubeBuf, TERRA_SAND);
						}
						else
						{
							if (oZ + z < seaLevel_ - 1)
								SetSCubeTerrainType (&sCubeBuf, TERRA_GRAVEL);
							else
								SetSCubeTerrainType (&sCubeBuf, TERRA_DIRT);
						}
					}
					else
					{
						
						// Set rock types
						if ((COMBINED3DNOISE1 + COMBINED3DNOISE2) * 1.4f + NARROW2DNOISE * 0.2f + 
								QUARTERMILE2DNOISE * 0.8f - DEPTHFACTOR * 4.5f + (float32(oZ + z) / float32(depthMod_ / 3)) + 
								(slopeMap_[y * SECTOR_BLENGTH + x] * 1.2f) < 2.93f ||
							COMBINED3DNOISE1 > 0.5f)
						{
							SetSCubeTerrainType (&sCubeBuf, TERRA_GRAVEL);
						}
						else
						{
							// STODO: optimize this as needed

							// determine resource types

							// these declarations are ordered by rarity, with the least rare being declared first
							float32 coalFactor = QUARTERMILE2DNOISE
								+ VERYNARROW3DNOISE * 3 + NARROW3DNOISE * 2 + MEDIUM3DNOISE + BROAD3DNOISE
								- (float32(Abs(HEIGHTOFFSET + 35)) * 0.010f); // makes coal most commonly found 35 blocks below the surface

							float32 copperFactor = BROAD2DNOISE - QUARTERMILE2DNOISE
								 - VERYNARROW3DNOISE * 5 - NARROW3DNOISE * 2 - MEDIUM3DNOISE - BROAD3DNOISE;

							float32 ironFactor = QUARTERMILE2DNOISE * 2
								 + COMBINED3DNOISE1 - VERYNARROW3DNOISE * 4 + NARROW3DNOISE - MEDIUM3DNOISE - BROAD3DNOISE;

							float32 tinFactor = -BROAD2DNOISE - QUARTERMILE2DNOISE
								 - COMBINED3DNOISE2 + VERYNARROW3DNOISE * 4 + NARROW3DNOISE * 2 + MEDIUM3DNOISE + BROAD3DNOISE;

							float32 silverFactor = -NARROW2DNOISE - BROAD2DNOISE - QUARTERMILE2DNOISE - HALFMILE2DNOISE
								 + COMBINED3DNOISE1 + VERYNARROW3DNOISE * 5 - NARROW3DNOISE * 2 + MEDIUM3DNOISE + BROAD3DNOISE;

							float32 goldFactor = NARROW2DNOISE + BROAD2DNOISE - QUARTERMILE2DNOISE - HALFMILE2DNOISE - MILE2DNOISE
								 - COMBINED3DNOISE2 * 2 - VERYNARROW3DNOISE * 6 - NARROW3DNOISE * 4 - MEDIUM3DNOISE * 5 + BROAD3DNOISE * 2;

							if( VERYNARROW3DNOISE > -0.05f && VERYNARROW3DNOISE < 0.05f )
							{
								coalFactor += 1.0f;
								copperFactor += 1.0f;
								if( VERYNARROW3DNOISE > -0.0225f && VERYNARROW3DNOISE < 0.0225f )
								{
									ironFactor += 1.0f;
									tinFactor += 1.0f;
									if( VERYNARROW3DNOISE > -0.01f && VERYNARROW3DNOISE < 0.01f )
									{
										silverFactor += 1.0f;
										goldFactor += 1.0f;
									}
								}

								if( NARROW3DNOISE > -0.040f && NARROW3DNOISE < 0.040f )
								{
									coalFactor += 1.0f;
									copperFactor += 1.0f;
									if( NARROW3DNOISE > -0.018f && NARROW3DNOISE < 0.018f )
									{
										ironFactor += 1.0f;
										tinFactor += 1.0f;
										if( NARROW3DNOISE > -0.008f && NARROW3DNOISE < 0.008f )
										{
											silverFactor += 1.0f;
											goldFactor += 1.0f;
										}
									}

									if( MEDIUM3DNOISE > -0.030f && MEDIUM3DNOISE < 0.030f )
									{
										coalFactor += 1.0f;
										copperFactor += 1.0f;
										if( MEDIUM3DNOISE > -0.012f && MEDIUM3DNOISE < 0.012f )
										{
											ironFactor += 1.0f;
											tinFactor += 1.0f;
											if( MEDIUM3DNOISE > -0.005f && MEDIUM3DNOISE < 0.005f )
											{
												silverFactor += 1.0f;
												goldFactor += 1.0f;
											}
										}
									}
								}
							}

							// STODO: ensure appropriate rarity by counting each resource block on a larger map
							// these checks are ordered by rarity with the least rare occuring first
							
							if( coalFactor > 2.80f )
								SetSCubeTerrainType (&sCubeBuf, TERRA_COALORE);
							else if( copperFactor > 3.35f )
								SetSCubeTerrainType (&sCubeBuf, TERRA_COPPERORE);
							else if( ironFactor > 3.64f )
								SetSCubeTerrainType (&sCubeBuf, TERRA_IRONORE);
							else if( tinFactor > 2.90f )
								SetSCubeTerrainType (&sCubeBuf, TERRA_TINORE);
							else if( silverFactor > 4.20f )
								SetSCubeTerrainType (&sCubeBuf, TERRA_SILVERORE);
							else if( goldFactor > 8.90f )
								SetSCubeTerrainType (&sCubeBuf, TERRA_GOLDORE);
							else
								SetSCubeTerrainType (&sCubeBuf, TERRA_STONE);
							

							/*
							// notes on noise checks
							 // good for uniformly scattered resource allocation
							if( VERYNARROW3DNOISE > 0.51f )
								SetSCubeTerrainType (&sCubeBuf, TERRA_AIR); 
							else
								SetSCubeTerrainType (&sCubeBuf, TERRA_STONE); 
								*/

							/* good for semi rare scattered resource allocation and isolated larger veins
							if( NARROW3DNOISE * VERYNARROW3DNOISE > 0.25f )
								SetSCubeTerrainType (&sCubeBuf, TERRA_AIR); 
							else
								SetSCubeTerrainType (&sCubeBuf, TERRA_STONE); 
								*/

							/*
							// good for mix of scattered blocks and larger veins
							if( NARROW3DNOISE + VERYNARROW3DNOISE > 0.80f )
								SetSCubeTerrainType (&sCubeBuf, TERRA_AIR); 
							else
								SetSCubeTerrainType (&sCubeBuf, TERRA_STONE);
								*/
						}						
					}
				}
				CENTER = sCubeBuf;
			}
		}
	}
}

void 
WorldGenerator::FillHeightMaps_ (uint32 oX, uint32 oY, uint32 oZ, uint16* heightMaps[])
{
	// move heightmaps to the left to reduce calls to generate height map.
	if (oX > 0)
	{
		if (oY > 0)
		{
			for (uint32 i = 0; i < SECTOR_BAREA; ++i)
			{
				heightMaps[0][i] = heightMaps[1][i];
				heightMaps[1][i] = heightMaps[2][i];
			}
		}
		for (uint32 i = 0; i < SECTOR_BAREA; ++i)
		{
			heightMaps[3][i] = heightMaps[4][i];
			heightMaps[4][i] = heightMaps[5][i];
		}
		if (oY < size_ - SECTOR_BLENGTH)
		{
			for (uint32 i = 0; i < SECTOR_BAREA; ++i)
			{
				heightMaps[6][i] = heightMaps[7][i];
				heightMaps[7][i] = heightMaps[8][i];
			}
		}
	}

	for (int32 oYMod = 0 - int32(SECTOR_BLENGTH); oYMod <= SECTOR_BLENGTH; oYMod += SECTOR_BLENGTH)
	{
		for (int32 oXMod = 0 - int32(SECTOR_BLENGTH); oXMod <= SECTOR_BLENGTH; oXMod += SECTOR_BLENGTH)
		{
			// the following checks will determine if one or more heightmaps are out of world bounds and will set all values 
				// in said heightmap(s) to the corresponding edge of the center height map.
			// corner cases of world bounds
			if (oXMod < 0 && oYMod < 0 && (oX == 0 || oY == 0)) // front, left
			{
				for (uint32 i = 0; i < SECTOR_BAREA; ++i)
					heightMaps[0][i] = heightMap_[0];
			}
			else if (oXMod > 0  && oYMod < 0 && (oX == size_ - SECTOR_BLENGTH || oY == 0)) // front, right
			{
				for (uint32 i = 0; i < SECTOR_BAREA; ++i)
					heightMaps[2][i] = heightMap_[SECTOR_BLENGTH - 1];
			}
			else if (oXMod < 0 && oYMod > 0 && (oX == 0 || oY == size_ - SECTOR_BLENGTH)) // back, left
			{
				for (uint32 i = 0; i < SECTOR_BAREA; ++i)
					heightMaps[6][i] = heightMap_[SECTOR_BAREA - SECTOR_BLENGTH];
			}
			else if (oXMod > 0 && oYMod > 0 && (oX == size_ - SECTOR_BLENGTH || oY == size_ - SECTOR_BLENGTH)) // back, right
			{
				for (uint32 i = 0; i < SECTOR_BAREA; ++i)
					heightMaps[8][i] = heightMap_[SECTOR_BAREA - 1];
			}

			// side cases of world bounds
			else if (oXMod < 0 && oX == 0) // left
			{
				for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
				{
					for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
					{
						heightMaps[3][x + y * SECTOR_BLENGTH] = heightMap_[y * SECTOR_BLENGTH];
					}
				}
			}
			else if (oXMod > 0 && oX == size_ - SECTOR_BLENGTH) // right
			{
				for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
				{
					for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
					{
						heightMaps[5][x + y * SECTOR_BLENGTH] = heightMap_[SECTOR_BLENGTH - 1 + y * SECTOR_BLENGTH];
					}
				}
			}
			else if (oYMod < 0 && oY == 0) // front
			{
				for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
				{
					for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
					{
						heightMaps[1][x + y * SECTOR_BLENGTH] = heightMap_[x];
					}
				}
			}
			else if (oYMod > 0 && oY == size_ - SECTOR_BLENGTH) // back
			{
				for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
				{
					for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
					{
						heightMaps[7][x + y * SECTOR_BLENGTH] = heightMap_[x + (SECTOR_BLENGTH - 1) * SECTOR_BLENGTH];
					}
				}
			}
			// fill height maps.
			else if (oX == 0 || oXMod > 0)
			{
				PrepTerraNoise (uint32(int32(oX) + oXMod), uint32(int32(oY) + oYMod), oZ, SECTOR_BLENGTH, SECTOR_BLENGTH, false);
				GenerateHeightMap(uint32(int32(oX) + oXMod), uint32(int32(oY) + oYMod), heightMaps[(oXMod + SECTOR_BLENGTH + (oYMod + SECTOR_BLENGTH) * 3) / SECTOR_BLENGTH], SECTOR_BLENGTH, SECTOR_BLENGTH);
			}
		}
	}
}

#define SNOWLEVEL	(uint16((int16)depthMod_ - (int16(depthMod_ / 6) + int16(float32(depthMod_ / 20) * (NARROW2DNOISE + BROAD2DNOISE + QUARTERMILE2DNOISE)))))

void 
WorldGenerator::ReviseSectors_ (uint32 oX, uint32 oY)
{
	uint32 oZ;
	uint32 sCubeBuf;
	int32 snowDepth;
	uint16 height;
	S_Sector** sectors;
	uint32** sectorDatas;

	sectors = (S_Sector**)malloc ((depth_ / SECTOR_BLENGTH) * sizeof (S_Sector*));
	sectorDatas = (uint32**)malloc ((depth_ / SECTOR_BLENGTH) * sizeof (uint32*));

	for (oZ = 0; oZ < depth_; oZ += SECTOR_BLENGTH)
	{
		sectors[oZ / SECTOR_BLENGTH] = GetWorld()->GetSector (oX, oY, oZ);
		sectorDatas[oZ / SECTOR_BLENGTH] = sectors[oZ / SECTOR_BLENGTH]->BeginModify ();
	}

	oZ = depthMod_ / 2;

	for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
	{
		for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
		{
			height = heightMap_[x + y * SECTOR_BLENGTH];

			//navigate to appropriate sector
			while ((int32)height - (int32)oZ < 0)
			{
				oZ -= SECTOR_BLENGTH;
			}
			while ((int32)height - (int32)oZ >= SECTOR_BLENGTH)
			{
				oZ += SECTOR_BLENGTH;
			}

			uint32 z = (uint32)height - oZ;

			sCubeBuf = sectorDatas[oZ / SECTOR_BLENGTH][x + y * SECTOR_BLENGTH + z * SECTOR_BAREA];

			// terrain surface check(s)
			// grass check:
			if (GetSCubeTerrainType (sCubeBuf) == TERRA_DIRT && slopeMap_[y * SECTOR_BLENGTH + x] < 0.6f && height >= seaLevel_ &&
				oZ + z < (depthMod_ - (depthMod_ / 8) - uint16(float32(depthMod_ / 16) * (NARROW2DNOISE + 1.0f))))
			{
				sCubeBuf = 0;//ClearSCubeTerrainType (&sCubeBuf);
				SetSCubeTerrainType (&sCubeBuf, TERRA_GRASS);

				// STODO: make trees sector independent
				if (slopeMap_[y * SECTOR_BLENGTH + x] < 0.6f && ARIDTY > -0.2f && 
					(rand () % 1000 + int32(((ARIDTY + 0.2f) * (ARIDTY + 0.22f) * (ARIDTY + 0.24f) * 60.0f))) >= 1000 && 
					z < SECTOR_BLENGTH - 15 && x >= 6 && y >= 6 && x < SECTOR_BLENGTH - 6 && y < SECTOR_BLENGTH - 6)
				TreeGenerator_ (x + y * SECTOR_BLENGTH + z * SECTOR_BAREA, height, ARIDTY, sectorDatas[oZ / SECTOR_BLENGTH]);
			}

			sectorDatas[oZ / SECTOR_BLENGTH][x + y * SECTOR_BLENGTH + z * SECTOR_BAREA] = sCubeBuf;

			// above terrain surface check(s)
			// snow check: eventually make this mainly dependent on latitude, elevation, slope
			// STODO: deal with trees by not placing snow on tree blocks.
			
			// snow height (above sea level) midpoint: 102 range: 77 - 128
			if (height > SNOWLEVEL)
			{
				// aridty acts as a multiplier to snowdepth. increasing slope reduces snow depth additively.
				//rough range: ~0 - 3.75
				snowDepth = int32 ((((ARIDTY + 1.0f) * 0.5f) * (1.0f + ARIDTY + NARROW2DNOISE + BROAD2DNOISE + QUARTERMILE2DNOISE - slopeMap_[y * SECTOR_BLENGTH + x] * 1.3f) + 2) *
					(float32(height - SNOWLEVEL) / float32(depthMod_ / 5) + 0.5f));
				//snowDepth = 1;
				//add snow depth based on height/noise as well.
				//if (height_ > depthMod_ - (depthMod_ / 4))
					//++snowDepth;

				while (snowDepth > 0)
				{
					++height;

					if (height >= depth_ - 1)
						break;

					//navigate to appropriate sector
					while ((int32)height - (int32)oZ < 0)
					{
						oZ -= SECTOR_BLENGTH;
					}
					while ((int32)height - (int32)oZ >= SECTOR_BLENGTH)
					{
						oZ += SECTOR_BLENGTH;
					}

					z = (height - oZ);

					if (GetSCubeTerrainType (sectorDatas[oZ / SECTOR_BLENGTH][x + y * SECTOR_BLENGTH + z * SECTOR_BAREA]) == TERRA_AIR ||
						GetSCubeTerrainType (sectorDatas[oZ / SECTOR_BLENGTH][x + y * SECTOR_BLENGTH + z * SECTOR_BAREA]) == TERRA_OLDAIR)
					{
						sCubeBuf = 0;
						SetSCubeTerrainType (&sCubeBuf, TERRA_SNOW);
						sectorDatas[oZ / SECTOR_BLENGTH][x + y * SECTOR_BLENGTH + z * SECTOR_BAREA] = sCubeBuf;
					}
					--snowDepth;
				}
			}
		}
	}

	for (oZ = 0; oZ < depth_; oZ += SECTOR_BLENGTH)
	{
		sectors[oZ / SECTOR_BLENGTH]->EndModify ();
	}

	free (sectors);
	free (sectorDatas);
}

void 
WorldGenerator::ReviseHeightMap_ (uint32 oX, uint32 oY)
{
	uint32 oZ;
	S_Sector** sectors;
	uint32** sectorDatas;

	sectors = (S_Sector**)malloc ((depth_ / SECTOR_BLENGTH) * sizeof (S_Sector*));
	sectorDatas = (uint32**)malloc ((depth_ / SECTOR_BLENGTH) * sizeof (uint32*));

	for (oZ = 0; oZ < depth_; oZ += SECTOR_BLENGTH)
	{
		sectors[oZ / SECTOR_BLENGTH] = GetWorld()->GetSector (oX, oY, oZ);
		sectorDatas[oZ / SECTOR_BLENGTH] = sectors[oZ / SECTOR_BLENGTH]->BeginModify ();
	}

	oZ = depthMod_ / 2;

	for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
	{
		for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
		{
			uint32 height = (uint32)heightMap_[x + y * SECTOR_BLENGTH];

			// navigate to appropriate sector
			while ((int32)height - (int32)oZ < 0)
			{
				oZ -= SECTOR_BLENGTH;
			}
			while ((int32)height - (int32)oZ >= SECTOR_BLENGTH)
			{
				oZ += SECTOR_BLENGTH;
			}

			// check if location in sector is an air/water/lava block, if it is keep checking downward till air is no longer found
			while (GetSCubeTerrainType (sectorDatas[oZ / SECTOR_BLENGTH][x + y * SECTOR_BLENGTH + (height - oZ) * SECTOR_BAREA]) == TERRA_AIR ||
				GetSCubeTerrainType (sectorDatas[oZ / SECTOR_BLENGTH][x + y * SECTOR_BLENGTH + (height - oZ) * SECTOR_BAREA]) == TERRA_OLDAIR ||
				GetSCubeTerrainType (sectorDatas[oZ / SECTOR_BLENGTH][x + y * SECTOR_BLENGTH + (height - oZ) * SECTOR_BAREA]) == TERRA_WATER ||
				GetSCubeTerrainType (sectorDatas[oZ / SECTOR_BLENGTH][x + y * SECTOR_BLENGTH + (height - oZ) * SECTOR_BAREA]) == TERRA_LAVA) // if loop isnt entered, value for height doesnt change.
			{
				//navigate to appropriate sector
				--height;
				while ((int32)height - (int32)oZ < 0)
				{
					oZ -= SECTOR_BLENGTH;
				}
				while ((int32)height - (int32)oZ >= SECTOR_BLENGTH)
				{
					oZ += SECTOR_BLENGTH;
				}	
			}
			heightMap_[x + y * SECTOR_BLENGTH] = (uint16)height; // STODO: sample adjacent sectors... or just dont bother remaking slope map. test how terrain looks if slopemap is not remade.
		}
	}
	for (oZ = 0; oZ < depth_; oZ += SECTOR_BLENGTH)
	{
		sectors[oZ / SECTOR_BLENGTH]->EndModify ();
	}

	free (sectors);
	free (sectorDatas);
}

#define SMOOTHSECTORS_ACCESS	((oXMod + SECTOR_BLENGTH + (oYMod + SECTOR_BLENGTH) * 3 + (oZMod + SECTOR_BLENGTH) * 9) / SECTOR_BLENGTH)

void 
WorldGenerator::SmoothSectorsAndFindOldAir_ ()
{
	uint32* sectorDatas[27];
	S_Sector* sectors[27];

	for (uint32 i = 0; i < 27; ++i)
		sectors[i] = NULL;

	// make a sector filled with air.
	uint32* airSector = (uint32*)Calloc (SECTOR_BVOLUME, sizeof (uint32));

	for (uint32 oZ = 0; oZ < depth_; oZ += SECTOR_BLENGTH)
	{
		for (uint32 oY = 0; oY < size_; oY += SECTOR_BLENGTH)
		{
			for (uint32 oX = 0; oX < size_; oX += SECTOR_BLENGTH)
			{
				// Have all sectorDatas point to a sector filled with air by default. 
				// This is done so the terrain smoother algoritm does not have to worry about world extents.
				for (uint32 i = 0; i < 27; ++i)
					sectorDatas[i] = airSector;

				// Call begin modify on every sector in 3x3x3 cube based on conditions.
				for (int32 oZMod = 0 - int32(SECTOR_BLENGTH); oZMod <= SECTOR_BLENGTH; oZMod += SECTOR_BLENGTH)
				{
					for (int32 oYMod = 0 - int32(SECTOR_BLENGTH); oYMod <= SECTOR_BLENGTH; oYMod += SECTOR_BLENGTH)
					{
						for (int32 oXMod = 0 - int32(SECTOR_BLENGTH); oXMod <= SECTOR_BLENGTH; oXMod += SECTOR_BLENGTH)
						{
							if ((oXMod < 0 && oX == 0) || (oYMod < 0 && oY == 0) || (oZMod < 0 && oZ == 0) ||
								(oXMod > 0 && oX == size_ - SECTOR_BLENGTH) || (oYMod > 0 && oY == size_ - SECTOR_BLENGTH) || (oZMod > 0 && oZ == depth_ - SECTOR_BLENGTH))
							{
								continue;
							}

							sectors[(oXMod + SECTOR_BLENGTH + (oYMod + SECTOR_BLENGTH) * 3 + (oZMod + SECTOR_BLENGTH) * 9) / SECTOR_BLENGTH] = 
								GetWorld()->GetSector (uint32(int32(oX) + oXMod), uint32(int32(oY) + oYMod), uint32(int32(oZ) + oZMod));

							if (!sectors[(oXMod + SECTOR_BLENGTH + (oYMod + SECTOR_BLENGTH) * 3 + (oZMod + SECTOR_BLENGTH) * 9) / SECTOR_BLENGTH]->GetIsModifying ())
							{
								sectorDatas[(oXMod + SECTOR_BLENGTH + (oYMod + SECTOR_BLENGTH) * 3 + (oZMod + SECTOR_BLENGTH) * 9) / SECTOR_BLENGTH] = 
									sectors[(oXMod + SECTOR_BLENGTH + (oYMod + SECTOR_BLENGTH) * 3 + (oZMod + SECTOR_BLENGTH) * 9) / SECTOR_BLENGTH]->BeginModify (); 
							}
							else
							{
								sectorDatas[(oXMod + SECTOR_BLENGTH + (oYMod + SECTOR_BLENGTH) * 3 + (oZMod + SECTOR_BLENGTH) * 9) / SECTOR_BLENGTH] = 
									sectors[(oXMod + SECTOR_BLENGTH + (oYMod + SECTOR_BLENGTH) * 3 + (oZMod + SECTOR_BLENGTH) * 9) / SECTOR_BLENGTH]->GetData (); 
							}	
						}
					}
				}

				// Remove old air from sectors
				FindOldAir_ (oX, oY, oZ, sectorDatas);

				// Add scubes to surface of terrain.
				TerrainSmoother_ (sectorDatas);

				// Only call end modify on a max of 9 sectors at a time.
				for (uint32 i = 0; i < 9; ++i)
				{
					if (sectors[i * 3] != NULL && sectors[i * 3]->GetIsModifying ())
					{
						sectors[i * 3]->EndModify (); // in a 3x3x3 cube with x,z,y, x equals 0.
						sectors[i * 3] = NULL;
					}
				}
			}
			// Call end modify on all remaining sectors if the end of the map is reached along the x axis.
			for (uint32 i = 0; i < 27; ++i)
			{
				if (sectors[i] != NULL && sectors[i]->GetIsModifying ())
				{
					sectors[i]->EndModify ();
					sectors[i] = NULL;
				}
			}
		}
	}

	free (airSector);
}

void
WorldGenerator::TerrainSmoother_ (uint32** sectorDatas)
{
	TSAddCubesPass_ (sectorDatas);
	TSAddSCubesToCubesPass_ (sectorDatas);
	TSAddSCubesToSCubesPass_ (sectorDatas);
	TSPostPass_ (sectorDatas);
}

void
WorldGenerator::TSAddCubesPass_ (uint32** sectorDatas)
{
	uint32 sCubeBuf = 0;
	{
		bool continueFlag = false;

		// First pass (add cube cases)
		for (uint32 z = 0; z < SECTOR_BLENGTH; ++z)
		{
			for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
			{
				for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
				{
					if (sCubeBuf != 0) // triggered by continue cases
					{
						BOX_CENTER = sCubeBuf;
					}
					sCubeBuf = BOX_CENTER;

					if (GetSCubeTerrainType (sCubeBuf) == TERRA_AIR || GetSCubeTerrainType (sCubeBuf) == TERRA_OLDAIR)
					{
						// Cube cases (first pass only)
						if (IsBlockSmoothable_ (BOX_DOWN) && GetSCubeType (BOX_DOWN) == SCUBE_CUBE)
						{
							if (IsBlockSmoothable_ (BOX_LEFT) && IsBlockSmoothable_ (BOX_RIGHT) &&
								(!IsBlockSmoothableToOnlySelf_(BOX_DOWN) || (GetSCubeTerrainType(BOX_DOWN) == GetSCubeTerrainType(BOX_RIGHT) && GetSCubeTerrainType(BOX_LEFT) == GetSCubeTerrainType(BOX_RIGHT))))	
							{
								sCubeBuf = 0;
								SetSCubeType (&sCubeBuf, SCUBE_CUBE);
							
								switch (rand () % 2)
								{
								case 0:
									SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_LEFT));
									break;
								case 1:
									SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_RIGHT));
									break;
								}
								continue;
							}
							if (IsBlockSmoothable_ (BOX_BACK) && IsBlockSmoothable_ (BOX_FRONT) &&
								(!IsBlockSmoothableToOnlySelf_(BOX_DOWN) || (GetSCubeTerrainType(BOX_DOWN) == GetSCubeTerrainType(BOX_BACK) && GetSCubeTerrainType(BOX_BACK) == GetSCubeTerrainType(BOX_FRONT))))
							{
								sCubeBuf = 0;
								SetSCubeType (&sCubeBuf, SCUBE_CUBE);
								switch (rand () % 2)
								{
								case 0:
									SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_BACK));
									break;
								case 1:
									SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_FRONT));
									break;
								}
								continue;
							}

							for (uint32 i = 0; i < 4; ++i)
							{
								uint32 block1;
								uint32 block2;
								uint32 block3;

								switch (i)
								{
								case 0:
									block1 = BOX_LEFT_BACK;
									block2 = BOX_RIGHT_BACK;
									block3 = BOX_FRONT;
									break;
								case 1:
									block1 = BOX_LEFT_BACK;
									block2 = BOX_LEFT_FRONT;
									block3 = BOX_RIGHT;
									break;
								case 2:
									block1 = BOX_LEFT_FRONT;
									block2 = BOX_RIGHT_FRONT;
									block3 = BOX_BACK;
									break;
								case 3:
									block1 = BOX_RIGHT_FRONT;
									block2 = BOX_RIGHT_BACK;
									block3 = BOX_LEFT;
									break;
								}

								if (IsBlockSmoothable_ (block1) && IsBlockSmoothable_ (block2) && IsBlockSmoothable_ (block3) &&
									(!IsBlockSmoothableToOnlySelf_(BOX_DOWN) || (GetSCubeTerrainType(BOX_DOWN) == GetSCubeTerrainType(block1) && 
									GetSCubeTerrainType(block1) == GetSCubeTerrainType(block2) && GetSCubeTerrainType(block2) == GetSCubeTerrainType(block3))))
								{
									sCubeBuf = 0;
									SetSCubeType (&sCubeBuf, SCUBE_CUBE);
									switch (rand () % 3)
									{
									case 0:
										SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (block1));
										break;
									case 1:
										SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (block2));
										break;
									case 2:
										SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (block3));
										break;
									}
									continueFlag = true;
									break;
								}
							}
							if (continueFlag)
							{
								continueFlag = false;
								continue;
							}
								
							if (IsBlockSmoothable_ (BOX_LEFT_FRONT) && IsBlockSmoothable_ (BOX_LEFT_BACK) && 
								IsBlockSmoothable_ (BOX_RIGHT_FRONT) && IsBlockSmoothable_ (BOX_RIGHT_BACK) &&
									(!IsBlockSmoothableToOnlySelf_(BOX_DOWN) || (GetSCubeTerrainType(BOX_DOWN) == GetSCubeTerrainType(BOX_LEFT_FRONT) && 
									GetSCubeTerrainType(BOX_LEFT_FRONT) == GetSCubeTerrainType(BOX_LEFT_BACK) && GetSCubeTerrainType(BOX_LEFT_BACK) == GetSCubeTerrainType(BOX_RIGHT_FRONT) &&
									GetSCubeTerrainType(BOX_RIGHT_FRONT) == GetSCubeTerrainType(BOX_RIGHT_BACK))))
							{
								sCubeBuf = 0;
								SetSCubeType (&sCubeBuf, SCUBE_CUBE);
								switch (rand () % 4)
								{
								case 0:
									SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_LEFT_FRONT));
									break;
								case 1:
									SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_LEFT_BACK));
									break;
								case 2:
									SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_RIGHT_FRONT));
									break;
								case 3:
									SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_RIGHT_BACK));
									break;
								}
							}
						}
						BOX_CENTER = sCubeBuf;
					}
					sCubeBuf = 0;
				}
				if (sCubeBuf != 0) // triggered by continue cases
				{
					uint32 x = SECTOR_BLENGTH - 1;
					BOX_CENTER = sCubeBuf;
				}
			}
		}
	}
}

void
WorldGenerator::TSAddSCubesToCubesPass_ (uint32** sectorDatas)
{
	uint32 sCubeBuf = 0;

	for (uint32 z = 0; z < SECTOR_BLENGTH; ++z)
	{
		for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
		{
			for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
			{
				sCubeBuf = BOX_CENTER;

				if (GetSCubeTerrainType (sCubeBuf) == TERRA_AIR || GetSCubeTerrainType (sCubeBuf) == TERRA_OLDAIR)
				{
					// Note- these variables: down, left, right, front, back are added to reduce supersampling
					if (/*IsBlockSmoothable_ (BOX_DOWN) && */GetSCubeTerrainType (BOX_DOWN) != TERRA_AIR && GetSCubeType (BOX_DOWN) == SCUBE_CUBE)
					{
						// only cube adding case for this pass.
						if (IsBlockSmoothable_ (BOX_LEFT) && GetSCubeType (BOX_LEFT) == SCUBE_CUBE && 
							IsBlockSmoothable_ (BOX_RIGHT) && GetSCubeType (BOX_RIGHT) == SCUBE_CUBE && 
							IsBlockSmoothable_ (BOX_FRONT) && GetSCubeType (BOX_FRONT) == SCUBE_CUBE && 
							IsBlockSmoothable_ (BOX_BACK) && GetSCubeType (BOX_BACK) == SCUBE_CUBE &&
									(!IsBlockSmoothableToOnlySelf_(BOX_DOWN) || (GetSCubeTerrainType(BOX_DOWN) == GetSCubeTerrainType(BOX_LEFT) && 
									GetSCubeTerrainType(BOX_LEFT) == GetSCubeTerrainType(BOX_RIGHT) && GetSCubeTerrainType(BOX_RIGHT) == GetSCubeTerrainType(BOX_FRONT) &&
									GetSCubeTerrainType(BOX_FRONT) == GetSCubeTerrainType(BOX_BACK))))	
						{
							sCubeBuf = 0;
							SetSCubeType (&sCubeBuf, SCUBE_CUBE);
							
							switch (rand () % 2)
							{
							case 0:
								SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_LEFT));
								break;
							case 1:
								SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_RIGHT));
								break;
							case 2:
								SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_FRONT));
								break;
							case 3:
								SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_BACK));
								break;
							}
							continue;
						}

						for (uint32 i = 0; i < 16; ++i)
						{
							uint32 block1;
							uint32 block2;
							uint32 block3;
							uint32 block4;
							uint32 block5;
							uint32 block6;

							switch (i)
							{
								// THREEQUARTERCUBE cases
							case 0:
								block1 = BOX_BACK;
								block2 = BOX_RIGHT_FRONT;
								block3 = BOX_FRONT_DOWN; // block 3-4 in these cases determines block type
								block4 = BOX_LEFT_DOWN;
								block5 = BOX_FRONT;
								block6 = BOX_LEFT;
								break;
							case 1:
								block1 = BOX_RIGHT;
								block2 = BOX_LEFT_FRONT;
								block3 = BOX_LEFT_DOWN;
								block4 = BOX_BACK_DOWN;
								block5 = BOX_LEFT;
								block6 = BOX_BACK;
								break;
							case 2:
								block1 = BOX_FRONT;
								block2 = BOX_LEFT_BACK;
								block3 = BOX_BACK_DOWN;
								block4 = BOX_RIGHT_DOWN;
								block5 = BOX_BACK;
								block6 = BOX_RIGHT;
								break;
							case 3:
								block1 = BOX_LEFT;
								block2 = BOX_RIGHT_BACK;
								block3 = BOX_RIGHT_DOWN;
								block4 = BOX_FRONT_DOWN;
								block5 = BOX_RIGHT;
								block6 = BOX_FRONT;
								break;
							case 4:
								block1 = BOX_BACK;
								block2 = BOX_RIGHT;
								block3 = BOX_FRONT_DOWN;
								block4 = BOX_LEFT_DOWN;
								block5 = BOX_FRONT;
								block6 = BOX_LEFT;
								break;
							case 5: 
								block1 = BOX_FRONT;
								block2 = BOX_RIGHT;
								block3 = BOX_LEFT_DOWN;
								block4 = BOX_BACK_DOWN;
								block5 = BOX_LEFT;
								block6 = BOX_BACK;
								break;
							case 6:
								block1 = BOX_FRONT;
								block2 = BOX_LEFT;
								block3 = BOX_BACK_DOWN;
								block4 = BOX_RIGHT_DOWN;
								block5 = BOX_BACK;
								block6 = BOX_RIGHT;
								break;
							case 7:
								block1 = BOX_BACK;
								block2 = BOX_LEFT;
								block3 = BOX_RIGHT_DOWN;
								block4 = BOX_FRONT_DOWN;
								block5 = BOX_RIGHT;
								block6 = BOX_FRONT;
								break;

								// HALFCUBETRIANGLE cases
							case 8:
								block1 = BOX_BACK;
								break;
							case 9:
								block1 = BOX_RIGHT;
								break;
							case 10:
								block1 = BOX_FRONT;
								break;
							case 11:
								block1 = BOX_LEFT;
								break;

								// QUARTERCUBETRIANGLE and QUARTERCUBECORNERPYRAMID cases
							case 12:
								block1 = BOX_LEFT_FRONT;
								block2 = BOX_RIGHT_BACK_DOWN;
								block3 = BOX_LEFT;
								block4 = BOX_FRONT;
								break;
							case 13:
								block1 = BOX_LEFT_BACK;
								block2 = BOX_RIGHT_FRONT_DOWN;
								block3 = BOX_LEFT;
								block4 = BOX_BACK;
								break;
							case 14:
								block1 = BOX_RIGHT_BACK;
								block2 = BOX_LEFT_FRONT_DOWN;
								block3 = BOX_RIGHT;
								block4 = BOX_BACK;
								break;
							case 15:
								block1 = BOX_RIGHT_FRONT;
								block2 = BOX_LEFT_BACK_DOWN;
								block3 = BOX_RIGHT;
								block4 = BOX_FRONT;
								break;
							}

							if (i < 8)
							{
								if (IsBlockSmoothable_ (block1) && GetSCubeType (block1) == SCUBE_CUBE &&
									IsBlockSmoothable_ (block2) && GetSCubeType (block2) == SCUBE_CUBE &&
										(!IsBlockSmoothableToOnlySelf_(BOX_DOWN) || (GetSCubeTerrainType(BOX_DOWN) == GetSCubeTerrainType(block1) && 
										GetSCubeTerrainType(block1) == GetSCubeTerrainType(block2))))	
								{
									sCubeBuf = 0;
									if (IsBlockSmoothable_ (BOX_UP) && GetSCubeType (BOX_UP) == SCUBE_CUBE)
									{
										SetSCubeType (&sCubeBuf, SCUBE_CUBE); // added to fix a vis issue when a cube is overhanging a threequartercube
									}
									else
									{
										SetSCubeType (&sCubeBuf, SCUBE_THREEQUARTERCUBE);
										SetSCubeSpin (&sCubeBuf, i % 4);
									}

									if( GetSCubeTerrainType(block1) == TERRA_SNOW && GetSCubeTerrainType(block2) == TERRA_SNOW )
										SetSCubeTerrainType (&sCubeBuf, TERRA_SNOW);
									else if (IsBlockSmoothableBuildingMat_ (block5))
										SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (block5)); // allows for better looking roofs
									else if (IsBlockSmoothableBuildingMat_ (block6))
										SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (block6)); // allows for better looking roofs
									else if (!IsBlockSmoothableToOnlySelf_(BOX_DOWN) && IsBlockSmoothableToOnlySelf_(block3))
										SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (block4)); // helps prevent road uneveness
									else if (!IsBlockSmoothableToOnlySelf_(BOX_DOWN) && IsBlockSmoothableToOnlySelf_(block4))
										SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (block3)); // helps prevent road uneveness
									else if (IsBlockSmoothable_ (block3) && rand () % 2 == 0)
										SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (block3)); // prevents saw tooth pattern in terrain
									else if (IsBlockSmoothable_ (block4))
										SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (block4)); // prevents saw tooth pattern in terrain
									else if (IsBlockSmoothable_ (block3))
										SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (block3)); // prevents saw tooth pattern in terrain
									else
										SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_DOWN));

									// elminate case where dirt is placed instead of grass
									if (GetSCubeTerrainType (sCubeBuf) == TERRA_DIRT &&
										(GetSCubeTerrainType (block2) == TERRA_GRASS || GetSCubeTerrainType (block2) == TERRA_GRASS ||
										GetSCubeTerrainType (block3) == TERRA_GRASS || GetSCubeTerrainType (block4) == TERRA_GRASS ||
										GetSCubeTerrainType (block5) == TERRA_GRASS || GetSCubeTerrainType (block6) == TERRA_GRASS))
									{
										ClearSCubeTerrainType (&sCubeBuf);
										SetSCubeTerrainType (&sCubeBuf, TERRA_GRASS);
									}
									break;
								}
							}
							else
							{	
								if (IsBlockSmoothable_ (block1) && GetSCubeType (block1) == SCUBE_CUBE &&
										(!IsBlockSmoothableToOnlySelf_(BOX_DOWN) || (GetSCubeTerrainType(BOX_DOWN) == GetSCubeTerrainType(block1))))
								{
									sCubeBuf = 0;
									if (i < 12)
									{
										SetSCubeType (&sCubeBuf, SCUBE_HALFCUBETRIANGLE);

										if (IsBlockSmoothableBuildingMat_ (block1) || GetSCubeTerrainType (block1) == TERRA_SNOW)
											SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (block1));
										else
											SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_DOWN));
													
										SetSCubeSpin (&sCubeBuf, i % 4);
										break;
									}
									else
									{
										if (GetSCubeTerrainType (block2) == TERRA_AIR || GetSCubeType (block2) == SCUBE_QUARTERCUBECORNERPYRAMID)
										{
											SetSCubeType (&sCubeBuf, SCUBE_QUARTERCUBECORNERPYRAMID);
													
											if (IsBlockSmoothableBuildingMat_ (block1)/* || GetSCubeTerrainType (block1) == TERRA_SNOW*/)
												SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (block1));
											else
												SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_DOWN));
													
											SetSCubeSpin (&sCubeBuf, i % 4);
										}
										else
										{
											SetSCubeType (&sCubeBuf, SCUBE_QUARTERCUBETRIANGLE);
											
											if( GetSCubeTerrainType(block3) == TERRA_SNOW || GetSCubeTerrainType(block4) == TERRA_SNOW )
												SetSCubeTerrainType (&sCubeBuf, TERRA_SNOW);
											else
												SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (BOX_DOWN));
													
											SetSCubeSpin (&sCubeBuf, (i + 3) % 4);
										}
										break;
									}				
								}
							}
						}
					}
					BOX_CENTER = sCubeBuf;
				}
			}
		}
	}
}

void
WorldGenerator::TSAddSCubesToSCubesPass_ (uint32** sectorDatas)
{
	uint32 sCubeBuf = 0;

	for (uint32 z = 0; z < SECTOR_BLENGTH; ++z)
	{
		for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
		{
			for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
			{
				sCubeBuf = BOX_CENTER;

				if (GetSCubeTerrainType (sCubeBuf) == TERRA_AIR || GetSCubeTerrainType (sCubeBuf) == TERRA_OLDAIR)
				{
					// quartercubetriangle cases
					uint32 block1 = BOX_DOWN;
					if (GetSCubeType (block1) == SCUBE_THREEQUARTERCUBE && IsBlockSmoothable_ (block1))
					{
						uint32 block2;
						uint32 block3;

						for (uint32 i = 0; i < 4; ++i)
						{			
							switch (i)
							{
							case 0:
								block2 = BOX_RIGHT;
								block3 = BOX_BACK;
								break;
							case 1:
								block2 = BOX_FRONT;
								block3 = BOX_RIGHT;
								break;
							case 2:
								block2 = BOX_LEFT;
								block3 = BOX_FRONT;
								break;
							case 3:
								block2 = BOX_BACK;
								block3 = BOX_LEFT;
								break;
							}
							if ((GetSCubeRotation (block1) == 0 && GetSCubeSpin (block1) == i) &&
									((IsBlockSmoothable_ (block2) && (GetSCubeType (block2) == SCUBE_HALFCUBETRIANGLE || GetSCubeType (block2) == SCUBE_THREEQUARTERCUBE) && GetSCubeSpin (block2) == i) ||
									(IsBlockSmoothable_ (block3) && (GetSCubeType (block3) == SCUBE_HALFCUBETRIANGLE || GetSCubeType (block3) == SCUBE_THREEQUARTERCUBE) && GetSCubeSpin (block3) == (i + 1) % 4) ||
									(IsBlockSmoothable_ (block2) && (GetSCubeType (block2) == SCUBE_CUBE)) || (IsBlockSmoothable_ (block3) && (GetSCubeType (block3) == SCUBE_CUBE))))
							{
								sCubeBuf = 0;
								SetSCubeType (&sCubeBuf, SCUBE_QUARTERCUBETRIANGLE);
								SetSCubeSpin (&sCubeBuf, (i + 1) % 4);
								if(GetSCubeTerrainType(block2) == TERRA_SNOW && GetSCubeTerrainType(block3) == TERRA_SNOW)
									SetSCubeTerrainType (&sCubeBuf, TERRA_SNOW);
								else
									SetSCubeTerrainType (&sCubeBuf, GetSCubeTerrainType (block1));
								break;
							}
						}
						BOX_CENTER = sCubeBuf;
					}
				}
			}
		}
	}
}

void
WorldGenerator::TSPostPass_ (uint32** sectorDatas)
{
	for (uint32 z = 0; z < SECTOR_BLENGTH; ++z)
	{
		for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
		{
			for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
			{
				uint32 sCubeBuf = BOX_CENTER;

				if (GetSCubeType (sCubeBuf) != SCUBE_CUBE)
				{
					if (GetSCubeType (sCubeBuf) == SCUBE_THREEQUARTERCUBE || GetSCubeType (sCubeBuf) == SCUBE_QUARTERCUBETRIANGLE && !IsBlockSmoothable_ (sCubeBuf)) // this check is used to fix cases where scubes were not added or typed properly
					{
						// this handles a case where roof scubes are set improperly to a wall type due to unavailable data during a pass due to the iteration being linear
						if (IsBlockSmoothableBuildingMat_ (BOX_LEFT))
						{
							ClearSCubeTerrainType (&BOX_CENTER);
							SetSCubeTerrainType (&BOX_CENTER, GetSCubeTerrainType (BOX_LEFT));
						}
						else if (IsBlockSmoothableBuildingMat_ (BOX_RIGHT))
						{
							ClearSCubeTerrainType (&BOX_CENTER);
							SetSCubeTerrainType (&BOX_CENTER, GetSCubeTerrainType (BOX_RIGHT));
						}
						else if (IsBlockSmoothableBuildingMat_ (BOX_FRONT))
						{
							ClearSCubeTerrainType (&BOX_CENTER);
							SetSCubeTerrainType (&BOX_CENTER, GetSCubeTerrainType (BOX_FRONT));
						}
						else if (IsBlockSmoothableBuildingMat_ (BOX_BACK))
						{
							ClearSCubeTerrainType (&BOX_CENTER);
							SetSCubeTerrainType (&BOX_CENTER, GetSCubeTerrainType (BOX_BACK));
						}

						if (GetSCubeType (sCubeBuf) == SCUBE_QUARTERCUBETRIANGLE)
						{
							ClearSCubeType (&BOX_CENTER);
							SetSCubeType (&BOX_CENTER, SCUBE_QUARTERCUBECORNERPYRAMID);
							uint32 oldSpin = GetSCubeSpin (BOX_CENTER);
							ClearSCubeSpin (&BOX_CENTER);
							SetSCubeSpin (&BOX_CENTER, (oldSpin + 1) % 4);
						}
					}

					// post pass to remove scubes that arent touching air or old air
					// this will improve the look of the terrain and reduce the number of sectors being drawn
					TERRATYPE type;

					// check for air or old air in adjacent blocks, continue if so
					type = GetSCubeTerrainType (BOX_LEFT);
					if (type == TERRA_AIR || type == TERRA_OLDAIR)
						continue;

					type = GetSCubeTerrainType (BOX_RIGHT);
					if (type == TERRA_AIR || type == TERRA_OLDAIR)
						continue;

					type = GetSCubeTerrainType (BOX_FRONT);
					if (type == TERRA_AIR || type == TERRA_OLDAIR)
						continue;

					type = GetSCubeTerrainType (BOX_BACK);
					if (type == TERRA_AIR || type == TERRA_OLDAIR)
						continue;

					type = GetSCubeTerrainType (BOX_DOWN);
					if (type == TERRA_AIR || type == TERRA_OLDAIR)
						continue;

					type = GetSCubeTerrainType (BOX_UP);
					if (type == TERRA_AIR || type == TERRA_OLDAIR)
						continue;

					// swap scube with a cube
					ClearSCubeType (&sCubeBuf);
					SetSCubeType (&sCubeBuf, SCUBE_CUBE);
					BOX_CENTER = sCubeBuf;
				}
			}
		}
	}
}

void 
WorldGenerator::TreeGenerator_ (uint32 loc, uint32 height, float32 aridty, uint32* sectorData)
{
	// STODO: use tree vectors!
	// STODO: make more interesting trees with this

	// STODO: tree growth should be able to spawn leaf growth blocks. these blocks
		//should store data that indicates the size and zOff offset of the leaf area generated

	// STODO: make leaves for trees by generating intersecting quads and painting a leaf texture on them.
	uint32 sCubeBuf = 0;
	uint32 treeHeight_;
	uint32 zOff;
	int32 xOff;
	int32 yOff;

	aridty = (aridty + 1.0f) / 2.0f; //range 0-1, 0 being dry 1 being wet

	treeHeight_ = 6 + uint32(float32(rand () % 5) * aridty);

	// generate trunk
	SetSCubeType (&sCubeBuf, SCUBE_CUBE);
	SetSCubeRotation (&sCubeBuf, 0);
	SetSCubeSpin (&sCubeBuf, 0);
	SetSCubeTerrainType (&sCubeBuf, TERRA_TREE); //change to TERRA_TREE when the texture is ready

	for (zOff = 1; zOff <= treeHeight_; ++zOff)
	{		
		//STODO: push direction node info into a data structure that a grow trees method(used when the game is running) can access.
		if (zOff == treeHeight_)
		{
			SetSCubeType (&sCubeBuf, SCUBE_QUARTERCUBEPYRAMID);
		}
		sectorData[loc + SECTOR_BAREA * zOff] = sCubeBuf;
	}

	// generate leaves
	sCubeBuf = 0;
	SetSCubeType (&sCubeBuf, SCUBE_CUBE);
	SetSCubeRotation (&sCubeBuf, 0);
	SetSCubeSpin (&sCubeBuf, 0);
	SetSCubeTerrainType (&sCubeBuf, TERRA_LEAVES);

	int32 minX = 0;
	int32 minY = 0;
	int32 maxX = 0;
	int32 maxY = 0;

	for (zOff = treeHeight_ / 2 + 1; zOff <= (treeHeight_ + treeHeight_ / 2 + 1); ++zOff)
	{
		if (zOff == treeHeight_ / 2 + 1)
		{
			minX -= 2;
			minY -= 2;
			maxX += 2;
			maxY += 2;
		}
		else if (zOff <= treeHeight_) // increasing leaf radius
		{
			minX -= rand () % 2;
			minY -= rand () % 2;
			maxX += rand () % 2;
			maxY += rand () % 2;
		}
		else if (zOff == treeHeight_ + 1) // increasing or decreasing
		{
			minX += (rand () % 3) - 1;
			minY += (rand () % 3) - 1;
			maxX += (rand () % 3) - 1;
			maxY += (rand () % 3) - 1;
		}
		else if (zOff != treeHeight_ + treeHeight_ / 2 + 1)// decreasing
		{
			minX += rand () % 2;
			minY += rand () % 2;
			maxX -= rand () % 2;
			maxY -= rand () % 2;
		}
		else
		{
			minX++;
			minY++;
			maxX--;
			maxY--;
		}
		for (yOff = minY; yOff <= maxY; ++yOff)
		{
			for (xOff = minX; xOff <= maxX; ++xOff)
			{
				if ((((xOff != 0 || yOff != 0) && zOff <= treeHeight_) || zOff > treeHeight_) && // makes sure leaves aren't placed in the tree trunk.
						((rand () % 2 == 0) || (xOff != minX && yOff != minY && xOff != maxX && yOff != maxY))) // this makes leaf borders appear more random
				{
					sectorData[loc + xOff + SECTOR_BLENGTH * yOff + SECTOR_BAREA * zOff] = sCubeBuf;
				}
			}
		}
	}
	// STODO: generate limbs
	// STODO: generate roots?
}

void
WorldGenerator::FindOldAir_ (uint32 oX, uint32 oY, uint32 oZ, uint32** sectorDatas)
{
	for (uint32 z = 0; z < SECTOR_BLENGTH; ++z)
	{
		for (uint32 y = 0; y < SECTOR_BLENGTH; ++y)
		{
			for (uint32 x = 0; x < SECTOR_BLENGTH; ++x)
			{
				if (GetSCubeTerrainType (BOX_CENTER) == TERRA_AIR)
				{
					if (GetSCubeTerrainType (BOX_LEFT) == TERRA_OLDAIR)
					{
						UPoint3D point;
						point.x = oX + x - 1;
						point.y = oY + y;
						point.z = oZ + z;
						ReplaceOldAir_ (point, int32(oX), int32(oY), int32(oZ));
					}
					if (GetSCubeTerrainType (BOX_RIGHT) == TERRA_OLDAIR)
					{
						UPoint3D point;
						point.x = oX + x + 1;
						point.y = oY + y;
						point.z = oZ + z;
						ReplaceOldAir_ (point, int32(oX), int32(oY), int32(oZ));
					}
					if (GetSCubeTerrainType (BOX_FRONT) == TERRA_OLDAIR)
					{
						UPoint3D point;
						point.x = oX + x;
						point.y = oY + y - 1;
						point.z = oZ + z;
						ReplaceOldAir_ (point, int32(oX), int32(oY), int32(oZ));
					}
					if (GetSCubeTerrainType (BOX_BACK) == TERRA_OLDAIR)
					{
						UPoint3D point;
						point.x = oX + x;
						point.y = oY + y + 1;
						point.z = oZ + z;
						ReplaceOldAir_ (point, int32(oX), int32(oY), int32(oZ));
					}
					if (GetSCubeTerrainType (BOX_DOWN) == TERRA_OLDAIR)
					{
						UPoint3D point;
						point.x = oX + x;
						point.y = oY + y;
						point.z = oZ + z - 1;
						ReplaceOldAir_ (point, int32(oX), int32(oY), int32(oZ));
					}
					if (GetSCubeTerrainType (BOX_UP) == TERRA_OLDAIR)
					{
						UPoint3D point;
						point.x = oX + x;
						point.y = oY + y;
						point.z = oZ + z + 1;
						ReplaceOldAir_ (point, int32(oX), int32(oY), int32(oZ));
					}
				}
			}
		}
	}
}

void
WorldGenerator::ReplaceOldAir_ (UPoint3D point, int32 centerOX, int32 centerOY, int32 centerOZ)
{
	assert (point.x < size_ && point.y < size_ && point.z < depth_);
	
	enum SECTORDATA_ACCESS 
	{
		SECTORDATA_LEFT = 0,
		SECTORDATA_CENTER,
		SECTORDATA_RIGHT,
		SECTORDATA_FRONT,
		SECTORDATA_BACK,
		SECTORDATA_DOWN,
		SECTORDATA_UP
	};

	// OPTIMIZATION: place points that will not leave the extra 6 sectors after one iteration into points, not other points.
	List<UPoint3D> points;
	List<UPoint3D> otherSectorPoints;
	// possible STODO: store otherSectorPoints in a vector so the exact sector its in can be identified. might not be needed considering contigious list elements will
		// likely be in the same sector.

	otherSectorPoints.InsertBack (point);

	bool firstPass = true;

	while (otherSectorPoints.Count () > 0)
	{
		point = otherSectorPoints.PopFront ();

		uint32 oX = (point.x / SECTOR_BLENGTH) * SECTOR_BLENGTH;
		uint32 oY = (point.y / SECTOR_BLENGTH) * SECTOR_BLENGTH;
		uint32 oZ = (point.z / SECTOR_BLENGTH) * SECTOR_BLENGTH;
		
		uint32* sectorDatas[7];
		if (!GetWorld()->GetSector (point.x, point.y, point.z)->GetIsModifying ())
			sectorDatas[SECTORDATA_CENTER] = GetWorld()->GetSector (point.x, point.y, point.z)->BeginModify ();
		else
			sectorDatas[SECTORDATA_CENTER] = GetWorld()->GetSector (point.x, point.y, point.z)->GetData ();
		sectorDatas[SECTORDATA_LEFT] = NULL;
		sectorDatas[SECTORDATA_RIGHT] = NULL;
		sectorDatas[SECTORDATA_FRONT] = NULL;
		sectorDatas[SECTORDATA_BACK] = NULL;
		sectorDatas[SECTORDATA_DOWN] = NULL;
		sectorDatas[SECTORDATA_UP] = NULL;

		Assert (!firstPass || GetSCubeTerrainType (sectorDatas[SECTORDATA_CENTER][point.x % SECTOR_BLENGTH + (point.y % SECTOR_BLENGTH) * SECTOR_BLENGTH + (point.z % SECTOR_BLENGTH) * SECTOR_BAREA]) == TERRA_OLDAIR);

		points.InsertBack (point);

		// this loop greatly speeds up the algorithm up by reducing the amount of sector packing/unpacking
		// STODO: speed the algorithm up further if a 3x3x3 box of sectors is used by checking if a point is in the box not just the center sector.
		for (uint32 i = 0; i < (uint32)otherSectorPoints.Count (); ++i)
		{
			UPoint3D tempPoint = otherSectorPoints.PopFront ();
			if (tempPoint.x / SECTOR_BLENGTH == point.x / SECTOR_BLENGTH && tempPoint.y / SECTOR_BLENGTH == point.y / SECTOR_BLENGTH && tempPoint.z / SECTOR_BLENGTH == point.z / SECTOR_BLENGTH)
				points.InsertBack (tempPoint);
			else
				otherSectorPoints.InsertBack (tempPoint);
		}

		while (points.Count () > 0)
		{
			// get next point in points list.
			point = points.PopFront ();

			// on the first pass, change the old air setting of the current block to air.
			if (firstPass)
			{
				firstPass = false;
				sectorDatas[SECTORDATA_CENTER][point.x % SECTOR_BLENGTH + (point.y % SECTOR_BLENGTH) * SECTOR_BLENGTH + (point.z % SECTOR_BLENGTH) * SECTOR_BAREA] = TERRA_AIR;
			}

			for (uint32 i = 0; i < 6; ++i)
			{
				UPoint3D newPoint;
				newPoint.x = point.x;
				newPoint.y = point.y;
				newPoint.z = point.z;
				switch (i) // determines sample direction from current point.
				{
				case 0: // left
					if (point.x > 0) 
						newPoint.x = point.x - 1;
					else
						continue;
					break;
				case 1: // right
					if (point.x < size_ - 1) 
						newPoint.x = point.x + 1;
					else
						continue;
					break;
				case 2: // front
					if (point.y > 0) 
						newPoint.y = point.y - 1;
					else
						continue;
					break;
				case 3: // back
					if (point.y < size_ - 1) 
						newPoint.y = point.y + 1;
					else
						continue;
					break;
				case 4: // down
					if (point.z > 0) 
						newPoint.z = point.z - 1;
					else
						continue;
					break;
				case 5: // up
					if (point.z < depth_ - 1) 
						newPoint.z = point.z + 1;
					else
						continue;
					break;
				}

				// determines sector access
				SECTORDATA_ACCESS access;
				if (newPoint.x < oX)
					access = SECTORDATA_LEFT;
				else if (newPoint.x >= oX + SECTOR_BLENGTH)
					access = SECTORDATA_RIGHT;
				else if (newPoint.y < oY)
					access = SECTORDATA_FRONT;
				else if (newPoint.y >= oY + SECTOR_BLENGTH)
					access = SECTORDATA_BACK;
				else if (newPoint.z < oZ)
					access = SECTORDATA_DOWN;
				else if (newPoint.z >= oZ + SECTOR_BLENGTH)
					access = SECTORDATA_UP;
				else
					access = SECTORDATA_CENTER;

				if (sectorDatas[access] == NULL)
				{
					if (!GetWorld()->GetSector (newPoint.x, newPoint.y, newPoint.z)->GetIsModifying ())
						sectorDatas[access] = GetWorld()->GetSector (newPoint.x, newPoint.y, newPoint.z)->BeginModify ();
					else
						sectorDatas[access] = GetWorld()->GetSector (newPoint.x, newPoint.y, newPoint.z)->GetData ();		
				}

				// sample and supersample nearby locations, looking for old air. if oldair is found, remove the old air and place the location into one of the lists.
				if (GetSCubeTerrainType (sectorDatas[access][newPoint.x % SECTOR_BLENGTH + (newPoint.y % SECTOR_BLENGTH) * SECTOR_BLENGTH + (newPoint.z % SECTOR_BLENGTH) * SECTOR_BAREA]) == TERRA_OLDAIR)
				{
					sectorDatas[access][newPoint.x % SECTOR_BLENGTH + (newPoint.y % SECTOR_BLENGTH) * SECTOR_BLENGTH + (newPoint.z % SECTOR_BLENGTH) * SECTOR_BAREA] = TERRA_AIR;
					if (access == SECTORDATA_CENTER)
					{
						points.InsertBack (newPoint);
					}
					else if (newPoint.x < oX - SECTOR_BLENGTH + 2 || newPoint.x > oX + SECTOR_BLENGTH * 2 - 3 || 
						newPoint.y < oY - SECTOR_BLENGTH + 2 || newPoint.y > oY + SECTOR_BLENGTH * 2 - 3 || 
						newPoint.z < oZ - SECTOR_BLENGTH + 2 || newPoint.z > oZ + SECTOR_BLENGTH * 2 - 3)
					{
						otherSectorPoints.InsertBack (newPoint);
					}
					else
					{
						uint32 outOfCenterBoundsCount = 0;
						if (newPoint.x < oX + 2 || newPoint.x > oX + SECTOR_BLENGTH - 3)
							++outOfCenterBoundsCount;
						if (newPoint.y < oY + 2 || newPoint.y > oY + SECTOR_BLENGTH - 3)
							++outOfCenterBoundsCount;
						if (newPoint.z < oZ + 2 || newPoint.z > oZ + SECTOR_BLENGTH - 3)
							++outOfCenterBoundsCount;

						if (outOfCenterBoundsCount > 1)
							otherSectorPoints.InsertBack (newPoint);
						else
							points.InsertBack (newPoint);
					}
				}
			}
		}

		// before calling EndModify(), make sure the sector in question is not already opened in the box of sectors from the calling method.
		if (int32(oX) < centerOX - SECTOR_BLENGTH || int32(oY) < centerOY - SECTOR_BLENGTH || int32(oZ) < centerOZ - SECTOR_BLENGTH ||
			int32(oX) > centerOX + SECTOR_BLENGTH || int32(oY) > centerOY + SECTOR_BLENGTH || int32(oZ) > centerOZ + SECTOR_BLENGTH)
		{
			GetWorld()->GetSector (oX, oY, oZ)->EndModify ();
		}
		if (sectorDatas[SECTORDATA_LEFT] != NULL && (
			int32(oX) - SECTOR_BLENGTH < centerOX - SECTOR_BLENGTH || int32(oY) < centerOY - SECTOR_BLENGTH || int32(oZ) < centerOZ - SECTOR_BLENGTH ||
			int32(oX) - SECTOR_BLENGTH > centerOX + SECTOR_BLENGTH || int32(oY) > centerOY + SECTOR_BLENGTH || int32(oZ) > centerOZ + SECTOR_BLENGTH))
		{
			GetWorld()->GetSector (oX - SECTOR_BLENGTH, oY, oZ)->EndModify ();
		}
		if (sectorDatas[SECTORDATA_RIGHT] != NULL && (
			int32(oX) + SECTOR_BLENGTH < centerOX - SECTOR_BLENGTH || int32(oY) < centerOY - SECTOR_BLENGTH || int32(oZ) < centerOZ - SECTOR_BLENGTH ||
			int32(oX) + SECTOR_BLENGTH > centerOX + SECTOR_BLENGTH || int32(oY) > centerOY + SECTOR_BLENGTH || int32(oZ) > centerOZ + SECTOR_BLENGTH))
		{
			GetWorld()->GetSector (oX + SECTOR_BLENGTH, oY, oZ)->EndModify ();
		}
		if (sectorDatas[SECTORDATA_FRONT] != NULL && (
			int32(oX) < centerOX - SECTOR_BLENGTH || int32(oY) - SECTOR_BLENGTH < centerOY - SECTOR_BLENGTH || int32(oZ) < centerOZ - SECTOR_BLENGTH ||
			int32(oX) > centerOX + SECTOR_BLENGTH || int32(oY) - SECTOR_BLENGTH > centerOY + SECTOR_BLENGTH || int32(oZ) > centerOZ + SECTOR_BLENGTH))
		{
			GetWorld()->GetSector (oX, oY - SECTOR_BLENGTH, oZ)->EndModify ();
		}
		if (sectorDatas[SECTORDATA_BACK] != NULL && (
			int32(oX) < centerOX - SECTOR_BLENGTH || int32(oY) + SECTOR_BLENGTH < centerOY - SECTOR_BLENGTH || int32(oZ) < centerOZ - SECTOR_BLENGTH ||
			int32(oX) > centerOX + SECTOR_BLENGTH || int32(oY) + SECTOR_BLENGTH > centerOY + SECTOR_BLENGTH || int32(oZ) > centerOZ + SECTOR_BLENGTH))
		{
			GetWorld()->GetSector (oX, oY + SECTOR_BLENGTH, oZ)->EndModify ();
		}
		if (sectorDatas[SECTORDATA_DOWN] != NULL && (
			int32(oX) < centerOX - SECTOR_BLENGTH || int32(oY) < centerOY - SECTOR_BLENGTH || int32(oZ) - SECTOR_BLENGTH < centerOZ - SECTOR_BLENGTH ||
			int32(oX) > centerOX + SECTOR_BLENGTH || int32(oY) > centerOY + SECTOR_BLENGTH || int32(oZ) - SECTOR_BLENGTH > centerOZ + SECTOR_BLENGTH))
		{
			GetWorld()->GetSector (oX, oY, oZ - SECTOR_BLENGTH)->EndModify ();
		}
		if (sectorDatas[SECTORDATA_UP] != NULL && (
			int32(oX) < centerOX - SECTOR_BLENGTH || int32(oY) < centerOY - SECTOR_BLENGTH || int32(oZ) + SECTOR_BLENGTH < centerOZ - SECTOR_BLENGTH ||
			int32(oX) > centerOX + SECTOR_BLENGTH || int32(oY) > centerOY + SECTOR_BLENGTH || int32(oZ) + SECTOR_BLENGTH > centerOZ + SECTOR_BLENGTH))
		{
			GetWorld()->GetSector (oX, oY, oZ + SECTOR_BLENGTH)->EndModify ();
		}
	}
}

void
WorldGenerator::LoadConfig_ ()
{
	; // STODO: load vars from 'World Generator.tag'
}