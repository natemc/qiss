#include <vm.h>
#include <algorithm>
#include <arith.h>
#include <at.h>
#include <bang.h>
#include <cat.h>
#include <converge.h>
#include <count.h>
#include <cstdlib>
#include <disassemble.h>
#include <distinct.h>
#include <dollar.h>
#include <dot.h>
#include <each.h>
#include <enlist.h>
#include <exception.h>
#include <fill.h>
#include <find.h>
#include <first.h>
#include <flip.h>
#include <floor.h>
#include <format.h>
#include <group.h>
#include <iaddr.h>
#include <iasc.h>
#include <in.h>
#include <idesc.h>
#include <iterator>
#include <key.h>
#include <match.h>
#include <merge_dicts.h>
#include <not.h>
#include <null.h>
#include <o.h>
#include <objectio.h>
#include <opcode.h>
#include <over.h>
#include <primio.h>
#include <prior.h>
#include <relop.h>
#include <reverse.h>
#include <scan.h>
#include <string_.h>
#include <sym.h>
#include <take.h>
#include <terminal_width.h>
#include <type_pair.h>
#include <ukv.h>
#include <uminus.h>
#include <under.h>
#include <uniform.h>
#include <utility>
#include <value.h>
#include <vcond.h>
#include <where.h>

namespace {
    template <class X> using OT  = ObjectTraits<X>;

    L<C>& print_stack_item(L<C>& os, O x);

    L<C>& print_stack_item(L<C>& os, UKV x) {
        auto [k, v] = std::move(x).kv();
        print_stack_item(os, std::move(k));
        return print_stack_item(os << '!', std::move(v));
    }

    L<C>& print_stack_item(L<C>& os, S x) {
        return os << '`' << x;
    }

    L<C>& print_stack_item(L<C>& os, L<S> x) {
        for (S s: x) os << '`' << s;
        return os;
    }

    L<C>& print_stack(L<C>& os, L<O> s) {
        os << '[';
        if (s.size()) {
            print_stack_item(os, s.back());
            std::for_each(s.rbegin() + 1, s.rend(), [&](O x) {
                print_stack_item(os << ';',  std::move(x));
            });
        }
        return os << ']';
    }

    L<C>& print_stack_item(L<C>& os, O x) {
        return x.is_dict ()? print_stack_item(os, UKV(std::move(x)))
            :  x.is_table()? print_stack_item(os << '+', UKV(addref(x->dict)))
            :  x.type() == OT<S>::typet()? print_stack_item(os, L<S>(std::move(x)))
            :  x.type() == -OT<S>::typet()? print_stack_item(os, x.atom<S>())
            :  /* else */   os << x;
    }

    const char lcyan  [] = "\033[30;48;5;159m";
    const char lblue  [] = "\033[30;48;5;195m";
    const char lgreen [] = "\033[30;48;5;193m";
    const char myellow[] = "\033[30;48;5;229m";
    const char pink   [] = "\033[30;48;5;213m";
    const char lpink  [] = "\033[30;48;5;225m";
    const char reset  [] = "\033[0m";
    const char* stripes[][2] = {{lcyan, lblue}, {lgreen, myellow}, {pink, lpink}};

    void do_trace(H h, const X* ip, const L<O>& vs, const char*(&strips)[2]) {
        static int strip = 0;
        const int tw = terminal_width();
        L<C> o;
        disassemble_instruction(o, ip);
        print_stack(o << "  ", vs);
        o.trunc(tw);
        h << strips[strip++ % 2] << left(tw, o) << reset << '\n' << flush;
    }

    const X* clean(L<O>& stack, const X* ip) {
        uint8_t n;
        memcpy(&n, ip + 1, sizeof n);
        O top(stack.pop());
        for (uint8_t i = 0; i < n; ++i) stack.pop_back();
        stack.emplace_back(std::move(top));
        return ip + sizeof n;
    }

    template <class Z>
    const X* push_atom_literal(L<O>& stack, const X* ip) {
        Z x;
        memcpy(static_cast<void*>(&x), ip + 1, sizeof x);
        stack.emplace_back(Z(x));
        return ip + sizeof x;
    }

    template <>
    const X* push_atom_literal<S>(L<O>& stack, const X* ip) {
        immlen_t n;
        memcpy(&n, ip + 1, sizeof n);
        L<C> lst(n);
        memcpy(static_cast<void*>(lst.begin()), ip + 1 + sizeof n, n * sizeof(C));
        stack.emplace_back(sym(lst.begin(), lst.end()));
        return ip + sizeof n + n * sizeof(C);
    }

