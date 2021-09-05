#include <compile.h>
#include <algorithm>
#include <ast.h>
#include <at.h>
#include <cat.h>
#include <converge.h>
#include <count.h>
#include <distinct.h>
#include <do.h>
#include <each.h>
#include <find.h>
#include <flip.h>
#include <group.h>
#include <iasc.h>
#include <in.h>
#include <iterator>
#include <key.h>
#include <kv.h>
#include <l.h>
#include <lutil.h>
#include <memory>
#include <o.h>
#include <objectio.h>
#include <over.h>
#include <primio.h>
#include <parse.h>
#include <returning.h>
#include <reverse.h>
#include <sym.h>
#include <take.h>
#include <ukv.h>
#include <vcond.h>

namespace {
    template <class X> using OT = ObjectTraits<X>;

    L<I> parent(L<X> depth) {
        assert(depth.size() < std::numeric_limits<I::rep>::max());

        // In k: ((#d)#0){[g;p;i]@[p;g i;:;g[i-1]g[i-1]bin g i]}[g]/1+!-1+#g:.=d
        // See Hsu page 88
        // Hsu says this is linear, and it seems so, but thanks to the
        // inner-most while loop, this is not obvious.  Can we prove that it is
        // linear?
        if (depth.empty()) return L<I>{};
        L<I> parent(depth.size());
        parent[0] = I(0);
        L<L<J>> v(UKV(group(depth)).val());
        for (index_t i = 1; i < v.size(); ++i) {
            L<J> p(v[i - 1]); // parents
            L<J> c(v[i]);     // children
            index_t pi = 1;
            for (J ch: c) {
                while (pi < p.size() && p[pi] < ch) ++pi;
                parent[ch] = I(I::rep(J::rep(p[pi - 1])));
            }
        }
        return parent;
    }

    L<I> ancestor_of_type(KV<S,O>& ast, L<X> types) {
        // See Hsu page 89
        // b:Ast::lambda=ast`type; {?[b x;x;x x]}/[ast`parent]
        L<B> b(in(ast["type"_s], std::move(types)));
        auto f = [&](L<I> x){ return L<I>(vcond(b[x], x, x[x])); };
        return converge_(f, L<I>(ast["parent"_s]));
    }

    O nb(O x) { H(1) << " NB " << x << '\n' << flush; return x; }
    O nb(const char* s, O x) { H(1) << " NB " << s << ": " << x << '\n' << flush; return x; }

    L<I> direct_children_of_type(KV<S,O>& ast,
                                 L<X>     types,
                                 L<I>     start)
    {
        // See Hsu page 89
        // b:Ast::lambda=ast[`type]@p:ast`parent; {?[b x;x;p x]}/[ast`parent]
        L<I> p(ast["parent"_s]);
        L<X> t(ast["type"_s]);
        L<B> b(!t.in(types) & t[p].in(types)); // cppcheck-suppress clarifyCondition
        auto f = [&](L<I> x){ return L<I>(vcond(b[x], x, p[x])); };
        return converge_(f, start);
    }

    L<I> lexical_contour(KV<S,O>& ast) {
        L<X> scope_types{X::rep(Ast::lambda), X::rep(Ast::module)};
        return ancestor_of_type(ast, scope_types);
    }

