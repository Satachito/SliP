import ReplaceNumeral, { code0s } from './_.js'
console.log( code0s.length )
console.log( ReplaceNumeral( '０１２３４５６７８９０１２３４５６７８９' ) )
console.log( ReplaceNumeral( '𐒠𐒡𐒢𐒣𐒤𐒥𐒦𐒧𐒨𐒩𐒠𐒡𐒢𐒣𐒤𐒥𐒦𐒧𐒨𐒩' ) )


const
$ = '𐒠𐒡𐒢𐒣𐒤𐒥𐒦𐒧𐒨𐒩'
console.log( $.length )
for ( let _ = 0; _ < $.length; _++ ) console.log( $.charCodeAt( _ ).toString( 16 ) )
