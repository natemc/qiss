#include <qasm.h>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <exception.h>
#include <iaddr.h>
#include <limits>
#include <lcutil.h>
#include <kv.h>
#include <match.h>
#include <o.h>
#include <opcode.h>
#include <prim_parse.h>
#include <prim.h>
#include <primio.h>
#include <objectio.h>
#include <split.h>
#include <sym.h>

// TODO Add support for a static section and the pushs instructions.

namespace {
    using Instruction = L<L<C>>;
  
    Exception dup_label(S label, J line) {
        L<C> s;
        s << "Duplicate label " << label << " found on line " << line;
        return lc2ex(s);
    }

    Exception undefined_label(S label, J line) {
        L<C> s;
        s << "Undefined label " << label << " found on line " << line;
        return lc2ex(s);
    }

    bool isspace (C x) { return std::isspace(C::rep(x)); }
    bool notspace(C x) { return !std::isspace(C::rep(x)); }
    bool isdigit (C x) { return std::isdigit(C::rep(x)); }

    J::rep from_digits(const C* first, const C* last) {
        J::rep r = 0;
        for (; first != last; ++first)
            r = r * 10 + C::rep(*first) - '0';
        return r;
    }

    // opcode ((ws+ operand) (ws* , ws* operand)*)? ws*
    Instruction cut_instruction(const C* first, const C* last) {
        assert(first != last && !isspace(*first));

        Instruction r;
        r.reserve(std::count(first, last, C(',')) + 2);
        auto i = std::find_if(first, last, isspace);
        r.emplace_back(first, i);
        i = std::find_if(i, last, notspace);
        while (i != last) { // found operand(s)
            const auto j = std::find(i + 1, last, C(','));
            r.emplace_back(rtrim(L<C>(i, j)));
            i = std::find_if(j + (j != last), last, notspace);
        }
        return r;
    }

    template <class T>
    void write_bytes(L<X>& code, T x) {
        X buf[sizeof x];
        memcpy(buf, &x, sizeof buf);
        code.append(buf, buf + sizeof buf);
    }

    template <class T>
    void write_bytes(L<X>& code, T x, size_t pos) {
        X buf[sizeof x];
        memcpy(buf, &x, sizeof buf);
        std::copy(buf, buf + sizeof buf, code.begin() + pos);
    }

    template <class X>
    auto find(const L<X>& x, const X& y) {
        return std::find_if(x.begin(), x.end(),
                            [&](const X& p){ return match_(p, y); });
    }

    template <class X> L<C> stringify(X x) {
        L<C> s;
        s << x;
        return s;
    }

    bool is_jump(const L<C>& op) {
        assert(op.size());
        const char   c1 = C::rep(op[0]);
        const char   c2 = op.size() == 1? '\0' : C::rep(op[1]);
        return  c1 == 'z' || c1 == 'g' // these 2 are old-fashioned
            || (c1 == 'b' && c2 == 'z')
            || (c1 == 'c' && c2 == 'a')
            || (c1 == 'j' && c2 == 'u');
    }

    std::pair<L<Instruction>, L<J>> phase1(const L<C>& in) {
        // instructions and line_numbers are parallel
        L<Instruction> instructions;
        L<J>           line_numbers;
        KV<S,J>        labels;
        L<S>           unattached_labels;
        L<L<C>>        lines = split(C('\n'), in);
        for (J::rep line_number = 1; line_number <= lines.size(); ++line_number) {
            const C* const first = lines[line_number - 1].begin();
            const C* const last  = lines[line_number - 1].end  ();
            if (first == last) continue;
            const C* i = first;
            if (!isspace(*first) && *first != C(';')) { // line starts w/a label
                i = std::find_if(first, last, [](C x){
                        return isspace(x) || x == C(';');});
                S label = sym(first, i);
                if (labels.has(label) ||
                    find(unattached_labels, label) != unattached_labels.end())
                    throw dup_label(label, J(line_number));
                unattached_labels.push_back(label);
            }

            // The reason for the weirdness is to support multiple
            // labels for the same instruction.  Why did I bother?
            i = std::find_if(i, last, notspace);
            if (i != last && *i != C(';')) {
                for (S lab: unattached_labels)
                    labels.add(lab, J(instructions.size()));
                unattached_labels.clear();
                const C* const j = std::find(i + 1, last, C(';'));
                instructions.push_back(cut_instruction(i, j));
                line_numbers.push_back(J(line_number));
            }
        }

        // replace jump target labels w/instruction indexes
        for (index_t j = 0; j < instructions.size(); ++j) {
            Instruction& i = instructions[j];
            if (is_jump(i[0])) {
                assert(2 <= i.size());
                const L<C>& label(i[1]);
                S symbol = sym(label.begin(), label.end());
                if (!labels.has(symbol))
                    throw undefined_label(symbol, line_numbers[j]);
                i[1] = stringify(labels[symbol]);
            }
        }
        return {instructions, line_numbers};
    }

