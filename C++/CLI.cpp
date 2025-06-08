#include "SliP.hpp"

shared_ptr< SliP >
ReadEvalPrint( const string& _ ) {
	return Print( Eval( make_shared< Context >(), Read( _ ) ) );
}

int
main() {
	ReadEvalPrint(
		string(
			istreambuf_iterator<char>( cin )
		,	istreambuf_iterator<char>()
		)
	);
}

