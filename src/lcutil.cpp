#include <lcutil.h>
#include <cctype>
#include <cstring>
#include <output.h>
#include <sym.h>
#include <utility>

bool operator==(const L<C>& x, const char* y) {
    const C* a = x.begin();
    while (a != x.end() && *y && C::rep(*a) == *y) ++a, ++y;
    return a == x.end() && !*y;
}

L<C>& operator<<(L<C>& buf, Adverb x) {
    switch (x) {
    case Adverb::each : return buf << ':';
    case Adverb::eachL: return buf << "\\:";
    case Adverb::eachR: return buf << "/:";
    case Adverb::over : return buf << '/';
    case Adverb::prior: return buf << "':";
    case Adverb::scan : return buf << "\\";
    }
}

L<C>& operator<<(L<C>& buf, Opcode x) {
    switch (x) {
    case Opcode::flip    : return buf << "+:";
    case Opcode::type    : return buf << "@:";
    case Opcode::key     : return buf << "!:";
    case Opcode::string  : return buf << "$:";
    case Opcode::enlist  : return buf << ",:";
    case Opcode::floor   : return buf << "_:";
    case Opcode::value   : return buf << ".:";
    case Opcode::group   : return buf << "=:";
    case Opcode::recip   : return buf << "%:";
    case Opcode::null    : return buf << "^:";
    case Opcode::distinct: return buf << "?:";
    case Opcode::idesc   : return buf << ">:";
    case Opcode::iasc    : return buf << "<:";
    case Opcode::not_    : return buf << "~:";
    case Opcode::rev     : return buf << "|:";
    case Opcode::where   : return buf << "&:";
    case Opcode::first   : return buf << "*:";
    case Opcode::neg     : return buf << "-:";
    case Opcode::count   : return buf << "#:";
    default              : return buf << char(x);
    }
   
}

L<C>& operator<<(L<C>& buf, AO op) {
    return buf << op.op << op.adverb;
}

L<C>& operator<<(L<C>& buf, char c) {
    buf.push_back(C(c));
    return buf;
}

L<C>& operator<<(L<C>& buf, const char* s) {
    for (; *s; ++s) buf.push_back(C(*s));
    return buf;
}

L<C>& operator<<(L<C>& buf, const void* x) {
    return write_pointer(buf, x);
}

L<C>& operator<<(L<C>& buf, const L<C>& x) {
    buf.append(x.begin(), x.end());
    return buf;
}

L<C>& operator<<(L<C>& buf, std::size_t x) {
    return write_uint64(buf, x);
}

L<C>& operator<<(L<C>& buf, int32_t x) {
    return write_int64(buf, x);
}

L<C>& operator<<(L<C>& buf, int64_t x) {
    return write_int64(buf, x);
}

L<C>& operator<<(L<C>& buf, uint32_t x) {
    return write_uint64(buf, x);
}

L<C>& operator<<(L<C>& buf, uint64_t x) {
    return write_uint64(buf, x);
}

L<C>& operator<<(L<C>& buf, B x) {
    return buf << "01"[B::rep(x)] << 'b';
}

L<C>& operator<<(L<C>& buf, C x) {
    return buf << C::rep(x);
}

L<C>& operator<<(L<C>& buf, std::pair<const char*, const char*> x) {
    buf.append(x.first, x.second);
    return buf;
}

L<C>& operator<<(L<C>& buf, std::pair<const C*, const C*> x) {
    buf.append(x.first, x.second);
    return buf;
}

L<C>& operator<<(L<C>& buf, D x) {
    return write_date(buf, x);
}

L<C>& operator<<(L<C>& buf, double x) {
    return write_double(buf, x);
}

L<C>& operator<<(L<C>& buf, F x) {
    return buf << F::rep(x);
}

L<C>& operator<<(L<C>& buf, H x) {
    return buf << H::rep(x);
}

L<C>& operator<<(L<C>& buf, I x) {
    if (x == NI) return buf << "0N";
    return buf << I::rep(x);
}

L<C>& operator<<(L<C>& buf, J x) {
    if (x.is_null()) return buf << "0N";
    if (x < J(0)) { buf << '-'; x = -x; }
    return buf << std::size_t(J::rep(x));
}

L<C>& operator<<(L<C>& buf, S x) {
    return buf << c_str(x);
}

L<C>& operator<<(L<C>& buf, T x) {
    return write_time(buf, x);
}

L<C>& operator<<(L<C>& buf, X x) {
    return write_byte(buf, x);
}

L<C> cstr2lc(const char* x) {
    return L<C>(x, x + strlen(x));
}

L<C>& escape(L<C>& h, C x) { return escapec(h, x); }

Exception lc2ex(const L<C>& s) {
    const C::rep* const first = reinterpret_cast<const C::rep*>(s.begin());
    return Exception(first, first + s.size());
}

L<C> ltrim(L<C> buf) {
    const auto not_space = [](C c){ return !std::isspace(C::rep(c)); };
    const auto left      = std::find_if(buf.begin(), buf.end(), not_space);
    if (!buf.mine()) return L<C>(left, buf.end());
    std::rotate(buf.begin(), left, buf.end());
    return buf.trunc(std::distance(left, buf.end()));
}

L<C> rtrim(L<C> buf) {
    const auto not_space = [](C c){ return !std::isspace(C::rep(c)); };
    const auto right     = std::find_if(buf.rbegin(), buf.rend(), not_space);
    return right == buf.rend()? L<C>()
        :  !buf.mine()        ? L<C>(buf.begin(), right.base())
        :  /* else */           buf.trunc(std::distance(buf.begin(), right.base()));
}

L<C> trim(L<C> buf) {
    const auto not_space = [](C c){ return !std::isspace(C::rep(c)); };
    const auto left      = std::find_if(buf.begin(), buf.end(), not_space);
    if (left == buf.end()) return L<C>();
    const auto right     = std::find_if(buf.rbegin(), buf.rend(), not_space);
    assert(right != buf.rend());

    if (!buf.mine()) return L<C>(left, right.base());
    std::rotate(buf.begin(), left, right.base());
    buf.trunc(std::distance(buf.begin(), left) + std::distance(buf.rbegin(), right));
    return buf;
}
