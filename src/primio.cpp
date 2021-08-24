#include <primio.h>
#include <algorithm>
#include <bits.h>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <exception.h>
#include <fcntl.h>
#include <kv.h>
#include <l.h>
#include <o.h>
#include <output.h>
#include <sym.h>
#include <unistd.h>

namespace {
    const index_t BUFFER_SIZE = 4064;

    H blush(H h);
    
    // stdout and stderr are initialized specially so they can be used for
    // debugging before write_buffers is initialized.
    List<X>* init_static_buf(void* p) {
        static_assert(sizeof(List<X>) == 24);
        assert((bitcast<std::uintptr_t>(p) & 31) == 0);
        List<X>* const buf = new (static_cast<char*>(p) + 8) List<X>;
        buf->a             = Attr::none;
        buf->m             = 0;
        buf->r             = 1;
        buf->n             = 0;
        buf->type          = ObjectTraits<X>::typet();
        return buf;
    }
    alignas(32) char ebuf[BUFFER_SIZE + 32];
    List<X>* err() {
        static List<X>* buf;
        if (!buf) buf = init_static_buf(ebuf);
        return buf;
    }
    alignas(32) char obuf[BUFFER_SIZE + 32];
    List<X>* out() {
        static List<X>* buf;
        if (!buf) buf = init_static_buf(obuf);
        return buf;
    }
    struct WriteBuffers {
        WriteBuffers() = default;
        ~WriteBuffers() { for (H h: d.key()) blush(h); }
        WriteBuffers(const WriteBuffers&) = delete;
        WriteBuffers& operator=(const WriteBuffers&) = delete;

        List<X>* operator[](H h) {
            return h == H(1)? out()
                :  h == H(2)? err()
                :  d.has(h) ? d[h].get()
                :  /* else */ nullptr;
        }
        void add(H h) { d.add(h, new_buffer()); }
        void remove(H h) { d.remove(h); }

    private:
        KV<H, L<X>> d;
        static L<X> new_buffer() { return L<X>(make_empty_list<X>(BUFFER_SIZE)); }
    } write_buffers;

    ssize_t writeall(int fd, const void* buf, std::size_t n) {
        const char* const p = static_cast<const char*>(buf);
        std::size_t total = 0;
        while (total < n) {
            const ssize_t written = write(fd, p + total, n - total);
            if      (0 <= written)   total += std::size_t(written);
            else if (errno != EINTR) return written;
        }
        return ssize_t(total);
    }

    H bblush(H h, char* buf, index_t* tail) {
        if (writeall(H::rep(h), buf, std::size_t(*tail)) < 0)
            throw Exception("'os");
        *tail = 0;
        return h;
    }

    H blush(H h) {
        List<X>* const buf = write_buffers[h];
        if (!buf) return h;
        return bblush(h, reinterpret_cast<char*>(buf->begin()), &buf->n);
    }

    H bbrite(H h, const void* p, std::size_t n, char* buf, index_t* tail) {
        const std::size_t avail = BUFFER_SIZE - std::size_t(*tail);
        if (n <= avail) {
            memcpy(buf + *tail, p, n);
            *tail += n;
        } else if (n <= BUFFER_SIZE + avail) {
            memcpy(buf + *tail, p, avail);
            *tail += avail;
            bblush(h, buf, tail);
            memcpy(buf, static_cast<const char*>(p) + avail, n - avail);
            *tail = index_t(n - avail);
        } else {
            bblush(h, buf, tail);
            if (writeall(H::rep(h), p, n) < 0)
                throw Exception("'os");
        }
        return h;
    }

    H brite(H h, const void* p, std::size_t n) {
        List<X>* const buf = write_buffers[h];
        if (buf)
            bbrite(h, p, n, reinterpret_cast<char*>(buf->begin()), &buf->n);
        else if (writeall(H::rep(h), p, n) < 0)
            throw Exception("'os");
        return h;
    }

    H hopen(const char* path, int flags) {
        const int allrw = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
        const int fd = open(path, flags, allrw);
        if (fd != -1) write_buffers.add(H(fd));
        return H(fd);
    }
} // unnamed

H hopen(const char* path) {
    return hopen(path, O_CREAT|O_RDWR);
}

H hopen(const char* path, Truncate) {
    return hopen(path, O_CREAT|O_RDWR|O_TRUNC);
}

void hclose(H h) {
    blush(h);
    write_buffers.remove(h);
    close(H::rep(h));
}

H escape(H h, C x) { return escapec(h, x); }
H flush(H h) { return blush(h); }

H operator<<(H h, H(*f)(H)) { return f(h); }

H operator<<(H h, const char* s) { return brite(h, s , strlen(s)); }
H operator<<(H h, char        c) { return brite(h, &c, 1        ); }
H operator<<(H h, uint8_t     x) { return brite(h, &x, 1        ); }
H operator<<(H h, double      x) { return write_double(h, x); }

H operator<<(H h, int32_t     x) { return write_int64(h, x); }
H operator<<(H h, int64_t     x) { return write_int64(h, x); }
H operator<<(H h, uint32_t    x) { return write_uint64(h, x); }
H operator<<(H h, uint64_t    x) { return write_uint64(h, x); }
H operator<<(H h, std::size_t x) { return write_uint64(h, x); }
H operator<<(H h, const void* x) { return write_pointer(h, x); }
H operator<<(H h, std::pair<const char*, const char*> x) {
    return brite(h, x.first, std::size_t(x.second - x.first));
}

H operator<<(H h, B x) { return h << "01"[B::rep(x)] << 'b'; }
H operator<<(H h, C x) { return h << C::rep(x); }
H operator<<(H h, D x) { return write_date(h, x); }
H operator<<(H h, F x) { return h << F::rep(x); }
H operator<<(H h, H x) { return h << H::rep(x); }

H operator<<(H h, I x) {
    if (x.is_null()) return h << "0N";
    return h << I::rep(x);
}

H operator<<(H h, J x) {
    if (x.is_null()) return h << "0N";
    return h << J::rep(x);
}

H operator<<(H h, S x) { return h << '`' << c_str(x); }
H operator<<(H h, T x) { return write_time(h, x); }
H operator<<(H h, X x) { return write_byte(h, x); }

H operator<<(H h, std::pair<const C*, const C*> x) {
    assert(x.first <= x.second);
    return brite(h, x.first, std::size_t(x.second - x.first));
}
