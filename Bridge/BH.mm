#include "BH.h"
#include "../C++/Embed.hpp"

//	The bridge to Swift.  Everything it exposes returns the JSON described in
//	Embed.hpp, so a SliP error arrives as data rather than as a C++ exception —
//	which is what matters here, because an exception thrown through this boundary
//	has nowhere to go but std::terminate.
//
//	Build() is latched on first use.  CLI, TEST and WASM each call it once from
//	their single entry point; the bridge has no such point, since Swift may call
//	any of these first and may call them again.  A file-scope initializer would
//	be a static-init-order race against BUILTINS in SliP.cpp.
static void
BuildOnce() {
	extern void Build();
	static auto
	$ = ( Build(), true );
	(void)$;
}

static char*
Copy( string const& _ ) {
	auto $ = new char[ _.length() + 1 ];
	strcpy( $, _.c_str() );
	return $;
}

extern "C" char*
BH_Version() {
	return Copy( Version() );
}

extern "C" char*
BH_REP( const char* source ) {
	BuildOnce();
	return Copy( REP( string( source ) ) );
}

extern "C" char*
BH_REPL( const char* source ) {
	BuildOnce();
	return Copy( REPL( string( source ) ) );
}

extern "C" char*
BH_Sugared( const char* source ) {
	BuildOnce();
	return Copy( Sugared( string( source ) ) );
}

extern "C" void
BH_Reset() {
	BuildOnce();
	Reset();
}

extern "C" void
BH_SetRoundPrecision( int _ ) {
	SetRoundPrecision( _ );
}

extern "C" BH_Session
BH_SessionCreate() {
	BuildOnce();
	return NewEmbedSession();
}

extern "C" void
BH_SessionDestroy( BH_Session session ) {
	DeleteEmbedSession( static_cast< EmbedSession* >( session ) );
}

extern "C" char*
BH_SessionREPL( BH_Session session, const char* source ) {
	return Copy( SessionREPL( static_cast< EmbedSession* >( session ), string( source ) ) );
}

extern "C" char*
BH_SessionSugared( BH_Session session, const char* source ) {
	return Copy( SessionSugared( static_cast< EmbedSession* >( session ), string( source ) ) );
}

extern "C" void
BH_SessionReset( BH_Session session ) {
	ResetEmbedSession( static_cast< EmbedSession* >( session ) );
}

extern "C" void
BH_Free( char* _ ) {
	delete[] _;
}
