#pragma once

struct O;

// Input AST from parser is a table with this shape:
//     type node    depth
//     ------------------
//     X    Object* X
//     Ast  data    tree
// 
// For example:
//     ([k:`p`q`r]a:`x`y`z;b:1 2 3+45)
// could yield the following ast:
//     type node   depth
//     -----------------
//     4f   !      00   
//     4f   9      01   
//     4f   !      02   
//     6c   ,`k    03   
//     45   ::     03   
//     6c   `p`q`r 04   
//     4f   9      01   
//     4f   !      02   
//     6c   `a`b   03   
//     45   ::     03   
//     6c   `x`y`z 04   
//     4f   +      04   
//     6c   1 2 3  05   
//     6c   45     05   
//
// Returns processed (ready for code gen) AST:
//     type node parent contour kids frame slot distance
//     -------------------------------------------------
//     4d   ::        0       0   01    0N   0N       0N
//     6e   3         0       0   00    0N   0N       0N
//     ..
O compile(O ast, bool trace=false);
