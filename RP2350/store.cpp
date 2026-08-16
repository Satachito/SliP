//	The session, kept across power cycles.
//
//	What is saved is the source, not the state: the lines that were run, in the
//	order they were run.  Replaying them rebuilds exactly the session they built,
//	which is not true of any shorter summary — a binding's value is an expression
//	that may close over the context it was made in, and writing out the values
//	alone would be a different session that happens to answer the same for a
//	while.  It also means nothing in the interpreter has to know this file exists.
//
//	The last 64 KB of flash, as sixteen slots of one sector.  A slot holds the
//	whole log, and every write goes to the next slot: sixteen times the erase
//	budget, and — because the slot being written is not the slot being read — a
//	power cut in the middle of a write costs the last line rather than the
//	session.  The newest slot is the one with the highest sequence number whose
//	contents still check out.
//
//	picotool writes the program, and the program does not reach this far up the
//	flash, so a session survives being reflashed.  That is a nice property and a
//	surprising one, which is the other reason `:reset` has to erase it.

#include <cstdint>
#include <cstring>
#include <string>

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

using namespace std;

static constexpr uint32_t	SLOT_BYTES	= FLASH_SECTOR_SIZE;		//	4096
static constexpr uint32_t	SLOTS		= 16;
static constexpr uint32_t	BASE		= PICO_FLASH_SIZE_BYTES - SLOTS * SLOT_BYTES;
static constexpr uint32_t	MAGIC		= 0x536C6950;				//	'SliP'

struct Head {
	uint32_t	magic;
	uint32_t	seq;
	uint32_t	len;
	uint32_t	crc;
};

static constexpr uint32_t	MAX_LOG	= SLOT_BYTES - sizeof( Head );

//	Erased flash is 0xFF everywhere, which is a valid-looking header for nothing
//	at all, so a slot is only believed if the magic, the length and the checksum
//	all agree.
static uint32_t
CRC( uint8_t const* p, uint32_t n ) {
	uint32_t c = 0xFFFFFFFF;
	while( n-- ) {
		c ^= *p++;
		for( auto i = 0; i < 8; i++ ) c = ( c >> 1 ) ^ ( c & 1 ? 0xEDB88320 : 0 );
	}
	return ~c;
}

static uint8_t const*
Slot( uint32_t i ) { return (uint8_t const*)( XIP_BASE + BASE + i * SLOT_BYTES ); }

static string	theLog;
static uint32_t	theSlot	= 0;
static uint32_t	theSeq	= 0;

//	Static rather than automatic: this is a sector's worth, and it is written
//	from inside a call with the interrupts off.
static uint8_t	buffer[ SLOT_BYTES ];

static void
Persist() {
	theSlot = ( theSlot + 1 ) % SLOTS;
	theSeq += 1;

	Head	h = {
		MAGIC
	,	theSeq
	,	(uint32_t)theLog.size()
	,	CRC( (uint8_t const*)theLog.data(), (uint32_t)theLog.size() )
	};
	memset( buffer, 0xFF, sizeof( buffer ) );
	memcpy( buffer, &h, sizeof( h ) );
	memcpy( buffer + sizeof( h ), theLog.data(), theLog.size() );

	//	Programming is by whole pages, so round up to one.
	auto	n = ( sizeof( h ) + theLog.size() + FLASH_PAGE_SIZE - 1 )
	          / FLASH_PAGE_SIZE * FLASH_PAGE_SIZE;

	//	Erasing and programming stop the flash answering, and the code is in it.
	//	One core is running and nothing else touches this, so turning the
	//	interrupts off is the whole of the exclusion. USB goes quiet for as long
	//	as it takes — tens of milliseconds — and the host waits.
	auto	ints = save_and_disable_interrupts();
	flash_range_erase( BASE + theSlot * SLOT_BYTES, SLOT_BYTES );
	flash_range_program( BASE + theSlot * SLOT_BYTES, buffer, n );
	restore_interrupts( ints );
}

void
StoreInit() {
	theLog.clear();

	auto		best	= -1;
	uint32_t	bestSeq	= 0;
	for( uint32_t i = 0; i < SLOTS; i++ ) {
		Head	h;
		memcpy( &h, Slot( i ), sizeof( h ) );
		if( h.magic != MAGIC || h.len > MAX_LOG ) continue;
		if( CRC( Slot( i ) + sizeof( h ), h.len ) != h.crc ) continue;
		//	Signed difference, so that a sequence number that has wrapped still
		//	compares the right way round.
		if( best < 0 || (int32_t)( h.seq - bestSeq ) > 0 ) { best = (int)i; bestSeq = h.seq; }
	}
	if( best < 0 ) return;

	Head	h;
	memcpy( &h, Slot( best ), sizeof( h ) );
	theLog.assign( (char const*)Slot( best ) + sizeof( h ), h.len );
	theSlot = (uint32_t)best;
	theSeq  = h.seq;
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
