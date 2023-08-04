const
code0sBMP = [
	0x30
,	0x0660
,	0x06F0
,	0x07C0
,	0x0966
,	0x09E6
,	0x0A66
,	0x0AE6
,	0x0B66
,	0x0BE6
,	0x0C66
,	0x0CE6
,	0x0D66
,	0x0DE6
,	0x0E50
,	0x0ED0
,	0x0F20
,	0x1040
,	0x1090
,	0x17E0
,	0x1810
,	0x1946
,	0x19D0
,	0x1A80
,	0x1A90
,	0x1B50
,	0x1BB0
,	0x1C40
,	0x1C50
,	0xA620
,	0xA8D0
,	0xA900
,	0xA9D0
,	0xA9F0
,	0xAA50
,	0xABF0
,	0xFF10
]
const
singleLists = [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ].map( $ => code0sBMP.map( _ => _ + $ ) )

const
code0sSMP = [
	0x104A0
,	0x10D30
,	0x11066
,	0x110F0
,	0x11136
,	0x111D0
,	0x112F0
,	0x11450
,	0x114D0
,	0x11650
,	0x116C0
,	0x11730
,	0x118E0
,	0x11950
,	0x11C50
,	0x11D50
,	0x11DA0
,	0x16A60
,	0x16AC0
,	0x16B50
,	0x1D7CE
,	0x1D7D8
,	0x1D7E2
,	0x1D7EC
,	0x1D7F6
,	0x1E140
,	0x1E2F0
,	0x1E950
,	0x1FBF0
]

export const
code0s = [ ...code0sBMP, ...code0sSMP ]

const
surrogatePair = _ => [ ( 0xd800 | _ >> 10 ) - 0x40, 0xdc00 | ( _ & 0x3ff ) ]

const
surrogatePairLists = [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ].map( $ => code0sSMP.map( _ => surrogatePair( _ + $ ) ) )

const
surrogateHs = Array.from( new Set( surrogatePairLists[ 0 ].map( _ => _[ 0 ] ) ) )

const
ReplaceNumeral = _ => {

	const charCodes = Array.from( { length: _.length } ).map( ( $, i ) => _.charCodeAt( i ) )
	{	let	_ = charCodes.length
		while ( _-- ) {
			const $ = charCodes[ _ ]
			singleLists.forEach(
				( singleList, i ) => singleList.includes( $ ) && charCodes.splice( _, 1, 0x30 + i ) 
			)
		}
	}

	{	let	_ = charCodes.length - 1
		while ( _-- ) {
			const $0 = charCodes[ _ ]
			if ( !surrogateHs.includes( $0 ) ) continue
			const $1 = charCodes[ _ + 1 ]
			surrogatePairLists.forEach(
				( surrogatePairList, i ) => surrogatePairList.some(
					_ => _[ 0 ] === $0 && _[ 1 ] === $1
				) && charCodes.splice( _, 2, 0x30 + i )
			)
		}
	}

	return String.fromCharCode( ...charCodes )
}

export default
ReplaceNumeral

