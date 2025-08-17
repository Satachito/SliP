#include "../C++/SliP.hpp"

extern SP< SliP >
Eval( SP< Context >, SP< SliP > );

extern SP< SliP >
Read( iReader&, char32_t );

static auto
READ( const string& _ ) {
	StringReader R( _ );
	return Read( R, -1 );
};

template< typename T, typename F > void
TestEval( SP< Context > C, string const& _, F f ) {
	f( Cast< T >( Eval( C, READ( _ ) ) ) );
//	try {
//		f( Cast< T >( Eval( C, READ( _ ) ) ) );
//	} catch( exception const& e ) {
//		cerr << "--------:" << _ << endl;
//		cerr << "e.what():" << e.what() << endl;
//		throw e;
//	}
}

static auto
TestEvalException( SP< Context > C, string const& _, string const& expected ) {
//cerr << _ << endl;
	try {
		Eval( C, READ( _ ) );
		cerr << _ << ':' << expected << endl;
		A( false );
	} catch( exception const& e ) {
//		if( e.what() != expected ) {
//			cerr << "--------:" << _ << endl;
//			cerr << "e.what():" << e.what() << endl;
//			cerr << "expected:" << expected << endl;
//		}
		A( e.what() == expected );
	}
}

extern bool	IsNil( SP< SliP > );
extern bool	IsT( SP< SliP > );

