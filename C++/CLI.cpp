#include "SliP.hpp"

void
Main() {
	ReadEvalPrint(
		string(
			istreambuf_iterator<char>( cin )
		,	istreambuf_iterator<char>()
		)
	);
}


int
main( int argc, char* argv[] ) {
	Main();
}