    const X* push_constant(const L<O>& constants, L<O>& stack, const X* ip) {
        sindex_t i;
        memcpy(&i, ip + 1, sizeof i);
        assert(i < constants.size());
        stack.emplace_back(constants[i]);
        return ip + sizeof i;
    }

    template <class Z>
    const X* push_list_literal(L<O>& stack, const X* ip) {
        immlen_t n;
        memcpy(&n, ip + 1, sizeof n);
        L<Z> lst(n);
        memcpy(static_cast<void*>(lst.begin()), ip + 1 + sizeof n, n * sizeof(Z));
        stack.emplace_back(std::move(lst));
        return ip + sizeof n + n * sizeof(Z);
    }

    const X* push_static(const L<O>& statics, L<O>& stack, const X* ip) {
        sindex_t i;
        memcpy(&i, ip + 1, sizeof i);
        assert(i < statics.size());
        stack.emplace_back(statics[i]);
        return ip + sizeof i;
    }

    const X* arg(I frame, L<O>& stack, const X* ip) {
        uint8_t n;
        memcpy(&n, ip + 1, sizeof n);
        stack.emplace_back(stack[I::rep(frame) - n - 1]);
        return ip + sizeof n;
    }

    const X* bindl(I frame, L<O>& stack, const X* ip) {
        uint8_t n;
        memcpy(&n, ip + 1, sizeof n);
        stack[I::rep(frame) + n] = stack.back();
        return ip + sizeof n;
    }

    const X* bindm(L<O>& statics, L<O>& stack, const X* ip) {
        sindex_t i;
        memcpy(&i, ip + 1, sizeof i);
        assert(i < statics.size());
        statics[i] = stack.back();
        return ip + sizeof i;
    }

    const X* bump(L<O>& stack, const X* ip) {
        uint8_t n;
        memcpy(&n, ip + 1, sizeof n);
        stack.reserve(stack.size() + n);
        for (uint8_t i = 0; i < n; ++i) stack.emplace_back(generic_null());
        return ip + sizeof n;
    }

    const X* dupnth(L<O>& stack, const X* ip) {
        uint8_t n;
        memcpy(&n, ip + 1, sizeof n);
        stack.emplace_back(stack[stack.size() - n - 1]);
        return ip + sizeof n;
    }

    const X* local(const L<I>& frames, L<O>& stack, const X* ip) {
        uint8_t n;
        memcpy(&n, ip + 1, sizeof n);
        const int     frame = n >> 6;
        const int     slot  = (n & 63) - 12;
        const index_t f     = frames.size() - frame - 1;
        assert(0 <= f && f < frames.size());
        stack.emplace_back(stack[I::rep(frames[f]) + slot]);
        return ip + sizeof n;
    }

    const X* rot(L<O>& stack, const X* ip) {
        uint8_t n;
        memcpy(&n, ip + 1, sizeof n);
        if (n != stack.size())
            std::rotate(stack.end() - n - 1, stack.end() - n, stack.end());
        return ip + sizeof n;
    }

    const struct Unop {
        template <class F> void operator()(L<O>& stack, F&& f) const {
            O x(stack.pop());
            stack.emplace_back(std::forward<F>(f)(std::move(x)));
        }
    } unop;

    const struct Binop {
        template <class F> void operator()(L<O>& stack, F&& f) const {
            O x(stack.pop());
            O y(stack.pop());
            stack.emplace_back(std::forward<F>(f)(std::move(x), std::move(y)));
        }
    } binop;

    const struct Ternop {
        template <class F> void operator()(L<O>& stack, F&& f) const {
            O x(stack.pop());
            O y(stack.pop());
            O z(stack.pop());
            stack.emplace_back(
                std::forward<F>(f)(std::move(x), std::move(y), std::move(z)));
        }
    } ternop;

    O type_(O x) { return O(I(int(x.type()))); }

    void apply_adverb(L<O>& vs, Opcode adverb) {
        O& o = vs.back();
        switch (int(o.type())) {
        case -OT<Opcode>::typei():
            o->type = -OT<AO>::typet();
            o->ao   = AO({o->op, adverb});
            break;
        case -OT<Proc>::typei():
            o->type = -OT<AP>::typet();
            o->apadv = adverb;
            break;
        default:
            H(1) << o << '\t' << o->type << '\n' << flush;
            throw Exception("nyi: adverb other than op");
        }
    }

