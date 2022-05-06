#include <algorithm>
#include <count.h>
#include <cstring>
#include <doctest.h>
#include <flip.h>
#include <interp.h>
#include <kv.h>
#include <l.h>
#include <module.h>
#include <o.h>
#include <objectio.h>
#include <primio.h>
#include <sym.h>
#include <type_traits>
#include <ukv.h>

namespace {
    template <class X> O oo(X x) {
        if constexpr(std::is_same_v<O,X>) return x;
        else                              return O(x);
    }

    L<S> vsym(const char* syms) {
        const index_t n = index_t(strlen(syms));
        L<S> r(n);
        std::transform(syms, syms + n, r.begin(), [](char c){
            const char b[2] = {c, '\0'};
            return sym(b);
        });
        return r;
    }
}

bool operator==(O x, O y) { return match_(x, y); }
#define MATCH(x, y) CHECK(oo(x) == oo(y))
#define INTERP(x, y) MATCH((x), interp(y))

O interp(const char* src) {
    KV<S, KV<S,O>> env;
    env.add(S(0), init_module(S(0)));
    L<S> no_infix_words;
    return interp(env, L<C>(src, src + strlen(src)), no_infix_words);
}

TEST_CASE("boolean literals eval to themselves") {
    INTERP(B(0), "0b");
    INTERP(B(1), "1b");
}

TEST_CASE("boolean list literals eval to themselves") {
    INTERP(L<B>({1,0,0,1,0,0,1}), "1001001b");
}

TEST_CASE("char literals eval to themselves") {
    INTERP(C('5' ), R"("5")");
    INTERP(C('\\'), R"("\\")");
    INTERP(C('"' ), R"("\"")");
    INTERP(C(' ' ), R"(" ")");
    INTERP(C('\n'), R"("\n")");
    INTERP(C('\r'), R"("\r")");
    INTERP(C('\t'), R"("\t")");
}

TEST_CASE("char list literals eval to themselves") {
    INTERP(L<C>({'x','y','z'}), R"("xyz")");
    INTERP(L<C>({'f','o','o','\n'}), R"("foo\n")");
}

TEST_CASE("date literals eval to themselves") {
    INTERP(D(0)  , "2000.01.01");
    INTERP(D(366), "2001-01-01");
    INTERP(ND    , "0Nd"       );
    INTERP(WD    , "0Wd"       );
}

TEST_CASE("date list literals eval to themselves") {
    INTERP(L<D>({0   ,366 }), "2000.01.01 2001-01-01");
    INTERP(L<D>({D(0),ND  }), "2000.01.01 0N");
    INTERP(L<D>({WD  ,D(0)}), "0W 2000.01.01");
}

TEST_CASE("float literals eval to themselves") {
    INTERP(F(3.0   ), "3.0"   );
    INTERP(F(3.0   ), "3f"    );
    INTERP(F(3.0   ), "3."    );
    INTERP(F(3.0   ), "3.f"   );
    INTERP(F(3.14  ), "3.14"  );
    INTERP(F(3.5   ), "3.5f"  );
    INTERP(F(-3.0  ), "-3f"   );
    INTERP(F(-3.14 ), "-3.14" );
    INTERP(F(-2.7  ), "-2.7"  );
    INTERP(F(31.4  ), "3.14e1");
    INTERP(F(300000), "3e5"   );
    INTERP(F(300000), "3e5f"  );
    INTERP(F(300000), "3.e5"  );
    INTERP(F(300000), "3.e5f" );
    INTERP(F(0     ), "0f"    );
    INTERP(NF       , "0n"    );
    INTERP(NF       , "0nf"   );
    INTERP(NF       , "0Nf"   );
    INTERP(WF       , "0w"    );
    INTERP(-WF      , "-0w"   );
}

