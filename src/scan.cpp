#include <scan.h>

O scan1(bfun_t f, O x)      { return scan(f, x); }
O scan2(bfun_t f, O x, O y) { return scan(f, x, y); }
