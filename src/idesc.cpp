#include <idesc.h>
#include <algorithm>
#include <at.h>
#include <exception.h>
#include <flip.h>
#include <l.h>
#include <o.h>
#include <sort.h>
#include <sym.h>
#include <ukv.h>

// NB why is idesc not simply reverse(iasc)?  STABILITY

namespace {
    template <class X> using OT = ObjectTraits<X>;

    template <class Z> L<J> compare_sort(const L<Z>& x) {
        return compare_sort(x, [&](J p, J q){ return x[q] < x[p]; });
    }

    L<J> compare_sort(const L<O>& x) {
        return compare_sort(x, [&](J p, J q){ return is_less(x[q], x[p]); });
    }

    L<J> idesc_(const L<B>& x) {
        const auto bias = [](B e){ return uint8_t(1 - B::rep(e)); };
        return x.size() < 1LL<< 8? radix_sort<uint8_t >(x, bias)
             : x.size() < 1LL<<16? radix_sort<uint16_t>(x, bias)
             : x.size() < 1LL<<32? radix_sort<uint32_t>(x, bias)
             : /* else */          radix_sort<index_t >(x, bias);
    }

    L<J> idesc_(const L<C>& x) {
        const auto bias = [](C e){ return uint8_t(255 - C::rep(e)); };
        return x.size() < 1LL<< 8? radix_sort0<uint8_t >(x, bias)
             : x.size() < 1LL<<16? radix_sort0<uint16_t>(x, bias)
             : x.size() < 1LL<<32? radix_sort0<uint32_t>(x, bias)
             : /* else */          radix_sort0<index_t >(x, bias);
    }

    L<J> idesc_(const L<D>& x) {
        const auto     mm   = std::minmax_element(x.begin(), x.end());
        const auto     bias = [=](D e){ return safe_subtract(*mm.second, e); };
        const uint32_t k    = safe_subtract(*mm.second, *mm.first);
        return k > x.size()? compare_sort(x) // measure to make sure this is right
             : k < 1ull<< 8? radix_sort0(x, bias)
             : k < 1ull<<16? radix_sort1(x, bias)
             : k < 1ull<<24? radix_sort2(x, bias)
             : /* else */    radix_sort3(x, bias);
    }

    L<J> idesc_(const L<F>& x) {
        const auto     key_ = [](F e){ // http://stereopsis.com/radix.html
            const uint64_t& i = reinterpret_cast<uint64_t&>(e);
            return i ^ (uint64_t(-int64_t(i>>63)) | 1ull<<63);
        };
        const auto     mm   = std::minmax_element(x.begin(), x.end(),
            [&](F p, F q){ return key_(p) < key_(q); });
        const auto     bias = [=](F e){ return key_(*mm.second) - key_(e); };
        const uint64_t k    = key_(*mm.second) - key_(*mm.first);
        return k > uint64_t(x.size())? compare_sort(x)
             : k < 1ull<< 8? radix_sort0(x, bias)
             : k < 1ull<<16? radix_sort1(x, bias)
             : k < 1ull<<24? radix_sort2(x, bias)
             : k < 1ull<<32? radix_sort3(x, bias)
             : k < 1ull<<40? radix_sort4(x, bias)
             : k < 1ull<<48? radix_sort5(x, bias)
             : k < 1ull<<56? radix_sort6(x, bias)
             : /* else */    radix_sort7(x, bias);
    }

    L<J> idesc_(const L<I>& x) {
        const auto     mm   = std::minmax_element(x.begin(), x.end());
        const auto     bias = [=](I e){ return safe_subtract(*mm.second, e); };
        const uint64_t k    = safe_subtract(*mm.second, *mm.first);
        return k > uint64_t(x.size())? compare_sort(x)
             : k < 1ull<< 8? radix_sort0(x, bias)
             : k < 1ull<<16? radix_sort1(x, bias)
             : k < 1ull<<24? radix_sort2(x, bias)
             : /* else */    radix_sort3(x, bias);
    }

    L<J> idesc_(const L<J>& x) {
        const auto     mm   = std::minmax_element(x.begin(), x.end());
        const auto     bias = [=](J e){ return safe_subtract(*mm.second, e); };
        const uint64_t k    = safe_subtract(*mm.second, *mm.first);
        return k > uint64_t(x.size())? compare_sort(x)
             : k < 1ull<< 8? radix_sort0(x, bias)
             : k < 1ull<<16? radix_sort1(x, bias)
             : k < 1ull<<24? radix_sort2(x, bias)
             : k < 1ull<<32? radix_sort3(x, bias)
             : k < 1ull<<40? radix_sort4(x, bias)
             : k < 1ull<<48? radix_sort5(x, bias)
             : k < 1ull<<56? radix_sort6(x, bias)
             : /* else */    radix_sort7(x, bias);
    }

    L<J> idesc_(const L<O>& x) { return compare_sort(x); }

    L<J> idesc_(const L<S>& x) {
        const L<J>& sr(sym_rank());
        L<J> index(x.size());
        std::transform(x.begin(), x.end(), index.begin(),
                       [&](S s){ return sr[S::rep(s)]; });
        return idesc_(index);
    }

    L<J> idesc_(const L<T>& x) {
        const auto     mm   = std::minmax_element(x.begin(), x.end());
        const auto     bias = [=](T e){ return safe_subtract(*mm.second, e); };
        const uint32_t k    = safe_subtract(*mm.second, *mm.first);
        return k > x.size()? compare_sort(x) // measure to make sure this is right
             : k < 1ull<< 8? radix_sort0(x, bias)
             : k < 1ull<<16? radix_sort1(x, bias)
             : k < 1ull<<24? radix_sort2(x, bias)
             : /* else */    radix_sort3(x, bias);
    }

    L<J> idesc_(const L<X>& x) {
        const auto bias = [](X e){ return uint8_t(255 - X::rep(e)); };
        return x.size() < 1LL<< 8? radix_sort0<uint8_t >(x, bias)
             : x.size() < 1LL<<16? radix_sort0<uint16_t>(x, bias)
             : x.size() < 1LL<<32? radix_sort0<uint32_t>(x, bias)
             : /* else */          radix_sort0<index_t >(x, bias);
    }
    
    O idesc_dict(UKV x) {
        auto [k, v] = std::move(x).kv();
        return at(std::move(k), idesc(std::move(v)));
    }

    L<J> idesc_table(O x) {
        assert(x.is_table());
        // (!#t){x@>t[y]x}/|!+t
        L<O> v(UKV(+std::move(x)).val());
        L<J> i(idesc(v.back()));
        for (index_t j = v.size() - 2; 0 <= j; --j) {
            O k(idesc(at(v[j], i)));
            i = L<J>(at(std::move(i), std::move(k)));
        }
        return i;
    }
} // unnamed

O idesc(O x) {
#define CS(X) case OT<X>::typei(): return idesc_(L<X>(std::move(x)))
    switch (int(x.type())) {
    CS(B); CS(C); CS(D); CS(F); CS(I); CS(J); CS(S); CS(T); CS(X); CS(O);
    case '!': return idesc_dict(UKV(std::move(x)));
    case '+': return idesc_table(std::move(x));
    default : throw Exception("type: idesc");
    }
#undef CS
}
