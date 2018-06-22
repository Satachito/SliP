//	Written by Satoru Ogura, Tokyo.
//
#include	"JP.h"
using namespace JP;
using namespace	std;

#include	<map>


struct
Object;

struct
Context {
	vector<Object*>
	stack;

	vector< map< string, Object* > >
	dicts;

	Context() {
		dicts.emplace_back();
		dicts.emplace_back( map< string, Object* >() );
		dicts.last[ "for" ] = new Builtin(
			[]( c, p, a ) {
				if let wArgs = a as? List, wArgs.m.count >= 2 {
					if let wList = wArgs.m[ 0 ] as? List {
						var	v = [ Object ]()
						for w in wList.m {
							c.Push( w )
							defer{ c.Pop() }
							v.append( try wArgs.m[ 1 ].Eval( c, p ) )
						}
						return List( v, wList.type )
					}
				}
				throw SliPError.RuntimeError( "\(a):for" )
			}
		)
*/
	}
};



struct
Object {
/*	void*	operator	new( size_t p )		{ return ::malloc( p ); }
	void	operator	delete( void* p )	_NOEXCEPT { free( p ); }
*/
	static	int	sAllocated;
	~			Object() { sAllocated--; }
				Object() { sAllocated++; }

	int		ref = 0;
	void	Ref() { ref++; }
	void	Unref() { ref--; }
	
	virtual	void
	Dump() = 0;

	virtual	Object*
	Eval( Context const&, function< void( string ) > ) {
		return this;
	}
};
int
Object::sAllocated = 0;

struct
Name : Object {
	string	u;
	Name( string const& p ) : u( p ) {}
	void
	Dump() { cout << "Name:" << u << endl; }
	Object*
	Eval( Context const& c, function< void( string ) >& p ) {
		for ( auto i = c.dicts.rbegin(); i != c.dicts.rend(); i++ ) {
			auto w = i->find( u );
			if ( w != i->end() ) return w->second;
		}
		throw "UndefinedName";
	}
};

struct
List : Object {
	vector<Object*>	u;
	List( vector<Object*> const& p ) : u( p ) {}
	virtual	void
	_Dump( const char* p ) {
		cout << '>' << p << endl;
		for ( auto i = u.begin(); i != u.end(); i++ ) (*i)->Dump();
		cout << '<' << p << endl;
	}
};

struct
Sentence : List {
	Sentence( vector<Object*> const& p ) : List( p ) {}
	void
	Dump() { _Dump( "Sentence" ); }
};

struct
Bracket : List {
	Bracket( vector<Object*> const& p ) : List( p ) {}
	void
	Dump() { _Dump( "Bracket" ); }
};

struct
Brace : List {
	Brace( vector<Object*> const& p ) : List( p ) {}
	void
	Dump() { _Dump( "Brace" ); }
};

struct
Paren : List {
	Paren( vector<Object*> const& p ) : List( p ) {}
	void
	Dump() { _Dump( "Paren" ); }
};

struct
Angle : List {
	Angle( vector<Object*> const& p ) : List( p ) {}
	void
	Dump() { _Dump( "Angle" ); }
};

struct
Number : Object {
	int	u;
	Number( int const& p ) : u( p ) {}
	void
	Dump() { cout << "Number:" << u << endl; }
};

struct
String : Object {
	string	u;
	String( string const& p ) : u( p ) {}
	void
	Dump() { cout << "String:" << u << endl; }
};


typedef	function< Object*( Object*, Object*, Object*, Object* ) >	OPERATOR;
struct
Operator : Object {
	UNICODE		uName;
	OPERATOR	u;
	Operator( UNICODE pName, OPERATOR p ) : uName( pName ), u( p ) {}
	void
	Dump() { cout << "Operator:" << uName << endl; }
};

Number*
ReadNumber( UnicodeStream& p ) {
	int	v = 0;
	while ( true ) {
		auto w = p.Read();
		if ( '0' <= w && w <= '9' ) v = v * 10 + w - '0';
		else {
			p.Unread( w );
			return new Number( v );
		}
	}
}

String*
ReadString( UnicodeStream& p ) {
	string	v = "";
	auto	wEscaped = false;
	while ( true ) {
		auto	w = p.Read();
		if ( wEscaped ) {
			wEscaped = false;
			switch ( w ) {
			case '0'	:	v += '\0';	break;
			case 't'	:	v += '\t';	break;
			case 'n'	:	v += '\n';	break;
			case 'r'	:	v += '\r';	break;
			default		:	v += w;		break;
			}
		} else {
			switch ( w ) {
			case '"'	:	return new String( v );
			case '\\'	:	wEscaped = true;	break;
			default		:	v += w;				break;
			}
		}
	}
}

Name*
ReadName( UnicodeStream& p ) {
	string	v;
	while ( true ) {
		auto	w = p.Read();
		switch ( w ) {
		case '_':
		case 'A':	case 'B':	case 'C':	case 'D':	case 'E':	case 'F':	case 'G':	case 'H':	case 'I':	case 'J':	case 'K':	case 'L':	case 'M':	case 'N':
		case 'O':	case 'P':	case 'Q':	case 'R':	case 'S':	case 'T':	case 'U':	case 'V':	case 'W':	case 'X':	case 'Y':	case 'Z':
		case 'a':	case 'b':	case 'c':	case 'd':	case 'e':	case 'f':	case 'g':	case 'h':	case 'i':	case 'j':	case 'k':	case 'l':	case 'm':	case 'n':
		case 'o':	case 'p':	case 'q':	case 'r':	case 's':	case 't':	case 'u':	case 'v':	case 'w':	case 'x':	case 'y':	case 'z':
		case '0':	case '1':	case '2':	case '3':	case '4':	case '5':	case '6':	case '7':	case '8':	case '9':
			v += w;
			break;
		default:
			p.Unread( w );
			break;
		}
	}
	return new Name( v );
}

