//
//  main.swift
//  CUI
//
//  Created by Satoru Ogura on 2016/03/11.
//  Copyright © 2016年 Satoru Ogura. All rights reserved.
//

import Foundation

print( "SliP ver 0.2(Swift) 2016 written by Satoru Ogura, Tokyo.×÷¡¿·¬«»¦¯" )

//var	sReader	=	StdinUnicodeReader()
//while true {
//	do {
//		if let w = try Read( sReader ) { print( try w.Eval() ) } else { break }
//	} catch let e {
//		print( e )
//	}
//}


class
PreProcessor: Reader< UnicodeScalar > {
	var
	u	= String.UnicodeScalarView()
	override func
	_Read() -> UnicodeScalar? {
		while u.count == 0 {
			if let w = readLine( stripNewline: false ) {
				u = w.componentsSeparatedByString( "//" )[ 0 ].unicodeScalars
			} else {
				return nil
			}
		}
		let v = u.first
		u = u.dropFirst()
		return v
	}
}

SetupBuiltin()
var	sReader	=	PreProcessor()
while true {
	do {
		print( try List( try ReadObjects( sReader, ";" as UnicodeScalar ), .Sentence ).Eval() )
	} catch let e {
		print( e )
	}
}
