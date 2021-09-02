#pragma once

#include <kv.h>
#include <l.h>
#include <o.h>
#include <prim.h>

KV<S,O> init_module(S name);
KV<S,O> init_module(S name, L<X> code);
