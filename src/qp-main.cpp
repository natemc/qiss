#include <cstdlib>
#include <exception.h>
#include <getline.h>
#include <lcutil.h>
#include <o.h>
#include <objectio.h>
#include <parse.h>
#include <primio.h>
#include <sym.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    bool trace         = false;
    const char* infile = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-t")) trace = true;
        else                        infile = argv[i];
    }

    L<S> infix_words{"in"_s};
    if (infile) {
        H(2) << "nyi: kparse file (work-around: stdin redirection)\n" << flush;
        return EXIT_FAILURE;
    }

    if (isatty(0)) H(1) << "  " << flush;
    for (L<C> line = getline(); trim(line) != "\\\\"; line = getline()) {
        try {
            O ast(parse(line, infix_words, trace));
            H(1) << ast << '\n' << flush;
        } catch (const Exception& e) {
            H(2) << e.what() << '\n' << flush;
        }
        if (isatty(0)) H(1) << "  " << flush;
    }

    return EXIT_SUCCESS;
}
