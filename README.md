# qiss
qiss is short and sweet

## What

qiss is a byte-code interpreter. It has the same high-level parts that most byte-code interpreters (e.g., Python, Ruby, Lua) do:

```
Lexer -> Parser -> Compiler -> Code Generator -> Virtual Machine
```

These high-level parts all share a runtime that implements features like memory management, I/O, and built-in functions & data structures.

The qiss language is a variant of k.

## Why

My goal for qiss is to have fun learning. Here are a few components I've found especially interesting:

* An adaptive radix sort
* A robin-hood hash table
* A buddy allocator
* A compiler using a table-based (rather than tree-based) AST. This based on Aaron Hsu's dissertation, [A Data Parallel Compiler Hosted on the GPU](https://scholarworks.iu.edu/dspace/bitstream/handle/2022/24749/Hsu%20Dissertation.pdf?sequence=1&isAllowed=y).
