#pragma once

#include <l.h>
#include <o.h>
#include <prim.h>

struct O;

O     inspect(O x);
H     inspect(H     h, O x);
L<C>& inspect(L<C>& s, O x);
