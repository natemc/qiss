#include <system.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <exception.h>
#include <lcutil.h>
#include <objectio.h>
#include <primio.h>
#include <qiss_alloc.h>

namespace {
    struct PCloser {
        PCloser(FILE* p_): p(p_) {}
        ~PCloser() { if (p) pclose(p); }
    private:
        FILE* p;
    };

    Exception ex_errno() {
        char buf[4096];
        strerror_r(errno, buf, sizeof buf);
        L<C> s;
        s << buf;
        throw lc2ex(s);
    }

    O shell_out(const char* cmd) {
        FILE* const p = popen(cmd, "r");
        if (!p) throw ex_errno();
        const PCloser pcloser(p);
        L<L<C>> r;
        char buf[65536];
        while (fgets(buf, sizeof buf, p)) {
            const std::size_t n = strlen(buf);
            if (0 < n && buf[n - 1] == '\n') buf[n - 1] = '\0';
            L<C> line;
            line << buf;
            r.push_back(line);
        }
        if (ferror(p)) throw ex_errno();
        return O(r);
    }
}

O system_(L<C> cmd) {
    cmd.push_back(C('\0'));
    const C* const start = cmd.begin() + (cmd[0] == C('\\'));
    switch (C::rep(*start)) {
    case 'm': qiss_print_old(); return O();
    case 'n': qiss_print(); return O();
    default: return shell_out(reinterpret_cast<const char*>(start));
    }
}
