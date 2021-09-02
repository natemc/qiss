#pragma once

#include <kv.h>
#include <o.h>
#include <prim.h>

O run(KV<S, KV<S,O>>& env, S module, index_t start=0, bool trace=false);
