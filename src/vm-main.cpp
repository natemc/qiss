#include <cstdlib>
#include <cstring>
#include <exception.h>
#include <in.h>
#include <module.h>
#include <objectio.h>
#include <primio.h>
#include <qasm.h>
#include <read_file.h>
#include <sym.h>
#include <vm.h>

namespace {
    bool ends_with(const char* suffix, const char* s) {
        const size_t m = strlen(suffix);
        const size_t n = strlen(s);
        if (n < m) return false;
        s += n - m;
        while (*suffix && *suffix == *s) ++suffix, ++s;
        return !*s;
    }
    
    void run_qo(const char* module_name, L<X> qo, bool trace) {
        S m = sym(module_name);
        KV<S,KV<S,O>> env;
        env.add(m, init_module(m, std::move(qo)));
        const index_t start = 0;
        O result(run(env, m, start, trace));
        H(1) << result << '\n';
    }

    void run_qasm(const char* module, const L<C>& qasm, bool trace) {
        run_qo(module, parse_qasm(qasm), trace);
    }
}

int main(int argc, char* argv[]) {
    try {
        bool trace         = false;
        const char* infile = nullptr;
        for (int i = 1; i < argc; ++i) {
            if (!strcmp(argv[i], "-t")) trace = true;
            else                        infile = argv[i];
        }

        if      (!infile)                    run_qasm("stdin", read_text  (0)     , trace);
        else if (ends_with(".qo"  , infile)) run_qo  (infile , read_binary(infile), trace);
        else if (ends_with(".qasm", infile)) run_qasm(infile , read_text  (infile), trace);
        else {
            H(2) << "Input file must be .qasm or .qo\n";
            return EXIT_FAILURE;
        }
    } catch (const Exception& e) {
        H(2) << e.what() << '\n' << flush;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
