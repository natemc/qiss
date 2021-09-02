#include <getline.h>
#include <errno.h>
#include <exception.h>
#include <unistd.h>

// TODO we need a way to signal EOF; right now it returns an empty line.

L<C> getline(int fd) {
    L<C> r;
    char ch;
    ssize_t bytes = read(fd, &ch, 1);
    for (; 0 < bytes && ch != '\n'; bytes = read(fd, &ch, 1))
        r.push_back(C(ch));
    if (bytes < 0) throw Exception(strerror(errno));
    return r;
}
