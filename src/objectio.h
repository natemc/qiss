#pragma once

#include <l.h>
#include <o.h>

H     operator<<(H     h, AO          x);
H     operator<<(H     h, Opcode      x);
H     operator<<(H     h, const O&    x);
H     operator<<(H     h, const L<C>& x);
H     operator<<(H     h, Type        x);
L<C>& operator<<(L<C>& h, const O&    x);
L<C>& operator<<(L<C>& h, Type        x);
