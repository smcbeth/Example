//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "public\nstdlib\stools.h"
#include "public\nstdlib\vector.h"
#include "iserverentity.h"
#include "serverentitylist.h"

//-----------------------------------------------------------------------------
// Class ServerKnownEntity
//-----------------------------------------------------------------------------
class ServerKnownEntity
{
public:
	ServerKnownEntity( );
	~ServerKnownEntity( );

	char*			name;			// entity name
	IServerEntity*	(*createFn)();	// function used to create a new instance
};

ServerKnownEntity::ServerKnownEntity( )
{
	name		= NULL;
	createFn	= NULL;
}

ServerKnownEntity::~ServerKnownEntity( )
{
	SAFE_FREE( name );
}

//-----------------------------------------------------------------------------
// Class ServerEntityList
//-----------------------------------------------------------------------------
class ServerEntityList: public IServerEntityList
{
public:
	ServerEntityList( );
	~ServerEntityList( );

	// Do not call this directly, use 'LINK_CLIENTENTITY_TO_CLASS'
	void			Learn( const char* name, IServerEntity* (*createFn)() );
	// Return true if we know about that entity
	bool			Verify( const char* name );
	IServerEntity*	Create( const char* name );

	const char*		GetName( uint32 idx );
	uint32			GetCount( );

private:
	Vector< ServerKnownEntity* > knownEntities_;
};

// Accessor
IServerEntityList*
GetServerEntityList( )
{
	static ServerEntityList serverEntityList;
	return &serverEntityList;
}

ServerEntityList::ServerEntityList( )
{
	;
}

ServerEntityList::~ServerEntityList( )
{
	knownEntities_.RemoveAndDeleteAll();
}

//-----------------------------------------------------------------------------
// Do not call this directly, use 'LINK_CLIENTENTITY_TO_CLASS'
//-----------------------------------------------------------------------------
void
ServerEntityList::Learn( const char* name, IServerEntity* (*createFn)() )
{
	ServerKnownEntity* knownEntity = new ServerKnownEntity;

	stools::MallocAndCopy( name, knownEntity->name );
	knownEntity->createFn = createFn;
	
	knownEntities_.InsertBack( knownEntity );
}

//-----------------------------------------------------------------------------
// Return true if we know about that entity
//-----------------------------------------------------------------------------
bool
ServerEntityList::Verify( const char* name )
{
	for( uint32 i = 0; i < knownEntities_.GetCount(); ++i )
	{
		if( stools::IsEqual( name, knownEntities_[ i ]->name ) )
		{
			return true;
		}
	}

	return false;
}

IServerEntity*
ServerEntityList::Create( const char* name )
{
	for( uint32 i = 0; i < knownEntities_.GetCount(); ++i )
	{
		if( stools::IsEqual( name, knownEntities_[ i ]->name ) )
		{
			return knownEntities_[ i ]->createFn();
		}
	}

	FAssert();
	return NULL;
}

const char*
ServerEntityList::GetName( uint32 idx )
{
	return knownEntities_[ idx ]->name;
}

uint32
ServerEntityList::GetCount( )
{
	return knownEntities_.GetCount();
}