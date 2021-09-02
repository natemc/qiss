#include <read_file.h>
#include <errno.h>
#include <exception.h>
#include <fcntl.h>
#include <lcutil.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {
    struct Closer {
        explicit Closer(int fd_): fd(fd_) {}
        ~Closer() { close(fd); }
        Closer(const Closer&) = delete;
        Closer& operator=(const Closer&) = delete;
        int fd;
    };

    int open_file(const char* path) {
        const int fd = open(path, O_RDONLY);
        if (fd < 0) {
            L<C> err(cstr2lc(strerror(errno)));
            L<C> s;
            s << path << ": " << err;
            throw lc2ex(s);
        }
        return fd;
    }

    template <class Z>
    L<Z> read_file(int fd) {
        static_assert(sizeof(Z) == sizeof(char));

        L<Z> r;
        typename Z::rep buf[4096];
        ssize_t bytes = read(fd, buf, sizeof buf);
        for (; 0 < bytes; bytes = read(fd, buf, sizeof buf))
            r.append(buf, buf + bytes);
        if (bytes < 0) throw Exception(strerror(errno));
        return r;
    }

    template <class Z>
    L<Z> read_file(int fd, off_t sz) {
        static_assert(sizeof(Z) == sizeof(char));

        L<Z> r(sz);
        Z* const p     = r.begin();
        ssize_t  total = 0;
        ssize_t  bytes = read(fd, p, std::size_t(sz - total));
        while (0 < bytes && bytes + total < sz) {
            total += bytes;
            bytes = read(fd, p + total, std::size_t(sz - total));
        }
        if (bytes < 0) throw Exception(strerror(errno));
        return r;
    }
} // unnamed

L<X> read_binary(const char* path) {
    const int fd = open_file(path);
    const Closer closer(fd);
    return read_binary(fd);
}

L<X> read_binary(int fd) {
    struct stat st;
    const int rc = fstat(fd, &st);
    return rc < 0? read_file<X>(fd) : read_file<X>(fd, st.st_size);
}

L<C> read_text(const char* path) {
    const int fd = open_file(path);
    const Closer closer(fd);
    return read_text(fd);
}

L<C> read_text(int fd) {
    struct stat st;
    const int rc = fstat(fd, &st);
    return rc < 0? read_file<C>(fd) : read_file<C>(fd, st.st_size);
}
