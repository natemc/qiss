#include <count.h>
#include <l.h>
#include <o.h>
#include <ukv.h>
#include <ul.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const struct Count {
        index_t operator()(O x) const {
            if (x.is_list ()) return UL(std::move(x)).size();
            if (x.is_dict ()) {
                UKV d(std::move(x));
                return d.key().is_table()? (*this)(d.key())
                    :  /* else */          UL(d.key()).size();
            }
            if (x.is_table()) {
                UKV d(addref(x->dict));
                return (*this)(L<O>(d.val())[0]);
            }
            return 1;
        }
    } count_;
}

index_t internal_count (O x) { return count_(std::move(x)); }
I       internal_counti(O x) { return I(I::rep(count_(std::move(x)))); }
J       internal_countj(O x) { return J(count_(std::move(x))); }
O       count          (O x) { return O(J(count_(std::move(x)))); }
O       counti         (O x) { return O(I(I::rep(count_(std::move(x))))); }
