#include <each.h>
#include <enlist.h>
#include <exception.h>
#include <lambda.h>
#include <o.h>
#include <type_pair.h>

O each1(ufun_t f, O x) { return eachbox1(f, std::move(x)); }

O each2(bfun_t f, O x, O y) { return eachbox2(f, std::move(x), std::move(y)); }

O each_left(bfun_t f, O x, O y) {
    return eachbox1([&](O a){ return f(std::move(a), y); }, std::move(x));
}

O each_right(bfun_t f, O x, O y) {
    return eachbox1([&](O b){ return f(x, std::move(b)); }, std::move(y));
}
