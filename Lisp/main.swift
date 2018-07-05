//	Written by Satoru Ogura, Tokyo. 2018 -
//

import Foundation

class
Chain<T>	{
	var
	m			: T
	let
	next		: Chain?
	init(	_ p	: T, _ pNext: Chain? = nil ) { m = p; next = pNext }
}

enum
ReaderError	: Error {
case		eod
}

class
Reader< T > {
	var
	_unread : Chain< T >?
	func
	_Read() throws -> T { throw ReaderError.eod }
	func
	Read() throws -> T {
		if let v = _unread { _unread = v.next ; return v.m }
		return try _Read()
	}
	func
	Unread( _ p: T ) { _unread = Chain< T >( p, _unread ) }
}

extension CharacterSet {
	func
	contains( _ p: Character ) -> Bool {
		var	wI = String( p ).unicodeScalars.makeIterator()
		while let w = wI.next() {
			if contains( w ) { return true }
		}
		return false
	}
}

class
CharacterReader: Reader< Character > {
	func
	SkipWhite() throws {
		while true {
			let w = try Read()
			if !CharacterSet.whitespacesAndNewlines.contains( w ) {
				Unread( w )
				break
			}
		}
	}
}

class
StdinCharacterReader: CharacterReader {
	var
	m		: IndexingIterator< [ Character ] >?
	override func
	_Read() throws -> Character {
		repeat {
			if m == nil {
				guard let w = readLine( strippingNewline: false ) else { throw ReaderError.eod }
				m = [ Character ]( w ).makeIterator()
			}
			if let v = m!.next() { return v }
			m = nil
		} while true
	}
}

class
StringCharacterReader: CharacterReader {
	var
	m		: IndexingIterator< [ Character ] >

	init( _ p: String ) {
		m = [ Character ]( p ).makeIterator()
	}

	override func
	_Read() throws -> Character {
		guard let v = m.next() else { throw ReaderError.eod }
		return v
	}
}

repeat {
	do {
		Print( try Eval( Read( StdinCharacterReader() ) ) )
	} catch let e {
		print( e )
	}
} while true
