# qiss

![build](https://github.com/natemc/qiss/actions/workflows/onpush.yml/badge.svg)

qiss is short and sweet.

## What

qiss is a byte-code interpreter for a variant of the k language, an APL descendant used in some high performance time-series database systems. It has the same high-level parts that most byte-code interpreters (e.g., Python, Ruby, Lua) do:

```
Lexer -> Parser -> Compiler -> Code Generator -> Virtual Machine
```

These high-level parts all share a runtime that implements features like memory management, I/O, and built-in functions & data structures.

qiss is very much a work in progress, unsuitable for serious use.

## Why

My goal for qiss is to have fun learning. Here are a few components I've found especially interesting:

* A [robin hood hash table](https://cs.uwaterloo.ca/research/tr/1986/CS-86-14.pdf).
* A [buddy allocator](https://en.wikipedia.org/wiki/Buddy_memory_allocation). Still lots of opportunity to improve here; see, e.g., [this paper](https://cs.au.dk/~gerth/papers/actainformatica05.pdf).
* A compiler using a table-based (rather than tree-based) AST. This based on Aaron Hsu's dissertation, [A Data Parallel Compiler Hosted on the GPU](https://scholarworks.iu.edu/dspace/bitstream/handle/2022/24749/Hsu%20Dissertation.pdf?sequence=1&isAllowed=y).

## Build

```
nate@Nates-iMac qiss % cmake -S . -B build
..
nate@Nates-iMac qiss % cmake --build build
```

## Test

There is a growing number of unit tests captured in the executable, `utest`:

```
qiss % build/utest
[doctest] doctest version is "2.4.6"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  68 |  68 passed | 0 failed | 0 skipped
[doctest] assertions: 270 | 270 passed | 0 failed |
[doctest] Status: SUCCESS!
qiss % 
```

## Run

```
qiss % rlwrap build/qiss
  +/!10
  45
  \\

qiss % rlwrap build/qiss
```

## The Language

Like all APL descendants, qiss derives its expressiveness from the following features:

* Syntax that requires minimal punctuation
* Automatic vectorization
* Uniform treatment of all types of maps (in qiss's case, lists, dictionaries, tables, and functions)
* Precedence-free, right-to-left evaluation of expressions

```
qiss % rlwrap build/qiss
  / / with whitespace (including newline) preceding is a comment
  / operators are ambivalent, i.e., they are unary or binary depending on context
  %5 / unary (called "monadic" in the APL community) % is reciprocal
0.2
  10%5 / binary (called "dyadic" in the APL community) % is floating-point division
2f
  / all expressions are evaluated right-to-left; there is no precedence
3*2+2
12

  / uniform list literals require minimal punctuation
  1 2 3 / integer (64-bit) list literal
1 2 3
  1 2. 3 / float (64-bit) list literal
1 2 3f
  1001b / boolean list literal.  no spaces allowed!
1001b
  `a`b`xyzyy / symbol list literal.  no spaces allowed!
`a`b`xyzyy

  / many operators are "atomic": they automatically vectorize
  1+!5 / ! is the key operator; !5 means "til 5" i.e. 0 1 2 3 4
1 2 3 4 5
  (!5)*2
0 2 4 6 8
  / relational operators are atomic, too
  (!5)<|!5 / unary | is reverse
11000b

  / indexing also vectorizes
  (2*!5)[3]
6
  (1+!5)[0 2 4]
1 3 5
  / the @ operator is an alternative way to express indexing
  / it is handy for forcing an ambivalent operator to be unary
  / and also for applying an adverb (see below) to the indexing operation
  `a`b`c`d`e@1 3
`b`d

  / : is assignment, and assignment is an expression
  p:42
42
  p
42
  / like all expressions, assignment is processed right-to-left
  a*3+a:6
54

  / lambdas are in curly braces
  K:{[a;b]a} / the K combinator takes 2 args and returns its 1st
proc
  / BEWARE! () do not invoke a function, [] do
  K[3;4]
3
  / x, y, and z are implicit args
  add:{x+y}
proc
  add[3;4]
7

  / juxtaposition: indexing does not require square brackets
  `a`b`c`d`e 0 2 4
`a`c`e
  / unary operators and functions can be applied without square brackets
  #1 2 3 / unary # is count
3
  count:{#x}
proc
  count`a`b`c`d`e
5

  / loops are expressed via higher-order functions called adverbs
  / adverbs are suffixed to the operator they modify
  / ' is "each", and it takes on the arity of the function it modifies
  1 2 3+'10 20 30 / ' is redundant here since + is atomic
11 22 33
  #'(1 2 3;(4 5 6;7 8 9)) / (;) is a generic list literal separator
3 2
  1 2+/:3 4 5 / /: is "each-right"
(4 5;5 6;6 7)
  1 2+\:3 4 5 / \: is "each-left"
(4 5 6;5 6 7)
  / / (without preceding whitespace) is "over" which is like reduce in Python
  0+/!5 / optional left-hand side with over is the initial value
10
  5+\!5 / \ is "scan" (like reductions in clojure)
5 6 8 11 15
  / ': is "each-prior"
  0-':2 6 6 10 14 18 21 23 26 28 / deltas
2 4 0 4 4 4 3 2 3 2

  // a dict literal is a sequence of ;-separated assignments enclosed in []
  [a:1;b:2;c:3]
a| 1
b| 2
c| 3
  / dicts can also be formed using the ! operator on two lists
  d:`a`b`c!1 2 3
a| 1
b| 2
c| 3
  / dict indexing is analogous to list indexing
  d`a
1
  d`a`c
1 3

  / unary = is "group": it creates a dict mapping distinct values to indexes
  n:`c`a`d`b`a`b`c`c`d`e`a`a`b`e`c`b`c`b`e`e
`c`a`d`b`a`b`c`c`d`e`a`a`b`e`c`b`c`b`e`e
  =n
c| 0 6 7 14 16
a| 1 4 10 11
d| 2 8
b| 3 5 12 15 17
e| 9 13 18 19
  / most operators work on dicts analogously to how they work on lists
  10+`a`b`c!1 2 3
a| 11
b| 12
c| 13
  / similarly, a dict can be used as an index
  n@=n / @ here forces unary interpretation of =
c| ccccc
a| aaaa
d| dd
b| bbbbb
e| eeee
  v:82 63 80 99 83 66 51 34 83 69 16 81 16 90 21 82 71 32 55 19
  v@=n
c| 82 51 34 21 71
a| 63 83 16 81
d| 80 83
b| 99 66 16 82 32
e| 69 90 55 19

  / ([ introduces a table literal
  t:([]a:,/3#/:1 2 3;b:9#10 20 30)
a b 
----
1 10
1 20
1 30
2 10
2 20
2 30
3 10
3 20
3 30
  / tables can be indexed using column names
  t`a
1 1 1 2 2 2 3 3 3
  / tables can also be indexed using row numbers (starting from zero)
  t@!3
a b 
----
1 10
1 20
1 30
  t 0 / a single row is a dict
a| 1
b| 10
  &25>t`b / unary & is where: it returns the true indexes
0 1 3 4 6 7
  t@&25>t`b / applying a table to an integer list index yields a table
a b 
----
1 10
1 20
2 10
2 20
3 10
3 20

  / there is no string type in qiss; strings are lists of chars
  "x" / char
"x"
  "xyzzy" / list of char
"xyzzy"
  / a list of char behaves like any other list
  "foo","bar"  / , is "concat"
"foobar"
  (,"foo"),,"bar" / to make a pair of strings, enlist them first
("foo";"bar")
  "foo"="foo" / relational operators on lists are atomic
111b
  "foo"~"foo" / use ~ (match) to compare strings as a unit
1b

  / exit
  \\
  
```

## Visibility

When you build qiss, it produces several executables that allow you to interact with a subset of the system. For example, `ql` exposes the lexer:

```
qiss % rlwrap build/ql
  1+!10
LONG      | 1
OPERATOR  | +
OPERATOR  | !
LONG      | 10
```

Similarly, you can run just up to parsing using `qp`:

```
qiss % rlwrap build/qp
  1+!10
type node depth
---------------
4d   ::   00   
4f   +    01   
6e   1    02   
4f   !:   02   
6e   10   03   
```

Moreover, the more complex programs (`qp`, `qc`, `vm`, and `qiss` itself) support a trace feature that is enabled with the `-t` flag. For example, running `qiss -t` shows the virtual machine assembly and stack as each expression is executed:

<pre>
  {(*|x),+/x}5 8
051f52ff  4b 13 00 00 00                   pushc   4939           []
051f5304  63 ea 00 00 00                   call    0x000000ea     [5 8]                                
051f530a  35 0b                            local   -1             [5 8]                                
051f530c  6f 2b                            op      +              [5 8;5 8]                            
051f530e  2f                               /                      [+;5 8;5 8]                          
051f530f  31 01                            invoke  1              [+/;5 8;5 8]                         
051f5311  35 0b                            local   -1             [13;5 8]                             
051f5313  45                               rev                    [5 8;13;5 8]                         
051f5314  30                               *:                     [8 5;13;5 8]                         
051f5315  2c                               ,                      [8;13;5 8]                           
051f5316  34 01                            clean   1              [8 13;5 8]                           
051f5318  72                               return                 [8 13]                               
051f5309  72                               return                 [8 13]                               
8 13
</pre>


