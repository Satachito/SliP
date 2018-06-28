//	Written by Satoru Ogura, Tokyo. 2015 -
//

import Foundation

enum
SliPError	:	Error {
	case	ReadError		( String )
	case	RuntimeError	( String )
	case	UserException
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Context {
	var
	printer			= { ( p: String ) in print( p ) }

	var
	dicts			: Cell< [ String: Object ] >?

	var
	stack			: Cell< Object >?
	func
	Push(	_ o		: Object ) {
		stack = Cell( o, stack )
	}
	func
	Pop() {
		guard stack != nil else { fatalError() }
		stack = stack!.next
	}
	
	init(	parent	: Context ) {
		printer = parent.printer
		dicts = Cell( [ String: Object ](), parent.dicts )
		stack = parent.stack
	}
	
	init( printer p	: @escaping ( String ) -> () ) {
		printer = p
		dicts = Cell( SliPBuiltins() )
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Object				 {
	func
	Eval(	_ c		: Context ) throws -> Object { return self }
	var
	description		: String { return "" }
	var
	debugDescription: String { return description }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Name				: Object {
	let
	m				: String
	init(	_ p		: String ) { m = p }
	override var
	debugDescription: String { return m }
	override func
	Eval(	_ c		: Context ) throws -> Object {
		var	wAL = c.dicts
		while let w = wAL {
			if let v = w.m[ m ] { return v }
			wAL = w.next
		}
		throw SliPError.RuntimeError( "UndefinedName \(m)" )
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Numeric				: Object {
	var
	v				: Float64 { fatalError() }
}

class
IntNumber			: Numeric {
	let
	m				: BigInteger
	init(	_ p		: BigInteger ) { m = p }
	init(	_ p		: Int ) { m = MakeBigInteger( p ) }
	override var
	description		: String { return m.description }
	override var
	v				: Float64 { return m.NativeFloat }
}
class
RealNumber			: Numeric {
	let
	m				: Float64
	init(	_ p		: Float64 ) { m = p }
	override var
	description		: String { return "\(m)" }
	override var
	v				: Float64 { return m }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
StringL				: Object {
	let
	m				: String
	init(	_ p		: String ) { m = p }
	override var
	description		: String { return m }
	override var
	debugDescription: String { return "\"" + m + "\"" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Primitive			: Object {
	var
	name			: String
	var
	m				: ( Context ) throws -> Object
	init(	_ pName : String, _ p: @escaping ( Context ) throws -> Object ) { name = pName; m = p }
	override var
	debugDescription: String { return name }
	override func
	Eval(	_ c		: Context ) throws	->	Object { return try m( c ) }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Unary				: Object {
	var
	name			: String
	var
	m				: ( Context, Object ) throws -> Object
	init(	_ pName : String, _ p: @escaping ( Context, Object ) throws -> Object ) { name = pName; m = p }
	override var
	debugDescription: String { return name }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Dyadic				: Object {
	var
	name			: String
	var
	priority		: Int
	var
	m				: ( Context, Object, Object ) throws -> Object
	init(	_ pName	: String, _ pPriority: Int, _ p: @escaping ( Context, Object, Object ) throws -> Object ) { name = pName; priority = pPriority; m = p }
	override var
	debugDescription: String { return name + "«\(priority)»" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Quote				: Object {
	let
	m				: Object
	init(	_ p		: Object ) { m = p }
	override var
	debugDescription: String { return "'" + m.debugDescription }
	override func
	Eval(	_ c		: Context ) throws	->	Object { return m }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Combiner			: Object {
	let
	m				: Object
	init(	_ p		: Object ) { m = p }
	override var
	debugDescription: String { return "`" + m.debugDescription }
	override func
	Eval(	_ c		: Context ) throws	-> Object {
		guard let wDicts = c.dicts else { fatalError() }
		return Assoc( m, wDicts.m )
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Assoc				: Object {
	let
	m				: Object
	let
	dict			: [ String: Object ]
	init(	_ p		: Object, _ pDict: [ String: Object ] ) { m = p; dict = pDict }
	override var
	debugDescription: String { return "<\(dict):\(m)>" }
	override func
	Eval(	_ c		: Context ) throws	->	Object {
		c.dicts = Cell< [ String: Object ] >( dict, c.dicts )
		defer{ c.dicts = c.dicts!.next }
		return try m.Eval( c )
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Dict				: Object {
	let
	m				: [ String: Object ]
	init(	_ p		: [ String: Object ] ) { m = p }
	override var
	debugDescription: String { return "<\(m)>" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class
List				: Object {
	static	let
	object			= Object()

	let
	m				: [ Object ]

	init(	_ p		: [ Object ] ) { m = p }
	
	override	var
	debugDescription: String {
 		var	v = " "
		for w in m { v += w.debugDescription + " " }
		return v
	}
}

class
ListL				: List {

	override
	init(	_ p		: [ Object ] ) { super.init( p ) }
	override	var
	debugDescription: String { return "[" + super.debugDescription + "]" }
}

class
Parallel			: List {
	override
	init(	_ p		: [ Object ] ) { super.init( p ) }
	override	var
	debugDescription: String { return "«" + super.debugDescription + "»" }

	override	func
	Eval(	_ c		: Context ) throws -> Object {
		var	v = [ Object ]( repeating: List.object, count: m.count )
		let	wG = DispatchGroup()
		let	wQ = DispatchQueue.global()
		
		for ( i, w ) in m.enumerated() {
			wQ.async( group: wG ) {
				do {
					v[ i ] = try w.Eval( Context( parent: c ) )
				} catch let e {
					print( e )
				}
			}
		}
		wG.wait()
		for w in m {
			if w === List.object { fatalError() }
		}
		return Parallel( v )
	}
}

class
Sentence			: List {
	override
	init(	_ p		: [ Object ] ) { super.init( p ) }
	override	var
	debugDescription: String { return "(" + super.debugDescription + ")" }
	override	func
	Eval(	_ c		: Context ) throws -> Object {
		return try Sentence.Eval( c, ArraySlice( m ) )
	}
	static	func
	Eval(	_ c		: Context, _ p: ArraySlice< Object > ) throws -> Object {
		if p.count == 1 { return try p[ p.startIndex ].Eval( c ) }

		var wTarget :	Dyadic?
		var wIndex  =	0
		for i in 1 ..< p.count - 1 {
			if let w = p[ p.startIndex + i ] as? Dyadic {
				if let wCurrent = wTarget {
					if w.priority >= wCurrent.priority {
						wTarget = w
						wIndex = p.startIndex + i
					}
				} else {
					wTarget = w
					wIndex = p.startIndex + i
				}
			}
		}
		if let w = wTarget {
/*
func
DebStr( _ p: ArraySlice< Object > ) -> String {
	var	v = "["
	for w in p { v += " " + w.debugDescription }
	return v + " ]"
}
print(
	c.stack != nil ? c.stack!.m.debugDescription : "NIL"
,	w.debugDescription
,	DebStr( p[ p.startIndex ..< wIndex ] )
,	DebStr( p[ wIndex + 1 ..< p.startIndex + p.count ] )
//,	try Eval( c, p[ p.startIndex ..< wIndex ] )
//,	try Eval( c, p[ wIndex + 1 ..< p.startIndex + p.count ] )
)
*/
			let v = try w.m( c, Eval( c, p[ p.startIndex ..< wIndex ] ), Eval( c, p[ wIndex + 1 ..< p.startIndex + p.count ] ) )
//print( ">>", v.debugDescription )
			return v
		} else {
			throw SliPError.RuntimeError( "No operator in \(Sentence( Array( p ) ) )" )
		}
	}
}

class
Procedure			: List {
	override
	init(	_ p		: [ Object ] ) { super.init( p ) }
	override	var
	debugDescription: String { return "{" + super.debugDescription + "}" }
	override	func
	Eval(	_ c		: Context ) throws -> Object {
		let	wContext = Context( parent: c )
		var	v = [ Object ]( repeating: List.object, count: m.count )
		for ( i, w ) in m.enumerated() { v[ i ] = try w.Eval( wContext ) }
		return Parallel( v )
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
func
IsNil( _ o: Object ) -> Bool {
	if let w = o as? List { return w.m.count == 0 }
	return false
}

let
T	=	StringL( "T" )

let
Nil	=	ListL( [] )

extension
Object: Equatable {
	static func
	== ( l: Object, r: Object ) -> Bool {
		if let wL = l as? IntNumber	, let wR = r as? IntNumber	{ return wL.m == wR.m }
		if let wL = l as? Numeric	, let wR = r as? Numeric	{ return wL.v == wR.v }
		if let wL = l as? StringL	, let wR = r as? StringL	{ return wL.m == wR.m }
		if let wL = l as? Name		, let wR = r as? Name		{ return wL.m == wR.m }
		if let wL = l as? ListL		, let wR = r as? ListL		{ return wL.m == wR.m }
		if let wL = l as? Procedure	, let wR = r as? Procedure	{ return wL.m == wR.m }
		if let wL = l as? Parallel	, let wR = r as? Parallel	{ return wL.m == wR.m }
		if let wL = l as? Sentence	, let wR = r as? Sentence	{ return wL.m == wR.m }
		return false
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

func
ReadNumber( _ r: Reader< UnicodeScalar >, _ neg: Bool = false ) throws -> Object {
 	var	v = ""
	var	wRealF = false
	while true {
		let u = try r.Read()
		switch u {
		case	( "0"..."9" ), "+", "-":
			v.append( Character( u ) )
		case	".", "e", "E":
			if wRealF {
				guard let w = Float64( v ) else { fatalError() }
				r.Unread( u )
				return RealNumber( neg ? -w : w )
			}
			v.append( Character( u ) )
			wRealF = true
		default:
			r.Unread( u )
			if wRealF	{
				guard let w = Float64( v ) else { fatalError() }
				return RealNumber( neg ? -w : w )
			} else {
				return IntNumber( MakeBigInteger( v, 10, neg ) )
			}
		}
	}
}

func
ReadName( _ r: Reader< UnicodeScalar > ) throws -> Object {
	var	v = ""
	while true {
		let u = try r.Read()
		switch u {
		case ( "0" ... "9" ), ( "a" ... "z" ), ( "A" ... "Z" ), "_":
			v.append( Character( u ) )
		default:
			r.Unread( u )
			return Name( v )
		}
	}
}

func
ReadStr( _ r: Reader< UnicodeScalar > ) throws -> StringL {
	var	v = ""
	var	wEscaped = false
	while true {
		let u = try r.Read()
		if wEscaped {
			wEscaped = false
			switch u {
			case "0"	:	v.append( Character( "\0" ) )
			case "t"	:	v.append( Character( "\t" ) )
			case "n"	:	v.append( Character( "\n" ) )
			case "r"	:	v.append( Character( "\r" ) )
			default		:	v.append( Character( u ) )
			}
		} else {
			switch u {
			case "\""	:	return StringL( v )
			case "\\"	:	wEscaped = true
			default		:	v.append( Character( u ) )
			}
		}
	}
}

func
ReadObjects( _ r: Reader< UnicodeScalar >, _ terminator: UnicodeScalar ) throws -> [ Object ] {
//print( terminator )
	var	v = [ Object ]()
	while let w = try Read( r, terminator ) {
		v.append( w )
	}
//print( v )
	return v
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

let	UserException	=	Primitive( "¡" ) { c in throw SliPError.UserException }
let	StackTop		=	Primitive( "@" ) { c in
							guard let w = c.stack else { throw SliPError.RuntimeError( "StackUnderflow" ) }
							return w.m
						}
let	CurretDict		=	Primitive( "·" ) { c in
							guard let w = c.dicts else { fatalError() }
							return Dict( w.m )
						}

let	Eval			=	Unary( "!" ) { c, p in try p.Eval( c ) }
let	Count			=	Unary( "#" ) { c, p in
							guard let w = p as? List else { throw SliPError.RuntimeError( "\(p):#" ) }
							return IntNumber( w.m.count )
						}
let	Cdr				=	Unary( "*" ) { c, p in
							guard let w = p as? List, w.m.count > 0 else { throw SliPError.RuntimeError( "\(p):*" ) }
							switch w {
							case is ListL		: return ListL		( Array( w.m.dropFirst() ) )
							case is Parallel	: return Parallel	( Array( w.m.dropFirst() ) )
							case is Sentence	: return Sentence	( Array( w.m.dropFirst() ) )
							case is Procedure	: return Procedure	( Array( w.m.dropFirst() ) )
							default				: fatalError()
							}
						}
let	Last			=	Unary( "$" ) { c, p in
							guard let w = p as? List, let v = w.m.last else { throw SliPError.RuntimeError( "\(p):$" ) }
							return v
						}
let	Print			=	Unary( "." ) { c, p in
							c.printer( p.description )
							return p
						}
let	Dump			=	Unary( "¦" ) { c, p in
							c.printer( p.debugDescription + "\n" )
							return p
						}

let	Apply			=	Dyadic( ":", 0 ) { c, l, r in
							switch r {
							case let wR as IntNumber:
								guard let wL = l as? List else { throw SliPError.RuntimeError( "Index operation \(r) to \(l)" ) }
								let wIndex = wR.m.NativeInt
								return wIndex < wL.m.count ? wL.m[ wL.m.startIndex + wIndex ] : Nil
							case let wR as Name:
								guard let wL = l as? Dict else { throw SliPError.RuntimeError( "Dict operation \(r) to \(l)" ) }
								return wL.m[ wR.m ] ?? Nil
							case let wR as Unary:
								return try wR.m( c, l )
							case is List, is Assoc:
								c.Push( l )
								defer{ c.Pop() }
								return try r.Eval( c )
							default:
								throw SliPError.RuntimeError( "\(l) : \(r)" )
							}
						}
let	EQ				=	Dyadic( "==", 8 ) { c, l, r in l == r ? T : Nil }
let	NE				=	Dyadic( "<>", 8 ) { c, l, r in l != r ? T : Nil }
let	LT				=	Dyadic( "<", 8 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m < wR.m ? T : Nil }
							if let wL = l as? Numeric, let wR = r as? Numeric { return wL.v < wR.v ? T : Nil }
							throw SliPError.RuntimeError( "\(l) < \(r)" )
						}
let	GT				=	Dyadic( ">", 8 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m > wR.m ? T : Nil }
							if let wL = l as? Numeric, let wR = r as? Numeric { return wL.v > wR.v ? T : Nil }
							throw SliPError.RuntimeError( "\(l) > \(r)" )
						}
let	LE				=	Dyadic( "<=", 8 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m <= wR.m ? T : Nil }
							if let wL = l as? Numeric, let wR = r as? Numeric { return wL.v <= wR.v ? T : Nil }
							throw SliPError.RuntimeError( "\(l) <= \(r)" )
						}
let	GE				=	Dyadic( ">=", 8 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m >= wR.m ? T : Nil }
							if let wL = l as? Numeric, let wR = r as? Numeric { return wL.v >= wR.v ? T : Nil }
							throw SliPError.RuntimeError( "\(l) >= \(r)" )
						}
						let	Minus = Dyadic( "-", 6 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m - wR.m ) }
							if let wL = l as? Numeric, let wR = r as? Numeric { return RealNumber( wL.v - wR.v ) }
							throw SliPError.RuntimeError( "\(l) - \(r)" )
						}
						let	Plus = Dyadic( "+", 6 ) { c, l, r in
							if let wL = l as? IntNumber	, let wR = r as? IntNumber	{ return IntNumber	( wL.m + wR.m ) }
							if let wL = l as? Numeric	, let wR = r as? Numeric	{ return RealNumber	( wL.v + wR.v ) }
							if let wL = l as? StringL	, let wR = r as? StringL	{ return StringL	( wL.m + wR.m ) }
							if let wL = l as? ListL		, let wR = r as? ListL		{ return ListL		( wL.m + wR.m ) }
							if let wL = l as? Parallel	, let wR = r as? Parallel	{ return Parallel	( wL.m + wR.m ) }
							if let wL = l as? Procedure	, let wR = r as? Procedure	{ return Procedure	( wL.m + wR.m ) }
							if let wL = l as? Sentence	, let wR = r as? Sentence	{ return Sentence	( wL.m + wR.m ) }
							throw SliPError.RuntimeError( "\(l) + \(r)" )
						}
let	Reminder		=	Dyadic( "%", 5 ) { c, l, r in
							do {
								if let wL = l as? IntNumber, let wR = r as? IntNumber { return try IntNumber( wL.m % wR.m ) }
							} catch {
							}
							throw SliPError.RuntimeError( "\(l) % \(r)" )
						}
let	IDiv			=	Dyadic( "/", 5 ) { c, l, r in
							do {
								if let wL = l as? IntNumber, let wR = r as? IntNumber { return try IntNumber( wL.m / wR.m ) }
							} catch {
							}
							throw SliPError.RuntimeError( "\(l) / \(r)" )
						}
let	Div				=	Dyadic( "÷", 5 ) { c, l, r in
							do {
								if let wL = l as? IntNumber, let wR = r as? IntNumber { return try IsZero( wL.m % wR.m ) ? IntNumber( wL.m / wR.m ) : RealNumber( wL.v / wR.v ) }
								if let wL = l as? Numeric, let wR = r as? Numeric { return RealNumber( wL.v / wR.v ) }
							} catch {
							}
							throw SliPError.RuntimeError( "\(l) ÷ \(r)" )
						}
let	Mul				=	Dyadic( "×", 5 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m * wR.m ) }
							if let wL = l as? Numeric,	let wR = r as? Numeric { return RealNumber( wL.v * wR.v ) }
							throw SliPError.RuntimeError( "\(l) × \(r)" )
						}
let	Or				=	Dyadic( "|", 4 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m | wR.m ) }
							return IsNil( l ) || IsNil( r ) ? T : Nil
						}
let	XOr				=	Dyadic( "^", 4 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m ^ wR.m ) }
							return IsNil( l ) != IsNil( r ) ? T : Nil
						}
let	And				=	Dyadic( "&", 4 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m & wR.m ) }
							return IsNil( l ) && IsNil( r ) ? T : Nil
						}
let	Cons			=	Dyadic( ",", 7 ) { c, l, r in
							switch r {
							case let w as ListL		: return ListL		( Array( [ l ] + w.m ) )
							case let w as Parallel	: return Parallel	( Array( [ l ] + w.m ) )
							case let w as Sentence	: return Sentence	( Array( [ l ] + w.m ) )
							case let w as Procedure	: return Procedure	( Array( [ l ] + w.m ) )
							default					: throw SliPError.RuntimeError( "\(l) , \(r)" )
							}
						}
let	MemberOf		=	Dyadic( "∈", 8 ) { c, l, r in
							guard let wR = r as? List else { throw SliPError.RuntimeError( "\(l) ∈ \(r)" ) }
							return wR.m.contains( l ) ? T : Nil
						}
let	Contains		=	Dyadic( "∋", 8 ) { c, l, r in
							guard let wL = l as? List else { throw SliPError.RuntimeError( "\(l) ∋ \(r)" ) }
							return wL.m.contains( r ) ? T : Nil
						}
let	IfElse			=	Dyadic( "?", 9 ) { c, l, r in
							guard let w = r as? List, w.m.count == 2 else { throw SliPError.RuntimeError( "\(l) ? \(r)" ) }
							return try w.m[ IsNil( l ) ? 1 : 0 ].Eval( c )
						}
let	If				=	Dyadic( "¿", 9 ) { c, l, r in
							return try IsNil( l ) ? Nil : r.Eval( c )
						}
let	Define			=	Dyadic( "=", 10 ) { c, l, r in
							if let w = l as? Name {
								guard let wDicts = c.dicts else { fatalError() }
								wDicts.m[ w.m ] = r
								return l
							}
							throw SliPError.RuntimeError( "\(l) = \(r)" )
						}

func
Read( _ r: Reader< UnicodeScalar >, _ terminator: UnicodeScalar = UnicodeScalar( 0 ) ) throws -> Object? {
	try SkipWhite( r )
	let u = try r.Read()
	switch u {
	case terminator			:	return nil
	case "}", "]", ")", "»"	:	throw SliPError.ReadError( "Unexpected \(u)" )
	case "«"				:	return Parallel	( try ReadObjects( r, "»" ) )
	case "["				:	return ListL	( try ReadObjects( r, "]" ) )
	case "("				:	return Sentence	( try ReadObjects( r, ")" ) )
	case "{"				:	return Procedure( try ReadObjects( r, "}" ) )
	case "\""				:	return try ReadStr( r )
	case "'"				:	guard let w = try Read( r ) else { throw SliPError.ReadError( "No object to quote" ) }
								return Quote( w )
	case "`"				:	guard let w = try Read( r ) else { throw SliPError.ReadError( "No object to combine" ) }
								return Combiner( w )
	case "¡"				:	return UserException
	case "@"				:	return StackTop
	case "·"				:	return CurretDict
	case "!"				:	return Eval
	case "#"				:	return Count
	case "*"				:	return Cdr
	case "$"				:	return Last
	case "."				:	return Print
	case "¦"				:	return Dump
	case "∈"					:	return MemberOf
	case "∋"					:	return Contains
	case "?"				:	return IfElse
	case "¿"				:	return If
	case ","				:	return Cons
	case ":"				:	return Apply
	case "&"				:	return And
	case "^"				:	return XOr
	case "|"				:	return Or
	case "×"				:	return Mul
	case "÷"				:	return Div
	case "/"				:	return IDiv
	case "%"				:	return Reminder
	case "+"				:
		let w = try r.Read()
		switch w {
		case ( "0"..."9" )	:	r.Unread( w )
								return try ReadNumber( r )
		default				:	return Plus
		}
	case "-"				:
		let w = try r.Read()
		switch w {
		case ( "0"..."9" )	:	r.Unread( w )
								return try ReadNumber( r, true )
		default				:	return Minus
		}
	case "="			:
		let w = try r.Read()
		switch w {
		case "="			:	return EQ
		case "<"			:	return LE
		case ">"			:	return GE
		default				:	r.Unread( w )
								return Define
		}
	case "<"				:
		let w = try r.Read()
		switch w {
		case ">"			:	return NE
		case "="			:	return LE
		default				:	r.Unread( w )
								return LT
		}
	case ">"				:
		let w = try r.Read()
		switch w {
		case "<"			:	return NE
		case "="			:	return GE
		default				:	r.Unread( w )
								return GT
		}
	case ( "0"..."9" )		:	r.Unread( u )
								return try ReadNumber( r )
	case ( "a"..."z" )
	,	 ( "A"..."Z" )
	,	"_"					:	r.Unread( u )
								return try ReadName( r )
	default					:	throw SliPError.ReadError( "Invalid character \(u):\(u.value)" )
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
PreProcessor: Reader< UnicodeScalar > {
	var
	m		: String.UnicodeScalarView.Iterator

	init( _ p: String ) {
		var wLines = p.components( separatedBy: NSCharacterSet.newlines )
		for ( i, w ) in wLines.enumerated() {
			wLines[ i ] = w.components( separatedBy: "//" )[ 0 ].trimmingCharacters( in: .whitespaces )
			if wLines[ i ].unicodeScalars.last == "=" {
				wLines[ i ] = "(" + wLines[ i ].dropLast() + "):¦;"
			}
		}
		m = wLines.joined( separator: "\n" ).unicodeScalars.makeIterator()
	}

	override func
	_Read() throws -> UnicodeScalar {
		guard let v = m.next() else { throw ReaderError.eod }
		return v
	}
}
