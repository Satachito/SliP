//	Written by Satoru Ogura, Tokyo.
//
import Foundation

enum
SliPError	:	Error {
	case	ReadError( String )
	case	RuntimeError( String )
	case	UserException
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Context {
	var
	stack			:	Cell< Object >?

	func
	Push( _ o: Object ) {
		stack = Cell( o, stack )
	}
	func
	Pop() {
		stack = stack!.next
	}

	var
	dicts			:	Cell< [ String: Object ] >

	init( _ c: Context ) {
		dicts = c.dicts
		stack = c.stack
	}
	
	init() {
		dicts = Cell< [ String: Object ] >( [ String: Object ]() )
		Register( dicts )
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Object				:	NSObject {
	func
	Eval( _ c: Context, _ p: @escaping ( String ) -> () ) throws -> Object { return self }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Name				:	Object {
	let
	m				:	String
	init(	_ a		:	String ) { m = a }
	override var
	description 	:	String { return m }
	override func
	Eval( _ c: Context, _ p: @escaping ( String ) -> () ) throws -> Object {
		var	wAL = c.dicts as Cell< [ String: Object ] >?
		while let w = wAL {
			if let v = w.m[ m ] { return v }
			wAL = w.next
		}
		throw SliPError.RuntimeError( "UndefinedName \(m)" )
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
	m				:	BigInteger
	init(	_ a		:	BigInteger ) { m = a }
	init(	_ a		:	Int ) { m = MakeBigInteger( a ) }
	override var
	description		:	String { return "\(m)" }
	override func
	Value()			->	Float64 { return m.Float() }
}
class
RealNumber			:	NumberL {
	let
	m				:	Float64
	init(	_ a		:	Float64 ) { m = a }
	override var
	description		:	String { return "\(m)" }
	override func
	Value()			->	Float64 { return m }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
StringL				:	Object {
	let
	m				:	String
	init(	_ a		:	String ) { m = a }
	override var
	description		:	String { return "\"\(m)\"" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Primitive			:	Object {
	var
	m				:	( Context ) throws -> Object
	init(	_ a		:	@escaping ( Context ) throws -> Object ) { m = a }
	override var
	description		:	String { return "P" }
	override func
	Eval( _ c: Context, _ p: @escaping ( String ) -> () ) throws	->	Object { return try m( c ) }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Builtin				:	Object {
	var
	m				:	( Context, @escaping ( String ) -> (), Object ) throws -> Object
	init(	_ a		:	@escaping ( Context, @escaping ( String ) -> (), Object ) throws -> Object ) { m = a }
	override var
	description		:	String { return "B" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Operator			:	Object {
	var
	bond			:	Int
	var
	m				:	( Context, @escaping ( String ) -> (), Object, Object ) throws -> Object
	init(	_ a		:	@escaping ( Context, @escaping ( String ) -> (), Object, Object ) throws -> Object, _ pBond: Int ) { m = a; bond = pBond }
	override var
	description		:	String { return ":\(bond)" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Quote				:	Object {
	let
	m				:	Object
	init(	_ a		:	Object ) { m = a }
	override var
	description		:	String { return "'\(m)" }
	override func
	Eval( _ c: Context, _ p: @escaping ( String ) -> () ) throws	->	Object { return m }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Combiner			:	Object {
	let
	m				:	Object
	init(	_ a		:	Object ) { m = a }
	override var
	description		:	String { return "`\(m)" }
	override func
	Eval( _ c: Context, _ p: @escaping ( String ) -> () ) throws	->	Object { return EvalAssoc( m, c.dicts.m ) }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
EvalAssoc			:	Object {
	let
	m				:	Object
	let
	dict			:	[ String: Object ]
	init(	_ a		:	Object, _ pDict: [ String: Object ] ) { m = a; dict = pDict }
	override var
	description		:	String { return "<\(dict):\(m)>" }
	override func
	Eval( _ c: Context, _ p: @escaping ( String ) -> () ) throws	->	Object {
		c.dicts = Cell< [ String: Object ] >( dict, c.dicts )
		defer{ c.dicts = c.dicts.next! }
		return try m.Eval( c, p )
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Dict				:	Object {
	let
	m				:	[ String: Object ]
	init(	_ a		:	[ String: Object ] ) { m = a }
	override var
	description		:	String { return "<\(m)>" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class
List				:	Object {
	static	let
	object = Object()

	func
	EvalSentence( _ c: Context, _ p: @escaping ( String ) -> (), _ o: [ Object ] ) throws -> Object {
		switch o.count {
		case  1:	return try o[ 0 ].Eval( c, p )
		case  0, 2:	throw SliPError.RuntimeError( "No operator in \(List( o, .Sentence ))" )
		default:	break
		}
		var wTarget :	Operator?
		var wIndex  =	0
		for i in 1 ..< o.count - 1 {
			if let w = o[ i ] as? Operator {
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
			return try w.m( c, p, EvalSentence( c, p, Array( o[ 0 ..< wIndex ] ) ), EvalSentence( c, p, Array( o[ wIndex+1 ..< o.count ] ) ) )
		} else {
			throw SliPError.RuntimeError( "No operator in \(List( o, .Sentence ))" )
		}
	}
	enum
	ListType {
		case Literal
		case Parallel
		case Block
		case Sentence
	}
	let
	type			:	ListType
	let
	m				:	[ Object ]
	init(	_ a		:	[ Object ], _ pType: ListType ) { m = a; type = pType }
	func
	_Str()			->	String {
 		var	v = " "
		for w in m { v += "\(w) " }
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
	Eval( _ c: Context, _ p: @escaping ( String ) -> () ) throws	->	Object {
		switch type {
		case .Literal:
			return self
		case .Parallel:
			var	v = [ Object ]( repeating: List.object, count: m.count )
			let	wG = DispatchGroup()
			let	wQ = DispatchQueue.global()
			
			for ( i, w ) in m.enumerated() {
				wQ.async( group: wG ) {
					do {
						v[ i ] = try w.Eval( Context( c ), p )
					} catch let e {
						print( e )
					}
				}
			}
			wG.wait()
			for w in m {
				if w == List.object { throw SliPError.RuntimeError( "Parallel" ) }	//	UC
			}
			return List( v, .Literal )
		case .Block:
			c.dicts = Cell< [ String: Object ] >( [ String: Object ](), c.dicts )
			defer{ c.dicts = c.dicts.next! }
			var	v = [ Object ]( repeating: Object(), count: m.count )
			for ( i, w ) in m.enumerated() { v[ i ] = try w.Eval( c, p ) }
			return List( v, .Literal )
		case .Sentence:
			return try EvalSentence( c, p, m )
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
func
IsNil( _ o: Object )	->	Bool {
	if let w = o as? List { return w.m.count == 0 }
	return false
}

let
T	=	StringL( "T" )

let
Nil	=	List( [], .Literal )

let
StackTop = Primitive(
	{	c in
		if let v = c.stack { return v.m }
		throw SliPError.RuntimeError( "StackUnderflow" )
	}
)

let
CurrentDict = Primitive( { c in return Dict( c.dicts.m ) } )

let
Exception = Primitive( { c in throw SliPError.UserException } )

let
Cdr = Builtin(
	{	c, p, a in
		if let w = a as? List, w.m.count > 0 { return List( Array( w.m.dropFirst() ), w.type ) }
		throw SliPError.RuntimeError( "\(a):*" )
	}
)

let
Count = Builtin(
	{	c, p, a in
		if let w = a as? List { return IntNumber( w.m.count ) }
		throw SliPError.RuntimeError( "\(a):#" )
	}
)

let
Last = Builtin(
	{	c, p, a in
		if let w = a as? List, w.m.count > 0 { return w.m.last! }
		throw SliPError.RuntimeError( "\(a):$" )
	}
)

let
Print = Builtin(
	{	c, p, a in
		switch a {
		case let w as IntNumber	:	p( "\(a)" )
		case let w as RealNumber:	p( "\(a)" )
		case let w as StringL	:	p( w.m )
		case let w as Name		:	p( w.m )
		default:					break
		}
		return a
	}
)

let
If = Operator(
	{	c, p, l, r in
		return try IsNil( l ) ? Nil : r.Eval( c, p )
	}
,	9
)

let
IfElse = Operator(
	{	c, p, l, r in
		if let w = r as? List, w.type == .Literal && w.m.count == 2 {
			return try IsNil( l ) ? w.m[ 1 ].Eval( c, p ) : w.m[ 0 ].Eval( c, p )
		}
		throw SliPError.RuntimeError( "\(l) ? \(r)" )
	}
,	9
)

let
Cons = Operator(
	{	c, p, l, r in
		if let w = r as? List {
			var	wM = w.m
			wM.insert( l, at: 0 )
			return List( wM, w.type )
		}
		throw SliPError.RuntimeError( "\(l) , \(r)" )
	}
,	7
)

let
Apply = Operator(
	{	c, p, l, r in
		switch r {
		case let w as IntNumber:
			if let wL = l as? List {
				let	wIndex = w.m.RemainderE()
				if wIndex < wL.m.count {
					return wL.m[ wIndex ]
				} else {
					throw SliPError.RuntimeError( "Index operation \(w) to \(l)" )
				}
			}
			throw SliPError.RuntimeError( "Index operation \(w) to \(l)" )
		case let w as Name:
			let	wR = w.m
			if let wL = l as? Dict {
				return wL.m[ w.m ] ?? Nil
			}
			throw SliPError.RuntimeError( "Dict operation \(w) to \(l)" )
		case let w as Builtin:
			return try w.m( c, p, l )
		case let w as List:
			c.Push( l )
			defer{ c.Pop() }
			return try w.Eval( c, p )
		case let w as EvalAssoc:
			c.Push( l )
			defer{ c.Pop() }
			return try w.Eval( c, p )
		default:
			throw SliPError.RuntimeError( "\(l) : \(r)" )
		}
	}
,	0
)

let
And = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m & wR.m ) }
		return IsNil( l ) && IsNil( r ) ? T : Nil
	}
,	4
)

let
Xor = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m ^ wR.m ) }
		return IsNil( l ) != IsNil( r ) ? T : Nil
	}
,	4
)

let
Or = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m | wR.m ) }
		return IsNil( l ) || IsNil( r ) ? T : Nil
	}
,	4
)

let
Mul = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m * wR.m ) }
		if let wL = l as? NumberL,	let wR = r as? NumberL { return RealNumber( wL.Value() * wR.Value() ) }
		throw SliPError.RuntimeError( "\(l) × \(r)" )
	}
,	5
)

let
Div = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber {
			if !IsZero( wR.m ) {
				do {
					return try IsZero( wL.m % wR.m ) ? IntNumber( wL.m / wR.m ) : RealNumber( wL.Value() / wR.Value() )
				} catch {
				}
			}
		} else if let wL = l as? NumberL, let wR = r as? NumberL {
			let wRv = wR.Value()
			if wRv != 0 {
				return RealNumber( wL.Value() / wRv )
			}
		}
		throw SliPError.RuntimeError( "\(l) ÷ \(r)" )
	}
,	5
)

let
IDiv = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber {
			do {
				return try IntNumber( wL.m / wR.m )
			} catch {
			}
		}
		throw SliPError.RuntimeError( "\(l) / \(r)" )
	}
,	5
)

let
Remainder = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber {
			do {
				return try IntNumber( wL.m % wR.m )
			} catch {
			}
		}
		throw SliPError.RuntimeError( "\(l) % \(r)" )
	}
,	5
)

let
Add = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m + wR.m ) }
		if let wL = l as? NumberL, let wR = r as? NumberL { return RealNumber( wL.Value() + wR.Value() ) }
		if let wL = l as? StringL, let wR = r as? StringL { return StringL( wL.m + wR.m ) }
		if let wL = l as? List, let wR = r as? List, wL.type == wR.type { return List( wL.m + wR.m, wL.type ) }
		throw SliPError.RuntimeError( "\(l) + \(r)" )
	}