    O lift_functions(O ast_) {
        KV<S,O> ast(+ast_);
        // p<>!#p is overkill at this point, because until we lift local
        // functions, there is only one root (zero), and it ain't a lambda!
        // See Hsu page 101
        // i:&(Ast::lambda=ast`type)&p<>!#p:ast`parent
        L<X> type(ast["type"_s]);
        L<I> lams(&(type == X::rep(Ast::lambda)));

        L<O> node(ast["node"_s]);
        // Must do this now, since we overwrite these node cells below.
        L<O> lifted(node[lams]);

        // The indices of the new nodes are (#p)+!#i    See Hsu page 102
        L<I> new_node(tili(lams.size()) + I(I::rep(type.size())));
        L<X> new_type(replicate(lams.size(), X(X::rep(Ast::lambda))));

        // Next, change old lambda nodes into refs. See Hsu p. 104
        DO(lams) type[lams[i_]] = X(X::rep(Ast::ref)); // cppcheck-suppress unreadVariable
        DO(lams) node[lams[i_]] = O(new_node[i_]);     // cppcheck-suppress unreadVariable

        // To save the children, we must fix those elements of ast[parent] that
        // currently point to the newly-made refs to point to the corresponding
        // new_nodes. While we're at it, we want to update the corresponding
        // lexical contour indices. We do this via a permutation vector called
        // bigv (Hsu's V; see Hsu p. 105).
        L<I> r(ast["contour"_s]);
        L<I> p(ast["parent"_s]);
        L<I> bigv(tili(p.size()));
        DO(lams) bigv[lams[i_]] = new_node[i_]; // V:@[!#p;lams;:;new_node]; 
        DO(bigv) p[i_]          = bigv[p[i_]];  // ast[`p]:V ast`p;
        DO(bigv) r[i_]          = bigv[r[i_]];  // ast[`r]:V ast`r;

        // Re: the 3rd arg to v's ctor: since the new nodes are roots, they are
        // their own parents IOW, ast[parent][new_node],:new_node
        L<O> v{O(std::move(new_type)), O(std::move(lifted)),
               O(std::move(new_node)), O(r[lams])};
        return cat(ast_, +UKV(ast.key(), v));
    }

    void count_kids(O ast_) {
        KV<S, O> ast(+ast_);
        assert(!ast.has("kids"_s));
        L<I> p(ast["parent"_s]);
        L<X> kids(p.size(), X(0));
        DO(p) kids[p[i_]] += B(p[i_] != I(I::rep(i_)));
        ast.add("kids"_s, kids);
    }
    
