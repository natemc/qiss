#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <exception.h>
#include <getline.h>
#include <interp.h>
#include <lcutil.h>
#include <module.h>
#include <o.h>
#include <objectio.h>
#include <primio.h>
#include <qiss_alloc.h>
#include <read_file.h>
#include <sym.h>
#include <unistd.h>

namespace {

L<C>& insert_semicolons(L<C>& script) {
    char last_non_space = '\0';
    for (C& ch: script) {
      const char c = C::rep(ch);
      if (c == '\n' && last_non_space != ';')
          ch = C(';');
      if (!isspace(c)) last_non_space = c;
    }
    return script;
}

} // unnamed

int main(int argc, char* argv[]) {
    L<S>           infix_words{"in"_s};
    KV<S, KV<S,O>> env;
    const S        module_name(0);
    env.add(module_name, init_module(module_name));
    KV<S,O>        sys(init_module("sys"_s));

    bool trace = false;
    for (int i = 1; i < argc; ) {
        const char* const arg = argv[i++];
        if (arg[0] == '-') {
            if (arg[1] == 't') trace = true;
        } else {
            L<L<C>> args(argc - i);
            for (int j = 0; i < argc; ++i, ++j)
                args[j] = L<C>(argv[i], argv[i] + strlen(argv[i]));
            sys.add("argv"_s, args);
            env.add("sys"_s , sys);
            L<C> script = read_text(arg);
            insert_semicolons(script);
            try {
                O result(interp(env, script, infix_words, trace));
                H(1) << result << '\n' << flush;
            } catch (const Exception& e) {
                H(2) << e.what() << '\n';
            }
        }
    }
  
    if (isatty(0)) H(1) << "  " << flush;
    L<C> line = getline();
    for (; trim(line) != "\\\\"; line = getline()) {
        try {
            const L<C> cmd = trim(line);
            if (cmd.size()) {
                if (cmd[0] != C('\\')) {
                    const O result(interp(env, cmd, infix_words, trace));
                    if (result->type != generic_null_type)
                        H(1) << result << '\n' << flush;
                } else if (1 < cmd.size()) {
                    if (cmd[1] == C('m')) {
                        qiss_print();
                    } else {
                        H(1) << "nyi\n";
                    }
                }
            }
        } catch (const Exception& e) {
            H(2) << '\'' << e.what() << '\n' << flush;
        }
        if (isatty(0)) H(1) << "  " << flush;
    }
}