TEST_CASE("float list literals eval to themselves") {
    INTERP(L<F>({1.0,2.0,3.0})       , "1 2 3f");
    INTERP(L<F>({1.0,2.0,3.0})       , "1. 2 3");
    INTERP(L<F>({1.0,2.0,3.0})       , "1 2. 3");
    INTERP(L<F>({1.0,-2.0,3.0})      , "1 -2. 3");
    INTERP(L<F>({-1.0,2.0,3.0})      , "-1 2 3f");
    INTERP(L<F>({1.0,-2.0,3.0})      , "1 -2 3f");
    INTERP(L<F>({F::rep(NF),2.0,3.0}), "0n 2 3");
    INTERP(L<F>({1.0,F::rep(NF),3.0}), "1 0n 3");
    INTERP(L<F>({1.0,2.0,F::rep(NF)}), "1 2 0n");
}

TEST_CASE("int literals eval to themselves") {
    INTERP(I(4 ), "4i");
    INTERP(I(-4), "-4i");
    INTERP(NI   , "0Ni");
    INTERP(WI   , "0Wi");
    INTERP(-WI  , "-0Wi");
}

TEST_CASE("int list literals eval to themselves") {
    INTERP(L<I>({1,2,3})         , "1 2 3i");
    INTERP(L<I>({1,2,3})         , "1 2i 3");
    INTERP(L<I>({1,2,3})         , "1i 2 3");
    INTERP(L<I>({I::rep(NI),2,3}), "0N 2 3i");
    INTERP(L<I>({I::rep(WI),2,3}), "0W 2 3i");
    INTERP(L<I>({1,I::rep(NI),3}), "1 0N 3i");
    INTERP(L<I>({1,I::rep(WI),3}), "1 0W 3i");
    INTERP(L<I>({1,2,I::rep(NI)}), "1 2 0Ni");
    INTERP(L<I>({1,2,I::rep(WI)}), "1 2 0Wi");
    INTERP(L<I>({I::rep(NI),2,3}), "0N 2i 3");
    INTERP(L<I>({I::rep(WI),2,3}), "0W 2i 3");
}

TEST_CASE("long literals eval to themselves") {
    INTERP(J(5) , "5");
    INTERP(J(-5), "-5");
    INTERP(NJ   , "0N");
    INTERP(WJ   , "0W");
    INTERP(-WJ  , "-0W");
    INTERP(J(5) , "5j");
    INTERP(J(-5), "-5j");
    INTERP(NJ   , "0Nj");
    INTERP(WJ   , "0Wj");
    INTERP(-WJ  , "-0Wj");
}

TEST_CASE("long list literals eval to themselves") {
    INTERP(L<J>({1,2,3})       , "1 2 3");
    INTERP(L<J>({1,2,3})       , "1 2 3");
    INTERP(L<J>({NJ,J(2),J(3)}), "0N 2 3");
    INTERP(L<J>({WJ,J(2),J(3)}), "0W 2 3");
    INTERP(L<J>({1,2,3})       , "1 2 3j");
    INTERP(L<J>({1,2,3})       , "1 2j 3");
    INTERP(L<J>({NJ,J(2),J(3)}), "0N 2 3j");
    INTERP(L<J>({WJ,J(2),J(3)}), "0W 2 3j");
}

TEST_CASE("symbol literals eval to themselves") {
    INTERP(""_s   , "`");
    INTERP("foo"_s, "`foo");
}

TEST_CASE("symbol list literals eval to themselves") {
    INTERP(L<S>({""_s, ""_s})      , "``");
    INTERP(L<S>({"foo"_s, "bar"_s}), "`foo`bar");
}

TEST_CASE("time literals eval to themselves") {
    INTERP(T(34200000), "09:30:00.000");
    INTERP(NT         , "0Nt");
    INTERP(WT         , "0Wt");
}

TEST_CASE("time list literals eval to themselves") {
    INTERP(L<T>({34200000,35145250}), "09:30:00.000 09:45:45.250");
    INTERP(L<T>({NT,T(34200000)})   , "0N 09:30:00.000");
    INTERP(L<T>({WT,T(35145250)})   , "0W 09:45:45.250");
}

