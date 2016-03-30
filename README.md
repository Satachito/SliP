#SliP

<img src=SliP.jpg />

Written by Satoru Ogura, Tokyo 2016.

Visit [project home](https://github.com/Satachito/SliP) for your feedback, thank you.

---

## As a calculator:

* Write formular in left text box.
* Each formular must be terminated by ':'.
* Each operator must be surrounded by ' '.
* Press 'CALCULATE' button.
* Evaluated values are shown in upper right text box.

## As a computer language:
SliP reads list of SliP object(s) until ';' treat it as Sentence and print its evaluated value.

### How sentence is evaluated.

* If the sentence has no object in it, an error is raised.

	- ex.) ; -> An error is raised.

* If the sentence has only one object in it, the value is evaluated value of the object.
	- ex.) 1; -> 1.

* If the sentence has two objects in it, an error is raised.
	- ex.) 1 2; -> An error is raised.

* If the sentence has three objects in it and if the second object is operator then first and third objects are passed to the operator, the second object.
	- ex.) 1 + 2; -> 3

	Else an error will be raised.

	- ex.) 1 2 3; -> An error is raised

* If the sentence has four or more objects in it.

	Find weakest operator in the sentnce and treat left side objects and right side objects as Sentence, evaluate those and passed evaluated values to the operator.
	
	- ex.) 1 + 2 * 3 + 4; -> 11


### Lists.
SliP treats four types of list.

* ( "A" + "C" )

	- List of objects which are evaluated as sentence described above.

* [ "A" "B" "C" ]

	- Literal list, i.e. when eavluatd return itself.

* { "A" "B" "C" }

	- List of objects which are evaluated sequencially. When evaluated the interpreter push the new dictionary in association list for local variables.

* « "A" "B" "C" »

	- List of objects which are evaluated concurrently. When evaluated the interpreter copies its contex for each object.


### Primitives.

* @
	- Top of the stack.
* ·
	- Current dict.
* ¡
	- Raise exception.


### Builtins.
* *
	- Rest of the list but first element.
* #
	- Number of elements in the list.
* $
	- The last elements of the list.
* ¦
	- Print the string representaion of the object.

### Operators.
* ¿
	- If. If the left argument is not nil evaluate the right element.
* ?
	- IfElse. If the left argument is not nil evaluate the second element of the right element( must be list ) or evaluate the first element of the right element.
* ,
	- Cons. Insert left element into right element( must be list ) as first element.
* :
	- Apply. Push left element on the stack and evaluate right element.
* +
	- Plus.
* -
	- Minus.
* ×
	- Multiple.
* ÷
	- Divide.
* /
	- Divide as integer.
* %
	- Remainder.
* &
	- And.
* |
	- Or.
* ^
	- Xor.
* =
	- Define.
* < > <= >= <> >< ==
	- Comparison.


### Name.
Name must begin with alphabet or _ ( underscore ) and followed by sequence of alphabet, _ and number digit.

When evaluated, find association list by the name and return associated value.

Ex.)

```
'a = 5;	//	The name "a" is Quoted, see below

a;	//	prints 5

```

### Quote.</h5>
When SliP reader reads ', it reads object after ' and make Quote object having read object i.e. quoted.

When Quote object is evaluated, it returns quoted object.



### SliP objects.

* Number
* String
* Name
* Primitive
* Builtin
* Operator
* Quote
* List

