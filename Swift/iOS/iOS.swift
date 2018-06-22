import UIKit

@UIApplicationMain class
iOSAD: UIResponder, UIApplicationDelegate {
	var window: UIWindow?
}

class
iOSVC: UIViewController {

	@IBOutlet	weak	var	oFormularTV	:	UITextView!
	@IBOutlet	weak	var	oResultTV	:	UITextView!
	@IBOutlet	weak	var	oOutputTV	:	UITextView!

	@IBAction func
	DoLoadSample( _: AnyObject ) {
		if	let wURL = ResourceURL( "Sample", "slip" )
		,	let	wData = try? Data( contentsOf: wURL )
		,	let	wString = UTF8String( wData ) {
			oFormularTV.text = wString
			oFormularTV.scrollRangeToVisible( NSRange() )
		}
	}
	
	@IBAction func
	Do( _: AnyObject ) {
		let	wContext = Context()
		let	wReader	= GUIPreProcessor( oFormularTV.text )
		oResultTV.text = ""
		oOutputTV.text = ""
		Sub {
			while true {
				do {
					let wObjects = try ReadObjects( wReader, ";" as UnicodeScalar )
					if wObjects.count == 0 { break }
					let w = try List( wObjects, .Sentence ).Eval(
						wContext
					,	{ a in Main{ self.oOutputTV.text = self.oOutputTV.text + a } }
					)
					Main{ self.oResultTV.text = self.oResultTV.text + "\(w)\n" }
				} catch let e {
					Main{ self.oResultTV.text = self.oResultTV.text + "\(e)\n" }
				}
			}
		}
	}

	@IBAction func
	DoInsert( a: UIButton ) {
		oFormularTV.insertText( a.titleLabel!.text! )
	}
}

