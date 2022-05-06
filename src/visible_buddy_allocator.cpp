#include <algorithm>
#include <cctype>
#include <cstring>
#include <system_alloc.h>
#include <terminal_width.h>
#include <unistd.h>
#include <utility>
#include <visible_buddy_allocator.h>

// Ideally I wouldn't include this, but let's see if it works.
#include <l.h>
#include <o.h>
#include <output.h>
#include <sym.h>

#define L1(e) [&](auto&& x){ return e; }
#define L2(e) [&](auto&& x, auto&& y){ return e; }

namespace {
    template <class X> using OT = ObjectTraits<X>;

    const std::size_t MAX_ALLOCS = 1'000'000;

    const char dred   [] = "\033[30;48;5;1m";
    const char dgreen [] = "\033[30;48;5;2m";
    const char canvas [] = "\033[30;48;5;3m";
    const char dblue  [] = "\033[30;48;5;4m";
    const char purple [] = "\033[30;48;5;5m";
    const char aqua   [] = "\033[30;48;5;6m";
    const char gray   [] = "\033[30;48;5;7m";
    const char dgray  [] = "\033[30;48;5;8m";
    const char bred   [] = "\033[30;48;5;9m";
    const char bgreen [] = "\033[30;48;5;10m";
    const char byellow[] = "\033[30;48;5;11m";
    const char blue   [] = "\033[30;48;5;12m";
    const char pink   [] = "\033[30;48;5;13m";
    const char cyan   [] = "\033[30;48;5;14m";
    const char white  [] = "\033[30;48;5;15m";
    const char lgreen [] = "\033[30;48;5;193m";
    const char* colors[] = {
        dred, dgreen, canvas, dblue, purple, aqua, gray, dgray,
        bred, bgreen, byellow, blue, pink, cyan, white, lgreen,
    };
    const char reset  [] = "\033[0m";

    template <class F, class X> bool all(F&& f, const L<X>& x) {
        return std::all_of(x.begin(), x.end(), std::forward<F>(f));
    }

    bool whole(F x) {
        double int_part;
        double fractional_part = modf(F::rep(x), &int_part);
        return fractional_part == 0;
    }

    template <class Z> decltype(auto) print_atom(Z&& h, O x) {
        assert(x.type() < 0);
        switch (-int(x.type())) {
        case OT<B>::typei()     : return h << x->b;
        case OT<C>::typei()     : return escape(h << '"', x->c) << '"';
        case OT<D>::typei()     : return x->d.is_null()? h << x->d << 'd' : h << x->d;
        case OT<F>::typei()     : return whole(x->f)? h << x->f << 'f' : h << x->f;
        case OT<H>::typei()     : return h << x->h << 'h';
        case OT<I>::typei()     : return h << x->i << 'i';
        case OT<J>::typei()     : return h << x->j;
        case OT<S>::typei()     : return h << x->s;
        case OT<T>::typei()     : return x->t.is_null()? h << x->t << 't' : h << x->t;
        case OT<X>::typei()     : return h << "0x" << x->x;
        case OT<AO>::typei()    : return h << x->ao;
        case OT<AP>::typei()    : return h << OT<AP>::get(x.get());
        case OT<Opcode>::typei(): return h << x->op;
        case OT<bfun_t>::typei(): return h << "builtin";
        case -int(generic_null_type): return h << "::";
        case OT<Proc>::typei()  : return h << "proc";
        default:
            return h << "nyi: operator<<(H, O) for type " << int(x->type);
        }
    }

    template <class S, class X>
    decltype(auto) print_list_no_suffix(S&& h, const L<X>& x, char sep=' ') {
        if (x.empty())
            h << '`' << OT<X>::name() << "$()";
        else {
            if (x.size() == 1) h << ',';
            h << x[0];
            for (index_t i = 1; i < x.size(); ++i) h << sep << x[i];
        }
        return std::forward<S>(h);
    }

    template <class S, class X>
    decltype(auto) print_list(S&& h, const L<X>& x) {
        return print_list_no_suffix(h, x) << OT<X>::ch();
    }

    template <class S> decltype(auto) print_list(S&& h, const L<B>& x) {
        if (x.empty()) return h << "`bool$()";
        if (x.size() == 1) h << ',';
        for (B b: x) h << bool(b);
        return h << 'b';
    }

    template <class S> decltype(auto) print_list(S&& h, const L<C>& x) {
        if (x.empty()) return h << "\"\"";
        if (x.size() == 1) h << ',';
        h << '"';
        for (C c: x) {
            if ('\0' == C::rep(c)) h << '.';
            else                   escape(h, c);
        }
        return h << '"';
    }

