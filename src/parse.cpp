#include <parse.h>
#include <adverb.h>
#include <algorithm>
#include <ast.h>
#include <binary2unary.h>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <exception.h>
#include <indexof.h>
#include <iterator>
#include <lcutil.h>
#include <qisslex.h>
#include <o.h>
#include <objectio.h>
#include <opcode.h>
#include <primio.h>
#include <sym.h>
#include <token.h>
#include <uniform.h>
#include <utility>

#ifndef NDEBUG
#include <format.h>
#include <terminal_width.h>
#endif

namespace {
    template <class X> using OT = ObjectTraits<X>;

    Adverb text2adverb(L<C> s) {
        assert(s.size() == 1 || s.size() == 2);
        if (s[0] == C('/'))
            return s.size() == 1? Adverb::over : Adverb::eachR;
        if (s[0] == C('\\'))
            return s.size() == 1? Adverb::scan : Adverb::eachL;
        assert(s[0] == C('\''));
        return s.size() == 1? Adverb::each : Adverb::prior;
    }

    struct Parser {
#ifdef NDEBUG
        explicit Parser(L<S> infix_words);
#else
        enum Trace { NOTRACE, TRACE };
        explicit Parser(L<S> infix_words, Trace trace_=NOTRACE);
#endif

        struct TokenInfo {
            const char* text;
            int         line;
            Token       type;
            Object*     value;
        };

        void push_back(TokenInfo t);
        O    parse    ();

    private:
        L<S>    infix_words;
        L<J>    line;
        L<L<C>> text;
        L<C>    type;
        L<O>    val;
        index_t current = 0;

        bool  is_infix(S x) const;
        bool  are_uniform_literals(index_t first, index_t last);
        void  consume();
        void  expect(Token t);
        bool  match(Token t);
        bool  more() const;
        bool  peek(Token t, int lookahead=0) const;
        enum  Infix { INFIX };
        bool  peek(Infix) const;
        enum  Verb { VERB };
        bool  peek(Verb) const;

#ifndef NDEBUG
        Token curr() const;
//        void debug_print();
        void enter(const char* rule) const;
        bool leave(const char* rule, bool result);
        index_t indent;
        Trace   trace;
#endif

        bool exprs  (index_t* n=nullptr);
        bool expr   ();
        bool formals(index_t start);
        bool lambda ();
        bool noun   ();
        bool term   ();
        bool verb   ();

        bool dict_literal(index_t start);
        void implicit_formals(index_t start);
        bool table_literal(index_t start);

        void deepen(index_t first, index_t last, X amount=X(1));
        void lift(index_t first, index_t last, X amount=X(1));
        void pop();
        void push_back(Opcode op);
        void push_back(O node, Ast type);
        void rotate(index_t first, index_t middle);
        void trunc(index_t last);

        // These are all parallel.
        // See the top of page 60 (and also page 70) of Hsu's thesis.
        // TODO add a ref column, have a separate list for things referred to,
        // make nodes L<I>, add a parallel L<I> token indexing the lexer output
        // lists above (line, text, type, and val).  What happened to the
        // token columns?
        L<X> depth;
        L<O> nodes;    // In Hsu, name or reference data
        L<X> types;
        // L<X> kinds; // aka sub-types.  Why separate types and sub-types?
                       // So we can search for a class of nodes faster.
};

#ifdef NDEBUG
#define ENTER(RULE)   const index_t  save__  = current;     \
                      const index_t  start__ = nodes.size()
#define LEAVE(RESULT) do { const auto r = (RESULT);                      \
                           if (!r) { current = save__; trunc(start__); } \
                           return r; } while (false)

    Parser::Parser(L<S> iw): infix_words(iw) {}
