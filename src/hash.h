#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <exception.h>
#include <l.h>
#include <limits>
#include <o.h>
#include <ukv.h>

struct Hash {
    template <class X> using OT = ObjectTraits<X>;

    index_t operator()(uint32_t logcap, uint64_t x) const {
        // Fibonacci (aka multiplicative) hashing from Knuth
        // See https://www.youtube.com/watch?v=lOIP_Z_-0Hs
        const index_t h = index_t((11400714819323198485ull * x) >> (64 - logcap));
        assert(0ll <= h && h < std::numeric_limits<index_t>::max());
        return h;
    }

    template <class Z> uint64_t operator()(Z x) const {
        static_assert(sizeof(Z) <= sizeof(uint64_t));
        uint64_t v = 0;
        memcpy(&v, &x, sizeof x);
        return v;
    }

    template <class Z> uint64_t operator()(L<Z> x) const {
        static_assert(sizeof(Z) <= sizeof(uint64_t));
        static_assert(sizeof(uint64_t) == 8);
        
        const uint64_t bytes = std::size_t(x.size()) * sizeof(Z);
        const char* p = reinterpret_cast<const char*>(x.begin());
        uint64_t g = 0;
        uint64_t i = 0;
        for (; i < bytes - 7; i += 8) {
            uint64_t v;
            memcpy(&v, p + i, sizeof v);
            g = (31 * g) + v;
        }
        if (i < bytes) {
            uint64_t v = 0;
            memcpy(&v, p + i, bytes - i);
            g = (31 * g) + v;
        }
        return g;
    }

    uint64_t operator()(UKV x) const {
        return (*this)(x.key()) * 31 + (*this)(x.val());
    }

    uint64_t operator()(L<O> x) const {
        uint64_t g = 0;
        for (const O& o: x) g = (31 * g) + (*this)(o);
        return g;
    }

    uint64_t operator()(O x) const {
#define CS(X) case -OT<X>::typei(): return (*this)(x.atom<X>()); \
              case OT<X>::typei() : return (*this)(L<X>(std::move(x)))
        switch (int(x->type)) {
        CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J); CS(S); CS(T); CS(X);
        case OT<O>::typei(): return (*this)(L<O>(x));
        case '!': return (*this)(UKV(x));
        case '+': return (*this)(UKV(addref(x->dict))) + 47;
        default: throw Exception("nyi: hash of unknown type");
        }
#undef CS
    }

    template <class X> index_t operator()(uint32_t logcap, X x) const {
        return (*this)(logcap, (*this)(std::move(x)));
    }
};