    O flatten_expressions(O ast_) {
        // We want to reorder the AST into code generation order.  That means
        // expressions right-to-left, sequences of expressions left-to-right.
        // Any node we don't move must stay where it is.  Meanwhile, moved
        // nodes must be lifted so that their parents are the same as their
        // (statment) heads.  Lastly, children of moved nodes must have their
        // parent indices updated so they keep the same parent (though so far
        // in qiss, there are no nodes that stay still in this pass other than
        // modules and lambdas, and lambdas have already been lifted).

        // See Hsu p. 125. I'm not able to follow his explanation completely.
        // What I notice is that in the array x used in his example on p. 125,
        // those expr nodes whose parents have type 3 (F, Ast::lambda here) -
        // i.e., 1, 4 and 10 - point to themselves.  This gives them low values
        // compared to the other nodes that all have high values because they
        // refer to functions that have been lifted to the end of the table.
        // Those nodes with parent of type 3 are easy to find:
        //     p3:&(~t=3)&3=t p
        // but the approach in Hsu is to go after the node types we want to
        // lift, because there are some nodes - at least in Hsu - we don't want
        // to lift (type 7 aka N).
        //     i:&t in 0 1 2 4 8 9 10                    / nodes to lift
        //     s:(~t=3)&3=t p                            / stmt heads
        //     x:{?[s x;x;p x]}/[i]                      / statement heads per i
        //                                               /   NOT Hsu's x, but
        //     P:@[p;i;:;p x]                            / lift nodes by making their parents the same as their stmt heads' parents
        //                                               / almost same as p[i]<-p[x]
        //     j:(|i)<|x                                 / NOT Hsu's j, but ...
        //     j:(),/|'i@.=x                             /   (alternate formula)
        //     k:@[!#p;i;:;j]                            / ... this K works to
        //     m:@[ast;`p;:;P]k                          / reorder exprs within stmts
        //                                               /   leaving N nodes w/wrong p
        //     @[m;`p;:;K m`p]                           / fix from p. 127
        //     AST:@[ast;`p;:;K P]K                      / shortcut: prove it's right
        // The following gives Hsu's x and j, but then goes awry:
        //     i:&t in 0 1 2 4 8 9 10    / nodes to lift
        //     w:&t[p i]in 3 4;          / immediate children of F nodes
        //     x:@[i;1_/:w_!#i;:;p i w]; / Hsu's x (!) note x is only needed for j
        //     j:(|i)<|x;                / maybe there's a shortcut to go to j
        //     k:@[!#p;i;:;j];           / or even better straight to k
        //     ast@:k;                   / except this k doesn't work :-/
        //
        // Consider the following example:
        //     f:{x+y}; f[3;4]
        //
        // When we arrive here, the AST is
        //
        //     row type node parent contour kids    to_lift heads goal
        //         -----------------------------
        //     0   4d   ::        0       0 02                    0
        //     1   3a   `f        0       0 01      1       1     2
        //     2   72   10i       1       0 00      2       1     1
        //     3   4f   +        10      10 02      3       3     9
        //     4   49   `x        3      10 00      4       3     8
        //     5   49   `y        3      10 00      5       3     7
        //     6   61   2i        0       0 03      6       6     6
        //     7   49   `f        6       0 00      7       6     5
        //     8   6e   3         6       0 00      8       6     4
        //     9   6e   4         6       0 00      9       6     3
        //     10  6c   `x`y     10       0 01                    10
        //
        // The goal column is a permutation vector that will put the nodes in
        // the desired order.  Once we fixup the parents, we just ast@goal.

        KV<S, O> ast        (+ast_);
        L<I>     p          (ast["parent"_s]);
        L<I>     r          (ast["contour"_s]);
        L<X>     t          (ast["type"_s]);
        // Unlike Hsu, we lift all node types except lambda and module (so far)
        L<X>     root_types {X::rep(Ast::lambda), X::rep(Ast::module)};
        L<I>     to_lift    (&!t.in(root_types));
        L<I>     heads      (direct_children_of_type(ast, root_types, to_lift));
        L<I>     new_parents(p[heads]);
        L<I>     np         (p.begin(), p.end());
        DO(to_lift) np[to_lift[i_]] = new_parents[i_];

        // The parents are (mostly) right and all exprs are lifted. Next we
        // sort by contour (asc), heads (asc), and node id (desc). Since the
        // node id is monotonically increasing, we can collapse
        //     jjj = to_lift[idesc(to_lift)];
        //     jj = jjj[iasc(heads[jjj])]
        // to
        //     jj = reverse(to_lift)[iasc(reverse(heads))]
        // then
        //     j = jj[iasc(r[jj])]
        // j is a partial (ie, just for to_lift) permutation vector; k is a full one
        L<I> jj(L<I>(reverse(to_lift))[L<J>(iasc(reverse(heads)))]);
        L<I> j(jj[L<J>(iasc(r[jj]))]);
        L<I> k(tili(p.size()));
        DO(to_lift) k[to_lift[i_]] = j[i_];
        // This is Hsu's trick p. 127 to fix parents of the nodes we didn't lift
        ast["parent"_s] =  k[np];
        return at(ast_, k);
    }

    const struct PackPairs {
        template <class X, class Y>
        L<J> operator()(L<X> x, L<Y> y) const {
            static_assert(sizeof(X) + sizeof(Y) == sizeof(J));
            assert(x.size() == y.size());
            L<J> pairs(x.size());
            for (index_t i = 0; i < x.size(); ++i) {
                char* const p = reinterpret_cast<char*>(&pairs[i]);
                // Because we sort packed pairs
                if constexpr(std::endian::native == std::endian::big) {
                    memcpy(p            , std::addressof(x[i]), sizeof(X));
                    memcpy(p + sizeof(X), std::addressof(y[i]), sizeof(Y));
                } else {
                    memcpy(p            , std::addressof(y[i]), sizeof(Y));
                    memcpy(p + sizeof(Y), std::addressof(x[i]), sizeof(X));
                }
            }
            return pairs;
        }
    } pack_pairs;