    bool all_(const L<B>& x) {
        return std::all_of(x.begin(), x.end(), [](B b){ return bool(b); });
    }

    O recip(O x) {
        return fdiv(O(J(1)), std::move(x));
    }

    const struct AmbivalentOp {
        Opcode op;
        ufun_t f1;
        bfun_t f2;
        tfun_t f3;
    } ops[] = {
        {Opcode::add  , flip    , add    },
        {Opcode::at   , type_   , at     },
        {Opcode::bang , key     , bang   },
        {Opcode::cast , string_ , dollar },
        {Opcode::cat  , enlist  , cat    },
        {Opcode::cut  , floor_  , under  },
        {Opcode::dot  , value   , dot    },
        {Opcode::eq   , group   , eq     },
        {Opcode::fdiv , recip   , fdiv   },
        {Opcode::fill , null    , fill   },
        {Opcode::find , distinct, find   , vcond},
        {Opcode::great, idesc   , greater},
        {Opcode::less , iasc    , less   },
        {Opcode::match, not_    , match  },
        {Opcode::max  , reverse , max    },
        {Opcode::min  , where   , min    },
        {Opcode::mul  , first   , mul    },
        {Opcode::sub  , uminus  , sub    },
        {Opcode::take , count   , take   },
    };

    const struct AmbivalentAdverb {
        Opcode adverb;
        O(*f1)(ufun_t, O);
        O(*f21)(bfun_t, O);
        O(*f22)(bfun_t, O, O);
    } adverbs[] = {
        {Opcode::each , each1    , nullptr, each2     },
        {Opcode::left , nullptr  , nullptr, each_left },
        {Opcode::right, nullptr  , nullptr, each_right},
        {Opcode::over , converge1, over1  , over2     },
        {Opcode::prior, nullptr  , prior1 , prior2    },
        {Opcode::scan , nullptr  , scan1  , scan2     },
    };

    /*
    template <class FUN>
    void variadic_each(L<O>& stack, FUN f, int argc) {
        L<O> args;
        for (int i = 0; i < argc; ++i) args.push_back(stack.pop());
        index_t len = -1;
        for (O& x: args) {
            if (!x.is_atom()) {
                len = std::max(len, x->n);
                if (len != x->n) throw Exception("mismatched lengths to each");
            }
        }
        if (len < 0) len = 1;
        const auto ith = [&](index_t a, index_t i) {
            return args[a].is_atom()? args[a] : O(args[a][i]);
        };
        for (index_t i = 0; i < len; ++i) {
        }
    }
    */

    void variadic_enlist(L<O>& stack, int argc) {
        if (are_uniform_atoms(stack.end() - argc, stack.end())) {
            O r(uniform(std::reverse_iterator(stack.end()),
                        std::reverse_iterator(stack.end() - argc)));
            for (int i = 0; i < argc; ++i) stack.pop_back();
            stack.emplace_back(r);
        } else {
            L<O> r;
            r.reserve(argc);
            for (int i = 0; i < argc; ++i) r.emplace_back(stack.pop());
            stack.emplace_back(r);
        }
    }

