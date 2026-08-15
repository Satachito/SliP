//	The on-panel calculator.  main.cpp owns the interpreter and drives this; the
//	UI knows nothing about evaluation, only about cells and taps.

#pragma once

#include <string>
#include <cstdint>

#define	SLIP_UI_VERSION	"2.1.1"

void	UIInit();

//	What the REPL prints, coloured the way the transcript colours it: "= " is a
//	value, "! " is an error, anything else is plain.  The colours live in ui.cpp
//	and stay there.
void	UIPrint( std::string const& );
void	UIRedraw();

//	True when the transcript was tapped with something typed: the line is yours
//	to run.  False otherwise, having already drawn whatever the tap did.
bool	UIPoll( std::string& line );

void	UIBackspace();
void	UISetEditing( std::string const& );
