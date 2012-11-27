//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "clientstatemainmenu.h"
#include "public\newport lib\ipanel.h"
#include "public\newport lib\textbox.h"
#include "public\iguisystem.h"
#include "windowmainmenu.h"

ClientStateMainMenu::ClientStateMainMenu( )
{
	;
}

ClientStateMainMenu::~ClientStateMainMenu( )
{
	;
}

//-----------------------------------------------------------------------------
//
// Inherited from BaseState
//
//-----------------------------------------------------------------------------
const char*
ClientStateMainMenu::GetName( )
{
	return "ClientStateMainMenu";
}

void
ClientStateMainMenu::DisplayConnectFailureMessage( )
{
	FAssert(); // CTODO:
}

void
ClientStateMainMenu::Initialize_( )
{
	IPanel* frame = GuiAPI()->CreateFrame( "mainmenu" );
	GuiAPI()->ActivateFrame( frame );

	// create title text
	TextBox* title = new TextBox;
	title->SetFont( "mainmenu title" );
	title->SetText( "Operation Counter-Mine" );
	title->SetOriginY( 125 );
	title->SetIsVisible( true );
	frame->AddChild( title );
	title->CenterHorizontal();

	// create main window
	WindowMainMenu* windowMainMenu = new WindowMainMenu;
	windowMainMenu->SetIsVisible( true );
	frame->AddChild( windowMainMenu );
	frame->SetActiveChild( windowMainMenu );

	isInitialized_ = true;
}

void
ClientStateMainMenu::Run_( )
{
	;
}

void
ClientStateMainMenu::Shutdown_( )
{
	GuiAPI()->DestroyFrame( "mainmenu" );

	isShutdown_		= true;
	isInitialized_	= false;
}