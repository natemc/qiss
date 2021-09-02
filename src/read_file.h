#pragma once

#include <l.h>
#include <prim.h>

L<X> read_binary(const char* path);
L<X> read_binary(int         fd  );
L<C> read_text  (const char* path);
L<C> read_text  (int         fd  );