TEST_CASE("byte literals eval to themselves") {
    INTERP(X(0) , "0x00");
    INTERP(X(66), "0x42");
}

TEST_CASE("byte list literals eval to themselves") {
    INTERP(L<X>({174,33 }), "0xae21");
    INTERP(L<X>({190,239}), "0xBEEF");
}

TEST_CASE("mixed list literals eval to themselves") {
    INTERP(L<O>({O(L<S>{"a"_s,"b"_s}), O(J(1))}), "(`a`b;1)");
}

TEST_CASE("dict literals eval to themselves") {
    INTERP(UKV(vsym("abc"), L<J>{1,2,3}), "[a:1;b:2;c:3]");
}

TEST_CASE("table literals eval to themselves") {
    INTERP(+UKV(vsym("ab"), L<O>{O(vsym("abc")), O(L<J>{1,2,3})}),
           "([]a:`a`b`c;b:1 2 3)");
}

TEST_CASE("keyed table literals eval to themselves") {
    O kk(+UKV(vsym("a"), L<O>{vsym("abc")}));
    O kv(+UKV(vsym("b"), L<O>{L<J>{1,2,3}}));
    INTERP(UKV(kk, kv), "([a:`a`b`c]b:1 2 3)");
}

TEST_CASE("unary ~ on an atom is not") {
    INTERP(B(1), "~0b");
    INTERP(B(1), "~0");
    INTERP(B(0), "~1b");
    INTERP(B(0), "~5");
}

TEST_CASE("unary ~ on a list nots the elements") {
    INTERP(L<B>({0,1,0}), "~101b");
    INTERP(L<B>({0,0,1}), "~2 7 0");
}

TEST_CASE("unary ~ on a dict nots the value") {
    INTERP(UKV(vsym("abc"), L<B>({0,1,0})), "~`a`b`c!101b");
}

TEST_CASE("unary ~ on a table nots the cells") {
    INTERP(+UKV(vsym("a"), L<O>{L<B>{0,1,0}}), "~([]a:1 0 5)");
}

TEST_CASE("unary ~ on a keyed table nots the cells of the value") {
    O kk(+UKV(vsym("a"), L<O>{vsym("abc")}));
    O kv(+UKV(vsym("b"), L<O>{L<B>{0,1,0}}));
    INTERP(UKV(kk, kv), "~1!([]a:`a`b`c;b:1 0 5)");
}

TEST_CASE("unary ! on a boolean is til") {
    INTERP(L<J>{} , "!0b");
    INTERP(L<J>{0}, "!1b");
}

TEST_CASE("unary ! on an int is til") {
    INTERP(L<J>{}, "!0i");
    INTERP(L<J>({0,1,2})   , "!3i");
}

TEST_CASE("unary ! on a long is til") {
    INTERP(L<J>{}, "!0");
    INTERP(L<J>({0,1,2})   , "!3");
}

TEST_CASE("unary ! on a list is its indices") {
    INTERP(L<J>{}          , "!()");
    INTERP(L<J>({0,1,2,3}) , "!1 2 3 4");
}

TEST_CASE("unary ! on a dict is its key") {
    INTERP(vsym("abc"), "![a:1;b:2;c:3]");
}

TEST_CASE("unary @ is type") {
    INTERP(I(-8), "@42");
}

TEST_CASE("unary # is count") {
    INTERP(J(1), "#377");
    INTERP(J(3), "#`a`b`c");
    INTERP(J(4), "#[a:1;b:2;c:3;d:4]");
    INTERP(J(3), "#([]p:3 1 4)");
    INTERP(J(3), "#([a:`a`b`c]e:2 7 1)");
}

TEST_CASE("unary $ is string") {
    INTERP(L<C>({'4','2'}), "$42");
}

