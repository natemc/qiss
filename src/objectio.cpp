#include <objectio.h>
#include <algorithm>
#include <cmath>
#include <format.h>
#include <iterator>
#include <lcutil.h>
#include <numeric>
#include <output.h>
#include <primio.h>
#include <sum.h>
#include <sym.h>
#include <terminal_width.h>
#include <ukv.h>

namespace {
    index_t viewport_height = 25;

    template <class F, class X> bool all(F&& f, const L<X>& x) {
        return std::all_of(x.begin(), x.end(), std::forward<F>(f));
    }

    bool whole(F x) {
        double int_part;
        double fractional_part = modf(F::rep(x), &int_part);
        return fractional_part == 0;
    }

    template <class X> using OT = ObjectTraits<X>;

    template <class Z> decltype(auto) print_atom(Z&& h, O x) {
        assert(x.type() < 0);
        switch (-int(x.type())) {
        case OT<B>::typei()     : return h << x->b;
        case OT<C>::typei()     : return escape(h << '"', x->c) << '"';
        case OT<D>::typei()     : return x->d.is_null()? h << x->d << 'd' : h << x->d;
        case OT<F>::typei()     : return whole(x->f)? h << x->f << 'f' : h << x->f;
        case OT<H>::typei()     : return h << x->h << 'h';
        case OT<I>::typei()     : return h << x->i << 'i';
        case OT<J>::typei()     : return h << x->j;
        case OT<S>::typei()     : return h << x->s;
        case OT<T>::typei()     : return x->t.is_null()? h << x->t << 't' : h << x->t;
        case OT<X>::typei()     : return h << "0x" << x->x;
        case OT<AO>::typei()    : return h << x->ao;
        case OT<AP>::typei()    : return h << x->ao;
        case OT<Opcode>::typei(): return h << x->op;
        case OT<bfun_t>::typei(): return h << "builtin";
        case -int(generic_null_type): return h << "::";
        case OT<Proc>::typei()  : return h << "proc";
        default:
            return h << "nyi: operator<<(H, O) for type " << int(x->type);
        }
    }

    template <class S, class X>
    decltype(auto) print_list_no_suffix(S&& h, const L<X>& x, char sep=' ') {
        if (x.empty())
            h << '`' << OT<X>::name() << "$()";
        else {
            if (x.size() == 1) h << ',';
            h << x[0];
            for (index_t i = 1; i < x.size(); ++i) h << sep << x[i];
        }
        return std::forward<S>(h);
    }

    template <class S, class X>
    decltype(auto) print_list(S&& h, const L<X>& x) {
        return print_list_no_suffix(h, x) << OT<X>::ch();
    }

    template <class S> decltype(auto) print_list(S&& h, const L<B>& x) {
        if (x.empty()) return h << "`bool$()";
        if (x.size() == 1) h << ',';
        for (B b: x) h << bool(b);
        return h << 'b';
    }

    template <class S> decltype(auto) print_list(S&& h, const L<C>& x) {
        if (x.empty()) return h << "\"\"";
        if (x.size() == 1) h << ',';
        h << '"';
        for (C c: x) escape(h, c);
        return h << '"';
    }

    template <class S> decltype(auto) print_list(S&& h, const L<D>& x) {
        return print_list_no_suffix(h, x);
    }

    template <class S> decltype(auto) print_list(S&& h, const L<J>& x) {
        return print_list_no_suffix(h, x);
    }

    template <class Z> decltype(auto) print_list(Z&& h, const L<S>& x) {
        if (x.empty())
            h << "`$()";
        else {
            if (x.size() == 1) h << ',';
            for (S s: x) h << s;
        }
        return std::forward<Z>(h);
    }

    template <class S> decltype(auto) print_list(S&& h, const L<T>& x) {
        return print_list_no_suffix(h, x);
    }

    template <class S> decltype(auto) print_list(S&& h, const L<X>& x) {
        if (x.empty())
            h << "`byte$()";
        else {
            if (x.size() == 1) h << ',';
            h << "0x";
            for (X e: x) h << e;
        }
        return std::forward<S>(h);
    }

    template <class S> decltype(auto) print_list(S&& h, const L<O>& x) {
        if (x.empty())
            h << "()";
        else {
            h << (x.size() == 1? ',' : '(');
            h << x[0];
            for (index_t i = 1; i < x.size(); ++i) h << ';' << x[i];
        }
        if (1 < x.size()) h << ')';
        return std::forward<S>(h);
    }

