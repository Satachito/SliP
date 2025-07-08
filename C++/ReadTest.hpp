inline auto
READ( const string& _ ) {
	StringReader R( _ );
	return Read( R, -1 );
};

inline auto
TestRead( string const& _ ) {
	auto
	$ = READ( _ )->REPR();
//	cerr << $ << endl;
	A( $ == READ( $ )->REPR() );
}

template < ranges::range R > void
TestReads( R const& _ ) {
    ::apply( _, []( auto const& _ ) { TestRead( _ ); } );
}

inline auto
TestException( string const& _, string const& expected ) {
	try {
		READ( _ );
	} catch( exception const& e ) {
		A( e.what() == expected );
	}
}

inline auto
ReadTest() {

	A( Cast< Literal >( READ( "\"A\\0\"" ) )->$[ 1 ] == 0 );
	A( Cast< Literal >( READ( "\"A\\f\"" ) )->$[ 1 ] == U'\f' );
	A( Cast< Literal >( READ( "\"A\\n\"" ) )->$[ 1 ] == U'\n' );
	A( Cast< Literal >( READ( "\"A\\r\"" ) )->$[ 1 ] == U'\r' );
	A( Cast< Literal >( READ( "\"A\\t\"" ) )->$[ 1 ] == U'\t' );
	A( Cast< Literal >( READ( "\"A\\v\"" ) )->$[ 1 ] == U'\v' );
	A( Cast< Literal >( READ( "\"A\\v\"" ) )->$[ 1 ] == U'\v' );
	A( Cast< Literal >( READ( "\"A\\X\"" ) )->$[ 1 ] == U'X' );

	TestException( "\\", "Invalid escape" );

	TestRead( "[=]" );
	TestRead( "[A=]" );
	TestRead( "[AΩ]" );

	TestRead( "[ - -1 ]" );
	TestRead( "[ + -1 ]" );
	TestRead( "[ - +1 ]" );
	TestRead( "[ + +1 ]" );

	A( READ( "\"A\\\\B\"" )->REPR() == "\"A\\B\"" );

	A( READ( "" ) == 0 );

	TestException( "[ 3 + = 5 ]", "Syntax error: + =" );


	TestRead( "[ -1 ]" );

	TestException( "]", "Detect close parenthesis" );
	TestException( "⟩", "Detect close parenthesis" );
	TestException( "}", "Detect close parenthesis" );
	TestException( ")", "Detect close parenthesis" );
	TestException( "»", "Detect close parenthesis" );
	TestException( "`", "Unterminated string: " );

	TestRead( "[A]" );

	TestRead( "[1.23.45]" );

	TestException( "!@¡", "No such operator: !@¡" );

	TestRead( "[@1||2@]" );
	TestRead( "{@1||2@}" );
	TestRead( "(@1||2@)" );
	TestRead( "«@1||2@»" );
	TestRead( "⟨@1||2@⟩" );

	TestRead( "[ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩαβγδεζηθικλμνξοπρστυφχψως𝑒∞∅]" );

	TestReads(
		project(
			Functions
		,	[]( SP< Function > const& _ ){ return _->label; }
		)
	);

	TestReads(
		project(
			NumericConstants
		,	[]( SP< NumericConstant > const& _ ){ return _->$; }
		)
	);
}