    template <class S> decltype(auto) print_list(S&& h, const L<D>& x) {
        return print_list_no_suffix(h, x);
    }

    template <class S> decltype(auto) print_list(S&& h, const L<J>& x) {
        return print_list_no_suffix(h, x);
    }

    template <class Z> decltype(auto) print_list(Z&& h, const L<S>& x) {
        if (x.empty())
            h << "`$()";
        else {
            if (x.size() == 1) h << ',';
            for (S s: x) h << s;
        }
        return std::forward<Z>(h);
    }

    template <class S> decltype(auto) print_list(S&& h, const L<T>& x) {
        return print_list_no_suffix(h, x);
    }

    template <class S> decltype(auto) print_list(S&& h, const L<X>& x) {
        if (x.empty())
            h << "`byte$()";
        else {
            if (x.size() == 1) h << ',';
            h << "0x";
            for (X e: x) h << e;
        }
        return std::forward<S>(h);
    }

    template <class S> decltype(auto) print_list(S&& h, const L<O>& x) {
        if (x.empty())
            h << "()";
        else {
            h << (x.size() == 1? ',' : '(');
            h << x[0];
            for (index_t i = 1; i < x.size(); ++i) h << ';' << x[i];
        }
        if (1 < x.size()) h << ')';
        return std::forward<S>(h);
    }

    template <class Z> decltype(auto) print_list(Z&& h, O x) {
        assert(x.is_list());
#define CS(X) case OT<X>::typei(): print_list(h, L<X>(std::move(x))); break
        switch (int(x.type())) {
        CS(B);
        CS(C);
        CS(D); 
        case OT<F>::typei(): {
            L<F> fs(std::move(x));
            print_list_no_suffix(h, fs);
            if (all(whole, std::move(fs))) h << 'f';
            break;
            }
        CS(I);
        CS(J);
        case OT<O>::typei(): {
            L<O> os(std::move(x));
            if (os.size() <= 1)
                print_list_no_suffix(h, std::move(os));
            else
                print_list_no_suffix(h << '(', std::move(os), ';') << ')'; 
            break;
            }
        CS(S);
        CS(T);
        CS(X);
        default:
            h << "nyi: print_list(,  O) for type " << x.type();
        }
#undef CS
        return h;
    }

    template <class STREAM> decltype(auto) print_dict(STREAM&& h, O) { return h << "dict nyi"; }
    template <class STREAM> decltype(auto) print_table(STREAM&& h, O) { return h << "table nyi"; }

    template <class STREAM> decltype(auto) print_type(STREAM&& h, Type x) {
#define CS(X) case -OT<X>::typei(): return h << OT<X>::ch();             \
              case  OT<X>::typei(): return h << char(toupper(OT<X>::ch()))
        switch (int(x)) {
        CS(B); CS(C); CS(D); CS(F); CS(H); CS(I); CS(J);
        CS(S); CS(T); CS(X); CS(AO); CS(AP); CS(Opcode);
        default: return h << char(int(x));
        }
#undef CS
    }

    struct Buffer {
        explicit Buffer(std::size_t n_): i(0), n(n_) { assert(n < sizeof x); }
        Buffer& operator()(char c) { if (i < n) x[i++] = c; return *this; }
        Buffer& clear() { i = 0; return *this; }
        std::size_t i;
        std::size_t n;
        char        x[4096];
    };

    Buffer& operator<<(Buffer& h, std::pair<const char*, const char*> x) {
        for (const char* p = x.first; p != x.second; ++p) h(*p);
        return h;
    }
    Buffer& escape(Buffer& h, C x) { return escapec(h, x); }

    Buffer& operator<<(Buffer& h, char        x) { return h(isprint(x)? x : '.'); }
    Buffer& operator<<(Buffer& h, uint8_t     x) { return h << char(x); }
    Buffer& operator<<(Buffer& h, int32_t     x) { return write_int64(h, x); }
    Buffer& operator<<(Buffer& h, int64_t     x) { return write_int64(h, x); }
    Buffer& operator<<(Buffer& h, uint32_t    x) { return write_uint64(h, x); }
    Buffer& operator<<(Buffer& h, uint64_t    x) { return write_uint64(h, x); }
    Buffer& operator<<(Buffer& h, double      x) { return write_double(h, x); }
    Buffer& operator<<(Buffer& h, const void* x) { return write_pointer(h, x); }
    Buffer& operator<<(Buffer& h, const char* x) { while (*x) h(*x++); return h; }