,	6
)

let
Minus = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m - wR.m ) }
		if let wL = l as? NumberL, let wR = r as? NumberL { return RealNumber( wL.Value() - wR.Value() ) }
		throw SliPError.RuntimeError( "\(l) - \(r)" )
	}
,	6
)

func
== ( l: Object, r: Object ) -> Bool {
//	return Equal( l, r )
	if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() == wR.Value() }
	if let wL = l as? StringL, let wR = r as? StringL { return wL.m == wR.m }
	if let wL = l as? Name, let wR = r as? Name { return wL.m == wR.m }
	if let wL = l as? List, let wR = r as? List {
		if wL.type != wR.type { return false }
		if wL.m.count != wR.m.count { return false }
		for i in 0 ..< wL.m.count { if !( wL.m[ i ] == wR.m[ i ] ) { return false } }
		return true
	}
	return l as NSObject == r as NSObject
}

let
Eq = Operator(
	{ c, p, l, r in l == r ? T : Nil }
,	8
)

let
GE = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m >= wR.m ? T : Nil }
		if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() >= wR.Value() ? T : Nil }
		throw SliPError.RuntimeError( "\(l) >= \(r)" )
	}
,	8
)

let
LE = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m <= wR.m ? T : Nil }
		if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() <= wR.Value() ? T : Nil }
		throw SliPError.RuntimeError( "\(l) <= \(r)" )
	}
