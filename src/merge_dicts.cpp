#include <merge_dicts.h>
#include <arith.h>
#include <doctest.h>
#include <kv.h>
#include <l.h>
#include <lambda.h>
#include <match.h>
#include <sym.h>

TEST_CASE("Adding two conforming dicts adds the corresponding values") {
    L<S> xk{"a"_s, "b"_s, "c"_s};
    L<J> xv{1,2,3};
    L<J> yv{10,20,30};
    KV<S,J> e{xk, L<J>{11,22,33}};
    KV<S,J> r(merge_dicts(L2(x+y), xk, xv, xk, yv));
    CHECK(match_(e, r));
}

 TEST_CASE("Adding two conforming dicts with values of differing shape adds the corresponding values") {
    L<S> xk{"a"_s, "b"_s, "c"_s};
    KV<S,L<J>> x{xk, L<L<J>>{L<J>{0,1,2}, L<J>{3,4,5}, L<J>{6,7,8}}};
    KV<S,J>    y{xk, L<J>{10,20,30}};
    KV<S,L<J>> r(add(x, y));
    KV<S,L<J>> e{xk, L<L<J>>{L<J>{10,11,12}, L<J>{23,24,25}, L<J>{{36,37,38}}}};
    CHECK(match_(e, r));
}

TEST_CASE("Adding two non-conforming dicts adds matching and carries the rest") {
    L<S> xk{"a"_s, "b"_s, "c"_s};
    L<J> xv{1,2,3};
    L<S> yk{"a"_s, "b"_s, "c"_s};
    L<J> yv{10,20,30};
    L<S> ek{"a"_s, "b"_s, "c"_s};
    L<J> ev{11,22,33};
    KV<S,J> e{ek,ev};
    KV<S,J> r(merge_dicts(L2(x+y), xk, xv, yk, yv));
    CHECK(match_(e, r));
}
