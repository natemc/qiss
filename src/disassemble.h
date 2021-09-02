#pragma once

#include <cstddef>
#include <l.h>

L<C>    disassemble            (const L<X>& code,       index_t max_width=64);
index_t disassemble_instruction(L<C>& buf, const X* ip, index_t max_width=64);