    Buffer& operator<<(Buffer& h, Opcode x) {
        switch (x) {
        case Opcode::flip    : return h << "+:";
        case Opcode::type    : return h << "@:";
        case Opcode::key     : return h << "!:";
        case Opcode::string  : return h << "$:";
        case Opcode::enlist  : return h << ",:";
        case Opcode::floor   : return h << "_:";
        case Opcode::value   : return h << ".:";
        case Opcode::group   : return h << "=:";
        case Opcode::recip   : return h << "%:";
        case Opcode::null    : return h << "^:";
        case Opcode::distinct: return h << "?:";
        case Opcode::idesc   : return h << ">:";
        case Opcode::iasc    : return h << "<:";
        case Opcode::not_    : return h << "~:";
        case Opcode::rev     : return h << "|:";
        case Opcode::where   : return h << "&:";
        case Opcode::first   : return h << "*:";
        case Opcode::neg     : return h << "-:";
        case Opcode::count   : return h << "#:";
        default              : return h << char(x);
        }
    }
    Buffer& operator<<(Buffer& h, AO x) { return h << x.op << x.adverb; }
    Buffer& operator<<(Buffer& h, AP x) { return h << "proc" << x.adverb; }
    Buffer& operator<<(Buffer& h, B x) { return h(bool(x)? '1' : '0'); }
    Buffer& operator<<(Buffer& h, C x) { return h << C::rep(x); }
    Buffer& operator<<(Buffer& h, D x) { return write_date(h, x); }
    Buffer& operator<<(Buffer& h, F x) { return h << F::rep(x); }
    Buffer& operator<<(Buffer& h, H x) { return h << H::rep(x); }
    Buffer& operator<<(Buffer& h, I x) { return h << I::rep(x); }
    Buffer& operator<<(Buffer& h, J x) { return h << J::rep(x); }
    Buffer& operator<<(Buffer& h, S x) { return h << '`' << c_str(x); }
    Buffer& operator<<(Buffer& h, T x) { return write_time(h, x); }
    Buffer& operator<<(Buffer& h, X x) { return write_byte(h, x); }
    Buffer& operator<<(Buffer& h, Type        x) { return print_type(h, x); }
    Buffer& operator<<(Buffer& h, O x) {
        return x.is_atom() ? print_atom(h, x)
             : x.is_list() ? print_list(h, x)
             : x.is_dict() ? print_dict(h, x)
             : x.is_table()? print_table(h, x)
             : /* else */    h('n')('y')('i');
    }

    struct Region {
        const void* p;
        uint64_t    sz;
    };

    void out(int fd, const char* s, std::size_t n) {
        [[maybe_unused]] const ssize_t written = write(fd, s, n);
    }

    void out(int fd, const char* first, const char* last) {
        out(fd, first, std::size_t(last - first));
    }

    template <std::size_t N> void out(int fd, const char(&s)[N]) { out(fd, s, N - 1); }

    void out(int fd, const char* s) { out(fd, s, strlen(s)); }

    int out(int fd, uint64_t x) {
        char buf[64];
        int i = 0;
        for (uint64_t p = x; p; p /= 10, ++i) buf[i] = char('0' + p % 10);
        std::reverse(buf, buf + i);
        [[maybe_unused]] ssize_t written = write(fd, buf, std::size_t(i));
        return i;
    }
}

VisibleBuddyAllocator::VisibleBuddyAllocator():
    allocs(static_cast<Alloc*>(alloc_region(sizeof(Alloc) * MAX_ALLOCS))), i(0), allocs_sz(0)
{
}

VisibleBuddyAllocator::~VisibleBuddyAllocator() {
    free_region(allocs, sizeof(Alloc) * MAX_ALLOCS);
}

std::pair<void*, uint64_t> VisibleBuddyAllocator::alloc(uint64_t size) {
    if (allocs_sz == MAX_ALLOCS) {
        const char buf[] = "Too many outstanding memory allocations for memory tracer\n";
        [[maybe_unused]] ssize_t written = write(2, buf, sizeof(buf));
        exit(1);
    }

    const auto [p, sz] = ba.alloc(size);
    allocs[allocs_sz] = {i++, p, sz};
    ++allocs_sz;
    return std::pair{p, sz};
}

void VisibleBuddyAllocator::free(void* p) {
    const auto it = std::find_if(allocs, allocs + allocs_sz, L1(x.p == p));
    if (it == allocs + allocs_sz) {
        const char buf[] = "free requested for unknown allocation\n";
        [[maybe_unused]] ssize_t written = write(2, buf, sizeof buf);
        exit(1);
    }
    ba.free(p);
    std::copy(it + 1, allocs + allocs_sz--, it);
}