    template <class X, class Y>
    std::pair<L<X>, L<Y>> unpack_pairs(L<J> x) {
        static_assert(sizeof(X) + sizeof(Y) == sizeof(J));
        L<X> xr(x.size());
        L<Y> yr(x.size());
        for (index_t i = 0; i < x.size(); ++i) {
            const char* const p = reinterpret_cast<char*>(std::addressof(x[i]));
            if constexpr(std::endian::native == std::endian::big) {
                memcpy(std::addressof(xr[i]), p            , sizeof(X));
                memcpy(std::addressof(yr[i]), p + sizeof(X), sizeof(Y));
            } else {
                memcpy(std::addressof(yr[i]), p            , sizeof(Y));
                memcpy(std::addressof(xr[i]), p + sizeof(Y), sizeof(X));
            }
        }
        return std::pair(xr, yr);
    }

    // Returns a table (example below) and a dict of frame bumps:
    // contour name slot frame
    // -----------------------
    // fun i   var     0     0
    // ..
    // Explicitly bound variables start at slot 0 and go up.
    // Lambda parameters start at slot -1 and go down.
    // It seems to me that frame is redundant and weaker than contour.
    std::pair<O, KV<I,I>> stack_frames(O ast_) {
        KV<S,O> ast(+ast_);
        L<O>    n  (ast["node"_s]);
        L<I>    p  (ast["parent"_s]);
        L<I>    r  (ast["contour"_s]);
        L<X>    t  (ast["type"_s]);

        // Section 3.10 (pp. 127-129)
        // The contour for an explicit binding is the binding node's contour
        // entry, but for a param it's the parent (a lambda's contour is one
        // too high for its params).  Perhaps we should have left in a formals
        // node for uniformity.
        L<I> b     (&(t == X::rep(Ast::bind)));
        L<S> bnames(b.empty()? L<S>{} : L<S>(at(n, b)));
        L<I> pc; // parameter contours
        L<S> pn; // parameter names
        KV<I,I> pcount; // parameter count for each lambda
        L<I> lams(&(t == X::rep(Ast::lambda)));
        for (I lam: lams) {
            L<S> params(n[lam]);
            pcount.add(lam, I(I::rep(params.size())));
            std::reverse_copy(params.begin(), params.end(), std::back_inserter(pn));
            DO(params) pc.emplace_back(p[lam]);
        }
        L<I> rntop(cat(pc, r[b]));
        L<S> rnbot(L<S>(cat(pn, bnames)));
        L<J> rnord(iasc(rntop));
        L<J> rn   (pack_pairs(rntop[rnord], rnbot[rnord]));
        // Hsu sorts e; we don't. I believe Hsu does it to enable binary search
        // of e, but it doesn't work for us because we put both params and
        // local bindings in e; sorting would change the order so that simple
        // subtraction would not suffice to compute the correct slot.  If e is
        // ever large, we might need to come up with a different solution.
        L<J> e    (distinct(rn));
        auto [bc, bn] = unpack_pairs<I,S>(e); // bc (binding contours) is Hsu's x
        KV<I,L<I>> gbc(group(bc));
        L<I> framesz(each(internal_counti, gbc.val()));
        auto frame_slots = [&](I fsz, I frm){
            return tili(fsz) - (t[frm] == Ast::lambda? pcount[frm] : I(0));
        };
        L<I> slots(over(cat, L<I>{}, each(frame_slots, framesz, gbc.key())));
        // Hsu appends a -1 to slots, but we have 0N

        // We can't just #'=gbc, because we need to account for frames with
        // zero local bindings. For example: {{{x+p}x}p:1}[]. We also want to
        // skip modules, so we use p here instead of r since lambdas are their
        // own parents.
        KV<I,I> frame_bumps;
        L<I> emptyi;
        for (I scope: p[lams])
            frame_bumps.add(scope,
                I(I::rep(gbc.at(scope, emptyi).size() - L<S>(n[scope]).size())));

        // Hsu replaces n for function nodes with the number of slots (i.e.,
        // non-param locals) in that function.  We have the formals in those
        // cells of n, and we need them (or at least their count) later when we
        // generate code.  Meanwhile, there are other short arrays we need to
        // create, so I just make a table with all of them and leave n alone.
        // However, it may turn out better to treat formals as locals that are
        // initialized at the start of the function so their references can be
        // treated the same as other locals.  I believe Hsu treats them
        // differently, because in co-dfns, locals can be rebound but
        // parameters may not.  I'm against rebinding locals, so we may just
        // throw if ~e~rn.

        // Section 3.11 (p. 130)
        auto scope_depth = [&, i=gbc.key()](L<I> d) mutable {
            L<I> up(r[i]);
            RETURNING(L<I>(d + (i != up)), i = up);
        };
        // depth of binding relative to module scope
        L<I> depth(converge_(scope_depth, L<I>(framesz.size(), I(0))));

        // These frame numbers are the frame in which a name is bound counting
        // from zero==module frame.  To use these frame numbers at runtime, we
        // need the vm to know what frame 0 is.  We can prolly make that work,
        // but I think it's be simpler if we used frame numbers relative to
        // each reference, so we compute that distance later in resolve.
        L<I> frame(slots.size()); // Hsu's f
        index_t k = 0;
        DO(framesz) {
            for (index_t j = 0; j < I::rep(framesz[i_]); ++j)
                frame[k++] = depth[i_];
        }

        // Note: Because we have an explicit export, we don't need to save
        // exported names (section 3.12)
        L<S> result_key{"contour"_s, "name"_s, "slot"_s, "frame"_s};
        L<O> result_val{O(std::move(bc)), O(std::move(bn)),
                        O(std::move(slots)), O(std::move(frame))};
        return std::pair(+UKV(result_key, result_val), frame_bumps);
    }

