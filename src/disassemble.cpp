#include <disassemble.h>
#include <bit>
#include <cassert>
#include <iaddr.h>
#include <initializer_list>
#include <lcutil.h>
#include <opcode.h>
#include <o.h>
#include <primio.h>

// TODO lookup statics and display their values

namespace {
    struct Disassembler {
        Disassembler(L<C>& os_, const X* code_, index_t max_width_):
            os(os_), code(code_), max_width(max_width_)
        {}

        index_t operator()(index_t i);

    private:
        L<C>&    os;
        const X* code;
        index_t  max_width;

        void                       ch    (char x);
        void                       ch    (std::initializer_list<char> x);
        void                       byte  (X x);
        void                       byte  (uint8_t x);
        void                       byte  (index_t i);
        void                       bytesp(index_t i);
        void                       bytes (index_t first, index_t n);
        void                       dec   (uint8_t x);
        void                       dec   (X x);
        void                       instr (const char* inst);
        void                       spaces(index_t n);
        index_t                    jump  (const char* inst, index_t i);
        index_t                    local (index_t i);
        index_t                    decarg(const char* inst, index_t i);
        index_t                    noarg (const char* inst, index_t i);
        template <class Z> index_t arg   (index_t i);
        template <class Z> index_t onearg(const char* inst, index_t i);
        template <class Z> index_t vecarg(const char* inst, index_t i);
    };

    void Disassembler::ch(char x) { os.emplace_back(x); }

    void Disassembler::ch(std::initializer_list<char> x) {
        for (char c: x) ch(c);
    }

    void Disassembler::byte(uint8_t x) {
        const char digit[] = "0123456789abcdef";
        ch({digit[x / 16], digit[x % 16]});
    }

    void Disassembler::byte(X x) {
        byte(X::rep(x));
    }

    void Disassembler::byte(index_t i) {
        byte(code[i]);
    }

    void Disassembler::bytesp(index_t i) {
        byte(i);
        ch(' ');
    }

    void Disassembler::bytes(index_t first, index_t n) {
        const index_t param_bytes = max_width / 6;
        if (param_bytes < n) {
            for (index_t i = 0; i < (param_bytes-1); ++i) bytesp(first + i);
            if (0 < param_bytes) ch({'.', '.'});
            ch(' ');
            return;
        }
        for (index_t i = 0; i != n; ++i) bytesp(first + i);
        spaces((param_bytes - n) * 3);
    }

    void Disassembler::dec(uint8_t x) {
        if (100 <= x) ch('0' + x / 100);
        if ( 10 <= x) ch('0' + x / 10 % 10);
        ch('0' + x % 10);
    }

    void Disassembler::dec(X x) {
        dec(X::rep(x));
    }

    void Disassembler::spaces(index_t n) {
        const char s[] = "                               ";
        const index_t slen = sizeof s - 1;
        for (; slen < n; n -= slen)
            os.append(s, s + slen);
        os.append(s, s + n);
    }

    void Disassembler::instr(const char* i) {
        os.append(i, i + strlen(i));
        assert(strlen(i) <= 8);
        spaces(8 - index_t(strlen(i)));
    }

    index_t Disassembler::jump(const char* inst, index_t i) {
        const index_t addrlen = sizeof(iaddr_t);
        bytes(i+1, addrlen);
        instr(inst);
        ch({'0', 'x'});
        for (index_t j = 0; j < addrlen; ++j)
            byte(code[i + addrlen - j]);
        return i + 1 + addrlen;
    }

    index_t Disassembler::local(index_t i) {
        bytes(i+1, 1);
        instr("local");
        const X::rep arg(code[i + 1]);
        const uint8_t f = arg >> 6;
        const int8_t s = int8_t(arg & 63) - 12;
        if (f) { dec(f); ch(','); ch(' '); }
        if (s < 0) { ch('-'); dec(uint8_t(-s)); }
        else       { dec(uint8_t(s)); }
        return i+2;
    }

    template <class Z>
    index_t Disassembler::arg(index_t i) {
        Z z;
        memcpy(&z, code + i, sizeof z);
        os << z;
        return i + index_t(sizeof z);
    }

    index_t Disassembler::decarg(const char* inst, index_t i) {
        bytes(i+1, 1);
        instr(inst);
        dec(code[i + 1]);
        return i+2;
    }

