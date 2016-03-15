SliP

特徴

* 関数型言語である。
	* すべての関数はリストを受け取りリストを返す。
* Lisp に影響を受けている。
	* なので名前が SliP
	* ただしセルを前提としていない。ほとんどの実装で、Slice / Array を使用して実装されている。
* ガベコレを行う。
	* 実際のガベージコレクタは実装に依存する。
	* 例えば SliP の最初のインプリは GO で書かれているので、GO のガベコレを利用している。


* オブジェクト
	* 文	セミコロンまでの文字列
	
	* 文字列	ダブルクオートで囲まれたもの
	* 数値　ダブルクオートで囲まれてなくて、数値表現形式を満たしているもの。
	* 名前　ダブルクオートで囲まれてなくて、数値表現形式を満たしていないもの。


REP (Read, Eval, Print)
	* 文字列 -> 文字列
	* 数値 -> 数値
	* 名前 -> 連想された値
	



Slip (programming language)

Slip(/slípi/) is a programming language designed to treat first-class functions.
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



