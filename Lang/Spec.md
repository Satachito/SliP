Slip, the computer language.

Slip is a computer language designed to treat first-class functions.
This language is named 'Slip' because it is highly influenced by Lisp but its implemention isn't based on 'list' but 'slice'.

Installation

Run
	Slip reads source code from standard input and print result to standard output.

Sample.

The number.
> 3;
3

The string.
> "ABC";
"ABC"

Sentence.
> 1 + 2;
3

> "ABC" + "DEF";
"ABCDEF"

Quote
> '1
1
> '"ABC"
"ABC"

Name
> 'a = 1;
1
> 'b = 2;
2
> a + b;
3

List
> [ 'a 'b 'c ];
[ a b c ]

First element of list.
> [ 'a 'b 'c ].0;
a
> [ 'a 'b 'c ].car;
a

Rest of list.
> [ 'a 'b 'c ].cdr;
[ b c ]

Insert element to list
> [ 'a [ 'b 'c ] ].cons;
[ a b c ]

Concat lists and elements
> [ [ 'a ] [ 'b ] ].concat;
[ a b ]
> [ 'a [ 'b ] ].concat;
[ a b ]
> [ [ 'a ] 'b ].concat;
[ a b ]
> [ 'a 'b ].concat;
[ a b ]
> ( [ 'a ], [ 'b ] );
[ a b ]
> ( 'a, [ 'b ] );
[ a b ]
> ( [ 'a ], 'b );
[ a b ]
> ( 'a, 'b );
[ a b ]


Apply
> [ 3 2 1 ].0;
3
> [ 3 2 1 ].2;
1

Argument
> 3.'( ! )



