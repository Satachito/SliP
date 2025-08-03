#undef	DEBUG
#define	DEBUG

#include "../C++/SliP.hpp"

#include <cassert>

extern SP< SliP >
Eval( SP< Context >, SP< SliP > );

extern SP< SliP >
Read( iReader&, char32_t );

static auto
READ( const string& _ ) {
	StringReader R( _ );
	return Read( R, -1 );
};

void
ExtraTest() {

//	auto numeric = MS< Bits >( 3 );
//	auto nega = numeric->Negate();
//	A( nega->Double() == -3 );
	

	extern SP< SliP >	T;
	A( T->REPR() == "SliP" );
	extern bool _Compare( SP< SliP >, SP< SliP > );
	A( _Compare( T, T ) == 0 );

	extern SP< SliP >	Nil;
	A( Nil->REPR() == "[]" );

//	auto reader = new StringReader( "abc" );
//	iReader* i = reader;
//	assert( i->Avail() );
//	assert( i->Peek() == 'a' );
//	assert( i->Read() == 'a' );
//	delete reader;
	
//	auto um = new UM< string, string >{};
//	delete um;
//	auto us = new US< string >{};
//	delete us;
}

extern V< SP< SliP > >
theStack;

int
main( int argc, char* argv[] ) {

	extern void Build();
	Build();

	ExtraTest();
	
	void ReadTest();
	ReadTest();

	auto
	C = MS< Context >();
	
	try {
		theStack.clear();

		{	auto
			C2 = MS< Context >( C );


			cerr << Eval( C2, READ(
R"(	(   'forX = '(
			@.0:# > 0 ¿ '{
				( @.0 .0 :(@.1) )
				( { ( @.0:* ) (@.1) }:forX )
			}
		)
	)
)"
			) )->REPR() << endl;

			cerr << Eval( C2, READ( "( [ [ a b c ] ( @:¦ ) ]:forX )" ) )->REPR() << endl;



			Eval(
				C2
			,	READ(
R"(
( 'member = '(
    @.1:# == 0 ? [ 
        []  
        (   @.0 == @.1 .0 ? [ 
                ( @.0 )
                ( { (@.0) (@.1:*) }:member )
            ]   
        )   
    ]   
) )
)"
				)
			);
			cerr << Eval( C2, READ( "( [ a [ a b c ] ]:member )" ) )->REPR() << endl;
			cerr << Eval( C2, READ( "( [ b [ a b c ] ]:member )" ) )->REPR() << endl;
			cerr << Eval( C2, READ( "( [ c [ a b c ] ]:member )" ) )->REPR() << endl;
			cerr << Eval( C2, READ( "( [ d [ a b c ] ]:member )" ) )->REPR() << endl;
			cerr << Eval( C2, READ( "( [ a [] ]:member )" ) )->REPR() << endl;
				
			Eval(
				C2
			,	READ(
					R"(	(   'reduce = '(
								@.0:# == 1 ? [ 
									( @.0 .0 )
									(	«	( @.0 .0 )
											( « ( @.0:* ) ( @.1 ) »:reduce )
										»:@.1
									)
								]   
							)   
						)
					)"
				)
			);
			cerr << Eval( C2, READ( "( [ [ 1 ]( @.0 + @.1 ) ]:reduce )" ) )->REPR() << endl;
			cerr << Eval( C2, READ( "( [ [ 1 2 ]( @.0 + @.1 ) ]:reduce )" ) )->REPR() << endl;
			cerr << Eval( C2, READ( "( [ [ 1 2 3 ]( @.0 + @.1 ) ]:reduce )" ) )->REPR() << endl;
			cerr << Eval( C2, READ( "( [ [ 1 2 3 4 ]( @.0 + @.1 ) ]:reduce )" ) )->REPR() << endl;
			cerr << Eval( C2, READ( "( [ [ 1 2 3 4 ]( @.0 × @.1 ) ]:reduce )" ) )->REPR() << endl;
			
			Eval( C2, READ( "( 'factorial = '( @ == 1 ? [ 1 ( @ × ( @ - 1 ):factorial ) ] ) )" ) );
			cerr << Eval( C2, READ( "( 4 : factorial )" ) )->REPR() << endl;
			Eval( C2, READ( "( 'ZOT = '( @ < 2 ? [ @ 2 ] ) )" ) );	//	Zero One Two
			Eval( C2, READ( "( 'fib2 = '( ( [ 0 1 ( ( @ - 1 ):fib2 + ( @ - 2 ):fib2 ) ].(@:ZOT) ):! ) )" ) );
			cerr << Eval( C2, READ( "( 0 : fib2 )" ) )->REPR() << endl;
			cerr << Eval( C2, READ( "( 1 : fib2 )" ) )->REPR() << endl;
			cerr << Eval( C2, READ( "( 2 : fib2 )" ) )->REPR() << endl;
			cerr << Eval( C2, READ( "( 3 : fib2 )" ) )->REPR() << endl;
			cerr << Eval( C2, READ( "( 4 : fib2 )" ) )->REPR() << endl;
			cerr << Eval( C2, READ( "( 10 : fib2 )" ) )->REPR() << endl;

			cerr << Eval( C2, READ(
R"(	( 'MAX = '(
	    @.0 > @.1 ? [ ( @.0 ) ( @.1 ) ] 
	) )
)"
			) )->REPR() << endl;


		}
		
		


		void MathTest( SP< Context > );
		MathTest( C );
		
		void EvalTest( SP< Context > );
		EvalTest( C );

		cerr << "Stack depth: " << theStack.size() << endl;

		theStack.clear();
		try {
			extern SP< SliP > Pop();
			Pop();
		} catch( exception const& e ) {
			A( e.what() == string( "Stack underflow" ) );
		}
		try {
			extern SP< SliP > Top();
			Top();
		} catch( exception const& e ) {
			A( e.what() == string( "Stack underflow" ) );
		}

		_Z( "TESTING ENDS" );
	} catch ( const exception& e ) {
		cerr << e.what() << endl;
	}
}