TEST_CASE("unary $ on a list is atomic string") {
    INTERP(L<O>({L<C>{'4','2'}, L<C>{'4','7'}}), "$42 47");
}

TEST_CASE("unary $ on a dict is strings the value") {
    INTERP(UKV(vsym("pq"), L<O>{L<C>{'4','2'}, L<C>{'4','7'}}),
           "$`p`q!42 47");
}

TEST_CASE("unary $ on a table strings the cells") {
    L<O> val{L<O>{L<C>{'a'}, L<C>{'b'}, L<C>{'c'}},
             L<O>{L<C>{'1'}, L<C>{'2'}, L<C>{'3'}}};
    INTERP(+UKV(vsym("ab"), val), "$([]a:`a`b`c;b:1 2 3)");
}

TEST_CASE("unary $ on a keyed table strings the cells of the value") {
    UKV expected(+UKV(vsym("a"), L<O>{vsym("abc")}),
                 +UKV(vsym("b"), L<L<O>>{L<O>{L<C>{'1'}, L<C>{'2'}, L<C>{'3'}}}));
    INTERP(expected, "$([a:`a`b`c]b:1 2 3)");
}

TEST_CASE("unary % is reciprocal") {
    INTERP(F(0.5)                            , "%2");
    INTERP(F(0.3125)                         , "%3.2");
    INTERP(L<F>({1.0,.5,.25})                , "%1 2 4");
    INTERP(UKV(vsym("abc"), L<F>{1.0,.5,.25}), "%`a`b`c!1 2 4");
    INTERP(+UKV(vsym("a"), L<O>{L<F>{1.0,.5,.25}}), "%([]a:1 2 4)");
    INTERP(UKV(+UKV(vsym("a"), L<O>{vsym("ab")}),
               +UKV(vsym("b"), L<L<F>>{L<F>{1.0,0.5}})),
           "%([a:`a`b]b:1 2)");
}

TEST_CASE("unary ^ is null") {
    INTERP(B(0), "^0b");
    INTERP(B(0), "^1b");
    INTERP(B(0), "^\"x\"");
    INTERP(B(1), "^\" \"");
    INTERP(B(0), "^0f");
    INTERP(B(1), "^0n");
    INTERP(B(0), "^0i");
    INTERP(B(1), "^0Ni");
    INTERP(B(0), "^0");
    INTERP(B(1), "^0N");
    INTERP(B(0), "^`foo");
    INTERP(B(1), "^`");
    INTERP(L<B>({0,1,0}), "^1 0N 3");
    INTERP(UKV(vsym("abc"), L<B>({0,1,0})), "^`a`b`c!1 0N 3");
    INTERP(+UKV(vsym("a"), L<O>{L<B>{0,1,0}}), "^([]a:1 0N 3)");
    INTERP(UKV(+UKV(vsym("a"), L<O>{vsym("abc")}),
               +UKV(vsym("b"), L<L<B>>{L<B>{0,1,0}})),
           "^([a:`a`b`c]b:1 0N 3)");
}

TEST_CASE("unary & is where") {
    INTERP(L<J>({0,3})  , "&1001b");
    INTERP(L<J>()       , "&0000b");
    INTERP(L<J>({1,2,2}), "&!3");
    INTERP(vsym("bcc")  , "&`a`b`c!0 1 2");
}

TEST_CASE("unary * is first") {
    INTERP(J(4)             , "*4 5 6");
    INTERP(C('x')           , "*\"xyz\"");
    INTERP(J(1)             , "*\"xyz\"!1 2 3");
    INTERP(L<J>({1,2,3})    , "*(1 2 3;4 5 6)");
    INTERP(UKV(L<S>{"a"_s,"b"_s}, L<J>{1,10}), "*([]a:1 2 3;b:10 20 30)");
    INTERP(UKV(vsym("abc"), L<O>({O(C('x')), O(F(2.7)), O(J(1))})),
           "*([]a:\"xyz\";b:2.7 3.1 4.2;c:1 2 3)");
    INTERP(UKV(vsym("b"), L<J>{1}), "*([a:`a`b`c]b:1 2 3)");
}

