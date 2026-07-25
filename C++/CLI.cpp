#include "SliP.hpp"

#include	<fstream>

extern void			Build();
extern SP< SliP >	Read( iReader&, char32_t );
extern SP< SliP >	Eval( SP< Context >, SP< SliP > );

enum
Mode {
	SCRIPT		//	Run the program; print only what it prints ( ; and ¦ )
,	PRINT		//	Also print the value of every toplevel form
,	TRANSCRIPT	//	Echo each form and its value, as the pre-2.0 CLI did
};

static void
Usage( ostream& _ ) {
	_	<< "Usage: slip [options] [file]\n"
		<< "\n"
		<< "  Given a file, or input on a pipe, the program is run and nothing is\n"
		<< "  printed unless the program prints it ( ; to stdout, ¦ to stderr ).\n"
		<< "  Given a terminal and no file, input is read to EOF and echoed as a\n"
		<< "  transcript.\n"
		<< "\n"
		<< "  -e EXPR   evaluate EXPR and print its value\n"
		<< "  -p        run, printing the value of every toplevel form\n"
		<< "  -i        transcript: echo each form and its value\n"
		<< "  -v        print version\n"
		<< "  -h        print this help\n"
		<< "\n"
		<< "  Errors are reported as file:line: message, with exit status 1.\n"
		;
}

//	1-based line of a character index, for diagnostics.  The reader indexes the
//	decoded char32_t buffer, so counting newlines in it is exact.
static size_t
LineOf( StringReader const& R, size_t _ ) {
	size_t	$ = 1;
	for( size_t I = 0; I < _ && I < R.$.size(); I++ ) if( R.$[ I ] == U'\n' ) $++;
	return $;
}

//	Where the form starting at `_` actually begins, so an evaluation error points
//	at the form rather than at the blank lines in front of it.
static size_t
SkipBlank( StringReader const& R, size_t _ ) {
	while(
		_ < R.$.size()
	&&	( R.$[ _ ] == U' ' || R.$[ _ ] == U'\t' || R.$[ _ ] == U'\n' || R.$[ _ ] == U'\r' )
	) _++;
	return _;
}

int
main( int argc, char* argv[] ) {

	auto	mode	= SCRIPT;
	auto	hasExpr	= false;
	string	expr;
	string	path;

	for( int I = 1; I < argc; I++ ) {
		auto _ = string( argv[ I ] );
		if( _ == "-h" || _ == "--help"    ) { Usage( cout ); return 0; }
		if( _ == "-v" || _ == "--version" ) { cout << SLIP_VERSION << endl; return 0; }
		if( _ == "-p" ) { mode = PRINT     ; continue; }
		if( _ == "-i" ) { mode = TRANSCRIPT; continue; }
		if( _ == "-e" ) {
			if( ++I == argc ) { cerr << "slip: -e needs an expression" << endl; return 2; }
			expr	= argv[ I ];
			hasExpr	= true;
			continue;
		}
		if( _.size() > 1 && _[ 0 ] == '-' ) {
			cerr << "slip: unknown option: " << _ << endl;
			Usage( cerr );
			return 2;
		}
		path = _;
		break;
	}

	string	source;
	string	name;
	if( hasExpr ) {
		source	= expr;
		name	= "-e";
		if( mode == SCRIPT ) mode = PRINT;
	} else if( path.size() ) {
		ifstream	f( path );
		if( !f ) { cerr << "slip: cannot open " << path << endl; return 2; }
		source	= string( istreambuf_iterator< char >( f ), istreambuf_iterator< char >() );
		name	= path;
	} else {
		if( mode == SCRIPT && isatty( 0 ) ) mode = TRANSCRIPT;
		source	= string( istreambuf_iterator< char >( cin ), istreambuf_iterator< char >() );
		name	= "<stdin>";
	}

	Build();

	StringReader	R( source );
	auto			C = MS< Context >();

	size_t	start	= 0;		//	Index the current form began at
	auto	reading	= false;	//	Whether Read() or Eval() is what threw
	try {
		while( true ) {
			start	= R._;
			reading	= true;
			auto form = Read( R, -1 );
			reading	= false;
			if( !form ) break;
			if( mode == TRANSCRIPT ) cout << '>' << form->REPR() << endl;
			auto value = Eval( C, form );
			if( mode == TRANSCRIPT ) cout << '<' << value->REPR() << endl;
			if( mode == PRINT      ) cout << value->REPR() << endl;
		}
	} catch( exception const& e ) {
		//	A read error stopped where the reader choked; an evaluation error
		//	belongs to the form that was being evaluated.
		cout.flush();
		cerr
			<< name
			<< ':'
			<< LineOf( R, reading ? R._ : SkipBlank( R, start ) )
			<< ": "
			<< e.what()
			<< endl
		;
		return 1;
	}
	return 0;
}
