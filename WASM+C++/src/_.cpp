

#include <iostream>
using namespace std;

#include "SliP.hpp"
int
main() {
	cout << Integer64( 100 ).REP() << endl;
	cout << Float( 100 ).REP() << endl;
	cout << Literal( "abc", u'"' ).REP() << endl;
	cout << Name( "abc" ).REP() << endl;

	cout << Read( "abc" )->REP() << endl;
}
