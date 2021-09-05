#include <token.h>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>
#include <lcutil.h>
#include <prim_parse.h>
#include <primio.h>
#include <sym.h>

namespace {
    template <class P> Object* parse_atom(const char* x, P atom_parser) {
        const C* const s = reinterpret_cast<const C*>(x);
        const auto [ok, a] = atom_parser(s, s + strlen(x));
        assert(ok);
        return make_atom(a);
    }

    template <class X, class P>
    List<X>* parse_list(const char* x, P atom_parser) {
        const char* const last = x + strlen(x);
        L<X> r;
        while (x != last) {
            const char* const y = std::find_if(
                x + 1, last, [](char c){ return std::isspace(c); });
            const C* const s = reinterpret_cast<const C*>(x);
            const C* const t = reinterpret_cast<const C*>(y);
            const auto [ok, a] = atom_parser(s, t);
            assert(ok);
            r.push_back(a);
            x = std::find_if(y, last, [](char c){return !std::isspace(c); });
        }
        return r.release();
    }

    H print(H h, Token t) {
#define CS(X) case X: return h << #X;
        switch (t) {
        CS(ADVERB);
        CS(BOOL);
        CS(BOOL_LIST);
        CS(BYTE);
        CS(BYTE_LIST);
        CS(CHAR);
        CS(CHAR_LIST);
        CS(COMMENT);
        CS(DATE);
        CS(DATE_LIST);
        CS(FLOAT);
        CS(FLOAT_LIST);
        CS(IDENTIFIER);
        CS(INDENT);
        CS(INT);
        CS(INT_LIST);
        CS(LBRACE);
        CS(LBRACKET);
        CS(LPAREN);
        CS(LONG);
        CS(LONG_LIST);
        CS(KEYWORD);
        CS(NEWLINE);
        CS(OPERATOR);
        CS(QUIT);
        CS(RBRACE);
        CS(RBRACKET);
        CS(RPAREN);
        CS(SEMICOLON);
        CS(SYM);
        CS(SYM_LIST);
        CS(TIME);
        CS(TIME_LIST);
        CS(WHITESPACE);
        default: return h << "Unknown token type " << int(t);
#undef CS
        }
        return h;
    }

    L<C>& print(L<C>& h, Token t) {
#define CS(X) case X: return h << #X;
        switch (t) {
        CS(ADVERB);
        CS(BOOL);
        CS(BOOL_LIST);
        CS(BYTE);
        CS(BYTE_LIST);
        CS(CHAR);
        CS(CHAR_LIST);
        CS(COMMENT);
        CS(DATE);
        CS(DATE_LIST);
        CS(FLOAT);
        CS(FLOAT_LIST);
        CS(IDENTIFIER);
        CS(INDENT);
        CS(INT);
        CS(INT_LIST);
        CS(LBRACE);
        CS(LBRACKET);
        CS(LPAREN);
        CS(LONG);
        CS(LONG_LIST);
        CS(KEYWORD);
        CS(NEWLINE);
        CS(OPERATOR);
        CS(QUIT);
        CS(RBRACE);
        CS(RBRACKET);
        CS(RPAREN);
        CS(SEMICOLON);
        CS(SYM);
        CS(SYM_LIST);
        CS(TIME);
        CS(TIME_LIST);
        CS(WHITESPACE);
        default: return h << "Unknown token type " << int(t);
#undef CS
        }
        return h;
    }

    char escape(char x) {
        switch (x) {
        case '\\': return '\\';
        case 'n' : return '\n';
        case 'r' : return '\r';
        case 't' : return '\t';
        default  : return x;
        }
    }
}

extern "C" {

Object* parse_b(const char* x) {
    assert((*x == '0' || *x == '1') && x[1] == 'b' && !x[2]);
    return make_atom(B(*x == '1'));
}

Object* parse_B(const char* x) {
    const std::size_t len  = strlen(x);
    const char* const last = x + len - (x[len-1] == 'b');
    L<B> r(last - x);
    std::transform(x, last, r.begin(), [](char c){ return B(c == '1'); });
    return r.release();
}

Object* parse_c(const char* x) {
    assert(*x == '"');
    return make_atom(C(x[1] != '\\'? x[1] : escape(x[2])));
}

Object* parse_C(const char* x) {
    const std::size_t len = strlen(x);
    assert(2 <= len && *x == '"' && x[len-1] == '"');
    L<C> r(index_t(len - 2));
    index_t j = 0;
    for (std::size_t i = 1; i < len - 1; ++i, ++j)
        r[j] = C(x[i] == '\\'? escape(x[++i]) : x[i]);
    r.trunc(j);
    return r.release();
}

Object* parse_d (const char* x) { return parse_atom(x, parse_date); }
Object* parse_D (const char* x) { return parse_list<D>(x, parse_date); }
Object* parse_f (const char* x) { return parse_atom(x, parse_float); }
Object* parse_F (const char* x) { return parse_list<F>(x, parse_float); }
Object* parse_id(const char* x) { assert(*x); return make_atom(sym(x)); }
Object* parse_i (const char* x) { return parse_atom(x, parse_int); }
Object* parse_I (const char* x) { return parse_list<I>(x, parse_int); }
Object* parse_j (const char* x) { return parse_atom(x, parse_long); }
Object* parse_J (const char* x) { return parse_list<J>(x, parse_long); }

Object* parse_s(const char* x) { assert(*x == '`'); return make_atom(sym(x + 1)); }

Object* parse_S(const char* x) {
    L<S> r;
    while (x) {
        assert(*x == '`');
        const char* const y = strchr(x+1, '`');
        r.push_back(y? sym(x+1, y) : sym(x+1));
        x = y;
    }
    return r.release();
}

Object* parse_t(const char* x) { return parse_atom(x, parse_time); }
Object* parse_T(const char* x) { return parse_list<T>(x, parse_time); }

Object* parse_x(const char* x) {
    assert(*x && *(x+1) && *(x+2));
    const C* const s = reinterpret_cast<const C*>(x + 2);
    const C* const t = *(s + 1)? s + 2 : s + 1;
    const auto [ok, b] = parse_byte(s, t);
    assert(ok);
    return make_atom(b);
}

Object* parse_X(const char* x) {
    const std::size_t len = strlen(x);
    assert(2 <= len);
    L<X> r(index_t((len - 1) / 2));
    x += 2;
    index_t i = 0;
    if (len % 2) {
        const C* const s = reinterpret_cast<const C*>(x++);
        const auto [ok, b] = parse_byte(s, s + 1);
        assert(ok);
        r[i++] = b;
    }
    for (; *x; ++i, x += 2) {
        const C* const s = reinterpret_cast<const C*>(x);
        const C* const t = *(s + 1)? s + 2 : s + 1;
        const auto [ok, b] = parse_byte(s, t);
        assert(ok);
        r[i] = b;
    }
    return r.release();
}

Object* parse_Op(const char* x) {
    [[maybe_unused]] constexpr char ops[] = "~!@#$%^&*-_=+|:,<.>?";
    assert(std::find(ops, std::end(ops), *x) != std::end(ops));
    assert(!*(x + 1));
    return make_atom(Opcode(*x));
}

} // extern "C"

H     operator<<(H     h, Token t) { return print(h, t); }
L<C>& operator<<(L<C>& h, Token t) { return print(h, t); }
