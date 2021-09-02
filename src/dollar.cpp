#include <dollar.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <each.h>
#include <flip.h>
#include <o.h>
#include <prim_parse.h>
#include <sym.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    S c2s(C x) {
        const char s[] = { C::rep(x), '\0' };
        return sym(s);
    }

    O cannot_cast(const char* to, O y) {
        L<C> s;
        s << "'type: cannot convert " << std::move(y) << " to " << to;
        throw lc2ex(s);
    }

    template <class TO, class FROM> L<TO> cast_vec(L<FROM> y) {
        L<TO> r(y.size());
        std::transform(y.begin(), y.end(), r.begin(),
            [](FROM x){ return TO(typename TO::rep(typename FROM::rep(x))); });
        return r;
    }

    template <class Derived, class Result>
    struct Caster {
        const Derived& This() const { return *static_cast<const Derived*>(this); }

        // TODO don't be so permissive?
        template <class Z> Result operator()(Z x) const {
            return Result(typename Result::rep(typename Z::rep(x)));
        }
        template <class Z>
        O operator()(L<Z> x) const { return each(This(), std::move(x)); }
        O operator()(UKV x) const {
            auto [k, v] = std::move(x).kv();
            return UKV(std::move(k), This()(std::move(v)));
        }
        O operator()(L<O> x) const {
            if (!std::all_of(x.begin(), x.end(), [](const O& o){ return o.is_atom(); }))
                return each(This(), x);
            L<Result> r(x.size());
            for (index_t i = 0; i < x.size(); ++i) {
                O& e = x[i];
                switch (int(e.type())) {
#define CASE(X) case -OT<X>::typei(): r[i] = This()(e.atom<X>())
                CASE(B); CASE(C); CASE(D); CASE(F); CASE(I); CASE(J);
                CASE(S); CASE(T); CASE(X);
                default : throw Exception("nyi $ (cast)");
#undef CASE
                }
            }
            return std::move(r);
        }

        O operator()(O x) const {
#define CS(X) case OT<X>::typei() : return This()(L<X>(std::move(x))); \
              case -OT<X>::typei(): return O(This()(x.atom<X>()))
            switch (int(x.type())) {
            CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X);
            case OT<O>::typei() : return each(This(), L<O>(std::move(x)));
            case '!': return This()(UKV(std::move(x)));
          case '+': return +This()(+std::move(x));
            default : throw Exception("type ($ cast)");
            }
#undef CS
        }
    };

    const struct CastToBool: Caster<CastToBool, B> {
        using Caster<CastToBool, B>::operator();
        B operator()(C x) const { return B(B::rep(C::rep(x) != 0)); }
        B operator()(F x) const { return B(B::rep(F::rep(x) != 0)); }
        B operator()(J x) const { return B(B::rep(J::rep(x) != 0)); }
        B operator()(S x) const { return B(B::rep(S::rep(x) != 0)); }
        B operator()(X x) const { return B(B::rep(X::rep(x) != 0)); }
        template <class Z>
        B operator()(Z  ) const { throw Exception("type (`bool$)"); }
    } cast_to_bool;

    const struct CastToChar: Caster<CastToChar, C> {
        using Caster<CastToChar, C>::operator();
        C operator()(S x) const {
            if (x != S(0) && c_str(x)[1] == '\0') return C(c_str(x)[0]);
            throw Exception("type (can't cast sym to char)");
        }
    } cast_to_char;

    const struct CastToDate: Caster<CastToDate, D> {
        using Caster<CastToDate, D>::operator();
    } cast_to_date;

    const struct CastToFloat: Caster<CastToFloat, F> {
        using Caster<CastToFloat, F>::operator();
    } cast_to_float;

    const struct CastToInt: Caster<CastToInt, I> {
        using Caster<CastToInt, I>::operator();
        I operator()(B x) const { return I(B::rep(x)); }
        I operator()(F x) const { return I(I::rep(round(F::rep(x)))); }
        I operator()(J x) const { return I(I::rep(J::rep(x))); }
        I operator()(S  ) const { throw Exception("type (can't cast sym to int)"); }
    } cast_to_int;

    const struct CastToLong: Caster<CastToLong, J> {
        using Caster<CastToLong, J>::operator();
        J operator()(B x) const { return J(B::rep(x)); }
        J operator()(F x) const { return J(J::rep(round(F::rep(x)))); }
        J operator()(I x) const { return J(I::rep(x)); }
        J operator()(S  ) const { throw Exception("type (can't cast sym to long)"); }
    } cast_to_long;

    const struct CastToSym: Caster<CastToSym, S> {
        using Caster<CastToSym, S>::operator();
        S operator()(C x) const { return c2s(x); }
        O operator()(L<C> x) const { return O(parse_sym(std::move(x)).second); }
        template <class Z>
        S operator()(Z  ) const { throw Exception("type (`$)"); }
    } cast_to_sym;

    const struct CastToByte: Caster<CastToByte, X> {
        using Caster<CastToByte, X>::operator();
    } cast_to_byte;

    O cast(S to, O y) {
        const char* const s = c_str(to);
        if (!strcmp(s, "bool" )) return cast_to_bool (std::move(y));
        if (!strcmp(s, "char" )) return cast_to_char (std::move(y));
        if (!strcmp(s, "date" )) return cast_to_date (std::move(y));
        if (!strcmp(s, "float")) return cast_to_float(std::move(y));
        if (!strcmp(s, "int"  )) return cast_to_int  (std::move(y));
        if (!strcmp(s, "long" )) return cast_to_long (std::move(y));
        if (!strcmp(s, ""     )) return cast_to_sym  (std::move(y));
        if (!strcmp(s, "byte" )) return cast_to_byte (std::move(y));
        throw Exception("nyi: $");
    }

    O cast(C to, O y) {
        switch (C::rep(to)) {
        case 'b': return cast_to_bool (std::move(y));
        case 'c': return cast_to_char (std::move(y));
        case 'f': return cast_to_float(std::move(y));
        case 'j': return cast_to_long (std::move(y));
        case 's': return cast_to_sym  (std::move(y));
        case 'x': return cast_to_byte (std::move(y));
        default : throw Exception("nyi: $");
        }
    }

    O cannot_parse(const char* to, const O y) {
        L<C> s;
        s << "'type: cannot parse " << y << " as " << to;
        throw lc2ex(s);
    }

    O parse(C to, O y);

    constexpr char digits[] = "0123456789abcdef";

    bool all_digits(const C* first, const C* last) {
        while (first != last && isdigit(C::rep(*first))) ++first;
        return first == last;
    }

    J::rep from_digits(const C* first, const C* last) {
        J::rep r = 0;
        for (; first != last; ++first) r = r * 10 + C::rep(*first) - '0';
        return r;
    }

    X::rep parse_digit(C c) {
        return X::rep(std::find(digits, std::end(digits), tolower(C::rep(c))) - digits);
    };

    bool parse_j(J::rep* j, const C* first, const C* last) {
        // TODO handle infinities, na
        assert(first != last);
        const C* p    = first + (*first == C('-'));
        J::rep   r    = 0;
        J::rep   prev = 0;
        for (; p != last && prev <= r; ++p) {
            if (!isdigit(C::rep(*p))) return false;
            prev = r;
            r = r * 10 + C::rep(*p) - '0';
        }
        if (r < prev) return false;
        *j = *first != C('-')? r : -r;
        return true;
    }

    template <class P> struct Parser {
        O operator()(L<O> y) const {
            const auto isC = [](const O& o){ return o.type() == OT<C>::typet(); };
            if (std::all_of(y.begin(), y.end(), isC)) {
                using Z = decltype(This()(C(' ')));
                L<Z> r(y.size());
                std::transform(y.begin(), y.end(), r.begin(),
                    [this](O x){ return Z(This()(L<C>(x))); });
                return std::move(r);
            } else {
                L<O> r;
                r.reserve(y.size());
                std::transform(y.begin(), y.end(), std::back_inserter(r), *this);
                return std::move(r);
            }
        }

        O operator()(O y) const {
            using Z = decltype(This()(C(' ')));
            switch (int(y.type())) {
            case -OT<C>::typei(): return O(This()(y.atom<C>()));
            case OT<C>::typei() : return O(This()(L<C>(std::move(y))));
            case OT<O>::typei() : return (*this)(L<O>(std::move(y)));
            default             : return cannot_parse(OT<Z>::name(), y);
            }
        }
        const P& This() const { return *static_cast<const P*>(this); }
    };

    const struct BoolParser: Parser<BoolParser> {
        using Parser<BoolParser>::operator();
        B operator()(C x) const { return (*this)(L<C>{x}); }
        B operator()(L<C> x) const { return parse_bool(std::move(x)).second; }
    } bool_parser;

    const struct ByteParser: Parser<ByteParser> {
        using Parser<ByteParser>::operator();
        X operator()(C x) const { return (*this)(L<C>{x}); }
        X operator()(L<C> x) const { return parse_byte(std::move(x)).second; }
    } byte_parser;

    const struct CharParser: Parser<CharParser> {
        using Parser<CharParser>::operator();
        C operator()(C x) const { return x;  }
        C operator()(L<C> x) const { return parse_char(std::move(x)).second; }
    } char_parser;

    const struct DateParser: Parser<DateParser> {
        using Parser<DateParser>::operator();
        D operator()(C     ) const { return ND; }
        D operator()(L<C> x) const { return parse_date(std::move(x)).second; }
    } date_parser;

    const struct FloatParser: Parser<FloatParser> {
        using Parser<FloatParser>::operator();
        F operator()(C     ) const { return NF; }
        F operator()(L<C> x) const { return parse_float(std::move(x)).second; }
    } float_parser;

    const struct IntParser: Parser<IntParser> {
        using Parser<IntParser>::operator();
        I operator()(C     ) const { return NI; }
        I operator()(L<C> x) const { return parse_int(std::move(x)).second; }
    } int_parser;

    const struct LongParser: Parser<LongParser> {
        using Parser<LongParser>::operator();
        J operator()(C     ) const { return NJ; }
        J operator()(L<C> x) const { return parse_long(std::move(x)).second; }
    } long_parser;

    const struct SymParser: Parser<SymParser> {
        using Parser<SymParser>::operator();
        S operator()(C x) const { return c2s(x); }
        S operator()(L<C> x) const { return parse_sym(std::move(x)).second; }
    } sym_parser;

    const struct TimeParser: Parser<TimeParser> {
        using Parser<TimeParser>::operator();
        T operator()(C     ) const { return NT; }
        T operator()(L<C> x) const { return parse_time(std::move(x)).second; }
    } time_parser;

    O parse(C to, O y) {
        switch (C::rep(to)) {
        case 'B': return bool_parser (std::move(y));
        case 'C': return char_parser (std::move(y));
        case 'D': return date_parser (std::move(y));
        case 'F': return float_parser(std::move(y));
        case 'I': return int_parser  (std::move(y));
        case 'J': return long_parser (std::move(y));
        case 'S': return sym_parser  (std::move(y));
        case 'T': return time_parser (std::move(y));
        case 'X': return byte_parser (std::move(y));
        case '*': return y;
        default : throw Exception("nyi: $");
        }
    }
} // unnamed

O dollar(O x, O y) {
    switch (int(x.type())) {
    case -OT<C>::typei(): {
        C cx(x.atom<C>());
        if (C('a') <= cx && cx <= C('z')) return cast (cx, y);
        if (C('A') <= cx && cx <= C('Z')) return parse(cx, y);
        throw Exception("domain ($)");
        }
    case OT<C>::typei(): {
      if (y.type() != OT<O>::typei()) throw Exception("'type ($)");
      L<C> cx(std::move(x));
      L<O> oy(std::move(y));
      if (oy.size() < cx.size())  throw Exception("'length ($)");
      return each(parse, std::move(cx), std::move(oy));
      }
    case -OT<S>::typei(): return cast(x.atom<S>(), std::move(y));
    default : throw Exception("nyi: $");
    }
}