    const X* invoke(L<O>& vs, const X* ip) {
        const X::rep argc(*++ip);
        if (vs.back().is_list() || vs.back().is_dict() || vs.back().is_table()) {
            if      (X::rep(1) == argc)
                binop(vs, at);
            else if (X::rep(1) <  argc) {
                O target(vs.pop());
                L<O> args;
                args.reserve(argc);
                for (X::rep a = 0; a < argc; ++a) args.emplace_back(vs.pop());
                vs.emplace_back(std::move(args));
                vs.emplace_back(std::move(target));
                binop(vs, dot);
            }
            return ip;
        }

        const O target(vs.pop());
        switch (int(target.type())) {
        case -OT<bfun_t>::typei():
            binop(vs, target.atom<bfun_t>());
            break;
        case -OT<Opcode>::typei(): {
            const Opcode op = target.atom<Opcode>();
            if (op == Opcode::enlist && 1 < argc)
                variadic_enlist(vs, argc);
            else {
                const auto oi = std::find_if(ops, std::end(ops),
                    [=](AmbivalentOp x){ return x.op == op; });
                assert(oi != std::end(ops));
                if      (argc == 1)           unop (vs, oi->f1);
                else if (argc == 2)           binop(vs, oi->f2);
                else if (argc == 3 && oi->f3) ternop(vs, oi->f3);
                else {
                    L<C> s;
                    s << "nyi: invoke " << op << " w/ " << argc << " args";
                    throw lc2ex(s);
                }
            }
            break;
        }
        case -OT<AO>::typei(): {
            const AO ao = target.atom<AO>();
            const auto oi = std::find_if(ops, std::end(ops),
                [=](AmbivalentOp x){ return x.op == ao.op; });
            assert(oi != std::end(ops));
            const auto ai = std::find_if(adverbs, std::end(adverbs),
                [=](AmbivalentAdverb x){ return x.adverb == ao.adverb; });
            assert(ai != std::end(adverbs));
            if (argc == 1) {
                if (ai->f21)
                    unop(vs, [&](O x){return ai->f21(oi->f2, std::move(x));});
                else {
                    assert(ai->f1);
                    unop(vs, [&](O x){return ai->f1(oi->f1, std::move(x));});
                }
            } else if (argc == 2) {
                binop(vs, [&](O x, O y){return ai->f22(oi->f2, std::move(x), std::move(y));});
            } else {
                L<C> s;
                s << "nyi: invoke adverbed op " << ao << " with " << argc << " params";
                throw lc2ex(s);
            }
            break;
        }
        default:
            throw Exception("nyi: invoke");
        }
        return ip;
    }

    bool is_truthy(O x) {
        if (!x.is_atom()) return internal_count(x);
#define CS(X) case -OT<X>::typei(): return bool(x.atom<X>())
        switch (int(x.type())) {
        CS(B); CS(C); CS(I); CS(J); CS(X);
        default: throw Exception("type (truthy)");
        }
    }

    void do_swap(L<O>& stack) {
        if (stack.size() < 2) throw Exception("swap: stack underflow");
        std::iter_swap(&stack.back(), &stack.back() - 1);
    }

    struct Vm {
        Vm(KV<S, KV<S,O>>& env_, bool trace_): env(env_), fp({I(0)}), stripe(0), trace(trace_) {}

        O operator()(S module_name, index_t start);

        KV<S, KV<S,O>>& env;
        L<S>            ms;         // module stack
        L<I>            cs;         // call stack
        L<O>            vs;         // value stack
        L<I>            fp;         // frame pointers
        S               current_module;
        KV<S,O>         module;
        L<O>            constants;
        L<O>            statics;
        L<X>            code;
        int             stripe;
        bool            trace;
    };