,	8
)

let
Neq = Operator(
	{ c, p, l, r in l == r ? Nil : T }
,	8
)

let
LT = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m < wR.m ? T : Nil }
		if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() < wR.Value() ? T : Nil }
		throw SliPError.RuntimeError( "\(l) < \(r)" )
	}
,	8
)

let
GT = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m > wR.m ? T : Nil }
		if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() > wR.Value() ? T : Nil }
		throw SliPError.RuntimeError( "\(l) > \(r)" )
	}
,	8
)

let
Define = Operator(
	{	c, p, l, r in
		if let w = l as? Name {
			c.dicts.m[ w.m ] = r
			return l
		}
		throw SliPError.RuntimeError( "\(l) = \(r)" )
	}
,	10
)

let
MemberOf = Operator(
	{	c, p, l, r in
		if let wR = r as? List {
			return wR.m.contains( l ) ? T : Nil
		}
		throw SliPError.RuntimeError( "\(l) ∈ \(r)" )
	}
,	8
)

let
Contains = Operator(
	{	c, p, l, r in
		if let wL = l as? List {
			return wL.m.contains( r ) ? T : Nil
		}
		throw SliPError.RuntimeError( "\(l) ∋ \(r)" )
	}
,	8
)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


func
ReadNumber( _ r: Reader< UnicodeScalar >, _ neg: Bool = false ) throws -> Object {
 	var	v = ""
	var	wRealF = false
	while true {
		let u = try r.Read()
		switch u {
		case	( "0"..."9" ), "+", "-":
			v += String( u )
		case	".", "e", "E":
			v += String( u )
			wRealF = true
		default:
			r.Unread( u )
			if wRealF	{ if let w = Float64( v ) { return RealNumber( neg ? -w : w ) } }
			else		{ return IntNumber( MakeBigInteger( v, 10, neg ) ) }
			throw SliPError.ReadError( "NumberFormat\(v)" )
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
			v += String( u )
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
			default		:	v += String( u )
			}
		}
	}
}

