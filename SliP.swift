//	Written by Satoru Ogura, Tokyo. 2015 -
//

import Foundation

enum
SliPError	: Error {
	case	ReadError		( String )
	case	RuntimeError	( String )
	case	UserException
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Context {

	var
	dict			= [ String: Object ]()

	var
	stack			: Chain< Object >?

	func
	Push(	_ o		: Object ) {
		stack = Chain< Object >( o, stack )
	}
	func
	Pop() {
		guard let w = stack else { fatalError() }
		stack = w.next
	}
	
	func
	Load( _ p: [ String: Object ] ) {
		dict.merge( p ) { p, q in return q }
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Object {
	var
	str				: String								{ return "T" }
	
	func
	Eval( _			: Chain< Context > ) throws -> Object	{ return self }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Numeric				: Object {
	var
	v				: Float64								{ fatalError() }
}

class
IntNumber			: Numeric {
	let
	m				: BigInteger
	init(	_ p		: BigInteger )							{ m = p }
	init(	_ p		: Int )									{ m = MakeBigInteger( p ) }
	override var
	str				: String								{ return m.description }
	override var
	v				: Float64								{ return m.NativeFloat }
}
class
RealNumber			: Numeric {
	let
	m				: Float64
	init(	_ p		: Float64 )								{ m = p }
	override var
	str				: String								{ return "\( m )" }
	override var
	v				: Float64								{ return m }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
StringL				: Object {
	let
	m				: String
	init(	_ p		: String )								{ m = p }
	override var
	str				: String								{ return "\"\( m )\"" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Name				: Object {
	let
	m				: String
	init(	_ p		: String )								{ m = p }
	override var
	str				: String								{ return m }
	override func
	Eval(	_ c		: Chain< Context > ) throws -> Object	{
		var	w: Chain< Context >? = c
		while let wContext = w {
			if let v = wContext.m.dict[ m ] { return v }
			w = wContext.next
		}
		throw SliPError.RuntimeError( "UndefinedName \( m )" )
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Primitive			: Object {
	var
	name			: String
	var
	m				: ( Chain< Context > ) throws -> Object
	init(
	_	pName		: String
	, _	p			: @escaping ( Chain< Context > ) throws -> Object
	) {
		name = pName
		m = p
	}
	override var
	str				: String								{ return "P(\( name ))" }
	override func
	Eval(	_ c		: Chain< Context > ) throws -> Object	{ return try m( c ) }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Unary				: Object {
	var
	name			: String
	var
	m				: ( Chain< Context >, Object ) throws -> Object
	init(
	_	pName		: String
	, _	p			: @escaping ( Chain< Context >, Object ) throws -> Object
	) {
		name = pName
		m = p
	}
	override var
	str				: String								{ return "U(\( name ))" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Dyadic				: Object {
	var
	name			: String
	var
	priority		: Int
	var
	m				: ( Chain< Context >, Object, Object ) throws -> Object
	init(
	_	pName		: String
	, _	pPriority	: Int
	, _	p			: @escaping ( Chain< Context >, Object, Object ) throws -> Object
	) {
		name = pName
		priority = pPriority
		m = p
	}
	override var
	str				: String								{ return "D(\( name )(\( priority )))" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Quote				: Object {
	let
	m				: Object
	init(	_ p		: Object )								{ m = p }
	override var
	str				: String								{ return "'\( m.str )" }
	override func
	Eval(	_		: Chain< Context > ) throws -> Object	{ return m }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class
//Combiner			: Object {
//	let
//	m				: Object
//	init(	_ p		: Object ) { m = p }
//	override var
//	str				: String { return "`" + m.str }
//	override func
//	Eval(	_ c		: Context ) throws	-> Object {
//		guard let wDicts = c.dicts else { fatalError() }
//		return Assoc( m, wDicts.m )
//	}
//}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class
//Assoc				: Object {
//	let
//	m				: Object
//	let
//	dict			: [ String: Object ]
//	init(	_ p		: Object, _ pDict: [ String: Object ] ) { m = p; dict = pDict }
//	override func
//	Eval(	_ c		: Chain< Context > ) throws -> Object	{
//		c.dicts = Chain< [ String: Object ] >( dict, c.dicts )
//		defer{ c.dicts = c.dicts!.next }
//		return try m.Eval( c )
//	}
//	override var
//	debug			: String { return "<\(dict):\(m.debug)>" }
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class
Dict				: Object {
	let
	m				: [ String: Object ]
	init(	_ p		: [ String: Object ] )					{ m = p }
	override var
	str				: String								{
		return "\( m.reduce( "<\t" ) { r, p in "\( r )\t\( p.key )\t\( p.value )\n" } )\n>"
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class
List				: Object {
	let
	m				: ArraySlice< Object >
	init(	_ p		: ArraySlice< Object > )				{ m = p }
	override	var
	str				: String {
 		return m.reduce( " ", { r, p in "\( r )\( p.str ) " } )
	}
}

class
ListL				: List {
	override
	init(	_ p		: ArraySlice< Object > )				{ super.init( p ) }
	override	var
	str				: String								{ return "[\( super.str )]" }
}

class
Parallel			: List {
	override
	init(	_ p		: ArraySlice< Object > )				{ super.init( p ) }
	override	var
	str				: String								{ return "«\( super.str )»" }
	override	func
	Eval(	_ c		: Chain< Context > ) throws -> Object	{
		var	v = [ Object ]( repeating: Object(), count: m.count )
		let	wG = DispatchGroup()
		let	wQ = DispatchQueue.global()
		
		for ( i, w ) in m.enumerated() {
			wQ.async( group: wG ) {
				do {
					v[ i ] = try w.Eval( Chain< Context >( Context(), c ) )
				} catch let e {
					print( e )
				}
			}
		}
		wG.wait()
		return ListL( ArraySlice( v ) )
	}
}

class
Sentence			: List {
	override
	init(	_ p		: ArraySlice< Object > )				{ super.init( p ) }
	override	var
	str				: String								{ return "(\( super.str ))" }
	override	func
	Eval(	_ c		: Chain< Context > ) throws -> Object {
		return try Sentence.Eval( c, m )
	}
	static	func
	Eval(	_ c		: Chain< Context >, _ p: ArraySlice< Object > ) throws -> Object {
	
		switch p.count {
		case 0:	throw SliPError.RuntimeError( "Null sentence: \(Sentence( p ).str)" )
		case 1: return try p[ p.startIndex ].Eval( c )
		case 2:
			if let wUnary = p[ p.startIndex ] as? Unary {
				return try wUnary.m( c, p[ p.startIndex + 1 ].Eval( c ) )
			} else {
				throw SliPError.RuntimeError( "Syntax error: \(Sentence( p ).str)" )
			}
		default:
			var wIndex  =	0
			for i in 1 ..< p.count - 1 {
				if let w = p[ p.startIndex + i ] as? Dyadic {
					if let wCurrent = p[ p.startIndex + wIndex ] as? Dyadic {
						if wCurrent.priority > 0 && w.priority <= wCurrent.priority {
							wIndex = i
						}
					} else {
						wIndex = i
					}
				}
			}
			if wIndex > 0 {
				return try ( p[ p.startIndex + wIndex ] as! Dyadic ).m(
					c
				,	Eval( c, p[ p.startIndex ..< p.startIndex + wIndex ] )
				,	Eval( c, p[ p.startIndex + wIndex + 1 ..< p.startIndex + p.count ] )
				)
			} else {
				throw SliPError.RuntimeError( "Syntax error: \(Sentence( p ).str)" )
			}
		}
	}
}

class
Procedure			: List {
	override
	init(	_ p		: ArraySlice< Object > )				{ super.init( p ) }
	override	var
	str				: String								{ return "{\( super.str )}" }
	override	func
	Eval(	_ c		: Chain< Context > ) throws -> Object	{
		let	wContext = Chain< Context >( Context(), c )
		var	v = [ Object ]( repeating: Object(), count: m.count )
		for ( i, w ) in m.enumerated() { v[ i ] = try w.Eval( wContext ) }
		return ListL( ArraySlice( v ) )
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
func
IsNil( _ o: Object ) -> Bool {
	if let w = o as? List { return w.m.count == 0 }
	return false
}

let
T	=	Object()

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
ReadNumber( _ r: UnicodeReader ) throws -> Object {
 	var	v = ""
	var	wIsReal = false
	var	wHasE = false
	GLOBAL: while let u = r.Read() {
		switch u {
		case	"0" ... "9":
			v += String( u )
		case	"A" ... "D", "F" ... "Z", "a" ... "d", "f" ... "z", "_":
			throw SliPError.ReadError( "NumberFormat:" + v + String( u ) )
		case	"e", "E":
			if !wIsReal || wHasE { throw SliPError.ReadError( "NumberFormat: " + v + String( u ) ) }
			v += String( u )
			wIsReal = true
			wHasE = true
		case	"+", "-":
			if v == "" { v = String( u ) }
			else {
				if v.last == "e" || v.last == "E" {
					v += String( u )
				} else {
					r.Unread( u )
					break GLOBAL
				}
			}
		case	".":
			if wIsReal {
				r.Unread( u )
				break GLOBAL
			}
			v += String( u )
			wIsReal = true
		default:
			r.Unread( u )
			break GLOBAL
		}
	}
	if wIsReal {
		guard let w = Float64( v ) else { throw SliPError.ReadError( "NumberFormat: " + v )  }
		return RealNumber( w )
	} else {
		guard let w = MakeBigInteger( v, 10 ) else { throw SliPError.ReadError( "NumberFormat: " + v )  }
		return IntNumber( w )
	}
}

func
ReadName( _ r: UnicodeReader ) throws -> Object {
	var	v = ""
	GLOBAL: while let u = r.Read() {
		switch u {
		case "0" ... "9", "a" ... "z", "A" ... "Z", "_":
			v += String( u )
		default:
			r.Unread( u )
			break GLOBAL
		}
	}
	return Name( v )
}

func
ReadStr( _ r: UnicodeReader ) throws -> StringL {
	var	v = ""
	var	wEscaped = false
	while let u = r.Read() {
		if wEscaped {
			wEscaped = false
			switch u {
			case "0"	: v += String( "\0" )
			case "t"	: v += String( "\t" )
			case "n"	: v += String( "\n" )
			case "r"	: v += String( "\r" )
			default		: v += String( u )
			}
		} else {
			switch u {
			case "\""	: return StringL( v )
			case "\\"	: wEscaped = true
			default		: v += String( u )
			}
		}
	}
	throw SliPError.RuntimeError( "Unterminated string: \(v)" )
}

func
ReadList( _ r: UnicodeReader, _ terminator: UnicodeScalar ) throws -> ArraySlice< Object > {
	var	v = [ Object ]()
	while let w = try Read( r, terminator ) { v.append( w ) }
	return ArraySlice( v )
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

let	UserException	=	Primitive( "¡" ) { c in throw SliPError.UserException }
let	StackTop		=	Primitive( "@" ) { c in
							guard let v = c.m.stack?.m else { throw SliPError.RuntimeError( "Stack underflow" ) }
							return v
						}
let	CurretDict		=	Primitive( "·" ) { c in return Dict( c.m.dict ) }

let	Eval			=	Unary( "!" ) { c, p in try p.Eval( c ) }
let	Count			=	Unary( "#" ) { c, p in
							guard let w = p as? List else { throw SliPError.RuntimeError( "\(p.str):#" ) }
							return IntNumber( w.m.count )
						}
let	Cdr				=	Unary( "*" ) { c, p in
							guard let w = p as? List, w.m.count > 0 else { throw SliPError.RuntimeError( "\(p.str):*" ) }
							switch w {
							case is ListL		: return ListL		( w.m.dropFirst() )
							case is Parallel	: return Parallel	( w.m.dropFirst() )
							case is Sentence	: return Sentence	( w.m.dropFirst() )
							case is Procedure	: return Procedure	( w.m.dropFirst() )
							default				: fatalError()
							}
						}
let	Last			=	Unary( "$" ) { c, p in
							guard let w = p as? List, let v = w.m.last else { throw SliPError.RuntimeError( "\(p.str):$" ) }
							return v
						}
let	Print			=	Unary( "." ) { c, p in
							print( p.str, separator: "" )
							return p
						}
let	Dump			=	Unary( "¦" ) { c, p in
							print( p.str )
							return p
						}
let	Apply			=	Dyadic( ":", 10 ) { c, l, r in
							switch r {
							case let wR as IntNumber:
								guard let wL = l as? List else { throw SliPError.RuntimeError( "Index operation \(r.str) to \(l.str)" ) }
								let wIndex = wR.m.NativeInt
								return wIndex < wL.m.count ? wL.m[ wL.m.startIndex + wIndex ] : Nil
							case let wR as Name:
								guard let wL = l as? Dict else { throw SliPError.RuntimeError( "Dict operation \(r.str) to \(l.str)" ) }
								return wL.m[ wR.m ] ?? Nil
							case let wR as Unary:
								return try wR.m( c, l )
							default:
								c.m.Push( l )
								defer { c.m.Pop() }
								return try r.Eval( c )
							}
						}

let	Or				=	Dyadic( "|" ,  9 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m | wR.m ) }
							return IsNil( l ) || IsNil( r ) ? T : Nil
						}
let	XOr				=	Dyadic( "^" ,  9 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m ^ wR.m ) }
							return IsNil( l ) != IsNil( r ) ? T : Nil
						}
let	And				=	Dyadic( "&" ,  9 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m & wR.m ) }
							return IsNil( l ) && IsNil( r ) ? T : Nil
						}
let	Reminder		=	Dyadic( "%" ,  8 ) { c, l, r in
							do {
								if let wL = l as? IntNumber, let wR = r as? IntNumber { return try IntNumber( wL.m % wR.m ) }
							} catch {
							}
							throw SliPError.RuntimeError( "\(l.str) % \(r.str)" )
						}
let	IDiv			=	Dyadic( "/" ,  8 ) { c, l, r in
							do {
								if let wL = l as? IntNumber, let wR = r as? IntNumber { return try IntNumber( wL.m / wR.m ) }
							} catch {
							}
							throw SliPError.RuntimeError( "\(l.str) / \(r.str)" )
						}
let	Div				=	Dyadic( "÷" ,  8 ) { c, l, r in
							do {
								if let wL = l as? IntNumber, let wR = r as? IntNumber { return try IsZero( wL.m % wR.m ) ? IntNumber( wL.m / wR.m ) : RealNumber( wL.v / wR.v ) }
								if let wL = l as? Numeric, let wR = r as? Numeric { return RealNumber( wL.v / wR.v ) }
							} catch {
							}
							throw SliPError.RuntimeError( "\(l.str) ÷ \(r.str)" )
						}
let	Mul				=	Dyadic( "×" ,  8 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m * wR.m ) }
							if let wL = l as? Numeric,	let wR = r as? Numeric { return RealNumber( wL.v * wR.v ) }
							throw SliPError.RuntimeError( "\(l.str) × \(r.str)" )
						}
let	Minus			=	Dyadic( "-" ,  7 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m - wR.m ) }
							if let wL = l as? Numeric, let wR = r as? Numeric { return RealNumber( wL.v - wR.v ) }
							throw SliPError.RuntimeError( "\(l.str) - \(r.str)" )
						}
let	Plus			=	Dyadic( "+" ,  7 ) { c, l, r in
							if let wL = l as? IntNumber	, let wR = r as? IntNumber	{ return IntNumber	( wL.m + wR.m ) }
							if let wL = l as? Numeric	, let wR = r as? Numeric	{ return RealNumber	( wL.v + wR.v ) }
							if let wL = l as? StringL	, let wR = r as? StringL	{ return StringL	( wL.m + wR.m ) }
							if let wL = l as? ListL		, let wR = r as? ListL		{ return ListL		( wL.m + wR.m ) }
							if let wL = l as? Parallel	, let wR = r as? Parallel	{ return Parallel	( wL.m + wR.m ) }
							if let wL = l as? Procedure	, let wR = r as? Procedure	{ return Procedure	( wL.m + wR.m ) }
							if let wL = l as? Sentence	, let wR = r as? Sentence	{ return Sentence	( wL.m + wR.m ) }
							throw SliPError.RuntimeError( "\(l.str) + \(r.str)" )
						}
let	MemberOf		=	Dyadic( "∈" ,  5 ) { c, l, r in
							guard let wR = r as? List else { throw SliPError.RuntimeError( "\(l) ∈ \(r)" ) }
							return wR.m.contains( l ) ? T : Nil
						}
let	Contains		=	Dyadic( "∋" ,  5 ) { c, l, r in
							guard let wL = l as? List else { throw SliPError.RuntimeError( "\(l) ∋ \(r)" ) }
							return wL.m.contains( r ) ? T : Nil
						}
let	EQ				=	Dyadic( "==",  5 ) { c, l, r in l == r ? T : Nil }
let	NE				=	Dyadic( "<>",  5 ) { c, l, r in l != r ? T : Nil }
let	LT				=	Dyadic( "<" ,  5 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m < wR.m ? T : Nil }
							if let wL = l as? Numeric, let wR = r as? Numeric { return wL.v < wR.v ? T : Nil }
							throw SliPError.RuntimeError( "\(l.str) < \(r.str)" )
						}
let	GT				=	Dyadic( ">" ,  5 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m > wR.m ? T : Nil }
							if let wL = l as? Numeric, let wR = r as? Numeric { return wL.v > wR.v ? T : Nil }
							throw SliPError.RuntimeError( "\(l.str) > \(r.str)" )
						}
let	LE				=	Dyadic( "<=",  5 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m <= wR.m ? T : Nil }
							if let wL = l as? Numeric, let wR = r as? Numeric { return wL.v <= wR.v ? T : Nil }
							throw SliPError.RuntimeError( "\(l.str) <= \(r.str)" )
						}
let	GE				=	Dyadic( ">=",  5 ) { c, l, r in
							if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m >= wR.m ? T : Nil }
							if let wL = l as? Numeric, let wR = r as? Numeric { return wL.v >= wR.v ? T : Nil }
							throw SliPError.RuntimeError( "\(l.str) >= \(r.str)" )
						}
let	Cons			=	Dyadic( "," ,  3 ) { c, l, r in
							switch r {
							case let w as ListL		: return ListL		( [ l ] + w.m )
							case let w as Parallel	: return Parallel	( [ l ] + w.m )
							case let w as Sentence	: return Sentence	( [ l ] + w.m )
							case let w as Procedure	: return Procedure	( [ l ] + w.m )
							default					: throw SliPError.RuntimeError( "\(l.str) , \(r.str)" )
							}
						}
let	IfElse			=	Dyadic( "?" ,  1 ) { c, l, r in
							guard let w = r as? List, w.m.count == 2 else { throw SliPError.RuntimeError( "\(l.str) ? \(r.str)" ) }
							return try w.m[ IsNil( l ) ? 1 : 0 ].Eval( c )
						}
let	If				=	Dyadic( "¿" ,  1 ) { c, l, r in
							return try IsNil( l ) ? Nil : r.Eval( c )
						}
let	Define			=	Dyadic( "=" , -1 ) { c, l, r in
							if let w = l as? Name {
								c.m.dict[ w.m ] = r
								return l
							}
							throw SliPError.RuntimeError( "\(l.str) = \(r.str)" )
						}

func
Read( _ r: UnicodeReader, _ terminator: UnicodeScalar = UnicodeScalar( 0 ) ) throws -> Object? {
	r.SkipWhite()
	guard let u = r.Read() else { return nil }
	switch u {
	case	terminator			:	return nil
	case	"}", "]", ")", "»"	:	throw SliPError.ReadError( "Unexpected \(u)" )
	case	"«"					:	return Parallel	( try ReadList( r, "»" ) )
	case	"["					:	return ListL	( try ReadList( r, "]" ) )
	case	"("					:	return Sentence	( try ReadList( r, ")" ) )
	case	"{"					:	return Procedure( try ReadList( r, "}" ) )
	case	"\""				:	return try ReadStr( r )
	case	"'"					:	guard let w = try Read( r ) else { throw SliPError.ReadError( "No object to quote" ) }
									return Quote( w )
//	case	"`"					:	guard let w = try Read( r ) else { throw SliPError.ReadError( "No object to combine" ) }
//									return Combiner( w )
	case	"¡"					:	return UserException
	case	"@"					:	return StackTop
	case	"·"					:	return CurretDict
	case	"!"					:	return Eval
	case	"#"					:	return Count
	case	"*"					:	return Cdr
	case	"$"					:	return Last
	case	"."					:	return Print
	case	"¦"					:	return Dump
	case	"∈"					:	return MemberOf
	case	"∋"					:	return Contains
	case	"?"					:	return IfElse
	case	"¿"					:	return If
	case	","					:	return Cons
	case	":"					:	return Apply
	case	"&"					:	return And
	case	"^"					:	return XOr
	case	"|"					:	return Or
	case	"×"					:	return Mul
	case	"÷"					:	return Div
	case	"/"					:	return IDiv
	case	"%"					:	return Reminder
	case	"+"					:
		guard let w = r.Read() else { throw SliPError.ReadError( "+ what?" ) }
		switch w {
		case "0"..."9"			:	r.Unread( w )
									return try ReadNumber( r )
		default					:	return Plus
		}
	case	"-"					:
		guard let w = r.Read() else { throw SliPError.ReadError( "- what?" ) }
		switch w {
		case "0"..."9"			:	r.Unread( w )
									r.Unread( u )
									return try ReadNumber( r )
		default					:	return Minus
		}
	case	"="					:
		guard let w = r.Read() else { throw SliPError.ReadError( "= what?" ) }
		switch w {
		case "="				:	return EQ
		case "<"				:	return LE
		case ">"				:	return GE
		default					:	r.Unread( w )
									return Define
		}
	case	"<"					:
		guard let w = r.Read() else { throw SliPError.ReadError( "< what?" ) }
		switch w {
		case ">"				:	return NE
		case "="				:	return LE
		default					:	r.Unread( w )
									return LT
		}
	case	">"					:
		guard let w = r.Read() else { throw SliPError.ReadError( "> what?" ) }
		switch w {
		case "<"				:	return NE
		case "="				:	return GE
		default					:	r.Unread( w )
									return GT
		}
	case	"0"..."9"			:	r.Unread( u )
									return try ReadNumber( r )
	case	"a"..."z"
	,		"A"..."Z"
	,		"_"					:	r.Unread( u )
									return try ReadName( r )
	default						:	throw SliPError.ReadError( "Invalid character \(u):\(u.value)" )
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
func
PreProcess( _ p: String ) -> String{
	return p.components( separatedBy: .newlines ).map {
		let v = $0.components( separatedBy: "//" )[ 0 ].trimmingCharacters( in: .whitespaces )
		return v.unicodeScalars.last == "="
		?	"(" + String( v.unicodeScalars.dropLast() ) + "):¦;"
		:	v
	}.joined( separator: "\n" )
}
