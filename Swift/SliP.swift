//	Written by Satoru Ogura, Tokyo.
//
import Foundation

enum
Error				:	ErrorType {
	case				ReadError( String )
	case				RuntimeError( String )
	case				UserException
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Context {
	var
	stack				:	Cell< Object >?

	func
	Push( p: Object ) {
		stack = Cell( p, stack )
	}
	func
	Pop() {
		stack = stack!.next
	}

	var
	dicts				=	Cell< [ String: Object ] >( [ String: Object ]() )

	init() {
		dicts.u[ "for" ] = Builtin(
			{	c, p in
				if let wArgs = p as? List where wArgs.u.count >= 2 {
					if let wList = wArgs.u[ 0 ] as? List {
						var	v = [ Object ]()
						for w in wList.u {
							c.Push( w )
							defer{ c.Pop() }
							v.append( try wArgs.u[ 1 ].Eval( c ) )
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
		dicts.u[ "TwoElements" ] = Builtin(
			{	c, p in
				if let w = p as? List {
					switch w.u.count {
					case 0, 1	:	return IntNumber( 0 )
					case 2		:	return IntNumber( 1 )
					default		:	return IntNumber( 2 )
					}
				} else {
					throw Error.RuntimeError( "\(p):TwoElements" )
				}
			}
		)
	}
	
	func
	Print( p: Object ) {
		fatalError()
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Object				:	NSObject {
	func
	Eval( p: Context ) throws -> Object { return self }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Name				:	Object {
	let
	u				:	String
	init(	_ p		:	String ) { u = p }
	override var
	description 	:	String { return u }
	override func
	Eval( p: Context )	throws	->	Object {
		var	wAL = p.dicts as Cell< [ String: Object ] >?
		while let w = wAL {
			if let v = w.u[ u ] { return v }
			wAL = w.next
		}
		throw Error.RuntimeError( "UndefinedName \(u)" )
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
NumberL				:	Object {
	func
	Value()			->	Float64 { fatalError() }
}

class
IntNumber			:	NumberL {
	let
	u				:	Int
	init(	_ p		:	Int ) { u = p }
	override var
	description		:	String { return "\(u)" }
	override func
	Value()			->	Float64 { return Float64( u ) }
}
class
RealNumber			:	NumberL {
	let
	u				:	Float64
	init(	_ p		:	Float64 ) { u = p }
	override var
	description		:	String { return "\(u)" }
	override func
	Value()			->	Float64 { return u
	
	
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
StringL				:	Object {
	let
	u				:	String
	init(	_ p		:	String ) { u = p }
	override var
	description	:	String { return "\"\(u)\"" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Primitive			:	Object {
	var
	u				:	Context throws -> Object
	init(	_ p		:	Context throws -> Object ) { u = p }
	override var
	description		:	String { return "P" }
	override func
	Eval( p: Context ) throws	->	Object { return try u( p ) }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Builtin				:	Object {
	var
	u				:	( Context, Object ) throws -> Object
	init(	_ p		:	( Context, Object ) throws -> Object ) { u = p }
	override var
	description		:	String { return "B" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Operator			:	Object {
	var
	bond			:	Int
	var
	u				:	( Context, Object, Object ) throws -> Object
	init(	_ p		:	( Context, Object, Object ) throws -> Object, _ pBond: Int ) { u = p; bond = pBond }
	override var
	description		:	String { return ":\(bond)" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Quote				:	Object {
	let
	u				:	Object
	init(	_ p		:	Object ) { u = p }
	override var
	description		:	String { return "'\(u)" }
	override func
	Eval( p: Context ) throws	->	Object { return u }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Combiner			:	Object {
	let
	u				:	Object
	init(	_ p		:	Object ) { u = p }
	override var
	description		:	String { return "`\(u)" }
	override func
	Eval( p: Context ) throws	->	Object { return EvalAssoc( u, p.dicts.u ) }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
EvalAssoc			:	Object {
	let
	u				:	Object
	let
	dict			:	[ String: Object ]
	init(	_ p		:	Object, _ pDict: [ String: Object ] ) { u = p; dict = pDict }
	override var
	description		:	String { return "<\(dict):\(u)>" }
	override func
	Eval( p: Context ) throws	->	Object {
		p.dicts = Cell< [ String: Object ] >( dict, p.dicts )
		defer{ p.dicts = p.dicts.next! }
		return try u.Eval( p )
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Dict				:	Object {
	let
	u				:	[ String: Object ]
	init(	_ p		:	[ String: Object ] ) { u = p }
	override var
	description		:	String { return "<\(u)>" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
var
sObject = Object()

class
List				:	Object {
	func
	EvalSentence( c: Context, _ p: [ Object ] ) throws -> Object {
//		print( ">", p )
//		let	v = try _EvalSentence( p )
//		print( "<", v )
//		return v
//	}
//	func
//	_EvalSentence( p: [ Object ] ) throws -> Object {
		switch p.count {
		case  0:	return Nil
		case  1:	return try p[ 0 ].Eval( c )
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
			return try w.u( c, EvalSentence( c, Array( p[ 0 ..< wIndex ] ) ), EvalSentence( c, Array( p[ wIndex+1 ..< p.count ] ) ) )
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
	Eval( p: Context ) throws	->	Object {
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
						v[ i ] = try w.Eval( p )
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
			p.dicts = Cell< [ String: Object ] >( [ String: Object ](), p.dicts )
			defer{ p.dicts = p.dicts.next! }
			var	v = [ Object ]( count:u.count, repeatedValue:Object() )
			for ( i, w ) in u.enumerate() { v[ i ] = try w.Eval( p ) }
			return List( v, .Literal )
		case .Sentence:
			return try EvalSentence( p, u )
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
func
IsNil( p: Object )	->	Bool {
	if let w = p as? List { return w.u.count == 0 }
	return false
}

let
T	=	StringL( "T" )

let
Nil	=	List( [], .Literal )

let
StackTop = Primitive(
	{	c in
		if let v = c.stack { return v.u }
		throw Error.RuntimeError( "StackUnderflow" )
	}
)

let
CurrentDict = Primitive( { c in return Dict( c.dicts.u ) } )

let
Exception = Primitive( { c in throw Error.UserException } )

let
Cdr = Builtin(
	{	c, p in
		if let w = p as? List where w.u.count > 0 { return List( Array( w.u.dropFirst() ), w.type ) }
		throw Error.RuntimeError( "\(p):*" )
	}
)

let
Count = Builtin(
	{	c, p in
		if let w = p as? List { return IntNumber( w.u.count ) }
		throw Error.RuntimeError( "\(p):#" )
	}
)

let
Last = Builtin(
	{	c, p in
		if let w = p as? List where w.u.count > 0 { return w.u.last! }
		throw Error.RuntimeError( "\(p):$" )
	}
)

let
Print = Builtin(
	{	c, p in
		c.Print( p )
		return p
	}
)

let
If = Operator(
	{	c, l, r in
		return try IsNil( l ) ? Nil : r.Eval( c )
	}
,	9
)

let
IfElse = Operator(
	{	c, l, r in
		if let w = r as? List where w.type == .Literal && w.u.count == 2 {
			return try IsNil( l ) ? w.u[ 1 ].Eval( c ) : w.u[ 0 ].Eval( c )
		}
		throw Error.RuntimeError( "\(l) ? \(r)" )
	}
,	9
)

let
Cons = Operator(
	{	c, l, r in
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
	{	c, l, r in
		switch r {
		case let w as IntNumber:
			if let wL = l as? List {
				if w.u < wL.u.count { return wL.u[ w.u ] } else { throw Error.RuntimeError( "Index operation \(w) to \(l)" ) }
			}
			throw Error.RuntimeError( "Index operation \(w) to \(l)" )
		case let w as Name:
			let	wR = w.u
			if let wL = l as? Dict {
				return wL.u[ w.u ] ?? Nil
			}
			throw Error.RuntimeError( "Dict operation \(w) to \(l)" )
		case let w as Builtin:
			return try w.u( c, l )
		case let w as List:
			c.Push( l )
			defer{ c.Pop() }
			return try w.Eval( c )
		case let w as EvalAssoc:
			c.Push( l )
			defer{ c.Pop() }
			return try w.Eval( c )
		default:
			throw Error.RuntimeError( "\(l) : \(r)" )
		}
	}
,	0
)

let
And = Operator(
	{	c, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.u & wR.u ) }
		return IsNil( l ) && IsNil( r ) ? T : Nil
	}
,	4
)

let
Xor = Operator(
	{	c, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.u ^ wR.u ) }
		return IsNil( l ) != IsNil( r ) ? T : Nil
	}
,	4
)

let
Or = Operator(
	{	c, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.u | wR.u ) }
		return IsNil( l ) || IsNil( r ) ? T : Nil
	}
,	4
)

let
Mul = Operator(
	{	c, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.u * wR.u ) }
		if let wL = l as? NumberL,	let wR = r as? NumberL { return RealNumber( wL.Value() * wR.Value() ) }
		throw Error.RuntimeError( "\(l) × \(r)" )
	}
,	5
)

let
Div = Operator(
	{	c, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber {
			if wR.u != 0 {
				return wL.u % wR.u == 0 ? IntNumber( wL.u + wR.u ) : RealNumber( wL.Value() / wR.Value() )
			}
		} else if let wL = l as? NumberL, let wR = r as? NumberL {
			let wRv = wR.Value()
			if wRv != 0 {
				return RealNumber( wL.Value() / wRv )
			}
		}
		throw Error.RuntimeError( "\(l) ÷ \(r)" )
	}
,	5
)

let
IDiv = Operator(
	{	c, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.u / wR.u ) }
		throw Error.RuntimeError( "\(l) / \(r)" )
	}
,	5
)

let
Remainder = Operator(
	{	c, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.u % wR.u ) }
		throw Error.RuntimeError( "\(l) % \(r)" )
	}
,	5
)

let
Add = Operator(
	{	c, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.u + wR.u ) }
		if let wL = l as? NumberL, let wR = r as? NumberL { return RealNumber( wL.Value() + wR.Value() ) }
		if let wL = l as? StringL, let wR = r as? StringL { return StringL( wL.u + wR.u ) }
		if let wL = l as? List, let wR = r as? List where wL.type == wR.type { return List( wL.u + wR.u, wL.type ) }
		throw Error.RuntimeError( "\(l) + \(r)" )
	}
,	6
)

let
Minus = Operator(
	{	c, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.u - wR.u ) }
		if let wL = l as? NumberL, let wR = r as? NumberL { return RealNumber( wL.Value() - wR.Value() ) }
		throw Error.RuntimeError( "\(l) - \(r)" )
	}
,	6
)

func
Equal( l: Object, _ r: Object ) -> Bool {
	if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() == wR.Value() }
	if let wL = l as? StringL, let wR = r as? StringL { return wL.u == wR.u }
	if let wL = l as? Name, let wR = r as? Name { return wL.u == wR.u }
	if let wL = l as? List, let wR = r as? List {
		if wL.type != wR.type { return false }
		if wL.u.count != wR.u.count { return false }
		for i in 0 ..< wL.u.count { if Equal( wL.u[ i ], wR.u[ i ] ) == Nil { return false } }
		return true
	}
	return l == r
}

let
Eq = Operator(
	{ c, l, r in Equal( l, r ) ? T : Nil }
,	8
)

let
GE = Operator(
	{	c, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.u >= wR.u ? T : Nil }
		if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() >= wR.Value() ? T : Nil }
		throw Error.RuntimeError( "\(l) >= \(r)" )
	}
,	8
)

let
LE = Operator(
	{	c, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.u <= wR.u ? T : Nil }
		if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() <= wR.Value() ? T : Nil }
		throw Error.RuntimeError( "\(l) <= \(r)" )
	}
,	8
)

let
Neq = Operator(
	{ c, l, r in Equal( l, r ) ? Nil : T }
,	8
)

let
LT = Operator(
	{	c, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.u < wR.u ? T : Nil }
		if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() < wR.Value() ? T : Nil }
		throw Error.RuntimeError( "\(l) < \(r)" )
	}
,	8
)

let
GT = Operator(
	{	c, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.u > wR.u ? T : Nil }
		if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() > wR.Value() ? T : Nil }
		throw Error.RuntimeError( "\(l) > \(r)" )
	}
,	8
)

let
Define = Operator(
	{	c, l, r in
		if let w = l as? Name {
			c.dicts.u[ w.u ] = r
			return l
		}
		throw Error.RuntimeError( "\(l) = \(r)" )
	}
,	10
)

let
Contains = Operator(
	{	c, l, r in
		if let wR = r as? List {
			return wR.u.contains( l ) ? T : Nil
		}
		throw Error.RuntimeError( "\(l) ∈ \(r)" )
	}
,	8
)

let
ContainsR = Operator(
	{	c, l, r in
		if let wL = l as? List { return wL.u.contains( r ) ? T : Nil }
		throw Error.RuntimeError( "\(l) ∋ \(r)" )
	}
,	8
)

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
	var	wRealF = false
	while let c = p.Read() {
		switch c {
		case	( "0"..."9" ), "x", "X", "+", "-":
			v += String( c )
		case	".", "e", "E":
			v += String( c )
			wRealF = true
		default:
			p.Unread( c )
			if wRealF	{ if let w = Float64( v ) { return RealNumber( neg ? -w : w ) } }
			else		{ if let w = Int( v ) { return IntNumber( neg ? -w : w ) } }
			throw Error.ReadError( "NumberFormat\(v)" )
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
ReadStr( p: Reader< UnicodeScalar > ) throws -> StringL {
	var	v = ""
	var	wEscaped = false
	while let c = p.Read() {
		if wEscaped {
			wEscaped = false
			switch c {
			case "0"	:	v.append( Character( "\0" ) )
			case "t"	:	v.append( Character( "\t" ) )
			case "n"	:	v.append( Character( "\n" ) )
			case "r"	:	v.append( Character( "\r" ) )
			default		:	v.append( Character( c ) )
			}
		} else {
			switch c {
			case "\""	:	return StringL( v )
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
		case "·"				:	return CurrentDict
		case "*"				:	return Cdr
		case "#"				:	return Count
		case "$"				:	return Last
		case "¦"				:	return Print
		case "∈"				:	return Contains
		case "∋"				:	return ContainsR
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
		default					:	throw Error.ReadError( "Invalid character \(c):\(c.value)" )
		}
	}
	return nil
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
