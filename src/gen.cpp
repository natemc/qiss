#include <gen.h>
#include <adverb.h>
#include <arith.h>
#include <ast.h>
#include <at.h>
#include <bit>
#include <disassemble.h>
#include <exception.h>
#include <flip.h>
#include <iaddr.h>
#include <in.h>
#include <l.h>
#include <lcutil.h>
#include <limits>
#include <lutil.h>
#include <match.h>
#include <not.h>
#include <o.h>
#include <objectio.h>
#include <opcode.h>
#include <over.h>
#include <primio.h>
#include <relop.h>
#include <sym.h>
#include <take.h>
#include <value.h>
#include <where.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    template <class X> struct Immediate {};
#define CS(Z,O,L) template <> struct Immediate<Z> { \
    static constexpr Opcode value = Opcode::O;      \
    static constexpr Opcode list  = Opcode::L;      \
}
    CS(B, immb, immbv); CS(D, immd, immdv); CS(F, immf, immfv);
    CS(I, immi, immiv); CS(J, immj, immjv); CS(T, immt, immtv); CS(X, immx, immxv);
#undef CS
    template <class X> constexpr Opcode immediate_v = Immediate<X>::value;
    template <class X> constexpr Opcode immediate_l = Immediate<X>::list;

    template <class Z> void write(L<X>& code, const Z& z) {
        uint8_t b[sizeof z];
        memcpy(&b, &z, sizeof b);
        code.append(b, b + sizeof b);
    }

    template <class Y, class Z> void write(L<X>& code, const Y& y, const Z& z) {
        write(code, y); write(code, z);
    }

    template <class X> uint16_t two_bytes(X x) {
        assert(0 <= x && x <= std::numeric_limits<uint16_t>::max());
        uint16_t b;
        if constexpr(std::endian::native == std::endian::little)
            memcpy(&b, &x, sizeof b);
        else
            memcpy(&b, &x + sizeof x - 2, sizeof b); // TODO test this on arm
        return b;
    }

    void write_constant(L<X>& code, L<O>& constants, O x) {
        write(code, Opcode::pushc);
        write(code, sindex_t(constants.size()));
        constants.emplace_back(x);
    }

    template <class Z> void write_imm(L<X>& code, Z x) {
        write(code, immediate_v<Z>);
        write(code, x);
    }

    template <class Z> void write_imm(L<X>& code, L<Z> x) {
        assert(x.size() <= 65535);
        write(code, immediate_l<Z>, two_bytes(x.size()));
        for (Z z: x) write(code, z);
    }

    void write_imm(L<X>& code, C x) {
        write(code, Opcode::immstr, two_bytes(1));
        write(code, x);
        write(code, Opcode::first);
    }

    void write_imm(L<X>& code, L<C> x) {
        assert(x.size() <= 65535);
        write(code, Opcode::immstr, two_bytes(x.size()));
        for (C c: x) write(code, c);
    }

    void write_imm(L<X>& code, S x) {
        const char* const   s   = c_str(x);
        const X::rep* const r   = reinterpret_cast<const X::rep*>(s);
        const std::size_t   len = strlen(s);
        write(code, Opcode::immsym, two_bytes(len));
        code.append(r, r + len);
    }

    bool is_binary_op(Opcode op) {
        const Opcode bops[] = {
            Opcode::add  ,
            Opcode::at   ,
            Opcode::bang ,
            Opcode::cast ,
            Opcode::cat  ,
            Opcode::cut  ,
            Opcode::dot  ,
            Opcode::eq   ,
            Opcode::fdiv ,
            Opcode::fill ,
            Opcode::find ,
            Opcode::great,
            Opcode::less ,
            Opcode::match,
            Opcode::max  ,
            Opcode::min  ,
            Opcode::mul  ,
            Opcode::sub  ,
            Opcode::take ,
        };
        return std::find(std::begin(bops), std::end(bops), op) != std::end(bops);
    }

    // Having gotten this to work for simple binary ops, it's clear that this
    // is much less efficient than writing the loops in C++.
    void write_over(L<X>& code) {
        // What's the chance that some random byte value not representing
        // a binary op, but equal to the same, will be sitting here? Too high.
        if (!is_binary_op(Opcode(X::rep(code.back()))))
            throw Exception("nyi: over for anything besides binop");

        // At runtime, the lhs will be on the top of the stack
        // the rhs side will be one below
        const Opcode op(Opcode(X::rep(code.pop())));
        write_imm(code, J(0));
        iaddr_t loop = iaddr_t(code.size());
        write(code, Opcode::dup);
        write(code, Opcode::dupnth, X(3));
        write(code, Opcode::count);
        write(code, Opcode::sub);
        write(code, Opcode::bzero);
        X::rep* const out = reinterpret_cast<X::rep*>(code.end());
        write(code, iaddr_t(0));
        write(code, Opcode::dup);
        write(code, Opcode::dupnth, X(3));
        write(code, Opcode::at);
        write(code, Opcode::rot, X(2));
        write(code, op);
        write(code, Opcode::swap);
        write_imm(code, J(1));
        write(code, Opcode::add);
        write(code, Opcode::jump, loop);
        const iaddr_t outro = iaddr_t(code.size());
        memcpy(out, &outro, sizeof outro);
        write(code, Opcode::swap);
        write(code, Opcode::clean, X(2));
    }

    void write_adverb(L<X>& code, Adverb adv) {
        // Lots of decisions to make here.
        // In general, the effect of an adverb is to create a new function.
        // We want to support things like
        //    g:f/
        // In that example, maybe we can compile f/ and create an object
        // that refers to it.
        // What we cannot do we compile an adverb in isolation, so to implement
        // write_adverb, we need to look at what was compiled recently.
        // TODO optimized versions (for, e.g., +/), using a builtin
        // and invoke.
        // TODO decide if compiling adverbs is a worse idea than handling them
        // at runtime.

        switch (adv) {
        case Adverb::each : write(code, Opcode::each ); break;
        case Adverb::eachL: write(code, Opcode::left ); break;
        case Adverb::eachR: write(code, Opcode::right); break;
        case Adverb::over : write(code, Opcode::over ); break;
        case Adverb::prior: write(code, Opcode::prior); break;
        case Adverb::scan : write(code, Opcode::scan ); break;
        }
    }

    void write_enlist(L<X>& code, X arity) {
        if (arity == X(1))
            write(code, Opcode::enlist);
        else {
            write(code, Opcode::op, Opcode::enlist);
            write(code, Opcode::invoke, arity);
        }
    }

    void write_hole(L<X>&) {
        throw Exception("nyi: partial application / holes (gen.cpp)");
    }

    void write_imm(L<X>& code, L<O>& constants, O x) {
#define CS(X) case -OT<X>::typei(): write_imm(code, x.atom<X>()); break;                \
              case OT<X>::typei() : if (x->n <= 3) write_imm(code, L<X>(std::move(x))); \
                                    else           write_constant(code, constants, x);  \
                                    break
        switch (int(x.type())) {
        case -OT<S>::typei(): write_imm(code, x.atom<S>()); break;
        case OT<S>::typei() : write_constant(code, constants, x); break;
        CS(B); CS(C); CS(D); CS(F); CS(I); CS(J); CS(T); CS(X);
        default: write_constant(code, constants, x);
#undef CS
        }
    }

    void write_invoke(L<X>& code, I args) {
        assert(I(0) <= args && args < I(256));
        write(code, Opcode::invoke, X(X::rep(I::rep(args))));
    }

    void write_op(L<X>& code, Opcode op, X arity) {
        if (arity <= X(2)) { write(code, op); return; }
        throw Exception("nyi");
    }

    void write_static(L<X>& code, L<O>& statics, O x) {
        write(code, Opcode::pushm);
        write(code, sindex_t(statics.size()));
        statics.emplace_back(x);
    }

    // This may turn out to be handy
    L<X> write_fun(O ast_, I root) {
        KV<S,O> ast    (addref(ast_->dict));
        L<C>    t      (ast["type"_s]);
        L<O>    n      (ast["node"_s]);
        L<I>    p      (ast["parent"_s]);
        L<X>    k      (ast["kids"_s]);
        L<S>    formals(n[root]);
        L<I>    body   (&(p == root));
        L<X>    r;
        for (I b: body) {
            if (b == root) break;
            switch (Ast(C::rep(t[b]))) {
            case Ast::id: {
                auto it = std::find(formals.begin(), formals.end(), n[b].atom<S>());
                if (it == formals.end()) throw Exception("nyi: free vars");
                write(r, Opcode::local, X(X::rep(7 - (it - formals.begin()))));
                break;
            }
            case Ast::op: write_op(r, n[b].atom<Opcode>(), k[b]); break;
            default: throw Exception("nyi: write_fun");
            }
        }
        write(r, Opcode::clean, X(X::rep(formals.size())));
        write(r, Opcode::ret);
        return r;
    }

    KV<I,L<X>> write_funs(O ast_) {
        KV<S,O>    ast   (+ast_);
        L<C>       t     (ast["type"_s]);
        L<I>       funs  (&(t == X::rep(Ast::lambda)));
        KV<I,L<X>> r;
        for (I root: funs) r.add(root, write_fun(ast_, root));
        return r;
    }

} // unnamed