    index_t Disassembler::noarg(const char* inst, index_t i) {
        bytes(i+1, 0);
        instr(inst);
        return i+1;
    }

    template <class Z>
    index_t Disassembler::onearg(const char* inst, index_t i) {
        bytes(i+1, sizeof(Z));
        instr(inst);
        return arg<Z>(i + 1);
    }

    template <class Z>
    index_t Disassembler::vecarg(const char* inst, index_t i) {
        immlen_t n;
        memcpy(&n, code + i + 1, sizeof n);
        bytes(i+1, sizeof n+n*sizeof(Z));
        i += 1 + sizeof n;
        instr(inst);
        if (0 < n) {
            i = arg<Z>(i);
            for (index_t j = 1; j < n; ++j) {
                os.emplace_back(' ');
                i = arg<Z>(i);
            }
        }
        return i;
    }

    index_t Disassembler::operator()(index_t i) {
        uint8_t addr[sizeof code];
        memcpy(addr, &code, sizeof addr);
        if constexpr(std::endian::native == std::endian::big) {
            for (size_t j = 0; j < sizeof addr / 2; ++j) { byte(addr[j - 1]); }
        } else {
            for (size_t j = sizeof addr / 2; j > 0; --j) { byte(addr[j - 1]); }
        }
        spaces(2);
        bytesp(i);
        switch (Opcode(X::rep(code[i]))) {
        case Opcode::add     : return noarg("+", i);
        case Opcode::assign  : return noarg(":", i);
        case Opcode::at      : return noarg("@", i);
        case Opcode::bang    : return noarg("!", i);
        case Opcode::bindm   : return onearg<sindex_t>("bindm", i);
        case Opcode::bindl   : return decarg("bindl", i);
        case Opcode::bump    : return decarg("bump", i);
        case Opcode::bzero   : return jump("bzero", i);
        case Opcode::call    : return jump("call", i);
        case Opcode::cast    : return noarg("$", i);
        case Opcode::cat     : return noarg(",", i);
        case Opcode::clean   : return decarg("clean", i);
        case Opcode::count   : return noarg("#:", i);
        case Opcode::cut     : return noarg("_", i);
        case Opcode::dot     : return noarg(".", i);
        case Opcode::dup     : return noarg("dup", i);
        case Opcode::dupnth  : return decarg("dupnth", i);
        case Opcode::each    : return noarg("'", i);
        case Opcode::emit    : return decarg("emit", i);
        case Opcode::enlist  : return noarg(",:", i);
        case Opcode::eq      : return noarg("=", i);
        case Opcode::fdiv    : return noarg("%", i);
        case Opcode::fill    : return noarg("^", i);
        case Opcode::find    : return noarg("?", i);
        case Opcode::first   : return noarg("*:", i);
        case Opcode::flip    : return noarg("+:", i);
        case Opcode::floor   : return noarg("_:", i);
        case Opcode::great   : return noarg(">", i);
        case Opcode::group   : return noarg("=:", i);
        case Opcode::halt    : return noarg("halt", i);
        case Opcode::iasc    : return noarg("<:", i);
        case Opcode::idesc   : return noarg(">:", i);
        case Opcode::immb    : return onearg<B>("b", i);
        case Opcode::immbv   : {
            index_t k = i + 1;
            immlen_t n;
            memcpy(&n, code + k, sizeof n);
            k += sizeof n;
            bytes(i+1, n+sizeof n);
            instr("B");
            for (index_t j = 0; j < n; ++j, ++k) dec(code[k]);
            ch('b');
            return k;
        }
        case Opcode::immd    : return onearg<D>("d", i);
        case Opcode::immdv   : return vecarg<D>("D", i); 
        case Opcode::immf    : return onearg<F>("f", i);
        case Opcode::immfv   : return vecarg<F>("F", i);
        case Opcode::immi    : return onearg<I>("i", i);
        case Opcode::immiv   : return vecarg<I>("I", i);
        case Opcode::immj    : return onearg<J>("j", i);
        case Opcode::immjv   : return vecarg<J>("J", i);
        case Opcode::immo    :
            bytes(i+1, sizeof(O));
            instr("O");
            ch({'0', 'x'});
            memcpy(addr, code + i + 1, sizeof addr);
            for (size_t j = 0; j < sizeof addr; ++j) { byte(addr[j]); }
            return i + 1 + index_t(sizeof(O));
        case Opcode::immstr  : {
            index_t k = i + 1;
            immlen_t n;
            memcpy(&n, code + k, sizeof n);
            k += sizeof n;
            bytes(i+1, n+sizeof n);
            instr("\"");
            for (index_t j = 0; j < n; ++j, ++k) os.emplace_back(C::rep(X::rep(code[k])));
            return k;
        }
        case Opcode::immsym  : {
            index_t k = i + 1;
            immlen_t n;
            memcpy(&n, code + k, sizeof n);
            k += sizeof n;
            bytes(i+1, n+sizeof n);
            instr("`");
            for (index_t j = 0; j < n; ++j, ++k) os.emplace_back(C::rep(X::rep(code[k])));
            return k;
        }
        case Opcode::immt    : return onearg<T>("t", i);
        case Opcode::immtv   : return vecarg<T>("T", i);
        case Opcode::immx    : return onearg<T>("x", i);
        case Opcode::immxv   : {
            index_t k = i + 1;
            immlen_t n;
            memcpy(&n, code + k, sizeof n);
            k += sizeof n;
            bytes(i+1, n+sizeof n);
            instr("X");
            ch({'0', 'x'});
            for (index_t j = 0; j < n; ++j, ++k) byte(k);
            return k;
        }
        case Opcode::invoke  : return decarg("invoke", i);
        case Opcode::jump    : return jump("jump", i);
        case Opcode::key     : return noarg("!:", i);
        case Opcode::left    : return noarg("\\:", i);
        case Opcode::less    : return noarg("<", i);
        case Opcode::local   : return local(i);
        case Opcode::match   : return noarg("~", i);
        case Opcode::max     : return noarg("|", i);
        case Opcode::min     : return noarg("&", i);
        case Opcode::mul     : return noarg("*", i);
        case Opcode::neg     : return noarg("-:", i);
        case Opcode::nop     : return noarg("nop", i);
        case Opcode::not_    : return noarg("~:", i);
        case Opcode::null    : return noarg("^:", i);
        case Opcode::op      : return onearg<C>("op", i);
        case Opcode::over    : return noarg("/", i);
        case Opcode::pop     : return noarg("pop", i);
        case Opcode::prior   : return noarg("':", i);
        case Opcode::pushc   : return onearg<sindex_t>("pushc", i);
        case Opcode::pushm   : return onearg<sindex_t>("pushm", i);
        case Opcode::recip   : return noarg("%:", i);
        case Opcode::ret     : return noarg("return", i);
        case Opcode::rev     : return noarg("rev", i);
        case Opcode::right   : return noarg("/:", i);
        case Opcode::rot     : return decarg("rot", i);
        case Opcode::scan    : return noarg("\\", i);
        case Opcode::string  : return noarg("$:", i);
        case Opcode::sub     : return noarg("-", i);
        case Opcode::swap    : return noarg("swap", i);
        case Opcode::take    : return noarg("#", i);
        case Opcode::type    : return noarg("@:", i);
        case Opcode::distinct: return noarg("?:", i);
        case Opcode::value   : return noarg(".:", i);
        case Opcode::vcond   : return noarg("?[", i);
        case Opcode::where   : return noarg("&:", i);
        default              :
            H(1) << " disassemble: unrecognized opcode " << code[i] << '\n' << flush;
            assert(false);
        }
        return -1;
    }
} // namespace

index_t disassemble_instruction(L<C>& buf, const X* code, index_t max_width) {
    L<C> s;
    const index_t r = Disassembler(s, code, max_width)(0);
    if (r < 0) return r;
    if (s.size() <= max_width) {
        buf.append(s.begin(), s.end());
        for (index_t i = buf.size(); i < max_width; ++i) buf.emplace_back(' ');
    } else {
        buf.append(s.begin(), s.begin() + max_width - 2);
        buf.emplace_back('.');
        buf.emplace_back('.');
    }
    return r;
}

L<C> disassemble(const L<X>& code, index_t max_width) {
    L<C> s;
    const X* ip = code.begin();
    for (index_t i = 0; i < code.size(); ) {
        i += disassemble_instruction(s, ip+i, max_width);
        s.emplace_back('\n');
    }
    return s;
}
