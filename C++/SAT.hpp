#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <regex>
#include <functional>
#include <memory>
#include <limits>
#include <ranges>
#include <algorithm>
#include <numbers>

using namespace std;
using namespace views;

////////////////////////////////////////////////////////////////

#define	Cast	dynamic_pointer_cast

////////////////////////////////////////////////////////////////

template < typename T > vector< T >
concat( const vector< T >& l, const vector< T >& r ) {
	vector<T> $;
	$.reserve( l.size() + r.size() );
	$.insert( $.end(), l.begin(), l.end() );
	$.insert( $.end(), r.begin(), r.end() );
	return $;
}

template < typename Range, typename T > bool
contains( const Range& range, const T& value ) {
	return ranges::find( range, value ) != ranges::end( range );
}

////////////////////////////////////////////////////////////////

string
string_char32s( const vector< char32_t >& char32s ) {

	string $;

	for( char32_t _ : char32s ) {
		if ( _ <= 0x7F ) {
			$ += static_cast<char>( _ );
		} else if ( _ <= 0x7FF ) {
			$ += static_cast<char>( 0b11000000 | ( _ >> 6) );
			$ += static_cast<char>( 0b10000000 | ( _ & 0b00111111 ) );
		} else if ( _ <= 0xFFFF ) {
			$ += static_cast<char>( 0b11100000 | ( _ >> 12 ) );
			$ += static_cast<char>( 0b10000000 | ( ( _ >> 6 ) & 0b00111111 ) );
			$ += static_cast<char>( 0b10000000 | ( _ & 0b00111111 ) );
		} else if ( _ <= 0x10FFFF ) {
			$ += static_cast<char>( 0b11110000 | ( _ >> 18 ) );
			$ += static_cast<char>( 0b10000000 | ( ( _ >> 12 ) & 0b00111111 ) );
			$ += static_cast<char>( 0b10000000 | ( ( _ >> 6 ) & 0b00111111 ) );
			$ += static_cast<char>( 0b10000000 | ( _ & 0b00111111 ) );
		} else {
			$ += '?';
		}
	}

	return $;
}

bool
IsDigit( char32_t _ ) {
	return u'0' <= _ && _ <= u'9';
}