void
TestDict( SP< Context > C ) {
	TestEvalException( C, "a"		, "Undefined name: a" );
	TestEval< Bits >( C, "('a=3)"	, []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEval< Bits >( C, "('b=4)"	, []( auto const& _ ){ A( _->$ == 4 ); } );
	TestEval< Bits >( C, "(¶§'(a))"	, []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEval< Bits >( C, "(¶§'a)"	, []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEval< Bits >( C, "(¶§a)"	, []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEval< Bits >( C, "a"		, []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEval< Bits >( C, "(a)"		, []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEval< Bits >( C, "(-a)"		, []( auto const& _ ){ A( _->$ == -3 ); } );
	TestEval< List >(
		C
	,	"{ ( 'x = 2 ) x }"
	,	[]( auto const& _ ) {
			A( _->$.size() == 2 );
			A( Cast< Bits >( _->$[ 1 ] )->$ == 2 );
		}
	);
	TestEval< SliP >( C, "( 'A == 'B )", []( auto const& _ ){ A( IsNil( _ ) ); }  );
	TestEval< SliP >( C, "( 'B == 'A )", []( auto const& _ ){ A( IsNil( _ ) ); }  );
	TestEval< Dict >( C, "¶", []( auto const& _ ){ A( _->REPR() == "{\t( 'a = '3 )\n\t( 'b = '4 )\n}" ); } );
	TestEval< Bits >( C, "(¶.'a)", []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEvalException( C, "(¶.`abc`)", "Illegal operand combination" );
	TestEvalException( C, "(¶.'x)", "No such key in dict: x" );
	TestEvalException( C, "(¶.3)", "Illegal operand combination" );
	TestEvalException( C, "( [ a b c ].-1 )", "Index out of bounds: -1" );
	TestEvalException( C, "( [ a b c ].3 )", "Index out of bounds: 3" );
	TestEvalException( C, "( [ a b c ].`a` )", "Illegal operand combination" );

	Eval( C, READ( "( 'x = 1 )" ) );
	TestEval< Bits >( C, "x", []( auto const& _ ){ A( _->$ == 1 ); } );
	auto C2 = MS< Context >( C );
	TestEval< Bits >( C2, "x", []( auto const& _ ){ A( _->$ == 1 ); } );
}

void
TestMatrix( SP< Context > C ) {
	TestEval< SliP >(
		C
	,	"(⟨1 2 3 4 5 6 7 8 9 10 11 12 ⟩±4 == ⟨1 5 9     2 6 10    3 7 11  4 8 12 ⟩±-4)"
	,	[]( auto const& _ ){ A( IsT( _ ) ); }
	);

	TestEval< Matrix >(
		C
	,	"(⟨1 2 3 4 5 6⟩±3·⟨1 5 9     2 6 10    3 7 11  4 8 12 ⟩±-4)"
	,	[]( auto const& _ ){
			A( _->size == 8 );
			A( _->nCols == 4 );
			A( _->$[ 0 ] == 1 * 1 + 2 * 5 + 3 * 9 );
			A( _->$[ 1 ] == 1 * 2 + 2 * 6 + 3 * 10 );
			A( _->$[ 2 ] == 1 * 3 + 2 * 7 + 3 * 11 );
			A( _->$[ 3 ] == 1 * 4 + 2 * 8 + 3 * 12 );
			A( _->$[ 4 ] == 4 * 1 + 5 * 5 + 6 * 9 );
			A( _->$[ 5 ] == 4 * 2 + 5 * 6 + 6 * 10 );
			A( _->$[ 6 ] == 4 * 3 + 5 * 7 + 6 * 11 );
			A( _->$[ 7 ] == 4 * 4 + 5 * 8 + 6 * 12 );
		}
	);

	A( MS< Matrix >( new double[ 0 ], 0, 0 )->REPR() == "⟨⟩" );

	TestEval< SliP >(
		C
	,	"(⟨1 2 3⟩±3 == ⟨1 2 3⟩±1)"
	,	[]( auto const& _ ){ A( IsNil( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	"(⟨1 2 3⟩±1 == ⟨1 2 3⟩±3)"
	,	[]( auto const& _ ){ A( IsNil( _ ) ); }
	);

	TestEval< SliP >(
		C
	,	"(⟨2⟩±1 == ⟨1⟩±1)"
	,	[]( auto const& _ ){ A( IsNil( _ ) ); }
	);
	
	
	TestEval< SliP >(
		C
	,	"(⟨2⟩±1 == ⟨1⟩±1)"
	,	[]( auto const& _ ){ A( IsNil( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	"(⟨1⟩±1 == ⟨2⟩±1)"
	,	[]( auto const& _ ){ A( IsNil( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	"(⟨1 2 3⟩±1 == ⟨1 2⟩±1)"
	,	[]( auto const& _ ){ A( IsNil( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	"(⟨1⟩±1 == ⟨1⟩±1)"
	,	[]( auto const& _ ){ A( IsT( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	"(⟨1⟩±1 == ⟨1⟩)"
	,	[]( auto const& _ ){ A( IsT( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	"(⟨1⟩ == ⟨1⟩±1)"
	,	[]( auto const& _ ){ A( IsT( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	"(⟨1⟩ == ⟨1⟩)"
	,	[]( auto const& _ ){ A( IsT( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	"(⟨1⟩ == ⟨2⟩)"
	,	[]( auto const& _ ){ A( IsNil( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	"(⟨2⟩ == ⟨1⟩)"
	,	[]( auto const& _ ){ A( IsNil( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	"(⟨1⟩ == ⟨1 2⟩)"
	,	[]( auto const& _ ){ A( IsNil( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	"(⟨1 2⟩ == ⟨1⟩)"
	,	[]( auto const& _ ){ A( IsNil( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	"(⟨1⟩ == 0)"
	,	[]( auto const& _ ){ A( IsNil( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	"(0==⟨1⟩)"
	,	[]( auto const& _ ){ A( IsNil( _ ) ); }
	);

	TestEvalException( C, "(⟨1 2⟩·⟨1 2 3⟩)", "The number of elements must mutch in Vector." );
	TestEval< Float >( C, "(⟨1 2 3⟩·⟨1 2 3⟩)", []( auto const& _ ){ A( _->$ == 14 ); } );

	TestEval< Matrix >(
		C
	,	"(⟨1 2⟩·⟨1 2⟩±1)"
	,	[]( auto const& _ ){
			A( _->size == 1 );
			A( _->nCols == 1 );
			A( _->$[ 0 ] == 5 );
		}
	);

	TestEval< Matrix >(
		C
	,	"(⟨1 2⟩±1·⟨1 2⟩)"
	,	[]( auto const& _ ){
			A( _->size == 4 );
			A( _->nCols == 2 );
			A( _->$[ 0 ] == 1 );
			A( _->$[ 1 ] == 2 );
			A( _->$[ 2 ] == 2 );
			A( _->$[ 3 ] == 4 );
		}
	);

	TestEvalException(
		C
	,	"(⟨1 2 3 4 5 6⟩±3·⟨1 2 3 4 5 6⟩±3)"
	,	"The number of columns in the left matrix must match the number of rows in the right matrix."
	);

	TestEval< Matrix >(
		C
	,	"(⟨1 2 3 4 5 6⟩±3·⟨1 2 3 4 5 6 7 8 9 10 11 12⟩±4)"
	,	[]( auto const& _ ){
			A( _->size == 8 );
			A( _->nCols == 4 );
			A( _->$[ 0 ] == 1 * 1 + 2 * 5 + 3 * 9 );
			A( _->$[ 1 ] == 1 * 2 + 2 * 6 + 3 * 10 );
			A( _->$[ 2 ] == 1 * 3 + 2 * 7 + 3 * 11 );
			A( _->$[ 3 ] == 1 * 4 + 2 * 8 + 3 * 12 );
			A( _->$[ 4 ] == 4 * 1 + 5 * 5 + 6 * 9 );
			A( _->$[ 5 ] == 4 * 2 + 5 * 6 + 6 * 10 );
			A( _->$[ 6 ] == 4 * 3 + 5 * 7 + 6 * 11 );
			A( _->$[ 7 ] == 4 * 4 + 5 * 8 + 6 * 12 );
		}
	);
}





//			( `{ "A":1, "B":[ 2, 3, 4 ], "C":{ "D":[ 4, 5, 6 ] } }`:byJSON:'C )



void
EvalTest( SP< Context > C ) {

	TestEval< Dict >(
		C
	,	R"( ( `{ "A":1, "B":[ 2, 3, 4 ], "C":{ "D":[ 4, 5, 6 ] } }`:byJSON:'@ ) )"
	,	[]( auto const& _ ) {
			cerr << _->REPR() << endl;
		}
	);
	TestEvalException( C, "( ``:byJSON )", "Invalid JSON" );
	TestEvalException( C, "( `hoge`:byJSON )", "Invalid JSON" );
	TestEvalException( C, "( `test`:byJSON )", "Invalid JSON" );
	TestEvalException( C, "( `full`:byJSON )", "Invalid JSON" );
	TestEvalException( C, "( `none`:byJSON )", "Invalid JSON" );
	TestEval< Bits >(
		C
	,	"( `\n\t\r +123`:byJSON )"
	,	[]( auto const& _ ) {
			A( _->$ == 123 );
		}
	);
	TestEval< List >( C, "(`[ null,1.23 ]`:byJSON)", []( auto const& _ ){ A( _->$.size() == 2 ); } );
	TestEval< List >( C, "(`[ null ,1.23 ]`:byJSON)", []( auto const& _ ){ A( _->$.size() == 2 ); } );

	TestEval< List >(
		C
	,	R"(
			( `[]`:byJSON )
		)"
	,	[]( auto const& _ ) {
			A( _->$.size() == 0 );
		}
	);
	TestEvalException(
		C
	,	R"( ( `[`: byJSON ) )"
	,	"Invalid JSON Array"
	);
	TestEvalException(
		C
	,	R"( ( `[ 123 123 ]`: byJSON ) )"
	,	"Invalid JSON Array"
	);
	TestEvalException(
		C
	,	R"( ( `{`: byJSON ) )"
	,	"Invalid JSON Object"
	);
	TestEvalException(
		C
	,	R"( ( `{ 123 `: byJSON ) )"
	,	"Invalid JSON Object"
	);
	TestEvalException(
		C
	,	R"( ( `{ "a"`: byJSON ) )"
	,	"Invalid JSON Object"
	);
	TestEvalException(
		C
	,	R"( ( `{ "a" 123 `: byJSON ) )"
	,	"Invalid JSON Object"
	);
	TestEvalException(
		C
	,	R"( ( `{ "a": 123 "b" `: byJSON ) )"
	,	"Invalid JSON Object"
	);
	TestEvalException(
		C
	,	R"( ( `{ "a": 123,`: byJSON ) )"
	,	"Invalid JSON Object"
	);
	TestEvalException(
		C
	,	R"( ( `"`: byJSON ) )"
	,	"Invalid JSON String"
	);
	TestEval< Literal >(
		C
	,	R"( ( `{ "a": 3.14, "b": "3.14","C":[123,"ABC"] }`: byJSON : toJSON ) )"
	,	[]( auto const& _ ) {
			A( _->$ == R"({"a":3.14,"b":"3.14","C":[123,"ABC"]})" );
		}
	);
	TestEval< Literal >(
		C
	,	R"( ( 'abc: toJSON ) )"
	,	[]( auto const& _ ) {
			A( _->$ == "" );
		}
	);
	TestEval< Literal >(
		C
	,	R"( ( [ 3.14 "3.14" ]: toJSON ) )"
	,	[]( auto const& _ ) {
			A( _->$ == "[3.14,\"3.14\"]" );
		}
	);
	TestEval< Literal >(
		C
	,	R"( ( "3.14": toJSON ) )"
	,	[]( auto const& _ ) {
			A( _->$ == "\"3.14\"" );
		}
	);
	TestEval< Literal >(
		C
	,	"( 3.14: toJSON )"
	,	[]( auto const& _ ) {
			A( _->$ == "3.14" );
		}
	);
	TestEval< Literal >(
		C
	,	"( 3: toJSON )"
	,	[]( auto const& _ ) {
			A( _->$ == "3" );
		}
	);

	TestEval< Dict >(
		C
	,	R"(
			( `{ "a": "abc", "b": 3.14 }`:byJSON )
		)"
	,	[]( auto const& _ ) {
			A( _->$.size() == 2 );
			A( Cast< Literal >( _->$[ "a" ] ) != nullptr );
			A( Cast< Float >( _->$[ "b" ] ) != nullptr );
		}
	);
	TestEval< List >(
		C
	,	R"(
			( `[ "abc", 3.14 ]`:byJSON )
		)"
	,	[]( auto const& _ ) {
			A( _->$.size() == 2 );
			A( Cast< Literal >( _->$[ 0 ] ) != nullptr );
			A( Cast< Float >( _->$[ 1 ] ) != nullptr );
		}
	);
	TestEval< Float >(
		C
	,	R"(
			( `3.14`:byJSON )
		)"
	,	[]( auto const& _ ) {
			A( _->$ == 3.14 );
		}
	);
	TestEval< Float >(
		C
	,	R"(
			( `19223372036854775808`:byJSON )
		)"
	,	[]( auto const& _ ) {
			A( _->$ == 19223372036854775808.0 );
		}
	);
	TestEval< Float >(
		C
	,	R"(
			( `9223372036854775808`:byJSON )
		)"
	,	[]( auto const& _ ) {
			A( _->$ == 9223372036854775808ULL );
		}
	);
	TestEval< Bits >(
		C
	,	R"(
			( `-9223372036854775808`:byJSON )
		)"
	,	[]( auto const& _ ) {
			A( _->$ == -9223372036854775807LL - 1 );
		}
	);
	TestEval< Literal >(
		C
	,	R"(
			( `"ABC"`:byJSON )
		)"
	,	[]( auto const& _ ) {
			A( _->$ == "ABC" );
		}
	);
	TestEval< Bits >(
		C
	,	R"(
			( `3`:byJSON )
		)"
	,	[]( auto const& _ ) {
			A( _->$ == 3 );
		}
	);
	TestEval< Float >(
		C
	,	R"(
			( `3.5`:byJSON )
		)"
	,	[]( auto const& _ ) {
			A( _->$ == 3.5 );
		}
	);

	TestEval< SliP >(
		C
	,	R"( ( `null`:byJSON ))"
	,	[]( auto const& _ ) { A( IsNil( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	R"( ( `true`:byJSON ))"
	,	[]( auto const& _ ) { A( IsT( _ ) ); }
	);
	TestEval< SliP >(
		C
	,	R"( ( `false`:byJSON ))"
	,	[]( auto const& _ ) { A( IsNil( _ ) ); }
	);

	TestEval< SliP >( C, "( 'a == 1 )" , []( auto const& _ ){ A( IsNil(_ ) ); } );
	TestEvalException( C, "( + 'a )", "Not a numeric" );
	TestEval< SliP >( C, "( 1 == 'a )" , []( auto const& _ ){ A( IsNil(_ ) ); } );
	TestEval< SliP >( C, "( 'a == 'b )" , []( auto const& _ ){ A( IsNil(_ ) ); } );
	TestEval< SliP >( C, "( 'b == 'a )" , []( auto const& _ ){ A( IsNil(_ ) ); } );
	TestEvalException( C, "( 0 ? [ 1 2 3 ] )", "rhs list must be 2 element" );
	TestEval< List >( C, "(3:'£)", []( auto const& _ ){ A( _->REPR() == "[ 3 ]" ); } );
	TestEval< SliP >( C, "(3:'(@==3))", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEvalException( C, "([255 1]:str)", "base must be 2..36" );
	TestEvalException( C, "([255 37]:str)", "base must be 2..36" );
	TestEval< Literal >( C, "(0:str)", []( auto const& _ ){ A( _->$ == "0" ); } );
	TestEval< Literal >( C, "(-1:str)", []( auto const& _ ){ A( _->$ == "(-1)" ); } );
	TestEval< Literal >( C, "(123:string)", []( auto const& _ ){ A( _->$ == "123" ); } );

	TestEval< Literal >( C, "(255:str)", []( auto const& _ ){ A( _->$ == "255" ); } );
	TestEval< Literal >( C, "([255 16]:str)", []( auto const& _ ){ A( _->$ == "ff" ); } );

	//	TODO: Convert exception
	TestEvalException( C, "([`ffffffffffffffffffffffffffffffff` 16]:int)", "stoll: out of range" );
	TestEvalException( C, "([`ff` 37]:int)", "stoll: no conversion" );

	TestEval< Bits >( C, "([`ff` 16]:int)", []( auto const& _ ){ A( _->$ == 255 ); } );
	TestEval< Bits >( C, "(`123`:int)", []( auto const& _ ){ A( _->$ == 123 ); } );

	TestEval< Float >( C, "(abs -3)", []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEval< Bits >( C, "(3'3)", []( auto const& _ ){ A( _->$ == 9 ); } );

	TestMatrix( C );

	TestDict( C );

	TestEval< Bits >( C, "(3:'@)", []( auto const& _ ){ A( _->REPR() == "3" ); } );

	TestEval< SliP >( C, "( π == π )", []( auto const& _ ){ A( IsT( _ ) ); } );

	TestEval< SliP >( C, "()", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< Quote >( C, "(')", []( auto const& _ ){ A( _->label == "'" ); } );

	TestEval< List >( C, "(1+'a)", []( auto const& _ ){ A( _->REPR() == "[ 1 a ]" ); } );
	TestEval< List >( C, "('a+1)", []( auto const& _ ){ A( _->REPR() == "[ a 1 ]" ); } );

	TestEval< Sentence >( C, "( '( 1 ) + '( 2 ) )", []( auto const& _ ){ A( _->REPR() == "( 1 2 )" ); } );
	TestEval< List >( C, "( '( 1 ) + 0 )", []( auto const& _ ){ A( _->REPR() == "[ ( 1 ) 0 ]" ); } );
	TestEval< List >( C, "( 0 + '( 1 ) )", []( auto const& _ ){ A( _->REPR() == "[ 0 ( 1 ) ]" ); } );

	TestEval< Procedure >( C, "( '{ 1 } + '{ 2 } )", []( auto const& _ ){ A( _->REPR() == "{ 1 2 }" ); } );
	TestEval< List >( C, "( '{ 1 } + 0 )", []( auto const& _ ){ A( _->REPR() == "[ { 1 } 0 ]" ); } );
	TestEval< List >( C, "( 0 + '{ 1 } )", []( auto const& _ ){ A( _->REPR() == "[ 0 { 1 } ]" ); } );

	TestEval< Parallel >( C, "( '« 1 » + '« 2 » )", []( auto const& _ ){ A( _->REPR() == "« 1 2 »" ); } );
	TestEval< List >( C, "( '« 1 » + 0 )", []( auto const& _ ){ A( _->REPR() == "[ « 1 » 0 ]" ); } );
	TestEval< List >( C, "( 0 + '« 1 » )", []( auto const& _ ){ A( _->REPR() == "[ 0 « 1 » ]" ); } );

	TestEval< List >( C, "( [ 1 ] + [ 2 ] )", []( auto const& _ ){ A( _->REPR() == "[ 1 2 ]" ); } );
	TestEval< List >( C, "( [ 1 ] + 0 )", []( auto const& _ ){ A( _->REPR() == "[ [ 1 ] 0 ]" ); } );
	TestEval< List >( C, "( 0 + [ 1 ] )", []( auto const& _ ){ A( _->REPR() == "[ 0 [ 1 ] ]" ); } );

	TestEval< SliP >( C, "( 0 ^^ [] )", []( auto const& _ ){ A( IsT( _ ) ); } );

	TestEval< Literal >( C, "(`a`+`b`)", []( auto const& _ ){ A( _->$ == "ab" ); } );
	TestEval< Literal >( C, "(`a`+\"b\")", []( auto const& _ ){ A( _->REPR() == "`ab`" ); } );
	TestEval< List >( C, "( `a` + 0 )", []( auto const& _ ){ A( _->REPR() == "[ `a` 0 ]" ); } );

	TestEval< Bits >( C, "(1+1)", []( auto const& _ ){ A( _->$ == 2 ); } );
	TestEval< Float >( C, "( 9223372036854775807+9223372036854775807 )", []( auto const& _ ){ A( _->$ == 9223372036854775808.0+9223372036854775807.0 ); } );

	TestEval< SliP >( C, "( [ 1 ] == [ 1 ] )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( [ 1 ] == [ 2 ] )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( [ 2 ] == [ 1 ] )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( [ 1 ] == [ 1 2 ] )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( [ 1 2 ] == 1 )", []( auto const& _ ){ A( IsNil( _ ) ); } );

	TestEval< SliP >( C, "( `1` == `1` )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( `1` == `2` )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( `2` == `1` )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( `2` == 1 )", []( auto const& _ ){ A( IsNil( _ ) ); } );

	TestEval< SliP >( C, "( `1` == `2` )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( `1` <> `2` )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( `1` < `2` )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( `1` > `2` )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( `1` <= `2` )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( `1` >= `2` )", []( auto const& _ ){ A( IsNil( _ ) ); } );

	TestEval< SliP >( C, "( `12` == `2` )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( `12` <> `2` )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( `12` < `2` )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( `12` > `2` )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( `12` <= `2` )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( `12` >= `2` )", []( auto const& _ ){ A( IsNil( _ ) ); } );

	TestEval< SliP >( C, "( `1` == `12` )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( `1` <> `12` )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( `1` < `12` )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( `1` > `12` )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( `1` <= `12` )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( `1` >= `12` )", []( auto const& _ ){ A( IsNil( _ ) ); } );

	TestEval< List >( C, "( 1, '« 2 3  » )", []( auto const& _ ){ A( _->REPR() == "« 1 2 3 »" ); } );
	TestEval< List >( C, "( 1, '( 2 3 ))", []( auto const& _ ){ A( _->REPR() == "( 1 2 3 )" ); } );
	TestEval< List >( C, "( 1, '{ 2 3 } )", []( auto const& _ ){ A( _->REPR() == "{ 1 2 3 }" ); } );
	TestEval< List >( C, "( 1, [ 2 3 ] )", []( auto const& _ ){ A( _->REPR() == "[ 1 2 3 ]" ); } );

	TestEval< Float >( C, "( -9223372036854775807-9223372036854775807 )", []( auto const& _ ){ A( _->$ == -9223372036854775808.0-9223372036854775807.0 ); } );
	TestEval< Bits >( C, "( 5 - 3 )", []( auto const& _ ){ A( _->$ == 2 ); } );
	TestEvalException( C, "( 5 - `3` )", "Illegal operand type: `3`" );

	TestEvalException( C, "( `5` % 3 )", "Illegal operand type: `5`" );
	TestEvalException( C, "( 5 % `3` )", "Illegal operand type: `3`" );
	TestEval< Bits >( C, "( 5 % 3 )", []( auto const& _ ){ A( _->$ == 2 ); } );
	TestEvalException( C, "( `5` / 3 )", "Illegal operand type: `5`" );
	TestEvalException( C, "( 5 / `3` )", "Illegal operand type: `3`" );
	TestEval< Bits >( C, "( 5 / 3 )", []( auto const& _ ){ A( _->$ == 1 ); } );

	TestEval< Bits >( C, "( 3:3 )", []( auto const& _ ){ A( _->$ == 3 ); } );
	
	TestEvalException( C, "( 3:'abc )", "Undefined name: abc" );
	extern SP< SliP > Pop();
	A( Z( "TESTING: Must be Bits: ", Cast< Bits >( Pop() ) )->$ == 3 );

	TestEval< SliP >( C, "( 2.0 == 1.0 )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 1 == `1` )", []( auto const& _ ){ A( IsNil( _ ) ); } );

	TestEval< SliP >( C, "( [ 1 2 ] == [ 1 2 ] )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( [ 1 2 ] <> [ 1 2 ] )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( [ 1 2 ] < [ 1 2 ] )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( [ 1 2 ] > [ 1 2 ] )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( [ 1 2 ] <= [ 1 2 ] )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( [ 1 2 ] >= [ 1 2 ] )", []( auto const& _ ){ A( IsT( _ ) ); } );

	TestEval< SliP >( C, "( [ 1 2 3 ] == [ 1 2 ] )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( [ 1 2 3 ] <> [ 1 2 ] )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( [ 1 2 3 ] < [ 1 2 ] )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( [ 1 2 3 ] > [ 1 2 ] )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( [ 1 2 3 ] <= [ 1 2 ] )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( [ 1 2 3 ] >= [ 1 2 ] )", []( auto const& _ ){ A( IsT( _ ) ); } );

	TestEvalException( C, "( 3 ÷ `a` )", "Illegal operand type: `a`" );

	TestEval< SliP >( C, "( 0∈[ 1 2 3 ] )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 1∈[ 1 2 3 ] )", []( auto const& _ ){ A( IsT( _ ) ); } );

	TestEval< SliP >( C, "( [ 1 2 3 ]∋0 )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( [ 1 2 3 ]∋1 )", []( auto const& _ ){ A( IsT( _ ) ); } );

	TestEval< SliP >( C, "( []¿'( 3 + 5 ) )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< Bits >( C, "( 1¿'( 3 + 5 ) )", []( auto const& _ ){ A( _->$ == 8 ); } );
	TestEval< Bits >( C, "( 1?[ 1 0 ] )", []( auto const& _ ){ A( _->$ == 1 ); } );
	TestEval< Bits >( C, "( []?[ 1 0 ] )", []( auto const& _ ){ A( _->$ == 0 ); } );

	TestEval< List >( C, "( 0 ^^ 1 )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< List >( C, "( [] ^^ 2 )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< List >( C, "( [] ^^ [] )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< List >( C, "( 0 || [] )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< List >( C, "( 0 || 1 )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< List >( C, "( [] || 2 )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< List >( C, "( [] || [] )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< List >( C, "( 0 && [] )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< List >( C, "( 0 && 1 )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< List >( C, "( [] && 2 )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< List >( C, "( [] && [] )", []( auto const& _ ){ A( IsNil( _ ) ); } );


	TestEvalException( C, "( 3 × `a` )", "Illegal operand type: `a`" );

	TestEval< List >( C, "( '[ 1 2 3 ]:¦ )", []( auto const& _ ){ A( _->REPR() == "[ 1 2 3 ]" ); } );
	TestEval< List >( C, "( '[ 1 2 3 ]:; )", []( auto const& _ ){ A( _->REPR() == "[ 1 2 3 ]" ); } );

	TestEval< Bits >( C, "( '« 1 2 3 »:$ )", []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEval< Bits >( C, "( '( 1 2 3 ):$ )", []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEval< Bits >( C, "( '{ 1 2 3 }:$ )", []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEval< Bits >( C, "( '[ 1 2 3 ]:$ )", []( auto const& _ ){ A( _->$ == 3 ); } );

	TestEvalException( C, "( 0:* )", "Illegal operand type" );

	TestEval< List >( C, "( '« 1 2 3 »:* )", []( auto const& _ ){ A( _->REPR() == "« 2 3 »" ); } );
	TestEval< List >( C, "( '( 1 2 3 ):* )", []( auto const& _ ){ A( _->REPR() == "( 2 3 )" ); } );
	TestEval< List >( C, "( '{ 1 2 3 }:* )", []( auto const& _ ){ A( _->REPR() == "{ 2 3 }" ); } );
	TestEval< List >( C, "( '[ 1 2 3 ]:* )", []( auto const& _ ){ A( _->REPR() == "[ 2 3 ]" ); } );

	TestEvalException( C, "( 0:# )", "Illegal operand type" );
	
	TestEval< Bits >( C, "( [ 1 2 3 ]:# )", []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEval< Bits >( C, "( `abc`:# )", []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEval< Literal >( C, "( 3:string )", []( auto const& _ ){ A( _->$ == "3" ); } );
	TestEval< Literal >( C, "( `abc`:string )", []( auto const& _ ){ A( _->$ == "`abc`" ); } );
	TestEval< SliP >( C, "( ¬0 )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( ¬[] )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< Bits >( C, "( ~1 )", []( auto const& _ ){ A( _->$ == 0xfffffffffffffffeL ); } );
	TestEval< Bits >( C, "( 1 & 2 )", []( auto const& _ ){ A( _->$ == 0 ); } );
	TestEval< Bits >( C, "( 1 ^ 3 )", []( auto const& _ ){ A( _->$ == 2 ); } );


	TestEval< Bits >( C, "('(3 + 5 ):!)", []( auto const& _ ){ A( _->$ == 8 ); } );
	TestEvalException( C, "(¡`LET ME THROW`)", "`LET ME THROW`" );

	TestEval< SliP >( C, "( 1.0 == 1.0 )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( 1.0 <> 1.0 )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 1.0 == 2.0 )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 1.0 <> 2.0 )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( 1.0 < 2.0 )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( 1.0 > 2.0 )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 1.0 <= 2.0 )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( 1.0 >= 2.0 )", []( auto const& _ ){ A( IsNil( _ ) ); } );

	TestEval< SliP >( C, "( 1 == 1 )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( 1 <> 1 )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 1 < 1 )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 1 > 1 )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 1 <= 1 )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( 1 >= 1 )", []( auto const& _ ){ A( IsT( _ ) ); } );

	TestEval< SliP >( C, "( 1 == 2 )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 1 <> 2 )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( 1 < 2 )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( 1 > 2 )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 1 <= 2 )", []( auto const& _ ){ A( IsT( _ ) ); } );
	TestEval< SliP >( C, "( 1 >= 2 )", []( auto const& _ ){ A( IsNil( _ ) ); } );

	TestEvalException( C, "( π × `a` )", "Illegal operand type: `a`" );

	TestEval< Float >(
		C
	,	"(9223372036854775807×9223372036854775807)"
	,	[]( auto const& _ ){
			A( _->Double() == (double)9223372036854775807 * (double)9223372036854775807 );
		}
	);
	
	TestEval< Float >( C, "(π×π)", []( auto const& _ ){ A( _->Double() == numbers::pi * numbers::pi ); } );


	{	auto _ = MS< Bits >( numeric_limits< int64_t >::min() );
		auto $ = Cast< Float >( _->Negate() );
		A( $->$ == 9223372036854775808.0 );
	}
	{	auto _ = MS< NumericConstant >( "" );
		try {
			_->Double();
		} catch ( exception const& e ) {
			A( strcmp( e.what(), "eh?" ) == 0 );
		}
	}
	TestEval< NumericConstant >( C, "∞"	, []( auto const& _ ){ A( _->Double() == numeric_limits< double >::infinity() ); } );
	TestEval< NumericConstant >( C, "𝑒"	, []( auto const& _ ){ A( _->Double() == numbers::e ); } );
	TestEval< NumericConstant >( C, "π"	, []( auto const& _ ){ A( _->Double() == numbers::pi ); } );
	TestEval< NumericConstant >( C, "log2e"	, []( auto const& _ ){ A( _->Double() == numbers::log2e ); } );
	TestEval< NumericConstant >( C, "log10e"	, []( auto const& _ ){ A( _->Double() == numbers::log10e ); } );
	TestEval< NumericConstant >( C, "ln2"	, []( auto const& _ ){ A( _->Double() == numbers::ln2 ); } );
	TestEval< NumericConstant >( C, "ln10"	, []( auto const& _ ){ A( _->Double() == numbers::ln10 ); } );
	TestEval< NumericConstant >( C, "γ"	, []( auto const& _ ){ A( _->Double() == numbers::egamma ); } );
	TestEval< NumericConstant >( C, "φ"	, []( auto const& _ ){ A( _->Double() == numbers::phi ); } );
	TestEval< NumericConstant >( C, "(-π)"	, []( auto const& _ ){ A( _->REPR() == "(-π)" ); } );
	TestEval< Bits >( C, "(-9223372036854775807)"	, []( auto const& _ ){ A( _->$ == -9223372036854775807L );	} );
	TestEval< Bits >( C, "9223372036854775807"		, []( auto const& _ ){ A( _->$ == 9223372036854775807L );	} );
	TestEval< Float >( C, "9223372036854775808"		, []( auto const& _ ){ A( _->$ == 9223372036854775808.0 );	} );
	TestEval< Float >(C,  "(-9223372036854775808)"	, []( auto const& _ ){ A( _->$ == -9223372036854775808.0 );	} );

	TestEval< Float >( C, "(+3.0)"			, []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEval< Float >( C, "(-3.0)"			, []( auto const& _ ){ A( _->$ == -3 ); } );

	TestEval< Bits >( C, "( 3 × 4 + 2 )"	, []( auto const& _ ){ A( _->$ == 14 ); } );


	TestEval< List >( C, "∅"					, []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< List >( C, "«0»"				, []( auto const& _ ){ A( Cast< Bits >( _->$[ 0 ] )->$ == 0 ); } );
	TestEval< List >( C, "{0}"				, []( auto const& _ ){ A( Cast< Bits >( _->$[ 0 ] )->$ == 0 ); } );

	TestEval< Bits >( C, "( 3 2 )"			, []( auto const& _ ){ A( _->$ == 6 ); } );
	TestEval< Bits >( C, "( 2 + 3 × 4 )"	, []( auto const& _ ){ A( _->$ == 14 ); } );
	
	TestEvalException( C, "(`a` `b`)"		, "Syntax Error: No numeric value: `a`" );
	TestEvalException( C, "(×3)"			, "Syntax Error: No left operand for infix operator: ×" );
	TestEvalException( C, "(3×)"			, "Syntax Error: No right operand for infix operator: ×" );

	TestEval< Bits >( C, "(+3)"				, []( auto const& _ ){ A( _->$ == 3 ); } );
	TestEval< Bits >( C, "(-3)"				, []( auto const& _ ){ A( _->$ == -3 ); } );

	TestEval< Float >( C, "( 1.2 - 1.3 )"	, []( auto const& _ ){
cerr << _->REPR() << endl;
		A( _->REPR() == "-0.1" );
	} );

	TestEval< Bits >( C, "( 1+ +1 )"		, []( auto const& _ ){ A( _->$ == 2 ); } );
	TestEval< Bits >( C, "( 1+ -1 )"		, []( auto const& _ ){ A( _->$ == 0 ); } );
	TestEval< Bits >( C, "( 1- +1 )"		, []( auto const& _ ){ A( _->$ == 0 ); } );
	TestEval< Bits >( C, "( 1- -1 )"		, []( auto const& _ ){ A( _->$ == 2 ); } );
	TestEval< Bits >( C, "( 1 )"			, []( auto const& _ ){ A( _->$ == 1 ); } );
	TestEval< Bits >( C, "( +1 )"			, []( auto const& _ ){ A( _->$ == 1 ); } );
	TestEval< Bits >( C, "( -1 )"			, []( auto const& _ ){ A( _->$ == -1 ); } );
	TestEval< Bits >( C, "( 1+1 )"			, []( auto const& _ ){ A( _->$ == 2 ); } );
	TestEval< Bits >( C, "( 1-1 )"			, []( auto const& _ ){ A( _->$ == 0 ); } );

	TestEval< Float >( C, "( 3 ÷ 5 )"		, []( auto const& _ ){ A( _->$ == 0.6 ); } );

	TestEval< Bits >( C, "( 3 + 5 )"		, []( auto const& _ ){ A( _->$ == 8 ); } );
	TestEval< Bits >( C, "( 3 × 5 )"		, []( auto const& _ ){ A( _->$ == 15 ); } );
	TestEval< Bits >( C, "( 3 - 5 )"		, []( auto const& _ ){ A( _->$ == -2 ); } );
	TestEval< Bits >( C, "( 7 / 3 )"		, []( auto const& _ ){ A( _->$ == 2 ); } );
	TestEval< Bits >( C, "( 7 % 3 )"		, []( auto const& _ ){ A( _->$ == 1 ); } );
	TestEval< Bits >( C, "( 12 ^ 10 )"		, []( auto const& _ ){ A( _->$ == 6 ); } );
	TestEval< Bits >( C, "( 12 | 10 )"		, []( auto const& _ ){ A( _->$ == 14 ); } );
	TestEval< Bits >( C, "( 12 & 10 )"		, []( auto const& _ ){ A( _->$ == 8 ); } );
	TestEval< SliP >( C, "( 0 < 1 )"		, []( auto const& _ ){ A( !IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 0 < 0 )"		, []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 0 <= 0 )"		, []( auto const& _ ){ A( !IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 1 <= 0 )"		, []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 1 > 0 )"		, []( auto const& _ ){ A( !IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 0 > 0 )"		, []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 0 >= 0 )"		, []( auto const& _ ){ A( !IsNil( _ ) ); } );
	TestEval< SliP >( C, "( 0 >= 1 )"		, []( auto const& _ ){ A( IsNil( _ ) ); } );

	TestEval< Bits >( C, "( 0 )"			, []( auto const& _ ){ A( _->$ == 0 ); } );
}


//	Eval name 'a = 3, a