std::pair<void*, uint64_t> VisibleBuddyAllocator::grow(void* p, uint64_t new_size) {
    auto [newp, newsz] = ba.grow(p, new_size);
    if (newp) {
        const auto it = std::find_if(allocs, allocs + allocs_sz, L1(x.p == p));
        assert(it != allocs + allocs_sz);
        it->sz = newsz;
    }
    return std::pair{newp, newsz};
}

void VisibleBuddyAllocator::print_old() const {
    if (allocs_sz == 0) return;
    std::sort(allocs, allocs + allocs_sz, L2(x.p < y.p));
    constexpr uint64_t MAX_REGIONS = 1;
    Region regions[MAX_REGIONS] = {};
    uint64_t regions_sz = 0;
    for (uint64_t i = 0; i < allocs_sz; ++i) {
        const Alloc& alloc = allocs[i];
        const Block* const block = ba.header(alloc.p);
        const auto it = std::find_if(regions, regions + regions_sz, L1(x.p == block->region()));
        if (it == regions + regions_sz) {
            if (regions_sz == MAX_REGIONS) {
                out(2, "Too many outstanding memory regions for memory tracer\n");
                return;
            }
            regions[regions_sz++] = {block->region(), ba.bucket_to_bytes(block->region_bucket())};
        }
    }
    if (regions_sz != 1) {
        out(2, "Too many outstanding memory regions for memory tracer\n");
        return;
    }

    const int   tw = terminal_width();
    const char* m  = static_cast<const char*>(regions->p); // memory offset
    const int   iw = 4; // index width
    size_t c       = 0; // color
    int x          = 0; // column
    for (uint64_t i = 0; i < allocs_sz; ++i) {
        Alloc& alloc = allocs[i];
        char* const p = static_cast<char*>(alloc.p) - 8;
        char* const q = static_cast<char*>(alloc.p) + alloc.sz;
        for (; m < p; m += 8, ++x) out(1, " ");
        out(1, colors[c]);
        const int n = out(1, alloc.i);
        for (int j = 0; j < iw - n; ++j) out(1, " ");
        x += iw;
        for (m += iw * 8; m < q; m += 8, ++x) out(1, " ");
        out(1, reset);
        c = (c + 1) % std::size(colors);
        x = x % tw;
    }
    for (; x < tw; ++x) out(1, " ");
    out(1, "\n");
}

void VisibleBuddyAllocator::print() const {
    if (allocs_sz == 0) return;
    std::sort(allocs, allocs + allocs_sz, L2(x.p < y.p));
    constexpr uint64_t MAX_REGIONS = 1;
    Region regions[MAX_REGIONS] = {};
    uint64_t regions_sz = 0;
    for (uint64_t i = 0; i < allocs_sz; ++i) {
        const Alloc& alloc = allocs[i];
        const Block* const block = ba.header(alloc.p);
        const auto it = std::find_if(regions, regions + regions_sz, L1(x.p == block->region()));
        if (it == regions + regions_sz) {
            if (regions_sz == MAX_REGIONS) {
                out(2, "Too many outstanding memory regions for memory tracer\n");
                return;
            }
            regions[regions_sz++] = {block->region(), ba.bucket_to_bytes(block->region_bucket())};
        }
    }
    if (regions_sz != 1) {
        out(2, "Too many outstanding memory regions for memory tracer\n");
        return;
    }

    const int   tw = terminal_width();
    const char* m  = static_cast<const char*>(regions->p); // memory offset
    size_t c       = 0; // color
    int x          = 0; // column
    for (uint64_t i = 0; i < allocs_sz; ++i) {
        Alloc& alloc = allocs[i];
        char* const p = static_cast<char*>(alloc.p) - 8;
        char* const q = static_cast<char*>(alloc.p) + alloc.sz;
        const std::size_t w = std::size_t((q - p) / 8);
        for (; m < p; m += 8, ++x) out(1, " ");
        out(1, colors[c]);
        Buffer buf(w);
        buf << O(addref(static_cast<Object*>(alloc.p)));
        out(1, buf.x, buf.x + buf.i);
        for (std::size_t j = buf.i; j < w; ++j) out(1, " ");
        x += w;
        out(1, reset);
        c = (c + 1) % std::size(colors);
        for (m += w * 8; m < q; m += 8, ++x) out(1, " ");
        x = x % tw;
    }
    for (; x < tw; ++x) out(1, " ");
    out(1, "\n");
}

uint64_t VisibleBuddyAllocator::size(const void* p) const {
    return ba.size(p);
}

uint64_t VisibleBuddyAllocator::used() const {
    return ba.used();
}