    void write_op(L<X>& code, Opcode op) {
        code.push_back(X(static_cast<X::rep>(op)));
    }

    Exception bad_operand_count(J                  line,
                                const Instruction& instruction,
                                int                num_operands=1)
    {
        assert(0 < instruction.size());
        L<C> s;
        const L<C> op = instruction[0];
        s << "Line " << line << ": "
          << op << " takes " << num_operands << " operand(s) but got ";
        if (instruction.size() == 1)
            s << "none";
        else {
            s << instruction[1];
            for (index_t i = 2; i < instruction.size(); ++i)
                s << ", " << instruction[i];
        }
        return lc2ex(s);
    }

    Exception bad_literal(J                  line,
                          const Instruction& instruction,
                          const char*        type,
                          const char*        form)
    {
        assert(instruction.size() == 2);
        L<C> s;
        const L<C>& op  = instruction[0];
        const L<C>& arg = instruction[1];
        s << "Line " << line << ": "
          << op << " takes a(n) " << type << " operand"
          << " of the form " << form << " but got " << arg;
        return lc2ex(s);
    }

    Exception bad_boolean_literal(J line, const Instruction& instruction) {
        return bad_literal(line, instruction, "boolean", "0|0b|1|1b");
    }

    Exception bad_boolean_list_literal(J line, const Instruction& instruction) {
        return bad_literal(line, instruction, "boolean list", "(0|1)+b?");
    }

    Exception bad_date_literal(J line, const Instruction& instruction) {
        return bad_literal(line, instruction, "date", "YYYY.MM.DD");
    }

    Exception bad_float_literal(J line, const Instruction& instruction) {
        return bad_literal(line, instruction, "float", "-?[0-9]+(\\.[0-9]*)?");
    }

    Exception bad_int_literal(J line, const Instruction& instruction) {
        return bad_literal(line, instruction, "integer", "-?[0-9]+");
    }

    Exception bad_long_literal(J line, const Instruction& instruction) {
        return bad_literal(line, instruction, "long integer", "-?[0-9]+");
    }

    Exception bad_clean_operand(J line, const Instruction& instruction) {
        return bad_literal(line, instruction, "integer", "0-99");
    }

    Exception bad_invoke_operand(J line, const Instruction& instruction) {
        return bad_literal(line, instruction, "integer", "0-99");
    }

    Exception list_operand_too_long(J                  line,
                                    const Instruction& instruction,
                                    int                operand=1)
    {
        assert(operand < instruction.size());
        L<C> s;
        const L<C>& op  = instruction[0];
        const L<C>& arg = instruction[operand];
        s << "Line " << line << ": "
          << op << " immediate list operand too long; max length is "
          << std::numeric_limits<immlen_t>::max() << " but got " << arg;
        return lc2ex(s);
    }

    Exception unknown_opcode(J line, const Instruction& instruction) {
        L<C> s;
        const L<C>& op = instruction[0];
        s << "qasm (line " << line << "): nyi opcode " << op;
        return lc2ex(s);
    }