void gen(KV<S,O>& module, O ast_, bool trace) {
    KV<S,O> ast(+ast_);
    assert(module.has("name"_s));
    const S module_name = module["name"_s].atom<S>();
    L<X>    code         (module["code"_s]);
    L<O>    constants    (module["constants"_s]);
    L<O>    statics      (module["statics"_s]);
    KV<S,I> objects      (module["objects"_s]);
    L<S>    exports      (module["exports"_s]);
    L<I>    d            (ast["distance"_s]);
    L<I>    f            (ast["frame"_s]);
    L<X>    k            (ast["kids"_s]);
    L<O>    n            (ast["node"_s]);
    L<I>    p            (ast["parent"_s]);
    L<I>    s            (ast["slot"_s]);
    L<C>    t            (ast["type"_s]);
    KV<I,I> blocks; // block # (parent) -> first instruction
    L<I>    calls;
    L<I>    procs;
    const I statics_base(I::rep(statics.size()));
    for (index_t i = 0; i < t.size(); ) {
        const I block = p[i];
        blocks.add(block, I(I::rep(code.size())));
        L<S> formals(t[block] == Ast::lambda? L<S>(n[block]) : L<S>{});
        assert(formals.size() <= 8);
        const X kids(k[block]);
        assert(0 < X::rep(kids));
        const I fb(t[block] == Ast::lambda? f[block] : I(0));
        if (fb) write(code, Opcode::bump, X::rep(I::rep(fb)));
        for (; i < t.size() && p[i] == block; ++i) {
            switch (Ast(C::rep(t[i]))) {
            case Ast::adverb:
                write_adverb(code, Adverb(X::rep(n[i].atom<X>())));
                if (X(1) < k[i]) write_invoke(code, I(X::rep(k[i] - 1)));
                break;
            case Ast::apply : write_invoke(code, n[i].atom<I>());                 break;
            case Ast::bind  :
                if (trace) H(1) << "  !! Ast::bind w/f[i] " << f[i] << '\n' << flush;
                // TODO if we want to support .qo, we need another level of
                // indirection like immsym or statics
                if (f[i] != I(0))
                    write(code, Opcode::bindl, X::rep(I::rep(s[i])));
                else {
                    const S name = n[i].atom<S>();
                    if (const I o = objects.at(name, NI); o != NI)
                        write(code, Opcode::bindm, sindex_t(I::rep(o)));
                    else {
                        write(code, Opcode::bindm, sindex_t(statics.size()));
                        objects.upsert(name, I(I::rep(statics.size())));
                        if (i < t.size() - 1 && t[i+1] == Ast::export_) {
                            exports.emplace_back(name);
                            ++i;
                        }
                        statics.emplace_back(generic_null());
                    }
                }
                break;
            case Ast::cond  :
                throw Exception("nyi: $ (switch)");
                break;
            case Ast::export_: break;
            case Ast::hole  : write_hole(code);                               break;
            case Ast::id    : {
                if (trace) H(1) << " Ast::id w/f[i] " << f[i] << '\n' << flush;
                const I frame = f[i];
                if (frame == NI) {
                    if (const I slot = objects.at(n[i].atom<S>(), NI); slot != NI) {
                        write(code, Opcode::pushm, slot);
                        if (i < t.size()-1 && t[i+1] == Ast::bind && f[i+1] == I(0))
                            objects.upsert(n[++i].atom<S>(), slot);
                    }
                    else {
                        L<C> ss;
                        ss << c_str(n[i].atom<S>()) << " is not bound";
                        throw lc2ex(ss);
                    }
                    break;
                }
                assert(I(0) <= frame);
                const I::rep slot(s[i]);
                if (frame == I(0)) {
                    write(code, Opcode::pushm, I(slot) + statics_base);
                    if (i < t.size()-1 && t[i+1] == Ast::bind && f[i+1] == I(0)) {
                        // TODO does it make more sense to overwrite the object
                        // in statics instead of creating a new static and
                        // changing where the global points?
                        objects.upsert(n[++i].atom<S>(), I(slot) + statics_base);
                    }
                }
                else {
                    const I::rep distance(d[i]);
                    assert(0 <= distance);
                    if (distance > 3) throw Exception("limit: cannot refer to distant scope");
                    if (slot < -12) throw Exception("limit: max 12 params");
                    if (slot > 51) throw Exception("limit: max 52 locals");
                    write(code, Opcode::local, X::rep((distance << 6) | (slot + 12)));
                }
                break;
                }
            case Ast::infix:
                // TODO unify this with the module-frame id handling code
                if (const I slot = objects.at(n[i].atom<S>(), NI); slot != NI) {
                    write(code, Opcode::pushm, slot);
                    if (i < t.size()-1 && t[i+1] == Ast::bind && f[i+1] == I(0))
                        objects.upsert(n[++i].atom<S>(), slot);
                    if (k[i]) write_invoke(code, I(X::rep(kids)));
                }
                else {
                    L<C> ss;
                    ss << c_str(n[i].atom<S>()) << " is not bound";
                    throw lc2ex(ss);
                }
                break;
            case Ast::juxt  : write_invoke(code, I(1));                       break;
            case Ast::lambda: break;
                // We do the work in the Ast::ref case, because the entry
                // point is long gone by now.
            case Ast::list  : write_enlist(code, k[i]);                    break;
            case Ast::lit   : {
                // For small items, generate an immediate instruction.
                // Otherwise, stuff x into the constant area and generate a
                // constant load instruction.  TODO: use a hash set (see
                // distinct) for the constant area.  TODO consider well-known
                // constants for e.g., 0, 1, generic null.  TODO make constants
                // global rather than module level? Potential problem: we have
                // to make sure data in the static area is never modified.
                if (trace) H(1) << "    Ast::lit " << n[i] << '\n' << flush;
                const bool imm_bound_module_var =
                    i < t.size()-1 && t[i+1] == Ast::bind && f[i+1] == I(0);
                if (!imm_bound_module_var)
                    write_imm(code, constants, n[i]);
                else {
                    write_static(code, statics, n[i]);
                    objects.upsert(n[++i].atom<S>(), I(I::rep(statics.size() - 1)));
                }
                break;
                }
            case Ast::module: break;
            case Ast::op    :
                if (k[i]) write_op(code, n[i].atom<Opcode>(), k[i]);
                else      write   (code, Opcode::op, n[i].atom<Opcode>());
                break;
            case Ast::ref   : {
                const I ref = n[i].atom<I>();
                if (i < t.size() - 1) {
                    const Ast succ{C::rep(t[i+1])};
                    if (t[ref] == Ast::lambda) {
                        const index_t arity = L<S>(n[ref]).size();
                        if (succ == Ast::juxt || succ == Ast::apply) {
                            if (succ == Ast::juxt && arity == 0 ||
                                succ == Ast::apply && I(I::rep(arity)) < n[i+1].atom<I>())
                                throw Exception("rank"); // TODO link back to src
                            if (succ == Ast::juxt && arity == 1 ||
                                succ == Ast::apply && I(I::rep(arity)) == n[i+1].atom<I>())
                            {
                                // TODO inline short functions
                                // We put the address of the call
                                // instruction into calls and the block
                                // number in the place where the called
                                // function address will go.
                                calls.emplace_back(I::rep(code.size()));
                                write(code, Opcode::call, ref);
                                ++i;
                            } else {
                                throw Exception("nyi: code generation (partial application)");
                            }
                        } else {
                            // similar to calls to fixup code addr
                            procs.emplace_back(I::rep(code.size()));
                            O proc(make_proc(module_name, iaddr_t(I::rep(ref)), X(X::rep(arity))));
                            write_static(code, statics, proc);
                            if (succ == Ast::bind && f[i+1] == I(0))
                                objects.upsert(n[++i].atom<S>(), I(I::rep(statics.size() - 1)));
                        }
                    } else {
                        throw Exception("nyi: code generation (ref)");
                    }
                } else {
                    // TODO add ref'd node to statics and write(pushm)?
                    throw Exception("nyi: code generation (ref)");
                }
                break;
                }
            default         : throw Exception("nyi: code generation");
            }
        }
        index_t mess(formals.size() + I::rep(fb) + X::rep(kids-1));
        for (; mess; mess = std::max(index_t(0), mess - 255))
            write(code, Opcode::clean, X(X::rep(std::min(mess, index_t(255)))));
        write(code, Opcode::ret);
    }
    for (I c: calls) {
        I::rep block;
        memcpy(&block, &code[c] + 1, sizeof block);
        assert(blocks.has(I(block)));
        X::rep* const out = reinterpret_cast<X::rep*>(&code[c] + 1);
        memcpy(out, &blocks[I(block)], sizeof(I));
    }
    for (I proc: procs) {
        // proc is an index (into code) of the pushm instruction referring
        // to a proc (stored in statics) whose entry we need to patch.
        I::rep static_index;
        memcpy(&static_index, &code[proc] + 1, sizeof static_index);
        O& o = statics[I(static_index)];
        I block(I::rep(o->proc.entry));
        assert(blocks.has(block));
        memcpy(&o->proc.entry, &blocks[block], sizeof(I));
    }
    if (trace) H(1) << code << '\n' << flush;
    module["code"_s]      = std::move(code);
    module["constants"_s] = std::move(constants);
    module["statics"_s]   = std::move(statics);
    module["objects"_s]   = std::move(objects);
    module["exports"_s]   = std::move(exports);
}
