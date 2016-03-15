//	Written by Satoru Ogura, Tokyo.
//
import Foundation

enum
Error				:	ErrorType {
	case				ReadError( String )
	case				RuntimeError( String )
	case				UserException
}


class
Object				:	NSObject {
	func
	Eval() throws -> Object { return self }
}

var
sStack				:	Cell< Object >?
func
Push( p: Object ) {
//print( "Push:", p )
	sStack = Cell( p, sStack )
}

func
Pop() {
//print( "Pop:", sStack!.u )
	sStack = sStack!.next
}


var
sDicts				=	Cell< [ String: Object ] >( [ String: Object ]() )

class
Name				:	Object {
	let
	u				:	String
	init(	_ p		:	String ) { u = p }
	override var
	description 	:	String { return u }
	override func
	Eval()	throws	->	Object {
		var	wAL = sDicts as Cell< [ String: Object ] >?
		while let w = wAL {
			if let v = w.u[ u ] { return v }
			wAL = w.next
		}
		throw Error.RuntimeError( "UndefinedName \(u)" )
	}
}

class
Number				:	Object {
	func
	Value()			->	Float64 { assert( false ); return 0 }
}

class
Integer				:	Number {
	let
	u				:	Int
	init(	_ p		:	Int ) { u = p }
	override var
	description		:	String { return "\(u)" }
	override func
	Value()			->	Float64 { return Float64( u ) }
}

class
Float				:	Number {
	let
	u				:	Float64
	init(	_ p		:	Float64 ) { u = p }
	override var
	description		:	String { return "\(u)" }
	override func
	Value()			->	Float64 { return u }
}

class
Str					:	Object {
	let
	u				:	String
	init(	_ p		:	String ) { u = p }
	override var
	description	:	String { return "\"\(u)\"" }
}

class
Primitive			:	Object {
	var
	u				:	() throws -> Object
	init(	_ p		:	() throws -> Object ) { u = p }
	override var
	description		:	String { return "P" }
	override func
	Eval() throws	->	Object { return try u() }
}

class
Builtin				:	Object {
	var
	u				:	Object throws -> Object
	init(	_ p		:	Object throws -> Object ) { u = p }
	override var
	description		:	String { return "B" }
}

class
Operator			:	Object {
	var
	bond			:	Int
	var
	u				:	( Object, Object ) throws -> Object
	init(	_ p		:	( Object, Object ) throws -> Object, _ pBond: Int ) { u = p; bond = pBond }
	override var
	description		:	String { return ":\(bond)" }
}

class
Quote				:	Object {
	let
	u				:	Object
	init(	_ p		:	Object ) { u = p }
	override var
	description		:	String { return "'\(u)" }
	override func
	Eval() throws	->	Object { return u }
}

class
Combiner			:	Object {
	let
	u				:	Object
	init(	_ p		:	Object ) { u = p }
	override var
	description		:	String { return "`\(u)" }
	override func
	Eval() throws	->	Object { return EvalAssoc( u, sDicts.u ) }
}

class
EvalAssoc			:	Object {
	let
	u				:	Object
	let
	map				:	[ String: Object ]
	init(	_ p		:	Object, _ pMap: [ String: Object ] ) { u = p; map = pMap }
	override var
	description		:	String { return "<\(map):\(u)>" }
	override func
	Eval() throws	->	Object {
		sDicts = Cell< [ String: Object ] >( map, sDicts )
		defer{ sDicts = sDicts.next! }
		return try u.Eval()
	}
}

class
Map					:	Object {
	let
	u				:	[ String: Object ]
	init(	_ p		:	[ String: Object ] ) { u = p }
	override var
	description		:	String { return "'\(u)" }
}

var
sObject = Object()

