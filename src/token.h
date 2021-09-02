#pragma once

#ifdef __cplusplus
#include <l.h>
#include <prim.h>
extern "C" {
#endif

enum Token {
    ADVERB = 1, /* Skip 0, because yylex returns 0 on EOF. */
    BOOL,
    BOOL_LIST,
    BYTE,
    BYTE_LIST,
    CHAR,
    CHAR_LIST,
    COMMENT,
    DATE,
    DATE_LIST,
    FLOAT,
    FLOAT_LIST,
    IDENTIFIER,
    INDENT,
    INT,
    INT_LIST,
    LBRACE,
    LBRACKET,
    LPAREN,
    LONG,
    LONG_LIST,
    KEYWORD,
    NEWLINE,
    OPERATOR,
    QUIT,
    RBRACE,
    RBRACKET,
    RPAREN,
    SEMICOLON,
    SYM,
    SYM_LIST,
    TIME,
    TIME_LIST,
    WHITESPACE,
};

struct Object;
struct Object* parse_b (const char* x);
struct Object* parse_B (const char* x);
struct Object* parse_c (const char* x);
struct Object* parse_C (const char* x);
struct Object* parse_d (const char* x);
struct Object* parse_D (const char* x);
struct Object* parse_f (const char* x);
struct Object* parse_F (const char* x);
struct Object* parse_id(const char* x);
struct Object* parse_i (const char* x);
struct Object* parse_I (const char* x);
struct Object* parse_j (const char* x);
struct Object* parse_J (const char* x);
struct Object* parse_s (const char* x);
struct Object* parse_S (const char* x);
struct Object* parse_t (const char* x);
struct Object* parse_T (const char* x);
struct Object* parse_x (const char* x);
struct Object* parse_X (const char* x);
struct Object* parse_Op(const char* x);

#if __cplusplus
}
H     operator<<(H     h, Token t);
L<C>& operator<<(L<C>& h, Token t);
#endif
