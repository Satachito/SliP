#	Build Windows/slip.ico from the iOS artwork, so the two cannot drift.
#
#		python3 Windows/icon.py          # run from the repository root
#
#	Windows draws an icon as it is given — no adaptive mask, no safe zone — so
#	unlike the Android icon this one keeps the iOS tile whole, rounded corners and
#	all.  The only thing to undo is the white the artwork sits on: it has to
#	become transparent, or the icon shows as a white square on a dark taskbar.
#	Nothing in the design is anywhere near white, so whiteness is a safe test, and
#	taking alpha from how white a pixel is keeps the corners' antialiasing.
#
#	Needs Pillow.

from PIL import Image

SRC   = "SwiftUI-CPP/Assets.xcassets/AppIcon.appiconset/icon_ios_1024x1024.png"
OUT   = "Windows/slip.ico"
SIZES = ( 16, 20, 24, 32, 40, 48, 64, 128, 256 )

def main():
	src = Image.open( SRC ).convert( "RGB" )
	n   = src.size[ 0 ]

	im = Image.new( "RGBA", ( n, n ) )
	sp, ip = src.load(), im.load()
	for y in range( n ):
		for x in range( n ):
			r, g, b = sp[ x, y ]
			#	255 on the design, 0 on the white it is placed on, and the ramp
			#	between is the antialiasing along the rounded edge.
			white = min( r, g, b )
			a = 255 if white < 150 else max( 0, int( ( 240 - white ) * 255 / 90 ) )
			ip[ x, y ] = ( r, g, b, a )

	box = im.getchannel( "A" ).point( lambda v: 255 if v >= 8 else 0 ).getbbox()
	im  = im.crop( box )

	im.save( OUT, sizes = [ ( s, s ) for s in SIZES ] )
	print( f"{OUT}  {im.size[0]}px source, {len(SIZES)} sizes" )

main()
