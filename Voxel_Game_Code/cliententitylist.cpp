//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "cliententitylist.h"
#include "public\nstdlib\stools.h"
#include "public\nstdlib\vector.h"
#include "public\icliententity.h"

//-----------------------------------------------------------------------------
// Class ClienKnownEntity
//-----------------------------------------------------------------------------
class ClienKnownEntity
{
public:
	ClienKnownEntity( );
	~ClienKnownEntity( );

	char*			name;			// entity name
	IClientEntity*	(*createFn)();	// function used to create a new instance
};

ClienKnownEntity::ClienKnownEntity( )
{
	name		= NULL;
	createFn	= NULL;
}

ClienKnownEntity::~ClienKnownEntity( )
{
	SAFE_FREE( name );
}

//-----------------------------------------------------------------------------
// Class ClientEntityList
//-----------------------------------------------------------------------------
class ClientEntityList: public IClientEntityList
{
public:
	ClientEntityList( );
	~ClientEntityList( );

	// Do not call this directly, use 'LINK_CLIENTENTITY_TO_CLASS'
	void			Learn( const char* name, IClientEntity* (*createFn)() );
	// Return true if we know about that entity
	bool			Verify( const char* name );
	IClientEntity*	Create( const char* name );

private:
	Vector< ClienKnownEntity* > knownEntities_;
};

// Accessor
FORCE_INLINE IClientEntityList*
GetClientEntityList( )
{
	static ClientEntityList clientEntityList;
	return &clientEntityList;
}

ClientEntityList::ClientEntityList( )
{
	;
}

ClientEntityList::~ClientEntityList( )
{
	knownEntities_.RemoveAndDeleteAll();
}

//-----------------------------------------------------------------------------
// Do not call this directly, use 'LINK_CLIENTENTITY_TO_CLASS'
//-----------------------------------------------------------------------------
void
ClientEntityList::Learn( const char* name, IClientEntity* (*createFn)() )
{
	ClienKnownEntity* knownEntity = new ClienKnownEntity;

	stools::MallocAndCopy( name, knownEntity->name );
	knownEntity->createFn = createFn;
	
	knownEntities_.InsertBack( knownEntity );
}

//-----------------------------------------------------------------------------
// Return true if we know about that entity
//-----------------------------------------------------------------------------
bool
ClientEntityList::Verify( const char* name )
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

IClientEntity*
ClientEntityList::Create( const char* name )
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