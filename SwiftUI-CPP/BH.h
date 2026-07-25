//	Bridging header for the SliP engine.  Every call returns a malloc'd C string
//	holding JSON — see C++/Embed.hpp for the shapes — and the caller frees it
//	with BH_Free.  Errors arrive inside that JSON, never as exceptions.

char*	BH_Version( void );

//	{ "source": "…", "response": "…" } | { "error": "…" }
char*	BH_REP( const char* source );

//	[ { "source": "…", "response": "…" }, … ] — stops at the first failure,
//	whose entry carries "error".  BH_REPL is programming mode, one entry per
//	toplevel form; BH_Sugared is calculator mode, one entry per non-empty line.
char*	BH_REPL( const char* source );
char*	BH_Sugared( const char* source );

void	BH_Reset( void );
void	BH_SetRoundPrecision( int precision );

void	BH_Free( char* _ );
