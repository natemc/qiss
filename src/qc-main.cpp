#include <count.h>
#include <compile.h>
#include <cstdlib>
#include <disassemble.h>
#include <gen.h>
#include <getline.h>
#include <lcutil.h>
#include <module.h>
#include <o.h>
#include <objectio.h>
#include <parse.h>
#include <primio.h>
#include <sym.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    const char* infile = nullptr;
    bool object        = false;
    bool trace         = false;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            infile = argv[i];
        } else {
            for (int j = 1; argv[i][j]; ++j) {
                if (argv[i][j] == 'o') object = true;
                if (argv[i][j] == 't') trace  = true;
            }
        }
    }
    if (infile) {
        H(2) << "nyi: qc file (work-around: redirect to stdin)\n" << flush;
        return EXIT_FAILURE;
    }

    KV<S,O> module(init_module(S(0)));
    L<S> infix_words{"in"_s};
    if (isatty(0)) H(1) << "  " << flush;
    L<C> line = getline();
    for (; trim(line) != "\\\\"; line = getline()) {
        try {
            O ast(parse(line, infix_words));
            O flattened_ast(compile(ast, trace));
            gen(module, flattened_ast, trace);
            L<X> code(module["code"_s]);
            if (object) H(1) << code;
            else        H(1) << disassemble(code);
            H(1) << flush;
        } catch (const Exception& e) {
            H(2) << e.what() << '\n' << flush;
        }
        if (isatty(0)) H(1) << "  " << flush;
    }

    return EXIT_SUCCESS;
}
