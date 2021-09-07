#pragma once

#include <algorithm>
#include <in.h>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <not.h>
#include <o.h>
#include <relop.h>
#include <utility>
#include <where.h>

template <class Z> struct [[nodiscard]] L {
    template <class X> using OT = ObjectTraits<X>;

    using value_type             = Z;
    using iterator               = value_type*;
    using const_iterator         = const value_type*;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using size_type              = index_t;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    explicit L(Object* x_): x(x_) {
        assert(x); assert(x->type == OT<value_type>::typet());
    }
    explicit L(O x_): L(x_.release()) {}
    explicit L(List<value_type>* x_): x(x_) {
        assert(x);
        if constexpr(is_prim_v<value_type>)
            assert(x->type == ObjectTraits<value_type>::typet());
        else
            assert(x->type == ObjectTraits<O>::typet());
    }
    explicit L(size_type n = 0): L(make_list<value_type>(n)) {}
    template <class R> requires std::is_constructible_v<value_type, R>
    L(size_type n, R r): L(make_empty_list<value_type>(n)) {
        std::fill_n(std::back_inserter(*this), n, value_type(r));
    }
    template <class R>
    explicit L(std::initializer_list<R> x_): L(x_.begin(), x_.end()) {}
    template <class I>
        requires std::is_constructible_v<value_type, decltype(*std::declval<I>())>
    L(I first, I last): L(make_empty_list<value_type>(std::distance(first, last)))
    {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#elif defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif
        std::transform(first, last, std::back_inserter(*this),
                       [](const auto& e){ return value_type(e); });
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUG__)
#pragma GCC diagnostic pop
#endif
    }
    L(const L& x_): x(addref(x_.x)) {}
    L(L&& x_) noexcept: x(x_.release()) {}
    ~L() { deref(x); }
    L& operator=(L x_) { std::swap(x, x_.x); return *this; }

    operator O()       &  { return O(addref(x)); }
    operator O() const &  { return O(L(*this).release()); }
    operator O()       && { return O(release()); }

    size_type capacity () const { return list_capacity(get()); }
    bool      empty    () const { return size() == 0; }
    bool      is_sorted() const { return (x->a & Attr::sorted) != Attr::none; }
    bool      mine     () const { return !x->r; }
    size_type size     () const { return x->n; }

    reference operator[](size_type i) {
        assert(0 <= i && i < size());
        return begin()[i];
    }
    const_reference operator[](size_type i) const {
        return (*const_cast<L*>(this))[i];
    }
    reference       operator[](I i)       { return (*this)[I::rep(i)]; }
    const_reference operator[](I i) const { return (*this)[I::rep(i)]; }
    reference       operator[](J i)       { return (*this)[J::rep(i)]; }
    const_reference operator[](J i) const { return (*this)[J::rep(i)]; }
    reference       operator[](X i)       { return (*this)[X::rep(i)]; }
    const_reference operator[](X i) const { return (*this)[X::rep(i)]; }
    template <class I> L operator[](L<I> j) const {
        L r;
        r.reserve(j.size());
        std::transform(j.begin(), j.end(), std::back_inserter(r),
                       [&](I k){ return (*this)[k]; });
        return r;
    }

    L<B> operator!() &  { return L<B>(not_(*this)); }
    L<B> operator!() && { return L<B>(not_(std::move(*this))); }
    L<I> operator&() &  { return L<I>(wherei(*this)); }
    L<I> operator&() && { return L<I>(wherei(std::move(*this))); }
    L<B> in(L y) &  { return L<B>(::in(*this           , std::move(y))); }
    L<B> in(L y) && { return L<B>(::in(std::move(*this), std::move(y))); }

    iterator       begin  ()       { return get()->begin(); }
    iterator       end    ()       { return begin() + size(); }
    const_iterator begin  () const { return const_cast<L*>(this)->begin(); }
    const_iterator end    () const { return const_cast<L*>(this)->end  (); }
    const_iterator cbegin () const { return const_cast<L*>(this)->begin(); }
    const_iterator cend   () const { return const_cast<L*>(this)->end  (); }

    reverse_iterator       rbegin ()       { return reverse_iterator(end()); }
    reverse_iterator       rend   ()       { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin () const { return const_cast<L*>(this)->rbegin(); }
    const_reverse_iterator rend   () const { return const_cast<L*>(this)->rend  (); }
    const_reverse_iterator crbegin() const { return const_cast<L*>(this)->rbegin(); }
    const_reverse_iterator crend  () const { return const_cast<L*>(this)->rend  (); }

    reference       front()       { assert(size()); return *begin(); }
    reference       back ()       { assert(size()); return *(end() - 1); }
    const_reference front() const { assert(size()); return *begin(); }
    const_reference back () const { assert(size()); return *(end() - 1); }

    template <class I> 
        requires std::is_constructible_v<value_type, decltype(*std::declval<I>())>
    void append(I first, I last) {
        const size_type n = std::distance(first, last);
        reserve(size() + n);
        iterator it = end();
        for (; first != last; ++first, ++it) new(it) value_type(*first);
        get()->n += n;
    }

    void clear() {
        std::destroy(begin(), end());
        get()->n = 0;
    }
    value_type pop() {
        assert(size());
        const value_type r(std::move(back()));
        pop_back();
        return r;
    }
    void pop_back() {
        assert(size());
        back().~value_type();
        --get()->n;
    }
    template <class... A> requires std::is_constructible_v<value_type, A...>
    reference emplace_back(A&&... a) {
        reserve(size() + 1);
        new (end()) value_type(std::forward<A>(a)...);
        ++get()->n;
        return back();
    }
    void push_back(value_type e) {
        reserve(size() + 1);
        new(end()) value_type(std::move(e));
        ++get()->n;
    }
    void reserve(size_type sz) { x = grow_list(get(), uint64_t(sz)); }
    void reserve(I         sz) { reserve(I::rep(sz));  }
    void reserve(J         sz) { reserve(J::rep(sz));  }
    L& trunc(size_type sz) {
        assert(0 <= sz);
        const index_t new_n = std::min(x->n, sz);
        std::destroy(begin() + new_n, end());
        get()->n = new_n;
        return *this;
    }

    friend void swap(L& a, L& b) { std::swap(a.x, b.x); }

    List<value_type>*       get    ()       { return static_cast<List<value_type>*>(x); }
    const List<value_type>* get    () const { return static_cast<const List<value_type>*>(x); }
    List<value_type>*       release()       { return static_cast<List<value_type>*>(std::exchange(x, generic_null())); }

private:
    Object* x;
    friend struct O;
};

template <class X> L<X> operator&(L<X> x, L<X> y) {
    return L<X>(min(std::move(x), std::move(y)));
}

template <class X> L<X> operator|(L<X> x, L<X> y) {
    return L<X>(max(std::move(x), std::move(y)));
}
