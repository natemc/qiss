#pragma once

#include <cstdint>

enum class Opcode: uint8_t {
    //       opcode  hex arg1    arg2    note
    // ---------------------------------------------------------------
    immsym = '`', // 60  len     `xyzzy  len is 2 bytes
    match  = '~', // 7e
    bang   = '!', // 21                  make dict, mod, key table
    at     = '@', // 40                  surface index/apply
    take   = '#', // 23                  take, replicate, reshape
    cast   = '$', // 24                  also parse
    fdiv   = '%', // 25
    fill   = '^', // 5e
    min    = '&', // 26
    mul    = '*', // 2a
    sub    = '-', // 2d
    cut    = '_', // 5f                  also drop
    eq     = '=', // 3d
    add    = '+', // 2b
    scan   = '\\',// 5c
    max    = '|', // 7c
    assign = ':', // 3a
    each   = '\'',// 27
    left   = 'L', // 4c                  \:
    prior  = 'P', // 50                  ':
    right  = 'R', // 52                  /:
    immstr = '"', // 22  len     foo     len is 2 bytes
    cat    = ',', // 2c
    less   = '<', // 3c
    dot    = '.', // 2e
    great  = '>', // 2e
    over   = '/', // 2f
    find   = '?', // 3f                  also deal/rand
    first  = '0', // 30                  unary *
    floor  = 'a', // 61                  unary _, also lower
    iasc   = 'A', // 41                  unary <
    immb   = 'b', // 62          1b
    immbv  = 'B', // 42  len     1001b   len is 2 bytes
    call   = 'c', // 63                  absolute
    count  = 'C', // 43                  unary #
    immd   = 'd', // 64  date
    immdv  = 'D', // 44  len     dates   len is 2 bytes
    emit   = 'e', // 65                  TODO remove
    rev    = 'E', // 45                  unary |
    immf   = 'f', // 66  3.1
    immfv  = 'F', // 46  len     1 2f    len is 2 bytes
    jump   = 'g', // 67  addr            absolute; g is for goto
    group  = 'G', // 47                  unary =
    halt   = 'h', // 68
    recip  = 'H', // 48                  unary %
    immi   = 'i', // 69  123i
    immiv  = 'I', // 49  len     1 2 3i  len is 2 bytes
    immj   = 'j', // 6a  123
    immjv  = 'J', // 4a  len     1 2 3   len is 2 bytes
    key    = 'k', // 6b                  unary !
    pushc  = 'K', // 4b                  constant
    enlist = 'l', // 6c                  unary (use op+invoke for variadic)
    neg    = 'M', // 4D                  unary -
    nop    = 'n', // 6e
    not_   = 'N', // 4e                  unary ~
    op     = 'o', // 6f                  TODO currently used for adverbed-ops; is there a better way?
    immo   = 'O', // 4f  O               push immediate; what was I thinking??
    pop    = 'p', // 70
    ret    = 'r', // 72
    swap   = 's', // 73
    string = 'S', // 53                  unary $
    immt   = 't', // 74  time
    immtv  = 'T', // 54  len     times   len is 2 bytes
    immx   = 'x', // 78
    immxv  = 'X', // 58  len     0x1234  len is 2 bytes
    distinct='U', // 55                  unary ? (unique)
    value  = 'V', // 56                  unary 
    vcond  = 'v', // 76                  ternary ?
    where  = 'W', // 57                  unary &
    type   = 'Y', // 59                  unary @
    bzero  = 'z', // 7a                  absolute
    null   = 'Z', // 5a                  unary ^
    invoke = '1', // 31  len             invoke the callable at top-of-stack with len params; len is 1 byte
    pushm  = '2', // 32  index           push value of module variable; index is 4 bytes
    dupnth = '3', // 33  index           dup the nth item relative to the top of the stack
    clean  = '4', // 34  len             pop elements below the top; len is 1 byte
    local  = '5', // 35  index           duplicate the item at index (relative to the frame pointer) to the top of the stack; index is 1 byte
    dup    = '6', // 36                  duplicate the item at the top of the stack
    idesc  = '7', // 37                  unary >
//    pushs  = '8', // 38  index           push static constant; index is 4 bytes
    flip   = '9', // 39                  unary +
    rot    = '[', // 5b                  like dupnth but remove original
    bindm  = ']', // 5d  index           bind module var (in current module); index is 4 bytes
    bindl  = '(', // 28  index           bind local; index is 1 byte
    bump   = ')', // 29  len             bump stack pointer; len is 1 byte
};

using immlen_t = uint16_t;
using sindex_t = uint32_t;
