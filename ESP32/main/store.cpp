//	The session, kept across power cycles.
//
//	What is saved is the source, not the state: the lines that were run, in the
//	order they were run.  Replaying them rebuilds exactly the session they built,
//	which is not true of any shorter summary — a binding's value is an expression
//	that may close over the context it was made in, and writing out the values
//	alone would be a different session that happens to answer the same for a
//	while.  It also means nothing in the interpreter has to know this file exists.
//
//	The same design as RP2350/store.cpp, and none of its machinery: NVS is a
//	key-value store with its own wear levelling and its own power-cut safety, so
//	the log is one blob under one key and this file is the four calls that put it
//	there.  The partition is the 24 KB `nvs` in partitions.csv, which was already
//	there — the default table has it, and nothing else on this board uses it.

#include <string>

#include "nvs_flash.h"
#include "nvs.h"

using namespace std;

//	The same ceiling the RP2350 has, so that a session that fits on one board
//	fits on the other.  NVS would take far more; matching is worth more than the
//	extra room.
static constexpr size_t	MAX_LOG	= 4080;

static char const*	NAMESPACE	= "slip";
static char const*	KEY			= "session";

static string	theLog;

static void
Persist() {
	nvs_handle_t	h;
	if( nvs_open( NAMESPACE, NVS_READWRITE, &h ) != ESP_OK ) return;
	if( theLog.empty() )	nvs_erase_key( h, KEY );
	else					nvs_set_blob( h, KEY, theLog.data(), theLog.size() );
	nvs_commit( h );
	nvs_close( h );
}

void
StoreInit() {
	theLog.clear();

	//	The partition is initialised here rather than in app_main because this is
	//	the only thing on this board that uses it. A partition left over from a
	//	larger build, or a new NVS format, is erased and started again: what is in
	//	it is a session, and a session is not worth refusing to boot over.
	auto	e = nvs_flash_init();
	if( e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND ) {
		nvs_flash_erase();
		e = nvs_flash_init();
	}
	if( e != ESP_OK ) return;

	nvs_handle_t	h;
	if( nvs_open( NAMESPACE, NVS_READONLY, &h ) != ESP_OK ) return;
	size_t	n = 0;
	if( nvs_get_blob( h, KEY, nullptr, &n ) == ESP_OK && n && n <= MAX_LOG ) {
		theLog.resize( n );
		nvs_get_blob( h, KEY, theLog.data(), &n );
	}
	nvs_close( h );
}

string const&
StoreLog() { return theLog; }

void
StoreAppend( string const& _ ) {
	if( _.empty() ) return;
	theLog += _;
	theLog += '\n';
	//	Full: the oldest lines go, and they go whole. Half a line replayed is
	//	worse than one line missing.
	while( theLog.size() > MAX_LOG ) {
		auto nl = theLog.find( '\n' );
		if( nl == string::npos ) { theLog.clear(); break; }
		theLog.erase( 0, nl + 1 );
	}
	Persist();
}

void
StoreForget() {
	theLog.clear();
	Persist();
}
