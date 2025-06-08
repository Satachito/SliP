#include "SliP.hpp"

int
main() {
	ReadEvalPrint(
		string(
			istreambuf_iterator<char>( cin )
		,	istreambuf_iterator<char>()
		)
	);
}

