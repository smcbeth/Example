//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "windowmainmenu.h"

WindowMainMenu::WindowMainMenu( )
{
	// create 'generate world' button
	Button* button;
	button = new Button();
	button->SetLabelFont( "mainmenu" );
	button->SetLabelText( "Generate World" );
	button->SetMsg( "generate" );
	button->SetIsVisible( true );
	AddChild( button );

	// create 'new game' button
	button = new Button();
	button->SetLabelFont( "mainmenu" );
	button->SetOriginY( 30 );
	button->SetLabelText( "New Game" );
	button->SetMsg( "newgame" );
	button->SetIsVisible( true );
	AddChild( button );

	// create 'load game button
	button = new Button();
	button->SetLabelFont( "mainmenu" );
	button->SetOriginY( 55 );
	button->SetLabelText( "Load Game" );
	button->SetIsVisible( true );
	AddChild( button );

	// create 'options' button
	button = new Button();
	button->SetLabelFont( "mainmenu" );
	button->SetOriginY( 80 );
	button->SetLabelText( "Options" );
	button->SetIsVisible( true );
	AddChild( button );

	// create 'quit' button
	button = new Button();
	button->SetLabelFont( "mainmenu" );
	button->SetOriginY( 125 );
	button->SetLabelText( "Quit" );
	button->SetMsg( "quit" );
	button->SetIsVisible( true );
	AddChild( button );

	// create generate world window
	windowGenerateWorld_ = new WindowGenerateWorld;
	GuiAPI()->GetFrame( "mainmenu" )->AddChild( windowGenerateWorld_ );
	windowGenerateWorld_->Center();

	SetOrigin( 20, MaterialAPI()->GetWindowHeight() - 200 );
	AutoSize();
	SetDoDrawBackground( false );
}

WindowMainMenu::~WindowMainMenu( )
{
	;
}

//-----------------------------------------------------------------------------
//
// Inherited from 'IPanel'
//
//-----------------------------------------------------------------------------
void
WindowMainMenu::ProcessMsg( KeyValues* msg, IPanel* sender )
{
	BaseWindow::ProcessMsg( msg, sender );

	if( stools::IsEqual( "generate", msg->GetName() ) )
	{
		windowGenerateWorld_->SetIsVisible( true );
	}
	else if( stools::IsEqual( "newgame", msg->GetName() ) )
	{
		// quit main menu
		EngineAPI()->ShutdownState( "ClientStateMainMenu" );
		
		// start client with hardcoded server ip
		IPAddress serverIPAddress;
		serverIPAddress.Init( 127, 0, 0, 1, 777 );
		EngineAPI()->RunState( new ClientStateConnect( serverIPAddress ) );

		// start server
		EngineAPI()->RunState( new ServerStatePlay );
	}
	else if( stools::IsEqual( "quit", msg->GetName() ) )
	{
		extern IModule* _g_ModuleInstance;
		EngineAPI()->ShutdownPump();
	}
}