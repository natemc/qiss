#pragma once

#include <kv.h>
#include <o.h>

// Generates code from the output of compile, i.e., ast is *totally* flat
void gen(KV<S,O>& module, O ast, bool trace=false);