    void arg_(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() != 2) throw bad_operand_count(line, inst);
        const L<C>& arg = inst[1];
        auto [ok, n] = parse_int(arg);
        if (!ok) throw Exception("bad arg (parameter) arg");
        if (n < I(0) || I(11) < n) throw Exception("arg arg out of range 0..11");
        write_op(code, Opcode::local);
        write_bytes(code, uint8_t(11 - I::rep(n)));
    }

    template <class P>
    void atom_literal(L<X>&              code,
                      J                  line,
                      const Instruction& inst,
                      P                  parse,
                      Exception        (*bad)(J,const Instruction&),
                      Opcode             opcode)
    {
        if (inst.size() != 2) throw bad_operand_count(line, inst);
        const L<C>& arg = inst[1];
        const auto [ok, x] = parse(arg);
        if (!ok) throw bad(line, inst);
        write_op(code, opcode);
        write_bytes(code, x);
    }

    void bindl(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() != 2) throw bad_operand_count(line, inst);
        const L<C>& arg = inst[1];
        auto [ok, n] = parse_int(arg);
        if (!ok) throw Exception("bad bindl arg");
        if (n < I(0) || I(51) < n) throw Exception("bindl arg out of range 0..51");
        write_op(code, Opcode::bindl);
        write_bytes(code, uint8_t(I::rep(n)));
    }

    void boolean_atom_literal(L<X>& code, J line, const Instruction& inst) {
        atom_literal(code, line, inst, parse_bool, bad_boolean_literal, Opcode::immb);
    }

    void boolean_list_literal(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() != 2) throw bad_operand_count(line, inst);
        const L<C>& arg = inst[1];
        if (std::numeric_limits<immlen_t>::max() < arg.size())
            throw list_operand_too_long(line, inst);
        const immlen_t n(immlen_t(arg.size() - (arg.back() == C('b'))));
        write_op(code, Opcode::immbv);
        write_bytes(code, n);
        for (index_t i = 0; i < n; ++i) {
            if (arg[i] != C('0') && arg[i] != C('1'))
                throw bad_boolean_list_literal(line, inst);
            code.push_back(X(X::rep(C::rep(arg[i]) - '0')));
        }
    }

    void bump(L<X>& code, J line, const Instruction& inst) {
        if (inst.size()!= 2) throw bad_operand_count(line, inst);
        const L<C>& arg = inst[1];
        auto [ok, n] = parse_int(arg);
        if (!ok) throw Exception("bad bump arg");
        if (n < I(0) || I(255) < n) throw Exception("bump arg out of range");
        write_op(code, Opcode::bump);
        write_bytes(code, uint8_t(I::rep(n)));
    }

    void call(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() != 2) throw bad_operand_count(line, inst, 2);
        write_op(code, Opcode::call);
        write_bytes(code, iaddr_t(0)); // leave space for target addr
    }

    void clean(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() != 2) throw bad_operand_count(line, inst);
        const L<C>& arg = inst[1];
        if (2 < arg.size() || !all_digits(arg.begin(), arg.end()))
            throw bad_clean_operand(line, inst);
        const J::rep argc = from_digits(arg.begin(), arg.end());
        write_op(code, Opcode::clean);
        code.push_back(X(X::rep(argc)));
    }

    void dup(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() == 1) { write_op(code, Opcode::dup); return; }
        if (inst.size() != 2) throw bad_operand_count(line, inst);
        const L<C>& arg = inst[1];
        auto [ok, n] = parse_int(arg);
        if (!ok) throw Exception("bad dup arg");
        if (n < I(0) || I(255) < n) throw Exception("dup arg out of range");
        write_op(code, Opcode::dupnth);
        write_bytes(code, uint8_t(I::rep(n)));
    }

    void date_atom_literal(L<X>& code, J line, const Instruction& inst) {
        atom_literal(code, line, inst, parse_date, bad_date_literal, Opcode::immd);
    }

    void float_atom_literal(L<X>& code, J line, const Instruction& inst) {
        atom_literal(code, line, inst, parse_float, bad_float_literal, Opcode::immf);
    }

    template <class P, class E>
    void num_list_literal(L<X>& code, J line, const Instruction& inst,
                          Opcode op, P parser, E error)
    {
        if (inst.size() != 2) throw bad_operand_count(line, inst);
        const L<C>& arg = inst[1];
        const C*    p   = arg.begin();
        using R = decltype(parser(p, p).second);
        L<R> xs;
        while (p != arg.end()) {
            const C* const q = std::find_if(p, arg.end(), isspace);
            const auto [ok, x] = parser(p, q);
            if (!ok) throw error(line, inst);
            xs.push_back(x);
            p = std::find_if(q, arg.end(), notspace);
        }
        if (std::numeric_limits<immlen_t>::max() < xs.size())
            throw list_operand_too_long(line, inst);
        write_op(code, op);
        write_bytes(code, immlen_t(xs.size()));
        for (auto x: xs) write_bytes(code, x);
    }

    void float_list_literal(L<X>& code, J line, const Instruction& inst) {
        num_list_literal(code, line, inst, Opcode::immfv, parse_float, bad_float_literal);
    }

    void halt(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() != 1) throw bad_operand_count(line, inst, 0);
        write_op(code, Opcode::halt);
    }

    void invoke(L<X>& code, J line, const Instruction& inst) {
        if (inst.size()!= 2) throw bad_operand_count(line, inst);
        const L<C>& arg = inst[1];
        if (2 < arg.size() || !all_digits(arg.begin(), arg.end()))
            throw bad_invoke_operand(line, inst);
        const J::rep argc = from_digits(arg.begin(), arg.end());
        write_op(code, Opcode::invoke);
        code.push_back(X(X::rep(argc)));
    }

    void int_atom_literal(L<X>& code, J line, const Instruction& inst) {
        atom_literal(code, line, inst, parse_int, bad_int_literal, Opcode::immi);
    }

    void int_list_literal(L<X>& code, J line, const Instruction& inst) {
        num_list_literal(code, line, inst, Opcode::immiv, parse_int, bad_int_literal);
    }

    void local(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() != 2 && inst.size() != 3)
            throw bad_operand_count(line, inst, 2);
        uint8_t frame = 0; // TODO yuck.  rewrite
        int8_t slot = 0;
        if (inst.size() == 2) {
            const auto [ok, s] = parse_int(inst[1]);
            if (!ok) throw Exception("bad local slot arg");
            if (s < I(-12) || I(52) <= s)
                throw Exception("local slot arg out of range -12..52");
            slot = int8_t(I::rep(s));
        }
        else {
            auto [okf, f] = parse_int(inst[1]);
            if (!okf) throw Exception("bad local frame arg");
            if (f < I(0) || I(4) <= f)
                throw Exception("local frame arg out of range 0..3");
            auto [ok, s] = parse_int(inst[2]);
            if (!ok) throw Exception("bad local slot arg");
            if (s < I(-12) || I(52) <= s)
                throw Exception("local slot arg out of range -12..51");
            frame = uint8_t(I::rep(f));
            slot = int8_t(I::rep(s));
        }
        write_op(code, Opcode::local);
        write_bytes(code, uint8_t((frame << 6) | ((slot + 12) & 63)));
    }

    void long_atom_literal(L<X>& code, J line, const Instruction& inst) {
        atom_literal(code, line, inst, parse_long, bad_long_literal, Opcode::immj);
    }

    void long_list_literal(L<X>& code, J line, const Instruction& inst) {
        num_list_literal(code, line, inst, Opcode::immjv, parse_long, bad_long_literal);
    }

    void nop(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() != 1) throw bad_operand_count(line, inst, 0);
        write_op(code, Opcode::nop);
    }

    void op_literal(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() != 2) throw bad_operand_count(line, inst);
        const L<C>& arg = inst[1];
        write_op(code, Opcode::op);
        code.push_back(X(X::rep(C::rep(arg[0]))));
    }

    void pop(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() != 1) throw bad_operand_count(line, inst, 0);
        write_op(code, Opcode::pop);
    }

    void ret(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() != 1) throw bad_operand_count(line, inst, 0);
        write_op(code, Opcode::ret);
    }

    void swap(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() != 1) throw bad_operand_count(line, inst, 0);
        write_op(code, Opcode::swap);
    }

    void bzero(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() != 2) throw bad_operand_count(line, inst);
        write_op(code, Opcode::bzero);
        write_bytes(code, iaddr_t(0)); // hole for target address
    }

    void jump(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() != 2) throw bad_operand_count(line, inst);
        write_op(code, Opcode::jump);
        write_bytes(code, iaddr_t(0)); // hole for target address
    }

    void rot(L<X>& code, J line, const Instruction& inst) {
        if (inst.size() > 2) throw bad_operand_count(line, inst);
        write_op(code, Opcode::rot);
        if (inst.size() == 1)
            write_bytes(code, uint8_t(2));
        else {
            const L<C>& arg = inst[1];
            auto [ok, n] = parse_int(arg);
            if (!ok) throw Exception("bad rot arg");
            if (n < I(0) || I(255) < n) throw Exception("rot arg out of range");
            write_bytes(code, uint8_t(I::rep(n)));
        }
    }

    void string_literal(L<X>& code, J line, const Instruction& inst, Opcode op) {
        if (inst.size() != 2) throw bad_operand_count(line, inst);
        const L<C>& arg = inst[1];
        if (std::numeric_limits<immlen_t>::max() < arg.size())
            throw list_operand_too_long(line, inst);
        write_op(code, op);
        write_bytes(code, immlen_t(arg.size()));
        std::transform(arg.begin(), arg.end(), std::back_inserter(code),
                       [](C c){ return X(X::rep(C::rep(c))); });
    }

    void sym_atom_literal(L<X>& code, J line, const Instruction& inst) {
        string_literal(code, line, inst, Opcode::immsym);
    }

    void char_list_literal(L<X>& code, J line, const Instruction& inst) {
        string_literal(code, line, inst, Opcode::immstr);
    }

    L<X> draft_code(const L<Instruction>& instructions, const L<J>& lines) {
        L<X> code;
        L<I> addresses;
        for (index_t i = 0; i < instructions.size(); ++i) {
            assert(code.size() < std::numeric_limits<I::rep>::max());
            addresses.push_back(I(I::rep(code.size())));
            const Instruction& inst = instructions[i];
            const L<C>&        op   = inst[0];
            switch (C::rep(op[0])) {
            case 'a': arg_(code, lines[i], inst); break;
            case 'b':
                if      (op == "bindl") bindl               (code, lines[i], inst);
                else if (op == "bump" )  bump                (code, lines[i], inst);
                else                     boolean_atom_literal(code, lines[i], inst);
                break;
            case 'B': boolean_list_literal(code, lines[i], inst); break;
            case 'c':
                if      (op == "clean") clean(code, lines[i], inst);
                else if (op == "call" ) call (code, lines[i], inst);
                else                    throw unknown_opcode(lines[i], inst);
                break;
            case 'd':
                if   (op == "dup") dup              (code, lines[i], inst);
                else               date_atom_literal(code, lines[i], inst);
                break;
            case 'f': float_atom_literal(code, lines[i], inst); break;
            case 'F': float_list_literal(code, lines[i], inst); break;
            case 'h': halt              (code, lines[i], inst); break;
            case 'i': (op.size() == 1? int_atom_literal : invoke)(code, lines[i], inst); break;
            case 'I': int_list_literal  (code, lines[i], inst); break;
            case 'j':
                if   (op == "jump") jump             (code, lines[i], inst);
                else                long_atom_literal(code, lines[i], inst);
                break;
            case 'J': long_list_literal (code, lines[i], inst); break;
            case 'l': local             (code, lines[i], inst); break;
            case 'n': nop               (code, lines[i], inst); break;
            case 'o': op_literal        (code, lines[i], inst); break;
            case 'p': pop               (code, lines[i], inst); break;
            case 'r':
                if (op == "rot") rot(code, lines[i], inst);
                else             ret(code, lines[i], inst);
                break;
            case 's': swap              (code, lines[i], inst); break;
            case 'z': bzero             (code, lines[i], inst); break;
            case '`': sym_atom_literal  (code, lines[i], inst); break;
            case '"': char_list_literal (code, lines[i], inst); break;
            case '\'': // each, each-prior
                if (inst.size() != 1) throw bad_operand_count(lines[i], inst);
                if      (op.size() == 1)                    write_op(code, Opcode::each);
                else if (op.size() == 2 || op[1] == C(':')) write_op(code, Opcode::prior);
                else                                        throw unknown_opcode(lines[i], inst);
                break;
            case '/': // over, each-right
                if (inst.size() != 1) throw bad_operand_count(lines[i], inst);
                if      (op.size() == 1)                    write_op(code, Opcode::over);
                else if (op.size() == 2 || op[1] == C(':')) write_op(code, Opcode::right);
                else                                        throw unknown_opcode(lines[i], inst);
                break;
            case '\\': // scan, each-left
                if (inst.size() != 1) throw bad_operand_count(lines[i], inst);
                if      (op.size() == 1)                    write_op(code, Opcode::scan);
                else if (op.size() == 2 || op[1] == C(':')) write_op(code, Opcode::left);
                else                                        throw unknown_opcode(lines[i], inst);
                break;
#define SIMPLE_OP(ch, unary, binary) case ch:                                     \
    if (inst.size() != 1) throw bad_operand_count(lines[i], inst, 0);             \
    if      (op.size()==1)                  write_op(code, Opcode::binary);       \
    else if (op.size()==2 && op[1]==C(':')) write_op(code, Opcode::unary );       \
    else                                    throw unknown_opcode(lines[i], inst); \
    break;
#define THREE_OP(ch, unary, binary, ternary) case ch:                             \
    if (inst.size() != 1) throw bad_operand_count(lines[i], inst, 0);             \
    if      (op.size()==1)                  write_op(code, Opcode::binary );      \
    else if (op.size()==2 && op[1]==C(':')) write_op(code, Opcode::unary  );      \
    else if (op.size()==2 && op[1]==C('[')) write_op(code, Opcode::ternary);      \
    else                                    throw unknown_opcode(lines[i], inst); \
    break;
            SIMPLE_OP('~', not_    , match);
            SIMPLE_OP('!', key     , bang);
            SIMPLE_OP('@', type    , at);
            SIMPLE_OP('#', count   , take);
            SIMPLE_OP('$', string  , cast);
            SIMPLE_OP('%', recip   , fdiv);
            SIMPLE_OP('^', null    , fill);
            SIMPLE_OP('&', where   , min);
            SIMPLE_OP('*', first   , mul);
            SIMPLE_OP('-', neg     , sub);
            SIMPLE_OP('_', floor   , cut);
            SIMPLE_OP('+', flip    , add);
            SIMPLE_OP('=', group   , eq);
            SIMPLE_OP('|', rev     , max);
            SIMPLE_OP(',', enlist  , cat);
            SIMPLE_OP('<', iasc    , less);
            SIMPLE_OP('>', idesc   , great);
            THREE_OP ('?', distinct, find, vcond);
            default: throw unknown_opcode(lines[i], inst);
            }
        }

        if (code.back() != X('h') && code.back() != X('r'))
            code.push_back(X('h'));
    
        // fixup jumps from instruction indexes to addresses
        for (index_t i = 0; i < instructions.size(); ++i) {
            const Instruction& inst = instructions[i];
            if (is_jump(inst[0])) {
                assert(1 < inst.size());
                const L<C>& arg = inst[1];
                const auto [ok, target] = parse_long(arg);
                assert(ok);
                write_bytes(code, addresses[target], std::size_t(I::rep(addresses[i]) + 1));
            }
        }

        return code;
    }
} // unnamed

L<X> parse_qasm(const L<C>& in) {
    auto [inst, lines] = phase1(in);
    return draft_code(inst, lines);
}
