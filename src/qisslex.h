#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <lex.yy.h>

#ifdef __cplusplus
} // extern "C"

struct Object;
extern Object* yylval;

#include <l.h>
#include <token.h>

struct Lexer {
    Lexer () { yylex_init(&scanner); }
    ~Lexer() { yylex_destroy(scanner); }

    Token next() { return Token(yylex(scanner)); }

    auto        len () const { return yyget_leng  (scanner); }
    int         line() const { return yyget_lineno(scanner); }    
    const char* text() const { return yyget_text  (scanner); }

protected:
    yyscan_t scanner;
};

struct StringLexer: Lexer {
    explicit StringLexer(const L<C>& s): buf(scan(yy_scan_bytes, s, scanner)) {}
    ~StringLexer() { yy_delete_buffer(buf, scanner); }

private:
    YY_BUFFER_STATE buf;

    // Different versions of flex have different types for the 2nd arg to yy_scan_bytes
    using scan_int_sz       = YY_BUFFER_STATE(*)(const char*, int      , yyscan_t);
    using scan_yy_size_t_sz = YY_BUFFER_STATE(*)(const char*, yy_size_t, yyscan_t);
    static YY_BUFFER_STATE scan(scan_int_sz scan_bytes, const L<C>& s, yyscan_t scanner) {
        return scan_bytes(reinterpret_cast<const C::rep*>(s.begin()), int(s.size()), scanner);
    }
    static YY_BUFFER_STATE scan(scan_yy_size_t_sz scan_bytes, const L<C>& s, yyscan_t scanner) {
        return scan_bytes(reinterpret_cast<const C::rep*>(s.begin()), yy_size_t(s.size()), scanner);
    }
};

#endif
