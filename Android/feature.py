#	The 1024x500 feature graphic for the Play listing.
#
#		python3 Android/feature.py       # run from the repository root
#
#	Built from the same two things the icon is: the marks lifted out of the iOS
#	artwork, and the navy gradient behind them.  The wordmark is set in Papyrus
#	because that is what the site's `font-family: fantasy` resolves to on macOS,
#	and the tagline in a monospace, as on the site.
#
#	Play crops this graphic on some surfaces and lays the app icon and title over
#	it on others, so everything that matters stays inside the middle — nothing
#	important within 100px of an edge.
#
#	Needs Pillow.

import os
from PIL import Image, ImageDraw, ImageFont

SRC  = "SwiftUI-CPP/Assets.xcassets/AppIcon.appiconset/icon_ios_1024x1024.png"
OUT  = "AppStore/Play/feature-1024x500.png"

TILE = ( 92, 92, 932, 932 )
TOP  = ( 52, 56, 72 )			#	opened up a little from the icon's gradient:
BOT  = ( 22, 23, 32 )			#	1024x500 is a lot of flat colour otherwise
MARK = ( 232, 91, 64 )

BRAND   = "/System/Library/Fonts/Supplemental/Papyrus.ttc"	#	= font-family: fantasy
MONO    = "/System/Library/Fonts/Menlo.ttc"

W, H = 1024, 500

def marks():
	src = Image.open( SRC ).convert( "RGB" ).crop( TILE )
	n   = src.size[ 0 ]
	fg  = Image.new( "RGBA", ( n, n ), ( 0, 0, 0, 0 ) )
	sp, fp = src.load(), fg.load()
	span = float( MARK[ 0 ] - MARK[ 2 ] )
	for y in range( n ):
		for x in range( n ):
			r, g, b = sp[ x, y ]
			a = ( r - b ) / span
			if a > 0.02: fp[ x, y ] = ( *MARK, min( 255, int( a * 255 ) ) )
	return fg.crop( fg.getchannel( "A" ).point( lambda v: 255 if v >= 128 else 0 ).getbbox() )

def main():
	im = Image.new( "RGB", ( W, H ) )
	p  = im.load()
	#	Diagonal, so the flat expanse has somewhere to go.
	for y in range( H ):
		for x in range( W ):
			t = ( x / W * 0.45 + y / H * 0.55 )
			p[ x, y ] = tuple( int( TOP[ i ] + ( BOT[ i ] - TOP[ i ] ) * t ) for i in range( 3 ) )

	m  = marks()
	mh = 190
	mw = int( m.size[ 0 ] * mh / m.size[ 1 ] )
	im.paste( m.resize( ( mw, mh ), Image.LANCZOS ), ( 118, ( H - mh ) // 2 ), m.resize( ( mw, mh ), Image.LANCZOS ) )

	d = ImageDraw.Draw( im )
	x = 118 + mw + 84

	brand = ImageFont.truetype( BRAND, 132, index = 1 )
	d.text( ( x, H // 2 - 96 ), "SliP", font = brand, fill = ( 240, 243, 248 ) )

	tag = ImageFont.truetype( MONO, 28 )
	d.text( ( x + 4, H // 2 + 52 ), "the awesome calculator", font = tag, fill = ( 150, 158, 175 ) )

	#	Play crops the edges on some surfaces.  Check, rather than assume, that
	#	nothing lands within 100px of one.
	flat = Image.new( "RGB", ( W, H ) ); fp = flat.load()
	for y in range( H ):
		for x_ in range( W ):
			t = ( x_ / W * 0.45 + y / H * 0.55 )
			fp[ x_, y ] = tuple( int( TOP[ i ] + ( BOT[ i ] - TOP[ i ] ) * t ) for i in range( 3 ) )
	from PIL import ImageChops
	box = ImageChops.difference( im, flat ).convert( "L" ).point( lambda v: 255 if v > 12 else 0 ).getbbox()
	margins = ( box[ 0 ], box[ 1 ], W - box[ 2 ], H - box[ 3 ] )
	print( f"content margins L{margins[0]} T{margins[1]} R{margins[2]} B{margins[3]}" )
	if min( margins ) < 100: print( "  ⚠ something is inside the 100px safe margin" )

	os.makedirs( os.path.dirname( OUT ), exist_ok = True )
	im.save( OUT )
	print( OUT, im.size )

main()
