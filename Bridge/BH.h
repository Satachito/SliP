//	Bridging header for the SliP engine.  Every call returns a malloc'd C string
//	holding JSON — see C++/Embed.hpp for the shapes — and the caller frees it
//	with BH_Free.  Errors arrive inside that JSON, never as exceptions.

#ifdef __cplusplus
extern "C" {
#endif

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

typedef void* BH_Session;
BH_Session	BH_SessionCreate( void );
void		BH_SessionDestroy( BH_Session );
char*		BH_SessionREPL( BH_Session, const char* source );
char*		BH_SessionSugared( BH_Session, const char* source );
void		BH_SessionReset( BH_Session );

void	BH_Free( char* _ );

#ifdef __cplusplus
}
#endif