func
ReadObjects( _ r: Reader< UnicodeScalar >, _ terminator: UnicodeScalar ) throws -> [ Object ] {
	var	v = [ Object ]()
	while let w = try Read( r, terminator ) { v.append( w ) }
	return v
}

func
Read( _ r: Reader< UnicodeScalar >, _ terminator: UnicodeScalar = UnicodeScalar( 0 ) ) throws -> Object? {
	try SkipWhite( r )
	let u = try r.Read()
	switch u {
	case terminator			:	return nil
	case "}", "]", ")", "»"	:	throw SliPError.ReadError( "Unexpected \(u)" )
	case "["				:	return List( try ReadObjects( r, "]" as UnicodeScalar ), .Literal )
	case "«"				:	return List( try ReadObjects( r, "»" as UnicodeScalar ), .Parallel )
	case "("				:	return List( try ReadObjects( r, ")" as UnicodeScalar ), .Sentence )
	case "{"				:	return List( try ReadObjects( r, "}" as UnicodeScalar ), .Block )
	case "\""				:	return try ReadStr( r )
	case "'"				:	if let w = try Read( r ) { return Quote( w ) }; throw SliPError.ReadError( "No object to quote" )
	case "`"				:	if let w = try Read( r ) { return Combiner( w ) }; throw SliPError.ReadError( "No object to combine" )
	case "¡"				:	return Exception
	case "@"				:	return StackTop
	case "·"				:	return CurrentDict
	case "*"				:	return Cdr
	case "#"				:	return Count
	case "$"				:	return Last
	case "¦"				:	return Print
	case "∈"					:	return MemberOf
	case "∋"					:	return Contains
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
		let w = try r.Read()
		r.Unread( w )
		switch w {
		case ( "0"..."9" )
		,	 "."			:	return try ReadNumber( r )
		default				:	return Add
		}
	case "-"				:
		let w = try r.Read()
		r.Unread( w )
		switch w {
		case ( "0"..."9" )
		,	 "."			:	return try ReadNumber( r, true )
		default				:	return Minus
		}
	case "="			:
		let w = try r.Read()
		switch w {
		case "="			:	return Eq
		case ">"			:	return GE
		case "<"			:	return LE
		default				:	r.Unread( w )
								return Define
		}
	case "<"				:
		let w = try r.Read()
		switch w {
		case ">"			:	return Neq
		case "="			:	return LE
		default				:	r.Unread( w )
								return LT
		}
	case ">"				:
		let w = try r.Read()
		switch w {
		case "<"			:	return Neq
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
GUIPreProcessor: Reader< UnicodeScalar > {
	var
	m		: String.UnicodeScalarView

	init( _ p: String ) {
		var wLines = p.components( separatedBy: NSCharacterSet.newlines )
		for ( i, w ) in wLines.enumerated() {
			wLines[ i ] = w.components( separatedBy: "//" )[ 0 ]
			if wLines[ i ].unicodeScalars.last == "=" {
				wLines[ i ] = wLines[ i ].dropLast() + ":¦;"
			}
		}
		m = ( wLines as NSArray ).componentsJoined( by: "\n" ).unicodeScalars
	}

	override func
	_Read() throws -> UnicodeScalar {
		if m.count == 0 { throw ReaderError.eod }
		let v = m.first!
		m = String.UnicodeScalarView( m.dropFirst() )
		return v
	}
}