#else
    struct Indenter {
        explicit Indenter(index_t& i_): i(i_) { i += 4; }
        ~Indenter() { i -= 4; }
        index_t& i;
    };
    const char red    [] = "\033[30;48;5;196m";
    const char orange [] = "\033[30;48;5;202m";
    const char gold   [] = "\033[30;48;5;214m";
    const char myellow[] = "\033[30;48;5;220m";
    const char lyellow[] = "\033[30;48;5;229m";
    const char lgreen [] = "\033[30;48;5;193m";
    const char mgreen [] = "\033[30;48;5;34m";
    const char bgreen [] = "\033[30;48;5;154m";
    const char xgreen [] = "\033[30;48;5;157m";
    const char lcyan  [] = "\033[30;48;5;159m";
    const char lblue  [] = "\033[30;48;5;195m";
    const char sblue  [] = "\033[30;48;5;117m";
    const char mblue  [] = "\033[30;48;5;111m";
    const char purple [] = "\033[30;48;5;165m";
    const char pink   [] = "\033[30;48;5;207m";
    const char mauve  [] = "\033[30;48;5;219m";
    const char reset  [] = "\033[0m";
    const char* stripes[] = {
        red, orange, gold, myellow, lyellow, lgreen, mgreen, bgreen,
        xgreen, lcyan, lblue, sblue, mblue, purple, pink, mauve
    };
    L<I>& color_stack() {
        static L<I> cs;
        return cs;
    }

#define ENTER(RULE)   const index_t     save__  = current;      \
                      const index_t     start__ = nodes.size(); \
                      const char* const rule__  = #RULE;        \
                      const Indenter indenter__(indent);        \
                      enter(rule__);
#define LEAVE(RESULT) do { const auto r = (RESULT);                      \
                           if (!r) { current = save__; trunc(start__); } \
                           return leave(rule__, r); } while (false)

    Token Parser::curr() const {
        assert(more());
        return Token(C::rep(type[current]));
    }

/*    void Parser::debug_print() {
        L<S> k{"type"_s, "node"_s, "depth"_s};
        L<O> v{O(types)   , O(nodes)   , O(depth)};
        O t(make_table(make_dict(k.release(), v.release())));
        L<C> s;
        s << t.get();
        const L<C> i(indent, ' ');
        H(1) << i;
        for (C c: s) {
            H(1) << c;
            if (c == C('\n')) H(1) << i;
        }
        H(1) << '\n' << flush;
    }*/

    void Parser::enter(const char* rule) const {
        if (trace) {
            static I::rep stripe = 0;
            const int tw = terminal_width();
            L<C> s;
            if (8 <= indent) s << L<C>(indent - 8, ' ') << " >> ";
            s << rule;
            if (more()) s << ": " << curr() << ' ' << text[current];
            H(1) << stripes[stripe] << left(tw, s) << reset << '\n' << flush;
            color_stack().emplace_back(stripe);
            stripe = (stripe + 1) % I::rep(sizeof stripes / sizeof *stripes);
        }
    }

    bool Parser::leave(const char* rule, bool result) {
        if (trace) {
            const int stripe(color_stack().pop());
            const int tw = terminal_width();
            L<C> s;
            if (8 <= indent) s << L<C>(indent - 8, ' ') << " << ";
            s << rule;
            if (result) s << ": " << types.back() << ' ' << nodes.back();
            if (s.size() + 4 <= tw)
                s << L<C>(tw - s.size() - 4, ' ') << " :-" << (result? ')' : '/');
            H(1) << stripes[stripe] << left(tw, s) << reset << '\n' << flush;
        }
        return result;
    }

    Parser::Parser(L<S> iw, Trace trace_): infix_words(iw), indent(0), trace(trace_)
    {}