TEST_CASE("unary - is negate") {
    INTERP(J(-4)          , "-(4)");
    INTERP(J(4)           , "4--4--4");
    INTERP(L<J>({0,-2,-4}), "-(0 2 4)");
    INTERP(L<J>({0,-2,-4}), "- 0 2 4");
    INTERP(UKV(vsym("abc"), L<J>{0,-1,-2}), "-`a`b`c!0 1 2");
    INTERP(+UKV(vsym("ab"), L<L<J>>{L<J>{0,-1,-2},L<J>{10,20,30}}),
           "-([]a:0 1 2;b:-10 -20 -30)");
    INTERP(UKV(+UKV(vsym("a"), L<L<S>>{vsym("abc")}),
               +UKV(vsym("b"), L<L<J>>{L<J>{0,-1,-2}})),
           "-([a:`a`b`c]b:0 1 2)");
}

TEST_CASE("unary _ is floor") {
    INTERP(J(3)           , "_3.14");
    INTERP(J(-5)          , "_-4.2");
    INTERP(L<J>({-3,3,10}), "_-2.7 3.1 10.99");
    INTERP(UKV(vsym("abc"), L<J>{0,1,2}), "_`a`b`c!0.5 1.2 2.7");
    INTERP(+UKV(vsym("ab"), L<L<J>>{L<J>{0,1,2}, L<J>{10,20,30}}),
           "_([]a:0.49 1.9 2.01;b:10.1 20.2 30.3)");
    INTERP(UKV(+UKV(vsym("a"), L<L<S>>{vsym("abc")}),
               +UKV(vsym("b"), L<L<J>>{L<J>{0,1,2}})),
           "_([a:`a`b`c]b:0.1 1.3 2.4)");
}

TEST_CASE("unary _ is lower") {
    INTERP(C('a')             , "_\"A\"");
    INTERP(L<C>({'x','y','z'}), "_\"XyZ\"");
    INTERP(UKV(vsym("abc"), L<C>{'f','r','o'}), "_[a:\"F\";b:\"R\";c:\"O\"]");
    INTERP(+UKV(vsym("a"), L<L<L<C>>>{L<L<C>>{L<C>{'f','r','o'}, L<C>{'b','o','z','z'}}}),
           "_([]a:(\"FrO\";\"BOzz\"))");
    INTERP(UKV(+UKV(vsym("a"), L<L<C>>{L<C>{'F','r','O'}}),
               +UKV(vsym("b"), L<L<C>>{L<C>{'b','o','z'}})),
           "_([a:\"FrO\"]b:\"BOz\")");
}

TEST_CASE("unary = is group") {
    INTERP(UKV(L<C>{'a','c','b'}, L<L<J>>{L<J>{0,1,9}, L<J>{2,4,6,7}, L<J>{3,5,8}}),
           "=\"aacbcbccba\"");
    INTERP(UKV(L<J>{0,1,2}, L<L<S>>{vsym("ad"), vsym("be"), vsym("c")}),
           "=`a`b`c`d`e!0 1 2 0 1");
    O expected(UKV(
        +UKV(vsym("a"), L<L<J>>{L<J>{1,2,3}}),
        +UKV(L<S>{""_s}, L<L<L<J>>>{L<L<J>>{L<J>{0,3}, L<J>{1,4}, L<J>{2,5}}})));
    INTERP(expected, "=([]a:1 2 3 1 2 3)");
}

TEST_CASE("unary + is flip") {
    INTERP(L<L<J>>({L<J>{0,2}, L<J>{1,3}}), "+(0 1;2 3)");
    INTERP(+UKV(vsym("a"), L<L<J>>{L<J>{1,0,5}}), "+(,`a)!,1 0 5");
    INTERP(UKV(vsym("a"), L<L<J>>{L<J>{1,0,5}}), "+([]a:1 0 5)");
}

