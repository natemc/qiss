#include <interp.h>
#include <compile.h>
#include <gen.h>
#include <parse.h>
#include <sym.h>
#include <vm.h>

O interp(KV<S, KV<S,O>>& env, L<C> src, L<S> infix_words, bool trace) {
    KV<S,O> module(env[S(0)]);
    // We're going to append the code compiled from src to module`code, so we
    // want to start execution at the start of the new code.
    const index_t start = L<X>(module["code"_s]).size();
    O ast(parse(src, infix_words));
    O flattened_ast(compile(ast));
    gen(module, flattened_ast);
//    if (code->type == OERROR) { return addref(code); }
    return run(env, S(0), start, trace);
}
