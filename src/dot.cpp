#include <dot.h>
#include <at.h>
#include <each.h>
#include <l.h>
#include <o.h>
#include <primio.h>
#include <take.h>
#include <ukv.h>

namespace {
    const struct Dot {
        template <class Y>
        O operator()(O x, L<Y> y) const {
            switch (y.size()) {
            case 0 : throw Exception("type: . requires a non-empty vec on rhs");
            case 1 : return at(x, O(y[0]));
            case 2 : return at(at(x, O(y[0])), O(y[1]));
            default: throw Exception("nyi: . with >2 dim rhs");
           }
      }

        O operator()(O x, L<O> y) const {
            switch (y.size()) {
            case 0 : return std::move(y);
            case 1 : return at(x, y[0]);
            // TODO index at depth if x is a container (not a function)
            case 2 : return y[1].is_list()? eachR(at, at(std::move(x), y[0]), y[1])
                                          : at(at(std::move(x), y[0]), y[1]);
            default: throw Exception("nyi: . with >2 dim rhs");
            }
        }
    } dot_;
}

O dot(O x, O y) {
#define CS(X) case ObjectTraits<X>::typei(): return dot_(std::move(x), L<X>(std::move(y)))
    switch (int(y->type)) {
    CS(B); CS(C); CS(D); CS(F); CS(H); CS(J); CS(S); CS(T); CS(X); CS(O);
    default: throw Exception("type: . requires vec on rhs");
    }
}
