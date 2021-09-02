#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <lex.yy.h>

#ifdef __cplusplus
}

struct Object;
extern Object* yylval;

#include <l.h>
#include <token.h>

struct Lexer {
    Lexer () { yylex_init(&scanner); }
    ~Lexer() { yylex_destroy(scanner); }

    Token next() { return Token(yylex(scanner)); }

    size_t      len () const { return yyget_leng  (scanner); }
    int         line() const { return yyget_lineno(scanner); }    
    const char* text() const { return yyget_text  (scanner); }

protected:
    yyscan_t scanner;
};

struct StringLexer: Lexer {
    explicit StringLexer(const L<C>& s) {
        const C::rep* const b = reinterpret_cast<const C::rep*>(s.begin());
        buf = yy_scan_bytes(b, yy_size_t(s.size()), scanner);
    }
    ~StringLexer() { yy_delete_buffer(buf, scanner); }

private:
    YY_BUFFER_STATE buf;
};

#endif