    template <class Z> decltype(auto) print_list(Z&& h, O x) {
        assert(x.is_list());
#define CS(X) case OT<X>::typei(): print_list(h, L<X>(std::move(x))); break
        switch (int(x.type())) {
        CS(B);
        CS(C);
        CS(D); 
        case OT<F>::typei(): {
            L<F> fs(std::move(x));
            print_list_no_suffix(h, fs);
            if (all(whole, std::move(fs))) h << 'f';
            break;
            }
        CS(I);
        CS(J);
        case OT<O>::typei(): {
            L<O> os(std::move(x));
            if (os.size() <= 1)
                print_list_no_suffix(h, std::move(os));
            else
                print_list_no_suffix(h << '(', std::move(os), ';') << ')'; 
            break;
            }
        CS(S);
        CS(T);
        CS(X);
        default:
            h << "nyi: print_list(,  O) for type " << x.type();
        }
#undef CS
        return h;
    }

    index_t max_width(const L<L<C>>& x) {
        return std::accumulate(x.begin(), x.end(), index_t(0),
            [](index_t a, const L<C>& s){return std::max(a, s.size());});
    }

    const struct Stringify {
        template <class X> L<C> operator()(X x) const {
            L<C> s;
            s << x;
            return s;
        }

        L<C> operator()(S x) const {
            L<C> s;
            s << c_str(x);
            return s;
        }

        L<C> operator()(B x) const {
            return L<C>{C("01"[B::rep(x)])};
        }
    } stringify;

    // I don't think this is necessary to be separate from Stringify but I
    // haven't figured out how to merge them without making the compiler upset.
    const struct StringifyListElements {
        template <class X> L<L<C>> operator()(const L<X>& x) const {
            L<L<C>> r;
            r.reserve(x.size());
            std::transform(x.begin(), x.end(), std::back_inserter(r), stringify);
            return r;
        }
    } stringify_list_elements;

    L<L<C>> stringify_table(O x);

    L<L<C>> stringify_elements(O x) {
#define CS(X) case OT<X>::typei(): return stringify_list_elements(L<X>(std::move(x)))
        switch (int(x.type())) {
        CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X);
        CS(O);
        case '+': return stringify_table(x);
        default: assert(false);
        }
#undef CS
        return L<L<C>>{}; // unreachable
    }

    template <class S> decltype(auto) print_dict(S&& h, O x) {
        assert(x.is_dict());
        UKV d(std::move(x));
        const L<L<C>> ks(stringify_elements(d.key()));
        const L<L<C>> vs(stringify_elements(d.val()));
        const int kw = int(max_width(ks));
        if (ks.size()) {
            for (index_t i = 0; i < ks.size() - 1; ++i)
                h << left(kw, ks[i]) << "| " << vs[i] << '\n';
            h << left(kw, ks.back()) << "| " << vs.back();
        }
        return h;
    }

    template <class FN> auto from_list(FN&& f, O x) {
#define CS(X) case OT<X>::typei(): return std::forward<FN>(f)(L<X>(std::move(x)))
        switch (int(x.type())) {
        CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X);
        CS(O);
        default: assert(false);
        }