class
List				:	Object {
	func
	EvalSentence( p: [ Object ] ) throws -> Object {
//		print( ">", p )
//		let	v = try _EvalSentence( p )
//		print( "<", v )
//		return v
//	}
//	func
//	_EvalSentence( p: [ Object ] ) throws -> Object {
		switch p.count {
		case  0:	return Nil
		case  1:	return try p[ 0 ].Eval()
		case  2:	throw Error.RuntimeError( "No operator in \(List( p, .Sentence ))" )
		default:	break
		}
		var wTarget :	Operator?
		var wIndex  =	0
		for i in 1 ..< p.count - 1 {
			if let w = p[ i ] as? Operator {
				if let wCurrent = wTarget {
					if w.bond >= wCurrent.bond {
						wTarget = w
						wIndex = i
					}
				} else {
					wTarget = w
					wIndex = i
				}
			}
		}
		if let w = wTarget {
			return try w.u( EvalSentence( Array( p[ 0 ..< wIndex ] ) ), EvalSentence( Array( p[ wIndex+1 ..< p.count ] ) ) )
		} else {
			throw Error.RuntimeError( "No operator in \(List( p, .Sentence ))" )
		}
	}
	enum
	Type {
		case Literal
		case Parallel
		case Block
		case Sentence
	}
	let
	type			:	Type
	let
	u				:	[ Object ]
	init(	_ p		:	[ Object ], _ pType: Type ) { u = p; type = pType }
	func
	_Str()			->	String {
 		var	v = " "
		for w in u { v += "\(w) " }
		return v
	}
	override	var
	description		:	String {
		switch type {
		case .Literal:	return "[" + _Str() + "]"
		case .Parallel:	return "«" + _Str() + "»"
		case .Block:	return "{" + _Str() + "}"
		case .Sentence:	return "(" + _Str() + ")"
		}
	}
	override	func
	Eval() throws	->	Object {
		switch type {
		case .Literal:
			return self
		case .Parallel:
			var	v = [ Object ]( count: u.count, repeatedValue: sObject )
			let	wG = dispatch_group_create()
			let	wQ = dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 )
			for ( i, w ) in u.enumerate() {
				dispatch_group_async( wG, wQ ) {
					do {
						v[ i ] = try w.Eval()
					} catch let e {
						print( e )
					}
				}
			}
			dispatch_group_wait( wG, DISPATCH_TIME_FOREVER )
			for w in u {
				if w == sObject { throw Error.RuntimeError( "Parallel" ) }	//	UC
			}
			return List( v, .Literal )
		case .Block:
			sDicts = Cell< [ String: Object ] >( [ String: Object ](), sDicts )
			defer{ sDicts = sDicts.next! }
			var	v = [ Object ]( count:u.count, repeatedValue:Object() )
			for ( i, w ) in u.enumerate() { v[ i ] = try w.Eval() }
			return List( v, .Literal )
		case .Sentence:
			return try EvalSentence( u )
		}
	}
}

