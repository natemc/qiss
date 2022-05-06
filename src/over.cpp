#include <over.h>

O over1(bfun_t f, O x)      { return over(f, x); }
O over2(bfun_t f, O x, O y) { return over(f, x, y); }
