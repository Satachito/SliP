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

//	Native document hosts can keep one interpreter context per window.  The
//	opaque session keeps Context out of the C bridge while the original
//	process-wide entry points remain available to WASM.
struct EmbedSession;

EmbedSession*	NewEmbedSession();
void			DeleteEmbedSession( EmbedSession* );
string			SessionREPL( EmbedSession*, string const& );
string			SessionSugared( EmbedSession*, string const& );
void			ResetEmbedSession( EmbedSession* );

//	The JSON above exists because the hosts that read it are on the other side of
//	a language boundary — JavaScript, Swift, Kotlin.  A host written in C++ has no
//	such boundary, and encoding a list only to parse it straight back is work for
//	nobody.  SessionRun is that same list, before it is written out; REPL and
//	Sugared are this with json_escape applied.
struct
EmbedEntry {
	string	source;		//	the form as the reader understood it, when it got that far
	string	value;		//	its printed value, when it evaluated
	string	error;		//	why it did not, when it did not
	bool	failed = false;
};

//	calculator: one entry per non-empty line, read as the contents of a sentence,
//	and it does not stop — a typo on the first line must not hide the answer on
//	the sixth.  Otherwise one entry per toplevel form, stopping at the first
//	failure, because a later form usually depends on an earlier one.
V< EmbedEntry >	SessionRun( EmbedSession*, string const& source, bool calculator );
