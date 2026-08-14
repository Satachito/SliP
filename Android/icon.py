#	Build the Android launcher icon from the iOS artwork, so the two cannot drift.
#
#		python3 Android/icon.py          # run from the repository root
#
#	iOS ships one square with the rounding baked in.  Android wants two layers and
#	masks them itself — circle on one launcher, squircle on the next — so dropping
#	the flattened tile in as a single image would cut its corners twice.  Both
#	layers here come from that same drawing: the gradient becomes the background,
#	extended over the whole square so the mask always lands on colour, and the
#	marks are lifted onto the foreground by how far each pixel leans from the navy
#	toward the orange, which keeps the antialiasing along their edges.
#
#	Needs Pillow.

import os
from PIL import Image

SRC  = "SwiftUI-CPP/Assets.xcassets/AppIcon.appiconset/icon_ios_1024x1024.png"
RES  = "Android/app/src/main/res"
PLAY = "AppStore/Play/icon-512.png"

TILE = ( 92, 92, 932, 932 )		#	the rounded tile inside the white margin
TOP  = ( 48, 52, 66 )			#	the gradient, sampled at the top of the tile
BOT  = ( 25, 26, 36 )			#	and at the bottom
MARK = ( 232, 91, 64 )			#	the orange of ( ) and ¡

def main():
	src = Image.open( SRC ).convert( "RGB" ).crop( TILE )
	n   = src.size[ 0 ]

	#	r - b is about -20 on the navy and +168 on the marks; in between is the
	#	edge of a stroke, and that ratio is the alpha it deserves.
	fg = Image.new( "RGBA", ( n, n ), ( 0, 0, 0, 0 ) )
	sp, fp = src.load(), fg.load()
	span = float( MARK[ 0 ] - MARK[ 2 ] )
	for y in range( n ):
		for x in range( n ):
			r, g, b = sp[ x, y ]
			a = ( r - b ) / span
			if a > 0.02: fp[ x, y ] = ( *MARK, min( 255, int( a * 255 ) ) )

	bg = Image.new( "RGB", ( n, n ) )
	bp = bg.load()
	for y in range( n ):
		t = y / float( n - 1 )
		c = tuple( int( TOP[ i ] + ( BOT[ i ] - TOP[ i ] ) * t ) for i in range( 3 ) )
		for x in range( n ): bp[ x, y ] = c

	#	Adaptive layers are 108dp, of which only the middle 72dp circle is
	#	guaranteed to survive the mask.  Mapping tile → 108 unchanged put the
	#	marks' half-diagonal at 42, and a round launcher clipped the ¡ off the
	#	right-hand side.  So measure the marks, shrink until their corners fit
	#	inside that circle, and centre them — the artwork is not quite centred in
	#	the tile either.
	#	Measured on the solid part of the marks.  getbbox() on the raw alpha also
	#	catches the faintest antialiasing, which reported the marks as half the
	#	tile again and shrank them to a third of their size.
	box = fg.getchannel( "A" ).point( lambda v: 255 if v >= 128 else 0 ).getbbox()
	if not box: raise SystemExit( "no marks found — did the artwork change?" )
	bw, bh = box[ 2 ] - box[ 0 ], box[ 3 ] - box[ 1 ]
	half   = ( ( bw / 2 ) ** 2 + ( bh / 2 ) ** 2 ) ** 0.5 * 108 / n
	scale  = min( 1.0, 34.0 / half )		#	34, not 36, to leave a hair of margin

	placed = Image.new( "RGBA", ( n, n ), ( 0, 0, 0, 0 ) )
	small  = fg.resize( ( max( 1, int( n * scale ) ), ) * 2, Image.LANCZOS )
	cx     = ( box[ 0 ] + box[ 2 ] ) / 2 * scale
	cy     = ( box[ 1 ] + box[ 3 ] ) / 2 * scale
	placed.alpha_composite( small, ( int( n / 2 - cx ), int( n / 2 - cy ) ) )

	for bucket, size in ( ( "mdpi", 108 ), ( "hdpi", 162 ), ( "xhdpi", 216 ),
	                      ( "xxhdpi", 324 ), ( "xxxhdpi", 432 ) ):
		d = f"{RES}/mipmap-{bucket}"
		os.makedirs( d, exist_ok = True )
		placed.resize( ( size, size ), Image.LANCZOS ).save( f"{d}/ic_launcher_foreground.png" )
		bg.resize( ( size, size ), Image.LANCZOS ).save( f"{d}/ic_launcher_background.png" )

	#	Play rounds the corners rather than cutting a circle, so its icon keeps
	#	the marks at the size and place iOS gives them.
	play = bg.resize( ( 512, 512 ), Image.LANCZOS ).convert( "RGBA" )
	play.alpha_composite( fg.resize( ( 512, 512 ), Image.LANCZOS ) )
	play.convert( "RGB" ).save( PLAY )
	print( f"marks {bw}x{bh} in the tile; foreground scaled {scale:.3f} for the mask" )

	print( f"{RES}/mipmap-*/ic_launcher_{{foreground,background}}.png and {PLAY}" )

main()