    void resolve(O ast_, O frames_, KV<I,I>& frame_bumps) {
        KV<S,O> ast   (+ast_);
        KV<S,O> frames(+frames_);
        L<X> t        (ast["type"_s]);
        L<O> n        (ast["node"_s]);
        L<I> p        (ast["parent"_s]);
        L<I> r        (ast["contour"_s]);
        L<I> slots    (frames["slot"_s]);

        L<I> lc;
        L<S> ln;
        for (I lam: &(t == X::rep(Ast::lambda))) {
            for (S ns: L<S>(n[lam])) {
                lc.emplace_back(p[lam]);
                ln.emplace_back(ns);
            }
        }

        L<I> b(&(t == X::rep(Ast::bind)));
        if (b.empty() && lc.empty()) {
            ast.add("distance"_s, L<I>(r.size(), NI));
            ast.add("frame"_s   , L<I>(r.size(), NI));
            ast.add("slot"_s    , L<I>(r.size(), NI));
            return;
        }
        L<I> v(&(t == X::rep(Ast::id))); // var refs; may be bound locally or free
        L<I> y(cat(v, b));               // indices of vars followed by bindings
                                         // if we disallow rebinding, don't need bindings
        L<S> x(at(n, y));                // names of such
        // e is the distinct (contour, name) bindings
        // IOW, if we write {p:1;p:2} then rn has 2 entries and e has 1
        // I plan to just outlaw that.  It does mean we'll have to do
        // something different at the module level for the interactive case.
        L<J> e(pack_pairs(L<I>(frames["contour"_s]), L<S>(frames["name"_s])));
        L<I> i(x.size(), I::rep(e.size())); // Hsu uses c to hold #e
        L<I> d(x.size(), I(0)); // distance from reference to binding
        auto closest_binding = [&](L<O> omega){
            L<I> top  (omega[0]);
            L<I> bot  (omega[1]);
            L<I> ztop(r[top]);
            L<J> sr  (find(e, pack_pairs(ztop, x[bot])));
            for (index_t j = 0; j < bot.size(); ++j)
                i[bot[j]] = I(I::rep(J::rep(sr[j])));
            L<I> not_found(&(i[bot] == I::rep(e.size())));
            for (I j: not_found) d[bot[j]]++;
            return L<O>{ztop[not_found], bot[not_found]};
        };
        // At the bottom of p. 140, Hsu says using r[b] instead of b is the
        // right way. I must be doing something different, because using b is
        // getting the result I want, and r[b] isn't.
        O not_found(converge_(closest_binding, L<O>{cat(v,b), O(tili(x.size()))}));
        L<I> f(frames["frame"_s]);
        L<I> fi(at(f, i));
        L<I> si(at(slots, i));
        L<I> dd(r.size(), NI);
        L<I> ff(r.size(), NI); // try NI instead of -1
        L<I> ss(r.size(), NI);
        for (index_t j = 0; j < i.size(); ++j) {
            dd[y[j]] = d [j];
            ff[y[j]] = fi[j];
            ss[y[j]] = si[j];
        }

        /* top of p. 140; we don't do, because the info he writes into n we
         * already have in frame, slot, and distance.  Plus, we want to
         * preserve the name in n.
        if (b.size()) {
            // If we did keep this, then When we disallow rebinding, it would
            // simplify to the following:
            // DO(b) deref(std::exchange(n[b[i_]], make_atom(I(i_))));

            // rn is (contour, data) for each binding
            L<J> rn(pack_pairs(r[b].get(), L<S>(at(n.get(), b.get())).get()));
            L<I> bs(slots[L<J>(find(e.get(), rn.get()))]);
            DO(b) deref(std::exchange(n[b[i_]], make_atom(bs[i_])));
        }*/

        // Stick each lambda's frame size into the frame cell for the
        // lambda's row.  TODO consider doing the same for module.
        DO(frame_bumps) ff[frame_bumps.key()[i_]] = frame_bumps.val()[i_];
        ast.add("frame"_s   , ff);
        ast.add("slot"_s    , ss);
        ast.add("distance"_s, dd);
    }
} // unnamed