#endif

    void Parser::push_back(TokenInfo t) {
        line.emplace_back(t.line);
        text.emplace_back(t.text, t.text + strlen(t.text));
        type.emplace_back(t.type);
        val .emplace_back(t.value? t.value : generic_null());
    }

    void Parser::consume() {
        match(Token(C::rep(type[current])));
    }

    void Parser::expect(Token t) {
        if (!match(t)) throw Exception("parse failed");
    }

    bool Parser::is_infix(S x) const {
        return infix_words.size() != indexof(infix_words, x);
    }

    bool Parser::are_uniform_literals(index_t first, index_t last) {
        const auto is_lit = [] (X t){ return t == X(X::rep(Ast::lit)); };
        return are_uniform_atoms(nodes.begin() + first, nodes.begin() + last)
            && std::all_of(types.begin() + first, types.begin() + last, is_lit);
    }

    bool Parser::match(Token t) {
        if (!peek(t)) return false;
        --current;
        return true;
    }

    bool Parser::more() const {
        return 0 <= current;
    }

    bool Parser::peek(Token t, int lookahead) const {
        return lookahead <= current && type[current - lookahead] == C(t);
    }

    bool Parser::peek(Verb) const {
        return peek(OPERATOR) || peek(ADVERB) || peek(INFIX);
    }

    bool Parser::peek(Infix) const {
        return peek(IDENTIFIER) && is_infix(val[current]->s);
    }

    O Parser::parse() {
        if (type.size()) {
            current = type.size() - 1;
            if (!exprs()) return O(generic_null());
            deepen(0, nodes.size());
            push_back(O(generic_null()), Ast::module);

            std::reverse(depth.begin(), depth.end());
            std::reverse(nodes.begin(), nodes.end());
            std::reverse(types.begin(), types.end());
        }

        L<S> k{"type"_s, "node"_s, "depth"_s};
        L<O> v{O(types), O(nodes), O(depth)}; // TODO consider std::move
        return O(make_table(make_dict(k.release(), v.release())));
    }

    void Parser::deepen(index_t first, index_t last, X amount) {
        // There must be some trick to make this efficient.
        // Using height instead of depth and fix up at the end?

        // auto me = *std::max_element(depth.begin() + m, depth.end());
        // std::transform(depth.begin() + di, depth.begin() + m,
        //                depth.begin() + di, [=](X x){ return x + me - h; });
        // depth[di] = depth[di] + me - h;

        std::transform(depth.begin() + first, depth.begin() + last,
                       depth.begin() + first, [=](X x) { return x + amount; });
    }

    void Parser::lift(index_t first, index_t last, X amount) {
        std::transform(depth.begin() + first, depth.begin() + last,
                       depth.begin() + first, [=](X x) { return x - amount; });
    }

    bool Parser::dict_literal(index_t start) {
        // We expect a series of assignments from start to size.
        // We transform them into a symlist!value

        ENTER(dict_literal);

        // ncd is the depth at which we know we've reached the next binding.
        const X ncd = depth.back();
        const index_t last = nodes.size() - 1;
        L<S> cnames;
        index_t target = start;
        for (index_t i = start; i < last; ++i) {
            if (ncd == depth[i]) {
                if (types[i] != Ast::bind) LEAVE(false);
                cnames.push_back(nodes[i]->s);
            } else {
                depth[target] = depth[i];
                types[target] = types[i];
                nodes[target] = nodes[i];
                ++target;
            }
        }
        cnames.push_back(nodes[last]->s);
        std::reverse(cnames.begin(), cnames.end());
        trunc(target);
        if (are_uniform_literals(start, nodes.size())) {
            O lit(uniform(std::reverse_iterator(nodes.end()),
                          std::reverse_iterator(nodes.begin() + start)));
            trunc(start);
            push_back(lit, Ast::lit);
        } else {
            const X parts_depth = depth.back();
            if      (parts_depth < X(2)) deepen(start, target, X(2) - parts_depth);
            else if (parts_depth > X(2)) lift(start, target, depth.back() - 1);
            push_back(O(I(I::rep(cnames.size()))), Ast::list);
        }
        depth.back() = X(1);
        push_back(std::move(cnames), Ast::lit);
        depth.back() = X(1);
        push_back(Opcode::bang);
        LEAVE(true);
    }

    bool Parser::expr() {
        ENTER(expr);
        const auto done = [this](){ return !more() ||
            peek(SEMICOLON) || peek(LPAREN) || peek(LBRACE) || peek(LBRACKET);
        };
        if (done()) { // epsilon alternate
            push_back(O(generic_null()), Ast::hole);
            LEAVE(true);
        }

        const index_t k = nodes.size();
        if (peek(VERB)) {
            if (!verb()) LEAVE(false);
            if (!done() && !peek(VERB)) { // partial application of verb, e.g., 3+
                const index_t m = nodes.size();
                push_back(O(generic_null()), Ast::hole);
                if (!noun()) LEAVE(false);
                deepen(m, nodes.size());
                rotate(k, m);
            }
        }
        else if (!noun()) LEAVE(false);

        // TODO support : suffix syntax to force unary?
        while (!done()) {
            const index_t m = nodes.size();
            if (peek(VERB)) {
                deepen(k, m);
                if (!verb()) LEAVE(false);
                if (done()) {
                    if (types.back() == Ast::op)
                        nodes.back()->op = binary2unary(nodes.back()->op);
                } else {

                    // We just saw a verb so, by defn, it could be binary.
                    // However, when the picture is VVN (ie we have another
                    // verb to our left), and the current (middle) verb is an
                    // operator (or each'd operator) other than @ or . we
                    // want to treat the verb in the middle of VVN as unary.
                    // We do the same for {}VN since the only operators where
                    // {} could be a sensible lhs are , and ^ and there is a
                    // work-around: give the {} a name.

                    const index_t n = nodes.size();
                    bool all_each = true;
                    index_t i = types.size() - 1;
                    for (; k <= i && types[i] == Ast::adverb; --i)
                        all_each &= nodes[i].atom<X>() == X(X::rep(Adverb::each));
                    if (k <= i && types[i] == Ast::op) {
                        const Opcode op = nodes[i].atom<Opcode>();
                        if (op != Opcode::at && op != Opcode::dot &&
                            (peek(VERB) || peek(RBRACE)))
                        {
                            if (all_each) nodes[i]->op = binary2unary(op);
                            continue;
                        }
                    }

                    // NVE alternate
                    if (!noun()) LEAVE(false);
                    if (types.back() == Ast::id && types[types.size()-2] == Ast::op
                     && nodes[nodes.size()-2].atom<Opcode>() == Opcode::assign)
                    {
                        // Hsu uses a special ast node type (binding) to
                        // facilitate associating frame slots with variables (
                        // could it be made to work with only the lexical
                        // contour?) . However, : is a function that can be
                        // passed (and often is to 4-ary at), so we can't just
                        // set all : nodes to Ast::bind. Changing assign nodes
                        // to bind nodes also affects dict and table literal
                        // parsing.  but that was easily fixed. We combine the
                        // id node with the bind node.
                        O id(nodes.back());
                        pop();
                        types.back() = X(X::rep(Ast::bind));
                        nodes.back() = id;
                    }
                    deepen(n, depth.size());
                    rotate(m, n);
                }
            } else { // TE alternate
                if (!noun()) LEAVE(false);
                if (types.back() != Ast::formals) {
                    if (types.back() == Ast::export_)
                        deepen(k, nodes.size() - 1);
                    else {
                        deepen(k, nodes.size());
                        push_back(O(generic_null()), Ast::juxt);
                    }
                }
            }
        }

        LEAVE(true);
    }

    bool Parser::exprs(index_t* n) {
        ENTER(exprs);
        if (!expr()) LEAVE(false);
        if (!more() || peek(LPAREN) || peek(LBRACE) || peek(LBRACKET)) {
            if (n) *n = 1;
            LEAVE(true);
        }

        index_t n_ = 1;
        for (; match(SEMICOLON); ++n_) if (!expr()) LEAVE(false);
        if (n) *n = n_;
        LEAVE(true);
    }

    bool Parser::formals(index_t start) {
        assert (start < nodes.size());
        ENTER(formals);
        if (nodes.size() - start == 1) {
            L<S> formals;
            if (types.back() == Ast::hole) ; // empty formals is what we want
            else if (types.back() == Ast::id) formals = L<S>{nodes.back()->s};
            else LEAVE(false);
            nodes.back() = formals;
            types.back() = X(X::rep(Ast::formals));
        } else if (8 < nodes.size() - start) {
            throw Exception("limit: functions have a max of 8 parameters");
        }
        else {
            L<S> formals(nodes.size() - start);
            for (index_t i = 0, j = nodes.size() - 1; j >= start; ++i, --j) {
                if (types[j] != Ast::id) LEAVE(false);
                formals[i] = nodes[j]->s;
            }
            trunc(start);
            push_back(formals, Ast::formals);
        }
        LEAVE(true);
    }

    void Parser::implicit_formals(index_t start) {
        L<S> xyz{"x"_s, "y"_s, "z"_s};
        index_t mif = -1; // max_implicit_formal
        for (index_t i = types.size() - 1; i >= start; --i) {
            if (types[i] == Ast::id) {
                const auto it = std::find(xyz.begin(), xyz.end(), nodes[i]->s);
                if (it != xyz.end())
                    mif = std::max(mif, index_t(it - xyz.begin()));
            } else if (types[i] == Ast::lambda) {
                // TODO allow a local function with explicit formals to capture
                // - and thus create - my implicit formals?  It's clearly not
                // necessary, and I think it would be confusing, but consider,
                // e.g., K:{{[b]x}} vs K:{[a]{[b]a}}
                const X d = depth[i] + 1;
                i -= 3;
                while (i >= start && depth[i] > d) --i;
                if (start <= i) ++i; // Hack to handle --i in for
            }
        }
        xyz.trunc(mif + 1);
        push_back(xyz, Ast::formals);
    }

    bool Parser::lambda() {
        ENTER(lambda);
        const index_t k = nodes.size();
        consume();
        if (!exprs()) LEAVE(false);
        expect(LBRACE);
        if (types.back() != Ast::formals) implicit_formals(k);
        deepen(k, depth.size() - 1);
        types.back() = X(X::rep(Ast::lambda));
        LEAVE(true);
    }

    bool Parser::noun() {
        ENTER(noun);
        switch (C::rep(type[current])) {
        case IDENTIFIER: push_back(O(sym(text[current--])), Ast::id); break;
        case RBRACE    : LEAVE(lambda());
        case RBRACKET  : {
            consume();
            const index_t k = nodes.size();
            index_t kids;
            if (!exprs(&kids)) LEAVE(false);
            // TODO ? detect $-switch and desugar $[a;b;c;d;e] -> $[a;b;$[c;d;e]]
            // Issue: that would make error reporting worse
            expect(LBRACKET);
            if (types.back() == Ast::bind) LEAVE(dict_literal(k));
            if (peek(LBRACE)) LEAVE(formals(k));
            if (peek(LPAREN)) LEAVE(true); // table lit or parenthesized dict lit
            if (kids == 1 && types.back() == Ast::hole) { // nullary invocation
                pop();
                kids = 0;
            }
            if (!term()) LEAVE(false);
            if (types.back() == Ast::op && nodes.back()->op == Opcode::cast && kids >= 3) {
                if (kids % 2 == 0) throw Exception("$ (switch) requires odd # of args");
                nodes.back() = O(I(I::rep(kids)));
                types.back() = X(X::rep(Ast::cond));
                deepen(k, nodes.size() - 1);
            }
            else {
                deepen(k, nodes.size());
                push_back(O(I(I::rep(kids))), Ast::apply);
            }
            LEAVE(true);
            }
        case RPAREN:
            consume();
            if (peek(LPAREN))
                push_back(L<O>{}, Ast::lit);
            else {
                const index_t k = nodes.size();
                index_t kids;
                if (!exprs(&kids)) LEAVE(false);
                if (kids > 1)  {
                    if (!are_uniform_literals(k, nodes.size())) {
                        deepen(k, nodes.size());
                        push_back(O(I(I::rep(kids))), Ast::list);
                    } else {
                        O lit(uniform(std::reverse_iterator(nodes.end()),
                                      std::reverse_iterator(nodes.begin() + k)));
                        trunc(k);
                        push_back(lit, Ast::lit);
                    }
                }
                if (peek(LBRACKET, -1)) {
                    if (!table_literal(k)) LEAVE(false);
                }
            }
            expect(LPAREN);
            break;
        case KEYWORD: {
            const S keyword = sym(text[current--]);
            if (keyword == "export"_s) {
                // TODO enforce that export can only be at the top level scope
                if (types.back() != Ast::bind)
                    throw Exception("export must be followed by a binding");
                push_back(nodes.back(), Ast::export_);
            } else {
                // TODO we need to undo the whole expression
                // and start over with a parser for the query grammar
                throw Exception("nyi: queries");
            }
            break;
            }
        default:
            push_back(val[current--], Ast::lit);
        }
        LEAVE(true);
    }

    void Parser::pop() {
        depth.pop_back();
        nodes.pop_back();
        types.pop_back();
    }

    void Parser::push_back(Opcode op) {
        push_back(O(op), Ast::op);
    }

    void Parser::push_back(O node, Ast node_type) {
        depth.emplace_back(X::rep(0));
        nodes.emplace_back(std::move(node));
        types.emplace_back(X::rep(node_type));
    }

    void Parser::rotate(index_t first, index_t middle) {
        std::rotate(depth.begin() + first, depth.begin() + middle, depth.end());
        std::rotate(nodes.begin() + first, nodes.begin() + middle, nodes.end());
        std::rotate(types.begin() + first, types.begin() + middle, types.end());
    }

    bool Parser::table_literal(index_t start) {
        // At this point in the parsing process, each value column is
        // represented as an assignment expression in an Ast::list (unless
        // there is only one value column, in which case there's no Ast::list
        // node). The first column is weird, because expr interprets ([]a:...)
        // as a juxtaposition of the [] being applied to a.  Meanwhile, if
        // there are key columns, they have already been transformed via
        // dict_literal.  So, these are the shapes:

        //                                            key depth      value depth
        // juxt                                       NA             1
        //     hole           no key columns
        //     assign         sole value column
        //
        // juxt                                       1              1
        //     bang           key columns
        //     assign         sole value column
        //
        // list                                       NA             2 -> 1
        //    juxt
        //        hole        no key columns
        //        assign      1st value column
        //    assign          2nd value column
        //    assign          3rd value column
        //    ..
        //
        // list                                       2              2 -> 1
        //    juxt
        //        bang        key columns
        //        assign      1st value column
        //    assign          2nd value column
        //    assign          3rd value column
        //    ..

        // Lastly, if there is no juxt, we have a dict literal in parens:
        // ([a:1])        single value => no list node
        // ([a:1;b:2])    multi-valued => list node

        // Overall strategy is
        //     Remove any key column dict from the stacks
        //         and place in a temporary set of stacks
        //     Compute the value table
        //     If there is a key column dict,
        //         Put the key column dict back
        //         Flip it
        //         Key the two tables

        ENTER(table_literal);

        auto build_value_table = [&](){
            // Since  ([]a:..) is parsed (so far) as TE (i.e., applying [] to a),
            // the first column is too deep by one compared to the rest of the
            // columns.  Thus, we lift it.
            index_t fcs = nodes.size() - 2; // 1st column subtree
            while (start < fcs && depth.back() < depth[fcs]) --fcs;;
            lift(fcs + (0<fcs), nodes.size());
            dict_literal(start);
            deepen(start, nodes.size());
            push_back(Opcode::flip);
        };

        const bool mvc = types.back() == Ast::list; // multiple value columns
        if (mvc) pop();

        if (types.back() == Ast::juxt)
            pop();
        else {
            // Either a parenthesized dict literal (already parsed),
            // or a spurious ; as in ([k:`p`q`r];a:1 2 3)
            const auto it = std::find_if(depth.begin() + start, depth.end(),
                                         [&](X d){ return d <= depth.back(); });
            if (it + 1 == depth.end())
                LEAVE(true); // dict literal already parsed
            LEAVE(false); // spurious ; (k forgives this, we don't)
        }

        if (types.back() == Ast::hole) { // no key columns
            pop();
            build_value_table();
            LEAVE(true);
        }
        if (types.back() != Ast::op || nodes.back()->op != Opcode::bang)
            LEAVE(false);

        // Move key columns dict out of the way.  Note that the key columns are 1
        // or 2 levels too deep depending on the shape (determined by whether we
        // have a single value column or multiple value columns).
        index_t i = nodes.size() - 2;
        while (depth[i-1] != depth.back()) --i;
        L<X> kdepth(depth.size() - i);
        std::transform(depth.begin() + i, depth.end(), kdepth.begin(),
                       [=](X d) { return d - (mvc? 2 : 1); });
        L<O> knodes(nodes.begin() + i, nodes.end());
        L<X> ktypes(types.begin() + i, types.end());
        trunc(i);

        build_value_table();

        // Restore the key columns dict and flip it
        const index_t n = nodes.size();
        std::copy(kdepth.begin(), kdepth.end(), std::back_inserter(depth));
        std::copy(knodes.begin(), knodes.end(), std::back_inserter(nodes));
        std::copy(ktypes.begin(), ktypes.end(), std::back_inserter(types));
        deepen(n, nodes.size());
        push_back(Opcode::flip);

        // Key the two tables together
        deepen(start, nodes.size());
        push_back(Opcode::bang);
        LEAVE(true);
    }

    bool Parser::term() {
        ENTER(term);
        if (!more()) LEAVE(false);
        if (peek(VERB)) LEAVE(verb());
        LEAVE(noun());
    }

    void Parser::trunc(index_t last) {
        depth.trunc(last);
        nodes.trunc(last);
        types.trunc(last);
    }

    bool Parser::verb() {
        ENTER(verb);
        if (peek(ADVERB)) {
            const index_t k = nodes.size();
            X d(0);
            for (; peek(ADVERB); d = d + 1) {
                const X adverb(X::rep(text2adverb(text[current--])));
                push_back(O(adverb), Ast::adverb);
                depth.back() = d;
            }
            const index_t m = nodes.size();
            if (!term()) LEAVE(false);
            deepen(m, depth.size());
            std::reverse(depth.begin() + k, depth.begin() + m);
            std::reverse(nodes.begin() + k, nodes.begin() + m);
            std::reverse(types.begin() + k, types.begin() + m);
            rotate(k, m);
            LEAVE(true);
        } else if (peek(OPERATOR)) {
            push_back(val[current--], Ast::op);
            LEAVE(true);
        } else if (peek(INFIX)) {
            push_back(val[current--], Ast::infix);
            LEAVE(true);
        }
        LEAVE(false);
    }
} // unnamed

O parse(L<C> text, L<S> infix_words, bool trace) {
#ifdef NDEBUG
    if (trace) throw Exception("Cannot trace parser unless compiled w/DEBUG");
    Parser parser(infix_words);
#else
    Parser parser(infix_words, trace? Parser::TRACE : Parser::NOTRACE);
#endif

    L<C> s(text);
    StringLexer lex(text);
    Token token;
    while ((token = lex.next()) && token != QUIT)
        parser.push_back({lex.text(), lex.line(), token, yylval});
    return parser.parse();
}
