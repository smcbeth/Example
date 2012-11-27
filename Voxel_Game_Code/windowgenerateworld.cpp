//---------- Copyright © 2012, Daedal Games, All rights reserved. -------------

#include "windowgenerateworld.h"

WindowGenerateWorld::WindowGenerateWorld( )
{
	TextBox* textBox;

	textBox = new TextBox();
	textBox->SetText( "Map Size" );
	textBox->SetIsVisible( true );
	AddChild( textBox );

	textBox = new TextBox();
	textBox->SetOriginY( 20 );
	textBox->SetText( "Map Type" );
	textBox->SetIsVisible( true );
	AddChild( textBox );

	textBox = new TextBox();
	textBox->SetOriginY( 40 );
	textBox->SetText( "Resource Abundance" );
	textBox->SetIsVisible( true );
	AddChild( textBox );

	Button* button;
	button = new Button();
	button->SetOriginY( 70 );
	button->SetLabelText( "Generate!" );
	button->SetMsg( "generate" );
	button->SetIsVisible( true );
	AddChild( button );

	AutoSize();

	// center 'Generate!' button
	button->CenterHorizontal();
}

WindowGenerateWorld::~WindowGenerateWorld( )
{
	;
}

void
WindowGenerateWorld::ProcessMsg( KeyValues* msg, IPanel* sender )
{
	BaseWindow::ProcessMsg( msg, sender );

	if( stools::IsEqual( "generate", msg->GetName() ) )
	{
		// STODO: this being a global is ugly as heck
		extern WorldGenerator* _g_WorldGeneratorInstance;
		_g_WorldGeneratorInstance = new WorldGenerator;

		WorldGeneratorAPI()->Generate( 3, STANDARD_WORLDDEPTH );

		delete( WorldGenerator* )_g_WorldGeneratorInstance;
	}
}