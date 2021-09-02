#include <module.h>
#include <in.h>
#include <sym.h>

KV<S,O> init_module(S name) {
    return init_module(name, L<X>());
}

KV<S,O> init_module(S name, L<X> code) {
    L<S> k{"code"_s, "constants"_s, "exports"_s, "objects"_s, "statics"_s, "name"_s};
    L<O> constants;
    L<S> exports;
    KV<S,I> objects;
    L<O> statics{O(in)};
    objects.add("in"_s, I(0));
    L<O> v{O(code), O(constants), O(exports), O(objects), O(statics), O(name)};
    return KV<S,O>(k, v);
}
