//	Written by Satoru Ogura, Tokyo. 2018 -
//

import Foundation

////////////////////////////////////////////////////////////////
enum
LispError	:	Error {
	case	ReadError		( String )
	case	RuntimeError	( String )
}

class
Object {
}

class
Cell: Object {
	let
	car: Object?
	let
	cdr: Object?
	init( _ pCar: Object?, _ pCdr: Object? ) {
		car = pCar
		cdr = pCdr
	}
}

class
Atom: Object {
	let
	m:	String
	init( _ p: String ) { m = p }
}

////////

func
Builtins() -> [ String: Object ] {
	var	v = [ String: Object ]()
	return v
}

var
dicts: Chain< [ String: Object ] >? = Chain( Builtins())

var
stack: Chain< Object >?
func
Push( _ p: Object ) {
	stack = Chain( p, stack )
}
func
Pop() {
	guard stack != nil else { fatalError() }
	stack = stack!.next
}

////////

var
sTerminators = CharacterSet.whitespacesAndNewlines.union( CharacterSet( charactersIn: "()." ) )
func
ReadObject( _ first: Character, _ r: CharacterReader ) -> Object {
	var	v = String( first )
	do {
		repeat {
			let	w = try r.Read()
			if sTerminators.contains( w ) {
				r.Unread( w )
				break
			}
			v.append( w )
		} while true
	} catch ReaderError.eod {
	} catch {
		fatalError()
	}
	return Atom( v )
}

func
ReadList( _ r: CharacterReader ) throws -> Object? {
	do {
		try r.SkipWhite()
		let w = try r.Read()
		switch w {
		case ".":	try r.SkipWhite()
					return ReadObject( try r.Read(), r )
		case ")":	return nil
		case "(":	let	wCell = try ReadList( r )
					return Cell( wCell, try ReadList( r ) )
		default	:	let	wAtom = ReadObject( w, r )
					return Cell( wAtom, try ReadList( r ) )
		}
	} catch ReaderError.eod {
		throw LispError.ReadError( "Unterminated list" )
	} catch {
		fatalError()
	}
}

func
Read( _ r: CharacterReader ) throws -> Object? {
	try r.SkipWhite()
	let w = try r.Read()
	switch w {
	case	")"					:	throw LispError.ReadError( "Unexpected )" )
	case	"("					:	return try ReadList( r )
	default						:	return ReadObject( w, r )
	}
}

func
Print( _ p: Object?, _ pAfterParen: Bool = false, _ pLevel: Int = 0 ) {
	if !pAfterParen { for _ in 0 ..< pLevel { print( "\t", terminator: "" ) } }
	switch p {
	case let wAtom as Atom:
		print( wAtom.m )
	case is Cell:
		print( "(", terminator: "\t" )
		var	wP = p
		var	wInit = true
		while let wCell = wP as? Cell {
			Print( wCell.car, wInit, pLevel + 1 )
			wInit = false
			wP = wCell.cdr
		}
		for _ in 0 ..< pLevel { print( "\t", terminator: "" ) }
		if let wAtom = wP as? Atom { print( ". " + wAtom.m + " ", terminator: "" ) }
		print( ")" )
	default:
		print( "nil" )
	}
}

let
T = Atom( "T" )

func
Equal( _ l: Object?, _ r: Object? ) -> Bool {
	var	wL = l
	var	wR = r
	while let wLC = wL as? Cell, let wRC = wR as? Cell {
		if !Equal( wLC.car, wRC.car ) { return false }
		wL = wLC.cdr
		wR = wRC.cdr
	}
	if wL == nil, wR == nil { return true }
	if let wL = l as? Atom, let wR = r as? Atom { return wL.m == wR.m }
	return false
}

func
Eval( _ p: Object? ) throws -> Object? {
	if let wCell = p as? Cell {
		if let wAtom = wCell.car as? Atom {
			if let wArgs = wCell.cdr as? Cell {
				switch wAtom.m {
				case "atom":
					guard !( wArgs.cdr is Cell ) else { throw LispError.RuntimeError( "extra argument(s) to: " + wAtom.m ) }
					return wArgs.car is Cell ? nil : T
				case "eq":
					guard let wArg2 = wArgs.cdr as? Cell else { throw LispError.RuntimeError( "Insufficient arguments to: " + wAtom.m ) }
					guard !( wArg2.cdr is Cell ) else { throw LispError.RuntimeError( "extra argument(s) to: " + wAtom.m ) }
					return Equal( wArgs.car, wArg2.car ) ? T : nil
				case "car":
					guard !( wArgs.cdr is Cell ) else { throw LispError.RuntimeError( "extra argument(s) to: " + wAtom.m ) }
					guard let wArg = wArgs.car as? Cell else { throw LispError.RuntimeError( "Apply car to atom." ) }
					return wArg.car
				case "cdr":
					guard !( wArgs.cdr is Cell ) else { throw LispError.RuntimeError( "extra argument(s) to: " + wAtom.m ) }
					guard let wArg = wArgs.car as? Cell else { throw LispError.RuntimeError( "Apply car to atom." ) }
					return wArg.cdr
				case "cons":
					guard let wArg2 = wArgs.cdr as? Cell else { throw LispError.RuntimeError( "Insufficient arguments to: " + wAtom.m ) }
					guard !( wArg2.cdr is Cell ) else { throw LispError.RuntimeError( "extra argument(s) to: " + wAtom.m ) }
					return Cell( wArgs.car, wArg2.car )
				default:
					throw LispError.RuntimeError( "Unknown function: " + wAtom.m )
				}
			} else {
				throw LispError.RuntimeError( "No argument to: " + wAtom.m )
			}
		} else {
			throw LispError.RuntimeError( "No function to eval." )
		}
	} else {
		return p
	}
}

/*
()


( atom () )
( atom a )
( atom ( a ) )

( atom a b )
( atom )


( eq () () )
( eq a a )
( eq ( a ) ( a ) )
( eq ( a ( b ) ) ( a ( b ) ) )
( eq a b )
( eq ( a ) ( b ) )
( eq a ( a ) )
( eq ( a ) a )
( eq () a )
( eq a () )
( eq () ( a ) )
( eq ( a ) () )

( eq a b c )
( eq a )


( car ( a ) )

( car )
( car a )
( car ( a ) a )


( cdr ( a ) )
( cdr ( a b ) )
( cdr ( a.b ) )

( cdr )
( cdr a )
( cdr ( a ) a )

( cons a (b) )
( cons a b )
( cons a )
( cons )
( cons a b c )
*/

