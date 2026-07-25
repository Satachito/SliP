#pragma once

//	The embedding layer: one evaluation session behind a JSON contract, shared by
//	every host that is not the CLI.
//
//	The CLI can let a SliP error propagate — it prints a diagnostic and exits.
//	A host that is a running application cannot: an escaping C++ exception either
//	crosses a language boundary that will not catch it ( the Objective-C++ bridge )
//	or unwinds out of the module entirely ( WASM ).  So errors are values here,
//	never exceptions, and both hosts get the same shapes.
//
//	  REP( expr )     -> { "response": "…" }
//	                  |  { "error": "…" }
//
//	  REPL( source )  -> [ { "source": "…", "response": "…" }, … ]
//	                     Programming mode.  Evaluation stops at the first failing
//	                     form, whose entry carries "error" instead of "response"
//	                     ( and "source" too, if the form was read before it threw ),
//	                     because later forms usually depend on earlier ones.
//
//	  Sugared( src )  -> the same array, one entry per non-empty line, each line
//	                     read as the contents of a sentence.  Calculator mode, and
//	                     it does *not* stop: a typo on one line must not hide the
//	                     answers below it.
//
//	Bindings persist across calls until Reset().

#include "SliP.hpp"

string	json_escape( string const& );

string	Version();

string	REP( string const& );
string	REPL( string const& );
string	Sugared( string const& );

void	Reset();
void	SetRoundPrecision( int );