func
NilSliP( p: Object )->	Bool {
	if let w = p as? List { return w.u.count == 0 }
	return false
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

func
SkipWhite( p: Reader< UnicodeScalar > ) {
	while let w = p.Read() {
		if w.value >= 0x10000 || !NSCharacterSet.whitespaceAndNewlineCharacterSet().characterIsMember( UInt16( w.value ) ) {
			p.Unread( w )
			break
		}
	}
}


func
ReadNumber( p: Reader< UnicodeScalar >, _ neg: Bool = false ) throws -> Object {
 	var	v = ""
	var	wFloatF = false
	while let c = p.Read() {
		switch c {
		case	( "0"..."9" ), "x", "X", "+", "-":
			v += String( c )
		case	".", "e", "E":
			v += String( c )
			wFloatF = true
		default:
			p.Unread( c )
			if wFloatF	{ if let w = Float64( v ) { return Float( neg ? -w : w ) } else { throw Error.ReadError( "NumberFormat\(v)" ) } }
			else		{ if let w = Int( v ) { return Integer( neg ? -w : w ) } else { throw Error.ReadError( "NumberFormat\(v)" ) } }
		}
	}
	throw Error.ReadError( "EOD" )
}

func
ReadName( p: Reader< UnicodeScalar > ) throws -> Object {
	var	v = ""
	while let c = p.Read() {
		switch c {
		case ( "0" ... "9" ), ( "a" ... "z" ), ( "A" ... "Z" ), "_":
			v += String( c )
		default:
			p.Unread( c )
			return Name( v )
		}
	}
	throw Error.ReadError( "EOD" )
}

func
ReadStr( p: Reader< UnicodeScalar > ) throws -> Str {
	var	v = ""
	var	wEscaped = false
	while let c = p.Read() {
		if wEscaped {
			wEscaped = false
			v.append( Character( c ) )
		} else {
			switch c {
			case "\""	:	return Str( v )
			case "\\"	:	wEscaped = true
			default		:	v += String( c )
			}
		}
	}
	throw Error.ReadError( "EOD" )
}

func
ReadObjects( p: Reader< UnicodeScalar >, _ terminator: UnicodeScalar ) throws -> [ Object ] {
	var	v = [ Object ]()
	while let w = try Read( p, terminator ) { v.append( w ) }
	return v
}

let
T	=	Str( "T" )

let
Nil	=	List( [], .Literal )

let
StackTop = Primitive(
	{	if let v = sStack { return v.u }
		throw Error.RuntimeError( "StackUnderflow" )
	}
)

let
CurrentContext = Primitive( { return Map( sDicts.u ) } )

let
Exception = Primitive( { throw Error.UserException } )

let
next = Builtin(
	{	p in
		if let w = p as? List where w.u.count > 0 { return List( Array( w.u.dropFirst() ), w.type ) }
		throw Error.RuntimeError( "\(p):*" )
	}
)

let
Count = Builtin(
	{	p in
		if let w = p as? List { return Integer( w.u.count ) }
		throw Error.RuntimeError( "\(p):#" )
	}
)

let
Last = Builtin(
	{	p in
		if let w = p as? List where w.u.count > 0 { return w.u.last! }
		throw Error.RuntimeError( "\(p):$" )
	}
)

let
Print = Builtin(
	{	p in print( p )
		return p
	}
)

let
If = Operator(
	{	l, r in
		return try NilSliP( l ) ? Nil : r.Eval()
	}
,	9
)

let
IfElse = Operator(
	{	l, r in
		if let w = r as? List where w.type == .Literal && w.u.count == 2 {
			return try NilSliP( l ) ? w.u[ 1 ].Eval() : w.u[ 0 ].Eval()
		}
		throw Error.RuntimeError( "\(l) ? \(r)" )
	}
,	9
)

let
Cons = Operator(
	{	l, r in
		if let w = r as? List {
			var	wU = w.u
			wU.insert( l, atIndex: 0 )
			return List( wU, w.type )
		}
		throw Error.RuntimeError( "\(l) , \(r)" )
	}
,	7
)

let
Apply = Operator(
	{	l, r in
		switch r {
		case let w as Integer:
			let	wR = w.u
			if let wL = l as? List {
				if wL.u.count > wR { return wL.u[ wR ] } else { throw Error.RuntimeError( "Index operation \(w) to \(l)" ) }
			}
			throw Error.RuntimeError( "Index operation \(w) to \(l)" )
		case let w as Name:
			let	wR = w.u
			if let wL = l as? Map {
				return wL.u[ w.u ] ?? Nil
			}
			throw Error.RuntimeError( "Map operation \(w) to \(l)" )
		case let w as Builtin:
			return try w.u( l )
		case let w as List:
			Push( l )
			defer{ Pop() }
			return try w.Eval()
		case let w as EvalAssoc:
			Push( l )
			defer{ Pop() }
			return try w.Eval()
		default:
			throw Error.RuntimeError( "\(l) : \(r)" )
		}
	}
,	0
)

let
And = Operator(
	{	l, r in
		if let wL = l as? Integer, let wR = r as? Integer { return Integer( wL.u & wR.u ) }
		return NilSliP( l ) && NilSliP( r ) ? T : Nil
	}
,	4
)

let
Xor = Operator(
	{	l, r in
		if let wL = l as? Integer, let wR = r as? Integer { return Integer( wL.u ^ wR.u ) }
		return NilSliP( l ) != NilSliP( r ) ? T : Nil
	}
,	4
)

let
Or = Operator(
	{	l, r in
		if let wL = l as? Integer, let wR = r as? Integer { return Integer( wL.u | wR.u ) }
		return NilSliP( l ) || NilSliP( r ) ? T : Nil
	}
,	4
)

let
Mul = Operator(
	{	l, r in
		if let wL = l as? Integer, let wR = r as? Integer { return Integer( wL.u * wR.u ) }
		if let wL = l as? Number, let wR = r as? Number { return Float( wL.Value() * wR.Value() ) }
		throw Error.RuntimeError( "\(l) × \(r)" )
	}
,	5
)

let
Div = Operator(
	{	l, r in
		if let wL = l as? Number, let wR = r as? Number { return Float( wL.Value() / wR.Value() ) }
		throw Error.RuntimeError( "\(l) ÷ \(r)" )
	}
,	5
)

let
IDiv = Operator(
	{	l, r in
		if let wL = l as? Integer, let wR = r as? Integer { return Integer( wL.u / wR.u ) }
		throw Error.RuntimeError( "\(l) / \(r)" )
	}
,	5
)

let
Remainder = Operator(
	{	l, r in
		if let wL = l as? Integer, let wR = r as? Integer { return Integer( wL.u % wR.u ) }
		throw Error.RuntimeError( "\(l) % \(r)" )
	}
,	5
)

let
Add = Operator(
	{	l, r in
		if let wL = l as? Integer, let wR = r as? Integer { return Integer( wL.u + wR.u ) }
		if let wL = l as? Number, let wR = r as? Number { return Float( wL.Value() + wR.Value() ) }
		if let wL = l as? Str, let wR = r as? Str { return Str( wL.u + wR.u ) }
		if let wL = l as? List, let wR = r as? List where wL.type == wR.type { return List( wL.u + wR.u, wL.type ) }
		throw Error.RuntimeError( "\(l) + \(r)" )
	}
,	6
)

let
Minus = Operator(
	{	l, r in
		if let wL = l as? Integer, let wR = r as? Integer { return Integer( wL.u - wR.u ) }
		if let wL = l as? Number, let wR = r as? Number { return Float( wL.Value() - wR.Value() ) }
		throw Error.RuntimeError( "\(l) - \(r)" )
	}
,	6
)

let
Eq = Operator(
	{	l, r in
		if let wL = l as? Number, let wR = r as? Number { return wL.Value() == wR.Value() ? T : Nil }
		if let wL = l as? Str, let wR = r as? Str { return wL.u == wR.u ? T : Nil }
		if let wL = l as? Name, let wR = r as? Name { return wL.u == wR.u ? T : Nil }
		return l == r ? T : Nil
	}
,	8
)

let
GE = Operator(
	{	l, r in
		if let wL = l as? Number, let wR = r as? Number { return wL.Value() >= wR.Value() ? T : Nil }
		throw Error.RuntimeError( "\(l) >= \(r)" )
	}
,	8
)

let
LE = Operator(
	{	l, r in
		if let wL = l as? Number, let wR = r as? Number { return wL.Value() <= wR.Value() ? T : Nil }
		throw Error.RuntimeError( "\(l) <= \(r)" )
	}
,	8
)

let
Define = Operator(
	{	l, r in
		if let w = l as? Name {
			sDicts.u[ w.u ] = r
			return l
		}
		throw Error.RuntimeError( "\(l) = \(r)" )
	}
,	10
)

let
Neq = Operator(
	{	l, r in
		if let wL = l as? Number, let wR = r as? Number { return wL.Value() != wR.Value() ? T : Nil }
		if let wL = l as? Str, let wR = r as? Str { return wL.u != wR.u ? T : Nil }
		return l != r ? T : Nil
	}
,	8
)

let
LT = Operator(
	{	l, r in
		if let wL = l as? Number, let wR = r as? Number { return wL.Value() < wR.Value() ? T : Nil }
		throw Error.RuntimeError( "\(l) < \(r)" )
	}
,	8
)

let
GT = Operator(
	{	l, r in
		if let wL = l as? Number, let wR = r as? Number { return wL.Value() > wR.Value() ? T : Nil }
		throw Error.RuntimeError( "\(l) > \(r)" )
	}
,	8
)

func
Read( pReader: Reader< UnicodeScalar >, _ terminator: UnicodeScalar = UnicodeScalar( 0 ) ) throws -> Object? {
	SkipWhite( pReader )
	if let c = pReader.Read() {
		switch c {
		case terminator			:	return nil
		case "}", "]", ")"		:	throw Error.ReadError( "Unexpected \(c)" )
		case "["				:	return List( try ReadObjects( pReader, "]" as UnicodeScalar ), .Literal )
		case "«"				:	return List( try ReadObjects( pReader, "»" as UnicodeScalar ), .Parallel )
		case "("				:	return List( try ReadObjects( pReader, ")" as UnicodeScalar ), .Sentence )
		case "{"				:	return List( try ReadObjects( pReader, "}" as UnicodeScalar ), .Block )
		case "\""				:	return try ReadStr( pReader )
		case "'"				:	if let w = try Read( pReader ) { return Quote( w ) }; throw Error.ReadError( "No object to quote" )
		case "`"				:	if let w = try Read( pReader ) { return Combiner( w ) }; throw Error.ReadError( "No object to combine" )
		case "!"				:	return Exception
		case "@"				:	return StackTop
		case "·"				:	return CurrentContext
		case "*"				:	return next
		case "#"				:	return Count
		case "$"				:	return Last
		case "¦"				:	return Print
		case "?"				:	return IfElse
		case "¿"				:	return If
		case ","				:	return Cons
		case ":"				:	return Apply
		case "&"				:	return And
		case "^"				:	return Xor
		case "|"				:	return Or
		case "×"				:	return Mul
		case "÷"				:	return Div
		case "/"				:	return IDiv
		case "%"				:	return Remainder
		case "+"				:
			if let w = pReader.Read() {
				pReader.Unread( w )
				switch w {
				case ( "0"..."9" ), ".":
									return try ReadNumber( pReader )
				default			:	return Add
				}
			}
			throw Error.ReadError( "EOD" )
		case "-"				:
			if let w = pReader.Read() {
				pReader.Unread( w )
				switch w {
				case ( "0"..."9" ), ".":
									return try ReadNumber( pReader, true )
				default			:	return Minus
				}
			}
			throw Error.ReadError( "EOD" )
		case "="			:
			if let w = pReader.Read() {
				switch w {
				case "="		:	return Eq
				case ">"		:	return GE
				case "<"		:	return LE
				default			:	pReader.Unread( w )
									return Define
				}
			}
			throw Error.ReadError( "EOD" )
		case "<"				:
			if let w = pReader.Read() {
				switch w {
				case ">"		:	return Neq
				case "="		:	return LE
				default			:	pReader.Unread( w )
									return LT
				}
			}
			throw Error.ReadError( "EOD" )
		case ">"				:
			if let w = pReader.Read() {
				switch w {
				case "<"		:	return Neq
				case "="		:	return GE
				default			:	pReader.Unread( w )
									return GT
				}
			}
			throw Error.ReadError( "EOD" )
		case ( "0"..."9" )		:	pReader.Unread( c )
									return try ReadNumber( pReader )
		case ( "a"..."z" ), ( "A"..."Z" ), "_":
									pReader.Unread( c )
									return try ReadName( pReader )
		default					:	throw Error.ReadError( "Invalid character \(c)" )
		}
	}
	return nil
}


func
SetupBuiltin() {
	sDicts.u[ "eval" ] = Builtin(
		{	p in
			let v = try p.Eval()
print( "Eval:", p, "->", v )
			return v
		}
	)
	sDicts.u[ "for" ] = Builtin(
		{	p in
			if let wArgs = p as? List where wArgs.u.count >= 2 {
				if let wList = wArgs.u[ 0 ] as? List {
					var	v = [ Object ]()
					for w in wList.u {
						Push( w )
						defer{ Pop() }
						v.append( try wArgs.u[ 1 ].Eval() )
					}
					return List( v, wList.type )
				} else {
					throw Error.RuntimeError( "\(p):for" )
				}
			} else {
				throw Error.RuntimeError( "\(p):for" )
			}
		}
	)
	sDicts.u[ "TwoElements" ] = Builtin(
		{	p in
			if let w = p as? List {
				switch w.u.count {
				case 0, 1	:	return Integer( 0 )
				case 2		:	return Integer( 1 )
				default		:	return Integer( 2 )
				}
			} else {
				throw Error.RuntimeError( "\(p):TwoElements" )
			}
		}
	)
	
}