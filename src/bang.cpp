#include <bang.h>
#include <algorithm>
#include <arith.h>
#include <cat.h>
#include <count.h>
#include <exception.h>
#include <flip.h>
#include <o.h>
#include <take.h>
#include <ukv.h>
#include <under.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    O unkey_keyed_table(UKV x) {
        auto [k , v ] = std::move(x).kv();
        auto [kk, kv] = UKV(+std::move(k)).kv();
        auto [vk, vv] = UKV(+std::move(v)).kv();
        return +UKV(cat(kk, vk), cat(kv, vv));
    }

    O key_table(index_t x, O y) {
        if (x < 0) throw Exception("domain: !");
        if (x == 0) return y;
        UKV d(addref(y->dict));
        if (L<S>(d.key()).size() <= x) throw Exception("length: !");
        O bx{J(x)};
        O new_key_cols (take (bx, d.key()));
        O new_key_cells(take (bx, d.val()));
        O new_val_cols (under(bx, d.key()));
        O new_val_cells(under(bx, d.val()));
        return UKV(O(make_table(UKV(std::move(new_key_cols),
                                    std::move(new_key_cells)).release())),
                   O(make_table(UKV(std::move(new_val_cols),
                                    std::move(new_val_cells)).release())));
    }

    O key_table(L<S>, O) {
        throw Exception("nyi: ! table using sym");
    }

    O key_table(O x, O y) {
        switch (int(x.type())) {
        case -OT<I>::typei(): return key_table(I::rep(x.atom<I>()), std::move(y));
        case -OT<J>::typei(): return key_table(J::rep(x.atom<J>()), std::move(y));
        case -OT<S>::typei(): return key_table(L<S>{x.atom<S>()}  , std::move(y));
        case OT<S>::typei() : return key_table(L<X>(std::move(x)) , std::move(y));
        default             : throw Exception("type: !");
        }
    }

    O rekey_table(index_t x, UKV y) {
        return key_table(x, unkey_keyed_table(std::move(y)));
    }

    O rekey_table(L<S>, UKV) {
        throw Exception("nyi: ! table using sym");
    }

    O rekey_table(O x, UKV y) {
        switch (int(x.type())) {
        case -OT<I>::typei(): return rekey_table(I::rep(x.atom<I>()), std::move(y));
        case -OT<J>::typei(): return rekey_table(J::rep(x.atom<J>()), std::move(y));
        case -OT<S>::typei(): return rekey_table(L<S>{x.atom<S>()}  , std::move(y));
        case OT<S>::typei() : return rekey_table(L<X>(std::move(x)) , std::move(y));
        default             : throw Exception("type: !");
        }
    }
} // unnamed

O bang(O x, O y) {
    if (y.is_table()) {
        if (!x.is_table()) return key_table(std::move(x), std::move(y));
        if (internal_count(x) != internal_count(y)) throw Exception("length: !");
        return UKV(std::move(x), std::move(y));
    } else if (y.is_dict()) {
//        if (!is_keyed_table(y)) throw Exception("type: !");
        return rekey_table(std::move(x), UKV(std::move(y)));
    } else if (x.is_list() && y.is_list()) {
        if (x->n != y->n) throw Exception("length: !");
        return UKV(std::move(x), std::move(y));
    } else {
        return mod(x, y);
    }
}
