//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------
//
// Purpose: 
//
//-----------------------------------------------------------------------------
#pragma once

#define MAX_ENTITYNAME			64
#define MAX_ENTITYCLASSNAME		32							// CTODO: put in newport public definitions?

//-----------------------------------------------------------------------------
// Terrain (and SCube) Related
//-----------------------------------------------------------------------------
#define TERRATEXSIZE			float32(256.0f / 512.0f)	// Note- If the size of 'terrain.png' changes, change this

#define TERRAINTYPE_COUNT	33

#define SCUBE_LENGTH		3.0f							// 3 feet
#define SCUBE_HALFLENGTH	(SCUBE_LENGTH / 2.0f)			// 1.5 feet

#define SECTOR_BLENGTH		128								// sector length in blocks, this should be a power of two
#define SECTOR_BAREA		Square( SECTOR_BLENGTH )
#define SECTOR_BVOLUME		Cube( SECTOR_BLENGTH )

#define SECTOR_BLENGTHF		128.0f								// sector length in blocks, this should be a power of two
#define SECTOR_BAREAF		Square( SECTOR_BLENGTHF )
#define SECTOR_BVOLUMEF		Cube( SECTOR_BLENGTHF )

#define SECTOR_LENGTH		(SECTOR_BLENGTHF * SCUBE_LENGTH)	// sector length in feet
#define SECTOR_AREA			Square( SECTOR_LENGTH )
#define SECTOR_VOLUME		Cube( SECTOR_LENGTH )

//-----------------------------------------------------------------------------
// Networking Related
//-----------------------------------------------------------------------------
enum MessageType: uint32
{
	MESSAGETYPE_CONNECTIONREQUST			= 0,
	MESSAGETYPE_CONNECTIONREQUEST_ACCEPTED,
	MESSAGETYPE_CONNECTIONREQUEST_DENIED,

	MESSAGETYPE_DISCONNECTION_NOTIFICATION,

	MESSAGETYPE_INITIALWORLDDATA,					// initial world data follows

	MESSAGETYPE_VERIFYENTITY,
	MESSAGETYPE_VERIFYENTITY_FAILURE,
	MESSAGETYPE_VERIFYENTITY_FINISHED,

	MESSAGETYPE_NEWENTITY,
	MESSAGETYPE_ENTITYUPDATE,
	MESSAGETYPE_DESTROYENTITY
};

struct MessageNewEntity
{
	char*	name;	// factory name
	uint32	uid;	// unique entity identifier
};

struct MessageEntityUpdate
{
	uint32	entityUID;	// unique entity identifier
	uint8*	data;		// serialized entity data
};