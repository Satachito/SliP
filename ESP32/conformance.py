#	Run conformance/ on the board itself, over the serial line.
#
#	Each case is fed in programming mode as one :{ … :} block, exactly as a person
#	would paste it, and the values the board prints are compared with the .out the
#	desktop interpreter is held to.
#	Needs pyserial and a board running this firmware.  ESP-IDF's own Python
#	environment has pyserial already:
#
#		~/.espressif/python_env/idf5.5_py3.9_env/bin/python ESP32/conformance.py
import glob, os, re, sys, time, serial

PORT   = "/dev/cu.usbserial-3110"
ROOT   = os.path.join( os.path.dirname( os.path.dirname( os.path.abspath( __file__ ) ) ), "conformance" )
#	Matched against bytes, and decoded only once at the end: a serial read can
#	split a multi-byte character across two chunks, and decoding per chunk turned
#	θ into two replacement characters.
PROMPT = re.compile( rb"(?:^|\n)(?:> |>> |\.\. )$" )

def reset( s ):
	s.dtr = False; s.rts = True
	time.sleep( 0.1 )
	s.rts = False
	time.sleep( 0.8 )
	s.reset_input_buffer()

def until_prompt( s, limit = 20.0 ):
	out = b""
	start = time.time()
	while time.time() - start < limit:
		n = s.in_waiting
		if n:
			out += s.read( n )
			if PROMPT.search( out ): break
		else:
			time.sleep( 0.01 )
	return out.decode( "utf-8", "replace" )

def send( s, line ):
	s.write( line.encode( "utf-8" ) + b"\r" )
	s.flush()
	return until_prompt( s )

with serial.Serial( PORT, 115200, timeout = 0.1 ) as s:
	reset( s )
	until_prompt( s )

	passed = failed = 0

	for path in sorted( glob.glob( ROOT + "/cases/*.slip" ) ):
		name = os.path.basename( path )
		send( s, ":reset" )
		send( s, ":prog" )
		send( s, ":{" )
		body = open( path, encoding = "utf-8" ).read().split( "\n" )
		for line in body[ :-1 ] if body and body[ -1 ] == "" else body:
			send( s, line )
		got = send( s, ":}" )

		values   = [ l[ 2: ] for l in got.replace( "\r", "" ).split( "\n" ) if l.startswith( "= " ) ]
		expected = open( path[ : -5 ] + ".out", encoding = "utf-8" ).read().rstrip( "\n" ).split( "\n" )

		if values == expected:
			passed += 1
			print( f"  ok    {name}  ({len(values)} values)" )
		else:
			failed += 1
			print( f"  FAIL  {name}" )
			for i in range( max( len( values ), len( expected ) ) ):
				g = values[ i ] if i < len( values ) else "<missing>"
				e = expected[ i ] if i < len( expected ) else "<missing>"
				if g != e: print( f"        line {i+1}: got {g!r}  want {e!r}" )

	for path in sorted( glob.glob( ROOT + "/errors/*.slip" ) ):
		name = os.path.basename( path )
		send( s, ":reset" )
		send( s, ":prog" )
		send( s, ":{" )
		body = open( path, encoding = "utf-8" ).read().rstrip( "\n" ).split( "\n" )
		for line in body:
			send( s, line )
		got = send( s, ":}" )

		errors = [ l[ 2: ] for l in got.replace( "\r", "" ).split( "\n" ) if l.startswith( "! " ) ]
		#	The .err file is  :LINE: MESSAGE  — a console line has no line number.
		want = open( path[ : -5 ] + ".err", encoding = "utf-8" ).read().strip()
		want = want.split( ": ", 1 )[ 1 ] if ": " in want else want

		if len( errors ) == 1 and errors[ 0 ] == want:
			passed += 1
			print( f"  ok    {name}  ({errors[0]!r})" )
		else:
			failed += 1
			print( f"  FAIL  {name}: got {errors!r}  want [{want!r}]" )

	print( f"\nboard conformance: {passed} passed, {failed} failed" )
	sys.exit( 1 if failed else 0 )
