#include <converge.h>

O converge1(ufun_t f, O x) { return converge(f, std::move(x)); }
