#include <cctype>
#include <cstdlib>
#include <cstring>
#include <disassemble.h>
#include <errno.h>
#include <exception.h>
#include <getline.h>
#include <l.h>
#include <lcutil.h>
#include <objectio.h>
#include <primio.h>
#include <read_file.h>
#include <sys/stat.h>
#include <terminal_width.h>
#include <unistd.h>

namespace {
    X parsex(C c) {
        const char        digits[] = "0123456789abcdef";
        const char* const p        = strchr(digits, tolower(C::rep(c)));
        if (!p) {
            char err[] = {'N', 'o', 't', ' ', 'h', 'e', 'x', ':', ' ', '.', '\0'};
            err[9] = C::rep(c);
            throw Exception(err);
        }
        return X(X::rep(p - digits));
    }

    X parsex(C msd, C lsd) {
        return X(X::rep(parsex(msd)) * 16 + X::rep(parsex(lsd)));
    }

    L<X> parsex(const L<C>& s) {
        const index_t n = s.size();
        if (n % 2) throw Exception("odd # of digits");
        if (n == 0) throw Exception("empty input");
        const bool prefix = s[0] == C('0') && s[1] == C('x');
        L<X> r((prefix? n-2 : n) / 2);
        for (index_t j = 0, i = prefix? 2 : 0; i < n; i += 2, ++j)
            r[j] = parsex(s[i], s[i + 1]);
        return r; 
    }
} // unnamed

int main(int argc, char* argv[]) {
    if (2 < argc) {
        H(2) << "Usage: " << argv[0] << " [file]";
        return EXIT_FAILURE;
    }

    if (2 == argc)
        H(1) << disassemble(read_binary(argv[1]), terminal_width() - 4);
    else {
        if (isatty(0)) H(1) << "  " << flush;
        L<C> line = getline();
        for (; 0 < line.size() && trim(line) != "\\\\"; line = getline()) {
            try {
                H(1) << disassemble(parsex(line), terminal_width());
            } catch (const Exception& e) {
                H(2) << e.what() << '\n';
            }
            if (isatty(0)) H(1) << "  " << flush;
        }
    }

    return EXIT_SUCCESS;
}