TEST_CASE("unary | is reverse") {
    INTERP(L<J>({3,2,1})      , "|1 2 3");
    INTERP(L<C>({'c','b','a'}), "|\"abc\"");
    INTERP(UKV(vsym("cba"), L<J>{3,2,1}), "|`a`b`c!1 2 3");
    INTERP(+UKV(vsym("a"), L<L<J>>{L<J>{3,2,1}}), "|([]a:1 2 3)");
    INTERP(UKV(+UKV(vsym("a"), L<L<S>>{vsym("cba")}),
               +UKV(vsym("b"), L<L<J>>{L<J>{3,2,1}})),
           "|([a:`a`b`c]b:1 2 3)");
}

TEST_CASE("unary , is enlist") {
    INTERP(L<J>({3})             , ",3");
    INTERP(L<L<J>>({L<J>{1,2,3}}), ",1 2 3");
    INTERP(+UKV(vsym("abc"), L<L<J>>{L<J>{1}, L<J>{2}, L<J>{3}}),
           ",`a`b`c!1 2 3");
}

TEST_CASE("unary < is iasc") {
    INTERP(L<J>({1,3,5,2,4,6,8,7,0,9}), "<4 0 2 1 2 1 2 3 2 4");
    INTERP(vsym("bdfcegihaj"), "<`a`b`c`d`e`f`g`h`i`j!4 0 2 1 2 1 2 3 2 4");
    INTERP(L<J>({1,3,5,2,4,6,8,7,0,9}), "<([]a:4 0 2 1 2 1 2 3 2 4)");
    INTERP(L<J>({1,5,3,2,8,4,6,7,0,9}),
           "<([]a:4 0 2 1 2 1 2 3 2 4;b:`c`b`a`c`b`a`c`b`a`c)");
    INTERP(+UKV(vsym("a"), L<L<S>>{vsym("bdfcegihaj")}),
           "<([a:`a`b`c`d`e`f`g`h`i`j]b:4 0 2 1 2 1 2 3 2 4)");
}

TEST_CASE("unary > is idesc") {
    INTERP(L<J>({0,9,7,2,4,6,8,3,5,1}), ">4 0 2 1 2 1 2 3 2 4");
    INTERP(vsym("ajhcegidfb"), ">`a`b`c`d`e`f`g`h`i`j!4 0 2 1 2 1 2 3 2 4");
    INTERP(L<J>({0,9,7,2,4,6,8,3,5,1}), ">([]a:4 0 2 1 2 1 2 3 2 4)");
    INTERP(L<J>({0,9,7,6,4,2,8,3,5,1}),
           ">([]a:4 0 2 1 2 1 2 3 2 4;b:`c`b`a`c`b`a`c`b`a`c)");
    INTERP(+UKV(vsym("a"), L<L<S>>{vsym("ajhcegidfb")}),
           ">([a:`a`b`c`d`e`f`g`h`i`j]b:4 0 2 1 2 1 2 3 2 4)");
}

TEST_CASE("unary . is value") {
    // Maybe these first two should not be allowed
    INTERP(J(1)         , ". 1");
    INTERP(L<J>({1,2,3}), ". 1 2 3");
    INTERP(L<J>({1,2,3}), ".`a`b`c!1 2 3");
    INTERP(+UKV(vsym("b"), L<L<J>>{L<J>{1,2,3}}), ".([a:`a`b`c]b:1 2 3)");
}

TEST_CASE("unary ? is distinct") {
    INTERP(L<J>({1,3,2}), "?1 1 3 2 3 2 3 3 2 1");
    INTERP(L<L<J>>({L<J>{1,2,3}, L<J>{4,5,6}, L<J>{7,8,9}}),
           "?(1 2 3;1 2 3;4 5 6;1 2 3;7 8 9;4 5 6;1 2 3;7 8 9)");
    INTERP(+UKV(vsym("a"), L<L<J>>{L<J>{1,2,3}}), "?([]a:1 2 3 1 2 3)");
}

