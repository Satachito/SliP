export default `//
//	Samples for normal mode
//

2
-3
2-3
2--3

2+3
2-3
2×3
2÷3
2+3×4
(2+3)×4
2×3+4
2×(3+4)
6÷2×3

1+2+3-4+5+6+78+9
123-45-67+89
1×2×3×4+5+6+7×8+9
1+2+3+4+5+6+7+8×9
1×2×3-4×5+6×7+8×9
123+45-67+8-9
123-4-5-6-7+8-9
1+2+34-5+67-8+9
1+23-4+5+6+78-9
12+3+4+5-6-7+89
123+4-5+67-89
12-3-4+5-6+7+89
-1+2-3+4+5+6+78+9
1+23-4+56+7+8+9
98-76+54+3+21
98+7-6+5-4+3-2-1
98+7-6×5+4×3×2+1

sqrt(4)
cbrt(8)

π
sin(0)
sinπ
cos(0)
cosπ
tan(0)
tanπ

e
exp(0)
exp(1)
exp(-1)
exp(2)

log( e )
log( e × e )

'log8 = '( log( @ ) ÷ log( 8 ) )
512:log8

//	 Variables
'a = 2
3a
'b = 3
a b
'r = 2
2πr

//	 String

""
"ABC"
"A\\tB\\"C"
"ABC" + "DEF"

//	 Memory

'a = 1 + 2
'b = 3 + 4
a + b


//	 Logical
//	"T" for true, Nil i.e. '[ ]' for false.

3 == 3
3 == 4

"A" == "A"

"A" == "B"


//	 If else

3 == 3 ? [ "TRUE" "FALSE" ]
3 == 4 ? [ "TRUE" "FALSE" ]

//	 List
[ a b ]
'[ a b ]
'( a b )
'{ a b }
[ a b c ]:#


//	 Primitives
3:'(@)
3:'@

3:'(@==3)

'[]:'(@=={})
'{}:'(@=={})
'{}:'(@=='{})

//	 Function

'add1 = '( @ + 1 )
3 : add1
//	'3 : add1' means push 3 to stack and evaluate function associated with name 'add1'.
//	@ is stack top.

'factorial = '( @ == 1 ? [ 1 ( @ × ( @ - 1 ):factorial ) ] )
4 : factorial

'sigma = '( @ == 0 ? [ 0 ( @ + ( @ - 1 ):sigma ) ] )
4 : sigma

//	 List

[ "A" "B" "C" ] + [ "D" "E" "F" ]
//	Lisp's CAR
[ "A" "B" "C" ]:0
[ "A" "B" "C" ]:1
[ "A" "B" "C" ]:$
[ "A" "B" "C" ]:#
//	Lisp's CDR
[ "A" "B" "C" ]:*

//	Lisp's CONS
"A", [ "B" "C" ]

//	 Members

"A" ∈ [ "A" "B" "C" ]
"D" ∈ [ "A" "B" "C" ]
[ "A" "B" "C" ] ∋ "A"
[ "A" "B" "C" ] ∋ "D"

//	 String <-> Integer

[ 65536 16 ] : string
[ "10000" 16 ] : int

//	 byJSON, toJSON
\`{ "A":1, "B":[ 2, 3, 4 ], "C":{ "D":[ 4, 5, 6 ] } }\`:byJSON
\`{ "A":1, "B":[ 2, 3, 4 ], "C":{ "D":[ 4, 5, 6 ] } }\`:byJSON:'C
\`{ "A":1, "B":[ 2, 3, 4 ], "C":{ "D":[ 4, 5, 6 ] } }\`:byJSON:toJSON

//	 Matrix
[ 1 2 3 ] · [ 1 2 3 ]
[ [ 1 2 3 4 5 6 ] 3 ] : matrix
[ [ 1 2 3 4 5 6 ] 2 ] : matrix · [ [ 1 2 3 4 5 6 ] 3 ]: matrix

//	 Builtins
"Show all"
¤


///////////////////////////////////	MATH

//	https://developer.mozilla.org/ja/docs/Web/JavaScript/Reference/Global_Objects/Math

π
e
∞
-∞

abs(-1)
//	1
abs(2)
//	2
acos 0.8
//	0.6435011087932843
acos ( 1.1 )
//	NaN
acosh(1)
//	0
acosh(2)
//	1.3169578969248166
asin 0.6
//	0.6435011087932844
asin 1.1
//	NaN
asinh(1)
//	0.881373587019543
asinh(0)
//	0
asinh(-1)
//	-0.881373587019543
asinh(2)
//	1.4436354751788103
atan 0.8
//	0.6747409422235527
atan( 5 ÷ 3 )
//	1.0303768265243125
atan2[ 1 1 ]
//	0.7853981633974483
'calcAngleDegrees = '( atan2[ ( @:1 ) (@:0 ) ] × 180 ÷ π ) 
[ 5 5 ]:calcAngleDegrees
//	45
[ 0 10 ]:calcAngleDegrees
//	90
atanh(-1)
//	-∞
atanh(0)
//	0
atanh(0.5)
//	0.549306144334055 (approximately)
atanh(1)
//	∞
cbrt(-1)
//	-1
cbrt(1)
//	1
cbrt( ∞ )
//	∞
cbrt(64)
//	4
ceil(0.95)
//	1
ceil(4)
//	4
ceil(7.004)
//	8
ceil(-7.004)
//	-7
'getCircleX = '( cos( @:0 ) × @:1 )
[ 1 10 ]:getCircleX
//	5.403023058681398
[ 2 10 ]:getCircleX
//	-4.161468365471424
{π 10 }:getCircleX
//	-10
«π 10 »:getCircleX
//	-10
cosh(0)
//	1
cosh(1)
//	1.543080634815244 (approximately)
cosh(-1)
//	1.543080634815244 (approximately)
cosh(2)
//	3.7621956910836314
exp(0)
//	1
exp(1)
//	2.718281828459 (approximately)
exp(-1)
//	0.36787944117144233
exp(2)
//	7.38905609893065
floor(5.95)
//	5
floor(5.05)
//	5
floor(5)
//	5
floor(-5.05)
//	-6
hypot[ 3 4 ]
//	5
hypot[ 5 12 ]
//	13
hypot[ 3 4 5 ]
//	7.0710678118654755
hypot[-5]
//	5
'getBaseLog = '( log( @:1 ) ÷ log( @: 0 ) )
[ 2 8 ]:getBaseLog
//	3
[ 5 625 ]:getBaseLog
//	4
log10(100000)
//	5
log10(2)
//	0.3010299956639812
log10(1)
//	0
log10(0)
//	-∞
log2(3)
//	1.584962500721156
log2(2)
//	1
log2(1)
//	0
log2(0)
//	-∞
max[ 1 3 2 ]
//	3
max[ (-1) (-3) (-2) ]
//	-1
'array1 = [ 1 3 2 ]
max( array1 )
//	3
min[ 2 3 1 ]
//	1
min[ -2 (-3) (-1) ]
//	-3
'array1 = [2 3 1]
min(array1)
//	1
pow[ 7 3 ]
//	343
pow[ 4 0.5 ]
//	2
pow[ 7 (-2) ]
//	0.02040816326530612
//                  (1/49)
pow[ -7 0.5 ]
//	NaN
random
//
'getRandomInt = '( floor( random × @ ) )
3:getRandomInt
//	0, 1 or 2
1:getRandomInt
//	0
round(0.9)
//	1
« (round(5.95))(round(5.5))(round(5.05))»
//	[6 6 5]
« (round(-5.05))(round(-5.5))(round(-5.95))»
//	[-5 -5 -6]
sign(3)
//	1
sign(-3)
//	-1
sign(0)
//	0
'getCircleY = '(sin(@:0) × @:1)
[ 1 10 ]:getCircleY
//	8.414709848078965
[ 2 10 ]:getCircleY
//	9.092974268256818
« π 10 »:getCircleY
//	1.2246467991473533e-15
round(0.9)
//	1
« (round(5.95))(round(5.5))(round(5.05))»
//	[6 6 5]
« (round(-5.05))(round(-5.5))(round(-5.95))»
//	[-5 -5 -6]
sign(3)
//	1
sign(-3)
//	-1
sign(0)
//	0
'getCircleY = '(sin(@:0) × @:1)
[ 1 10 ]:getCircleY
//	8.414709848078965
[ 2 10 ]:getCircleY
//	9.092974268256818
« π 10 »:getCircleY
//	1.2246467991473533e-15
sinh(0)
//	0
sinh(1)
//	1.1752011936438014
sinh(-1)
//	-1.1752011936438014
sinh(2)
//	3.626860407847019
'calcHypotenuse = '( sqrt( @:0 × @:0 + @:1 × @:1 ) )
[ 3 4 ]:calcHypotenuse
//	5
[ 5 12 ]:calcHypotenuse
//	13
[ 0 0 ]:calcHypotenuse
//	0
'getTanFromDegrees = '( tan(@ × π ÷ 180) )
0:getTanFromDegrees
//	0
45:getTanFromDegrees
//	0.9999999999999999
90:getTanFromDegrees
//	16331239353195370
tanh(-1)
//	-0.7615941559557649
tanh(0)
//	0
tanh(∞)
//	1
tanh(1)
//	0.7615941559557649
trunc(13.37)
//	13
trunc(42.84)
//	42
trunc(0.123)
//	0
trunc(-0.123)
//	-0

///////////////////////	Operators

3:'@
3:'@@
¤
¡"Exception"

~3
~~3

¬3
¬[] 

'{ ( 'a = 3 ) a }
!'{ ( 'a = 3 ) a }
!'{ ( 'a = 3 ) a }:0

\`{"a":3,"b":2}\`:byJSON§'( a b ) 
//	6

\`{"a":3,"b":2}\`:byJSON:toJSON
//	\`{"a":3,"b":2}\`

"[ 1, 2, 3 ]":byJSON:toJSON
//	\`[1,2,3]\`

"null":byJSON:toJSON
//	\`null\`

\`"string"\`: byJSON:toJSON
//	\`"string"\`

\`1\`: byJSON:toJSON
//	\`1\`

'{ 1 2 3 }:toJSON
//	{ 1 2 3 } can not be converted to JSON
//	Evaluating: ' { 1 2 3 } : toJSON

[ 2 3 4 ]:#
[ 2 3 4 ]:*
[ 2 3 4 ]:$

¶3 + "ab"
3:.:¦
//	See console

"abc"?[ ( 3 + 4 ) ( 5 + 6 ) ]
[]?[ ( 3 + 4 ) ( 5 + 6 ) ]

"abc"¿'( 3 + 4 )
[]¿'( 3 + 4 )
¬[]¿'( 3 + 4 )

0 && 0
0 && []
[] && 0
[] && []

0 || 0
0 || []
[] || 0
[] || []

0 ^^ 0
0 ^^ []
[] ^^ 0
[] ^^ []

1 ∈ [ 1 2 3 ]
1 ∈ [ 2 3 4 ]

[ 1 2 3 ] ∋ 1
[ 2 3 4 ] ∋ 1

2 == 1
2 == 2
2 == 3

2 <> 1
2 <> 2
2 <> 3

2 < 1
2 < 2
2 < 3

2 > 1
2 > 2
2 > 3

2 <= 1
2 <= 2
2 <= 3

2 >= 1
2 >= 2
2 >= 3

"a", [ "b" "c" ]
`
