#include <prior.h>

O prior1(bfun_t f, O x)      { return prior(f, std::move(x)); }
O prior2(bfun_t f, O x, O y) { return prior(f, std::move(x), std::move(y)); }