O compile(O orig_ast_, bool trace) {
    assert(orig_ast_.is_table());
    if (trace) H(1) << orig_ast_ << '\n' << flush;
    // TODO clone the columns so the original stays intact
    KV<S,O> orig_ast(+orig_ast_);

    // 3.3 (81) Converting from Depth Vector to Parent Vector
    L<S> k{"type"_s, "node"_s, "parent"_s};
    L<O> v{orig_ast["type"_s],
           orig_ast["node"_s],
           O(parent(L<X>(orig_ast["depth"_s])))};
    // 3.4 (89) Computing the Nearest Lexical Contour
    KV<S,O> ast(UKV(k, v));
    ast.add("contour"_s, lexical_contour(ast)); // called r in Hsu
    // 3.5 (96) Lifting Functions
    O ast_(lift_functions(+ast));
    // We'll need 3.6 and 3.7 when we implement $-switch
    // 3.8 (119) Counting Rank of Index Operations (I just count all of them)
    count_kids(ast_);
    if (trace) H(1) << ast_ << '\n' << flush;

    // 3.9 (122) Flattening Expressions
    O new_ast(flatten_expressions(ast_));
    if (trace) H(1) << new_ast << '\n' << flush;
    // 3.10-13 Frames and Lexical Resolution
    auto [f, fb] = stack_frames(new_ast);
    KV<I,I> frame_bumps(fb);
    if (trace) H(1) << f << '\n' << flush;
    resolve(new_ast, f, frame_bumps);
    if (trace) H(1) << new_ast << '\n' << flush;
    return new_ast;
}