TEST_CASE("binary ~ is match") {
    INTERP(O(B(1)), "1~1");
    INTERP(O(B(0)), "1~0");
    INTERP(O(B(1)), "1 2 3~1 2 3");
    INTERP(O(B(0)), "1 2 3~1 2 3f");
    INTERP(O(B(1)), "`a~`a");
    INTERP(O(B(0)), "`a~`b");
    INTERP(O(B(1)), "(`a`b`c!1 2 3)~`a`b`c!1+!3");
    INTERP(O(B(0)), "(`a`b`c!1 2 3)~`a`b`C!1+!3");
    INTERP(O(B(0)), "(`c`b`a!3 2 1)~`a`b`c!1+!3");
    INTERP(O(B(1)), "([]a:1 2 3)~+((),`a)!,1+!3");
    INTERP(O(B(0)), "([]a:1 2 3)~([]a:\"123\")");
}

TEST_CASE("binary ! is make-dict") {
    INTERP(UKV(vsym("abc"), L<O>{O(C('x')), O(F(2.7)), O(J(1))}),
           "`a`b`c!(\"x\";2.7;1)");
    INTERP(UKV(+UKV(vsym("a"), L<L<S>>{vsym("abc")}),
               +UKV(vsym("b"), L<L<J>>{L<J>{1,2,3}})),
           "1!([]a:`a`b`c;b:1 2 3)");
    INTERP(+UKV(vsym("ab"), L<O>{O(vsym("abc")), O(L<J>{1,2,3})}),
           "0!([]a:`a`b`c;b:1 2 3)");
    INTERP(+UKV(vsym("ab"), L<O>{O(vsym("abc")), O(L<J>{1,2,3})}),
           "0!1!([]a:`a`b`c;b:1 2 3)");
}

TEST_CASE("binary @ is surface index") {
    INTERP(J(2)                      , "1 2 3 4@1");
    INTERP(L<J>({1,2})               , "1 2 3 4@0 1");
    INTERP(J(42)                     , "(`a`b`c!31 42 53)@`b");
    INTERP(L<J>({53,42})             , "(`a`b`c!31 42 53)@`c`b");
    INTERP(vsym("b")                 , "t:([]a:`a`b`c;b:1 2 3);(t@`a)@&2=t@`b");
    INTERP(UKV(vsym("ab"), L<J>{4,5}), "([]a:0 2 4;b:1 3 5)@2");
    INTERP(+UKV(vsym("ab"), L<O>{O(vsym("ca")), O(L<J>{30,10})}),
           "([]a:`a`b`c;b:10 20 30)@2 0");
//    INTERP(UKV(vsym("b"), L<L<J>>{{5}}), "([a:0 2 4]b:1 3 5)@4"); // nyi
}

// go keyboard order left to right
// TODO - continue binary operators
TEST_CASE("binary # is take") {
    INTERP(L<J>()                            ,    "0#1 2 3 4");
    INTERP(L<J>{1}                           ,    "1#1 2 3 4");
    INTERP(L<J>{4}                           ,   "-1#1 2 3 4");
    INTERP(L<J>({1,2})                       ,    "2#1 2 3 4");
    INTERP(L<J>({1,2,3,4,1})                 ,    "5#1 2 3 4");
    INTERP(L<L<J>>({L<J>{1,2}, L<J>{3,4}})   ,  "2 2#1 2 3 4");
    INTERP(L<L<J>>({L<J>{1,2}, L<J>{3,4}})   , "0N 2#1 2 3 4");
    INTERP(L<L<J>>({L<J>{1,2}, L<J>{3,4,5}}) , "2 0N#1 2 3 4 5");
//    INTERP(L<J>({1,2})                       , "(,2)#1 2 3 4");
}