#undef CS
        return std::forward<FN>(f)(L<O>(std::move(x))); // unreachable
    }

    template <class Z> decltype(auto) print_table(Z&& h, O x) {
        assert(x.is_table());
        const int     tw(terminal_width());
        const L<L<C>> cn(stringify_elements(L<S>(addref(dk(x->dict)))));
        const L<O>    v (addref(dv(x->dict)));
        L<L<L<C>>> data;
        L<J> cw;
        for (index_t i = 0; i < v.size() && J::rep(sum(cw)) <= tw; ++i) {
            data.push_back(from_list(stringify_elements, v[i]));
            cw.emplace_back(1 + std::max(cn[i].size(), max_width(data.back())));
        }
        --cw.back();
        L<C> head;
        for (index_t i = 0; i < cw.size() && head.size() < tw; ++i)
            head << left(cw[i], cn[i]);
        if (head.size() < tw)
            h << head;
        else
            h << std::pair(head.cbegin(), head.cbegin() + tw - 2) << "..";
        h << '\n';
        h << L<C>(std::min(J::rep(tw), J::rep(sum(cw))), C('-'));
        const index_t rows = v[0]->n <= viewport_height?
            v[0]->n : viewport_height - 1;
        for (index_t row = 0; row < rows; ++row) {
            h << '\n';
            L<C> ss;
            for (index_t i = 0; i < cw.size() && ss.size() < tw; ++i) {
                switch (int(v[i]->type)) {
                case OT<F>::typei(): case OT<I>::typei(): case OT<J>::typei():
                    ss << right(int(J::rep(cw[i])-1), data[i][row]);
                    if (i < cw.size()-1) ss << ' ';
                    break;
                default:
                    ss << left(cw[i], data[i][row]);
                }
            }
            if (ss.size() < tw)
                h << ss;
            else
                h << std::pair(ss.cbegin(), ss.cbegin() + tw - 2) << "..";
        }
        return v[0]->n <= viewport_height? h : h << "\n..";
    }

    L<L<C>> stringify_table(O x) {
        assert(x.is_table());
        const int     tw(terminal_width());
        const L<L<C>> cn(stringify_elements(L<S>(addref(dk(x->dict)))));
        const L<O>    v (addref(dv(x->dict)));
        L<L<L<C>>> data;
        L<J> cw;
        for (index_t i = 0; i < v.size() && J::rep(sum(cw)) <= tw; ++i) {
            data.push_back(from_list(stringify_elements, v[i]));
            cw.push_back(J(1 + std::max(cn[i].size(), max_width(data.back()))));
        }
        --cw.back();

        L<C> head;
        for (index_t i = 0; i < cw.size() && head.size() < tw; ++i)
            head << left(cw[i], cn[i]);
        L<L<C>> result;
        if (tw <= head.size()) {
            head.trunc(tw - 2);
            head.push_back(C('.'));
            head.push_back(C('.'));
        }
        result.push_back(head);
        result.push_back(L<C>(std::min(J::rep(tw), J::rep(sum(cw))), C('-')));

        const index_t rows = v[0]->n <= viewport_height?
            v[0]->n : viewport_height - 1;
        for (index_t row = 0; row < rows; ++row) {
            L<C> ss;
            for (index_t i = 0; i < cw.size() && ss.size() < tw; ++i) {
                switch (int(v[i]->type)) {
                case OT<F>::typei(): case OT<I>::typei(): case OT<J>::typei():
                    ss << right(int(J::rep(cw[i])-1), data[i][row]);
                    if (i < cw.size()-1) ss << ' ';
                    break;
                default:
                    ss << left(cw[i], data[i][row]);
                }
            }
            if (tw <= ss.size()) {
                ss.trunc(tw - 2); ss.push_back(C('.')); ss.push_back(C('.'));
            }
            result.push_back(ss);
        }
        if (viewport_height < v[0]->n) result.push_back(cstr2lc(".."));
        return result;
    }

    template <class S> decltype(auto) print_object(S&& h, O x) {
        assert(x);

        if      (x.is_atom()) print_atom (h, std::move(x));
        else if (x.is_dict()) print_dict (h, std::move(x));
        else if (x.is_list()) print_list (h, std::move(x));
        else                  print_table(h, std::move(x));
        return h;
    }

    template <class STREAM> decltype(auto) print_type(STREAM&& h, Type x) {
#define CS(X) case -OT<X>::typei(): return h << OT<X>::ch();             \
              case  OT<X>::typei(): return h << char(toupper(OT<X>::ch()))
        switch (int(x)) {
        CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J);
        CS(S); CS(T); CS(X); CS(AO); CS(AP); CS(Opcode);
        default: return h << char(int(x));
        }
#undef CS
    }
} // unnamed

H     operator<<(H     h, Opcode      x) { return h << char(x); }
H     operator<<(H     h, AO          x) { return h << x.op << x.adverb; }
H     operator<<(H     h, AP          x) { return h << "proc" << x.adverb; }
H     operator<<(H     h, const L<C>& x) { for (C c: x) h << c; return h; }
H     operator<<(H     h, O           x) { return print_object(h, std::move(x)); }
H     operator<<(H     h, Type        x) { return print_type(h, x); }
L<C>& operator<<(L<C>& h, O           x) { return print_object(h, std::move(x)); }
L<C>& operator<<(L<C>& h, Type        x) { return print_type(h, x); }