Object*
Read( UnicodeStream& p, UNICODE pTerminator );

vector<Object*>
ReadObjects( UnicodeStream& p, UNICODE pTerminator ) {
	vector<Object*> v;
	while ( Object* w = Read( p, pTerminator ) ) v.emplace_back( w );
	return v;
	
}

Sentence*
ReadSentence( UnicodeStream& p ) {
	return new Sentence( ReadObjects( p, ';' ) );
}
Bracket*
ReadBracket( UnicodeStream& p ) {
	return new Bracket( ReadObjects( p, ']' ) );
}
Brace*
ReadBrace( UnicodeStream& p ) {
	return new Brace( ReadObjects( p, '}' ) );
}
Paren*
ReadParen( UnicodeStream& p ) {
	return new Paren( ReadObjects( p, ')' ) );
}
Angle*
ReadAngle( UnicodeStream& p ) {
	return new Angle( ReadObjects( p, '>' ) );
}

Object*
Read( UnicodeStream& p, UNICODE pTerminator ) {
	while ( true ) {
		auto	w = p.Read();
		if ( w == pTerminator ) return 0;
		switch ( w ) {
		case ' ':	break;
		case ']':	throw "Read ] without [";
		case '}':	throw "Read } without {";;
		case ')':	throw "Read ) without (";;
		case '>':	throw "Read > without <";;
		case '[':	return ReadBracket( p );
		case '{':	return ReadBrace( p );
		case '(':	return ReadParen( p );
		case '<':	return ReadAngle( p );
		case '"':	return ReadString( p );
		case '0':	case '1':	case '2':	case '3':	case '4':	case '5':	case '6':	case '7':	case '8':	case '9':
			p.Unread( w );
			return ReadNumber( p );
		case '+':	return new Operator(
				w
			,	[]( Object* c, Object* p, Object* l, Object* r ) -> Object* {
					{	auto	wL = dynamic_cast<Number*>( l );
						auto	wR = dynamic_cast<Number*>( r );
						if ( wL && wR ) return new Number( wL->u + wR->u );
					}
					{	auto	wL = dynamic_cast<String*>( l );
						auto	wR = dynamic_cast<String*>( r );
						if ( wL && wR ) return new String( wL->u + wR->u );
					}
					{	auto	wL = dynamic_cast<Bracket*>( l );
						auto	wR = dynamic_cast<Bracket*>( r );
						if ( wL && wR ) return new Bracket( wL->u + wR->u );
					}
					throw "Add";
				}
			);
		case '-':	return new Operator(
				w
			,	[]( Object* c, Object* p, Object* l, Object* r ) -> Object* {
					{	auto	wL = dynamic_cast<Number*>( l );
						auto	wR = dynamic_cast<Number*>( r );
						if ( wL && wR ) return new Number( wL->u - wR->u );
					}
					throw "Sub";
				}
			);
		case '/': 	case '%': 	case '=':
		case '|':	case ',':
		case U'×':	case U'÷':	case U'·':	case U'¬':	case U'«':	case U'»':	case U'¡':	case U'¿':	case U'¦':	case U'¯':
		case '\'':
			return new Number( w );
		case '_':
		case 'A':	case 'B':	case 'C':	case 'D':	case 'E':	case 'F':	case 'G':	case 'H':	case 'I':	case 'J':	case 'K':	case 'L':	case 'M':	case 'N':
		case 'O':	case 'P':	case 'Q':	case 'R':	case 'S':	case 'T':	case 'U':	case 'V':	case 'W':	case 'X':	case 'Y':	case 'Z':
		case 'a':	case 'b':	case 'c':	case 'd':	case 'e':	case 'f':	case 'g':	case 'h':	case 'i':	case 'j':	case 'k':	case 'l':	case 'm':	case 'n':
		case 'o':	case 'p':	case 'q':	case 'r':	case 's':	case 't':	case 'u':	case 'v':	case 'w':	case 'x':	case 'y':	case 'z':
			p.Unread( w );
			return ReadName( p );
		default:
			throw "Invalid character";
		}
	}
}
/*
Object*
Read( UnicodeStream& p, UNICODE pTerminator ) {
	auto v = _Read( p, pTerminator );
	if ( v ) v->Dump();
	return v;
}
*/

int
main( int argc, const char * argv[] ) {

	cerr << "SliP ver 0.2(Swift) 2016 written by Satoru Ogura, Tokyo.×÷¡¿·¬«»¦¯" << endl;

	Context			wContext( []( int p ) { printf( "%x\n", p ); } );
	PreProcessor	wUS;
	try {
		while ( true ) {
			Object* w = ReadSentence( wUS );
			if ( !w ) break;
			w->Eval( wContext )->Dump();
		}
	} catch ( const char* e ) {
		cerr << e << endl;
	}
	return 0;
}
