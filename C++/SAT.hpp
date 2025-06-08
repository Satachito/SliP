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

template < typename T > auto
concat( const vector< T >& l, const vector< T >& r ) {
	vector<T> $;
	$.reserve( l.size() + r.size() );
	$.insert( $.end(), l.begin(), l.end() );
	$.insert( $.end(), r.begin(), r.end() );
	return $;
}

template < typename Range, typename T > auto
contains( const Range& range, const T& value ) {
	return ranges::find( range, value ) != ranges::end( range );
}

template < typename T, typename F > auto
filter( const vector< T >& vec, F func ) {
    vector< T > $;
    $.reserve( vec.size() );
    for( const auto& _ : vec ) if( func( _ ) ) $.push_back( _ );
    return $;
}

template < typename T, typename F > auto
map( const vector< T >& vec, F func ) {
    vector< decltype( func( declval< T >() ) ) > $;
    $.reserve( vec.size() );
    for( const auto& _ : vec ) $.push_back( func( _ ) );
    return $;
}

template < typename T, typename F > auto
mapWithIndex( const vector< T >& vec, F func ) {
    vector< decltype( func( declval< T >(), declval< size_t >() ) ) > $;
    $.reserve( vec.size() );
    for( size_t _ = 0; _ < vec.size(); ++_ ) $.push_back( func( vec[ _ ], _ ) );
    return $;
}

template < typename T, typename F > auto
sliceFrom( const vector< T >& vec, size_t from ) {
    vector< T > $;
    $.reserve( vec.size() - from );
    for( size_t _ = from; _ < vec.size(); ++_ ) $.push_back( vec[ _ ] );
    return $;
}

template < typename T, typename F > auto
sliceTo( const vector< T >& vec, size_t to ) {
    vector< T > $;
    $.reserve( to );
    for( size_t _ = 0; _ < to; ++_ ) $.push_back( vec[ _ ] );
    return $;
}
////////////////////////////////////////////////////////////////

inline auto
string_char32( char32_t _ ) {

	string $;

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
	return $;
}

inline auto
string_char32s( const vector< char32_t >& char32s ) {
	string $;
	for( char32_t _ : char32s ) $ += string_char32( _ );
	return $;
}

inline auto
IsDigit( char32_t _ ) {
	return u'0' <= _ && _ <= u'9';
}

