#pragma once

#include <kv.h>
#include <l.h>
#include <o.h>

O interp(KV<S, KV<S,O>>& env, L<C> src, L<S> infix_words, bool trace=false);
