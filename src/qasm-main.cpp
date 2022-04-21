#include <cstdlib>
#include <cstring>
#include <exception.h>
#include <l.h>
#include <objectio.h>
#include <primio.h>
#include <qasm.h>
#include <read_file.h>
#include <sym.h>

namespace {
    struct HCloser {
        explicit HCloser(H h_): h(h_) {}
        ~HCloser() { hclose(h); }
        HCloser(const HCloser&) = delete;
        HCloser& operator=(const HCloser&) = delete;
    private:
        H h;
    };
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    const H err(2);
    try {
        if (argc < 3 || strcmp("-o", argv[1])) {
            err << "Usage: " << argv[0] << " -o outfile [infile]\n"
                << "    If infile is not specified, " << argv[0]
                << " will read from stdin.\n";
            return EXIT_FAILURE;
        }

        const H out = hopen(argv[2], TRUNCATE);
        if (out == H(-1)) {
            err << "Could not open output file " << argv[2] << "\n";
            return EXIT_FAILURE;
        }
        const HCloser hc(out);

        const L<C> qasm(argc == 3? read_text(0) : read_text(argv[3]));
        for (X x: parse_qasm(qasm)) out << X::rep(x);
    } catch (const Exception& e) {
        err << e.what() << '\n';
        return EXIT_FAILURE;
    }
  
    return EXIT_SUCCESS;
}
