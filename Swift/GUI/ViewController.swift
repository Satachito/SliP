import Cocoa

class
ViewController: NSViewController {

	func
	document() -> Document? {
		return view.window?.windowController?.document as? Document
	}
	
	@IBAction func
	Do( _: AnyObject ) {
		if let w = document() { w.Do() }
		print( "SliP ver 0.2(Swift) 2016 written by Satoru Ogura, Tokyo.×÷¡¿·¬«»¦¯" )
	}
}