    O Vm::operator()(S module_name, index_t start) {
        S        current_module(module_name);
        KV<S,O>  module        (env[current_module]);
        L<O>     constants     (module["constants"_s]);
        L<O>     statics       (module["statics"_s]);
        L<X>     code          (module["code"_s]);
        assert(start <= code.size());
        if (code.size() == start) return O{};

        const auto change_module = [&](S new_module){
            if (current_module != new_module) {
                current_module = new_module;
                module         = env[current_module];
                code           = L<X>(module["code"_s]);
                constants      = L<O>(module["constants"_s]);
                statics        = L<O>(module["statics"_s]);
            }
        };

        const auto last_frame = fp.size();
        for (const X* ip = code.begin() + start; Opcode(X::rep(*ip)) != Opcode::halt; ++ip) {
            if (trace) do_trace(H(1), ip, vs, stripes[stripe]);
            switch (Opcode(X::rep(*ip))) {
            case Opcode::bzero   : {
                iaddr_t target;
                memcpy(&target, ip + 1, sizeof target);
                ip = is_truthy(vs.pop())? ip + sizeof target : code.begin() + target - 1;
                break;
            }
            case Opcode::call    : {
                iaddr_t target;
                memcpy(&target, ip + 1, sizeof target);
                ip += 1 + sizeof target;
                ms.push_back(current_module);
                cs.push_back(I(I::rep(ip - code.begin())));
                fp.push_back(I(I::rep(vs.size())));
                ip = code.begin() + target - 1;
                stripe = (stripe + 1) % int(std::size(stripes));
                break;
            }
            case Opcode::jump    : {
                iaddr_t target;
                memcpy(&target, ip + 1, sizeof target);
                ip = code.begin() + target - 1;
                break;
            }
            case Opcode::ret     : {
                if (cs.empty()) return vs.empty()? O() : vs.pop();
                change_module(ms.pop());
                const iaddr_t target = iaddr_t(I::rep(cs.pop()));
                ip     = code.begin() + target - 1;
                stripe = (stripe? stripe : int(std::size(stripes))) - 1;
                fp.pop();
                if (fp.size() < last_frame) return vs.empty()? O() : vs.pop();
                break;
            }
            case Opcode::add     : binop(vs, add);                        break;
            case Opcode::at      : binop(vs, at);                         break;
            case Opcode::bang    : binop(vs, bang);                       break;
            case Opcode::bindl   : ip = bindl(fp.back(), vs, ip);         break;
            case Opcode::bindm   : ip = bindm(statics, vs, ip);           break;
            case Opcode::bump    : ip = bump(vs, ip);                     break;
            case Opcode::cast    : binop(vs, dollar);                     break;
            case Opcode::cat     : binop(vs, cat);                        break;
            case Opcode::clean   : ip = clean(vs, ip);                    break;
            case Opcode::count   : unop(vs, count);                       break;
            case Opcode::cut     : binop(vs, under);                      break;
            case Opcode::distinct: unop(vs, distinct);                    break;
            case Opcode::dot     : binop(vs, dot);                        break;
            case Opcode::dup     : vs.emplace_back(vs.back());            break;
            case Opcode::dupnth  : ip = dupnth(vs, ip);                   break;
            case Opcode::each    : apply_adverb(vs, Opcode(X::rep(*ip))); break;
            case Opcode::enlist  : unop(vs, enlist);                      break;
            case Opcode::eq      : binop(vs, eq);                         break;
            // ?? g++ knows about some other fdiv ??
            case Opcode::fdiv    : binop(vs,static_cast<O(*)(O,O)>(fdiv));break;
            case Opcode::fill    : binop(vs, fill);                       break;
            case Opcode::find    : binop(vs, find);                       break;
            case Opcode::first   : unop(vs, first);                       break;
            case Opcode::flip    : unop(vs, flip);                        break;
            case Opcode::floor   : unop(vs, floor_);                      break;
            case Opcode::great   : binop(vs, greater);                    break;
            case Opcode::group   : unop(vs, group);                       break;
            case Opcode::iasc    : unop(vs, iasc);                        break;
            case Opcode::idesc   : unop(vs, idesc);                       break;
            case Opcode::immb    : ip = push_atom_literal<B>(vs, ip);     break;
            case Opcode::immbv   : ip = push_list_literal<B>(vs, ip);     break;
            case Opcode::immd    : ip = push_atom_literal<D>(vs, ip);     break;
            case Opcode::immdv   : ip = push_list_literal<D>(vs, ip);     break;
            case Opcode::immf    : ip = push_atom_literal<F>(vs, ip);     break;
            case Opcode::immfv   : ip = push_list_literal<F>(vs, ip);     break;
            case Opcode::immi    : ip = push_atom_literal<I>(vs, ip);     break;
            case Opcode::immiv   : ip = push_list_literal<I>(vs, ip);     break;
            case Opcode::immj    : ip = push_atom_literal<J>(vs, ip);     break;
            case Opcode::immjv   : ip = push_list_literal<J>(vs, ip);     break;
            case Opcode::immstr  : ip = push_list_literal<C>(vs, ip);     break;
            case Opcode::immsym  : ip = push_atom_literal<S>(vs, ip);     break;
            case Opcode::immt    : ip = push_atom_literal<T>(vs, ip);     break;
            case Opcode::immtv   : ip = push_list_literal<T>(vs, ip);     break;
            case Opcode::immx    : ip = push_atom_literal<X>(vs, ip);     break;
            case Opcode::immxv   : ip = push_list_literal<X>(vs, ip);     break;
            case Opcode::invoke  :
                if (vs.back().type() == -OT<Proc>::typet()) {
                    const X::rep argc(*++ip);
                    O proc(vs.pop());
                    const X::rep arity(proc->arity);
                    if (arity < argc) throw Exception("rank");
                    if (argc < arity) throw Exception("nyi: partial application (invoke)");
                    ms.push_back(current_module);
                    cs.push_back(I(I::rep(ip + 1 - code.begin())));
                    fp.push_back(I(I::rep(vs.size())));
                    change_module(proc->proc.module);
                    ip = code.begin() + proc->proc.entry - 1;
                    stripe = (stripe + 1) % int(std::size(stripes));
                } else if (vs.back().type() != -OT<AP>::typet()) {
                    ip = invoke(vs, ip);
                } else {
                    const X::rep argc(*++ip);
                    assert(argc <= vs.size());
                    const O target = vs.pop();
                    const AP ap = target.atom<AP>();
                    const auto wrap1 = [&](Proc p){
                        return [=,this](O x){
                            vs.push_back(x);
                            ms.push_back(current_module);
                            cs.push_back(I(I::rep(ip - code.begin())));
                            fp.push_back(I(I::rep(vs.size())));
                            stripe = (stripe + 1) % int(std::size(stripes));
                            return (*this)(p.module, p.entry);
                        };
                    };
                    const auto wrap2 = [&](Proc p){
                        return [=,this](O x, O y){
                            vs.push_back(y);
                            vs.push_back(x);
                            ms.push_back(current_module);
                            cs.push_back(I(I::rep(ip - code.begin())));
                            fp.push_back(I(I::rep(vs.size())));
                            stripe = (stripe + 1) % int(std::size(stripes));
                            return (*this)(p.module, p.entry);
                        };
                    };
                    switch (ap.adverb) {
                    case Opcode::each:
                        if (target->arity == 1) {
                            if (argc != 1) throw Exception("rank: more than 1 arg to unary each");
                            vs.emplace_back(eachbox1(wrap1(ap.proc), vs.pop()));
                        } else if (target->arity == 2) {
                            if (argc == 1) {
                                throw Exception("nyi: implicit partial on adverbed udf");
                            } else if (argc == 2) {
                                O x_ = vs.pop();
                                O y_ = vs.pop();
                                vs.emplace_back(eachbox2(wrap2(ap.proc), std::move(x_), std::move(y_)));
                            } else {
                                L<C> s;
                                s << "nyi: more than 2 args for each " << ap.adverb;
                                throw lc2ex(s);
                            }
                        } else {
                            L<C> s;
                            s << "nyi: more than 2 args for each " << ap.adverb;
                            throw lc2ex(s);
                        }
                        break;
                    case Opcode::left:
                        if (target->arity == 1) {
                            throw Exception("cannot apply each-left to unary udf");
                        } else if (target->arity == 2) {
                            if (argc == 1) {
                                throw Exception("nyi: implicit partial on adverbed udf");
                            } else if (argc == 2) {
                                O x_ = vs.pop();
                                O y_ = vs.pop();
                                const auto f = wrap2(ap.proc);
                                vs.emplace_back(eachbox1([&](O e){return f(e, y_);}, std::move(x_)));
                            } else {
                                throw Exception("cannot pass more than 2 args for each-left");
                            }
                        } else {
                            throw Exception("cannot apply each-left to udf w/more than 2 args");
                        }
                        break;
                    case Opcode::right:
                        if (target->arity == 1) {
                            throw Exception("cannot apply each-right to unary udf");
                        } else if (target->arity == 2) {
                            if (argc == 1) {
                                throw Exception("nyi: implicit partial on adverbed udf");
                            } else if (argc == 2) {
                                O x_ = vs.pop();
                                O y_ = vs.pop();
                                const auto f = wrap2(ap.proc);
                                vs.emplace_back(eachbox1([&](O e){return f(e, x_);}, std::move(y_)));
                            } else {
                                throw Exception("cannot pass more than 2 args for each-right");
                            }
                        } else {
                            throw Exception("cannot apply each-right to udf w/more than 2 args");
                        }
                        break;
                    case Opcode::prior:
                        if (target->arity == 1) {
                            throw Exception("rank: prior requires a binary target");
                        } else if (target->arity == 2) {
                            if (argc == 1) {
                                vs.emplace_back(prior(wrap2(ap.proc), vs.pop()));
                            } else if (argc == 2) {
                                O x_ = vs.pop();
                                O y_ = vs.pop();
                                vs.emplace_back(prior(wrap2(ap.proc), std::move(x_), std::move(y_)));
                            } else {
                                L<C> s;
                                s << "rank: more than 2 args for each-prior" << ap.adverb;
                                throw lc2ex(s);
                            }
                        } else {
                            L<C> s;
                            s << "???: each-prior on fun w/more than two args " << ap.adverb;
                            throw lc2ex(s);
                        }
                        break;
                    case Opcode::over:
                        if (target->arity == 1) {
                            if (argc != 1) throw Exception("rank: more than 1 arg to converge");
                            vs.emplace_back(converge(wrap1(ap.proc), vs.pop()));
                        } else if (target->arity == 2) { // reduce
                            if (argc == 1) {
                                vs.emplace_back(over(wrap2(ap.proc), vs.pop()));
                            } else if (argc == 2) {
                                O x_ = vs.pop();
                                O y_ = vs.pop();
                                vs.emplace_back(over(wrap2(ap.proc), std::move(x_), std::move(y_)));
                            } else {
                                L<C> s;
                                s << "rank: more than 2 args for over" << ap.adverb;
                                throw lc2ex(s);
                            }
                        } else {
                            L<C> s;
                            s << "???: over on fun w/more than two args " << ap.adverb;
                            throw lc2ex(s);
                        }
                        break;
                    case Opcode::scan:
                        if (target->arity == 1) {
                            if (argc != 1) throw Exception("rank: more than 1 arg to converge");
                            vs.emplace_back(convergence(wrap1(ap.proc), vs.pop()));
                        } else if (target->arity == 2) { // reduce
                            if (argc == 1) {
                                vs.emplace_back(scan(wrap2(ap.proc), vs.pop()));
                            } else if (argc == 2) {
                                O x_ = vs.pop();
                                O y_ = vs.pop();
                                vs.emplace_back(scan(wrap2(ap.proc), std::move(x_), std::move(y_)));
                            } else {
                                L<C> s;
                                s << "rank: more than 2 args for scan" << ap.adverb;
                                throw lc2ex(s);
                            }
                        } else {
                            L<C> s;
                            s << "???: scan on fun w/more than two args " << ap.adverb;
                            throw lc2ex(s);
                        }
                        break;
                    default: {
                        L<C> s;
                        s << "nyi: invoke adverbed proc" << ap.adverb;
                        throw lc2ex(s);
                    }
                    }
                }
                break;
            case Opcode::key     : unop(vs, key);                         break;
            case Opcode::left    : apply_adverb(vs, Opcode(X::rep(*ip))); break;
            case Opcode::less    : binop(vs, less);                       break;
            case Opcode::local   : ip = local(fp, vs, ip);                break;
            case Opcode::match   : binop(vs, match);                      break;
            case Opcode::max     : binop(vs, max);                        break;
            case Opcode::min     : binop(vs, min);                        break;
            case Opcode::mul     : binop(vs, mul);                        break;
            case Opcode::neg     : unop(vs, uminus);                      break;
            case Opcode::nop     :                                        break;
            case Opcode::not_    : unop(vs, not_);                        break;
            case Opcode::null    : unop(vs, null);                        break;
            case Opcode::op      : ip = push_atom_literal<Opcode>(vs,ip); break;
            case Opcode::over    : apply_adverb(vs, Opcode(X::rep(*ip))); break;
            case Opcode::pop     : vs.pop_back();                         break;
            case Opcode::prior   : apply_adverb(vs, Opcode(X::rep(*ip))); break;
            case Opcode::pushc   : ip = push_constant(constants, vs, ip); break;
            case Opcode::pushm   : ip = push_static(statics, vs, ip);     break;
            case Opcode::recip   : unop(vs, recip);                       break;
            case Opcode::rev     : unop(vs, reverse);                     break;
            case Opcode::right   : apply_adverb(vs, Opcode(X::rep(*ip))); break;
            case Opcode::rot     : ip = rot(vs, ip);                      break;
            case Opcode::scan    : apply_adverb(vs, Opcode(X::rep(*ip))); break;
            case Opcode::sub     : binop(vs, sub);                        break;
            case Opcode::swap    : do_swap(vs);                           break;
            case Opcode::take    : binop(vs, take);                       break;
            case Opcode::type    : unop(vs, type_);                       break;
            case Opcode::string  : unop(vs, string_);                     break;
            case Opcode::value   : unop(vs, value);                       break;
            case Opcode::vcond   : ternop(vs, vcond);                     break;
            case Opcode::where   : unop(vs, where);                       break;
            default: {
                L<C> err;
                err << "nyi: opcode " << *ip;
                throw lc2ex(err);
            }
            }
        }
        return vs.empty()? O() : vs.pop();
    }
} // namespace

O run(KV<S, KV<S,O>>& env, S module_name, index_t start, bool trace) {
    Vm vm(env, trace);
    return vm(module_name, start);
}
