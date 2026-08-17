//	The panel's calculator.  main.cpp owns the interpreter and drives this; the
//	UI knows nothing about evaluation, only about cells and taps.

#pragma once

#include <string>

#define	SLIP_UI_VERSION	"2.1.1"

void	UIInit();

//	What the REPL prints, coloured the way the transcript colours it: "= " is a
//	value, "! " is an error, anything else is plain.  The colours live in ui.cpp
//	and stay there.
void	UIPrint( std::string const& );
void	UIRedraw();

//	Empty the transcript.  With the banner is what Reset leaves behind; without it
//	is what changing mode does, because what was on the screen was read under the
//	other one.
void	UIClearLog( bool banner = true );

//	True when a line is yours to run.  False otherwise, having already drawn
//	whatever the tap did.
bool	UIPoll( std::string& line );

//	Which mode the session is in: unticked is the calculator.  main.cpp owns the
//	mode, because a line typed over USB can change it too.
void	UISetProgram( bool );

void	UIBackspace();
void	UISetEditing( std::string const& );
