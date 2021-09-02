#pragma once

#include <algorithm>
#include <cstdint>
#include <key.h>
#include <o.h>
#include <type_traits>
#include <utility>

template <class Z>
bool is_less_(const L<Z>& x, const L<Z>& y) {
    return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
}

bool is_less(O x, O y);

template <class Z, class COMP>
L<J> compare_sort(const L<Z>& x, COMP comp) {
    L<J> i(til(x.size()));
    std::stable_sort(i.begin(), i.end(), comp);
    return i;
}

// TODO Can we use std::exclusive_scan?
template <class T, std::size_t N>
void prev_partial_sum(T(&a)[N]) {
    T total(0);
    for (std::size_t i = 0; i < N; ++i) total += std::exchange(a[i], total);
}

template <class Z>
auto safe_subtract(Z x, Z y) {
    using ZR = typename Z::rep;
    using UR = std::make_unsigned_t<ZR>;
    return UR(ZR(x)) + (UR(-1)^UR(ZR(y))) + 1;
}

template <class CT, class BIAS> // type to hold element CounTs
L<J> radix_sort(const L<B>& x, BIAS bias) {
    CT counts[2] = {};
    for (const B& e: x) ++counts[bias(e)];
    prev_partial_sum(counts);     
    L<J> j(x.size());
    for (index_t i = 0; i < x.size(); ++i) j[counts[bias(x[i])]++] = J(i);
    return j;
}

template <class CT, class Z, class BIAS>
L<J> radix_sort0(const L<Z>& x, BIAS bias) {
    CT counts0[256] = {};
    for (const Z& e: x) ++counts0[bias(e)];
    prev_partial_sum(counts0);
    L<J> j(x.size());
    for (index_t i = 0; i < x.size(); ++i) j[counts0[bias(x[i])]++] = J(i);
    return j;
}

template <class Z, class BIAS>
L<J> radix_sort0(const L<Z>& x, BIAS bias) {
    return x.size() < 1LL<< 8? radix_sort0<uint8_t >(x, bias)
         : x.size() < 1LL<<16? radix_sort0<uint16_t>(x, bias)
         : x.size() < 1LL<<32? radix_sort0<uint32_t>(x, bias)
         : /* else */          radix_sort0<index_t >(x, bias);
}

template <class CT, class Z, class BIAS>
L<J> radix_sort1i(const L<Z>& x, BIAS bias) {
    CT counts0[256] = {};
    CT counts1[256] = {};
    for (const Z& e: x) {
        ++counts0[0xffull & bias(e) >> 0ull];
        ++counts1[0xffull & bias(e) >> 8ull];
    }
    prev_partial_sum(counts0);
    prev_partial_sum(counts1);
    L<J> r(x.size());
    int32_t* const j = reinterpret_cast<int32_t*>(r.begin());
    int32_t* const k = j + x.size();
    for (int32_t i = 0; i < x.size(); ++i) j[counts0[0xff & bias(x[i]   ) >> 0ull]++] = i;
    for (int32_t i = 0; i < x.size(); ++i) k[counts1[0xff & bias(x[j[i]]) >> 8ull]++] = j[i];
    for (int32_t i = 0; i < x.size(); ++i) r[i] = J(k[i]);
    return r;
}

template <class CT, class Z, class BIAS>
L<J> radix_sort1(const L<Z>& x, BIAS bias) {
    CT counts0[256] = {};
    CT counts1[256] = {};
    for (const Z& e: x) {
        ++counts0[0xffull & bias(e) >> 0ull];
        ++counts1[0xffull & bias(e) >> 8ull];
    }
    prev_partial_sum(counts0);
    prev_partial_sum(counts1);
    L<J> j(x.size()); // \ts says Arthur doesn't pay this cost.  How?
    L<J> k(x.size());
    for (index_t i = 0; i < x.size(); ++i) j[counts0[0xff & bias(x[i]   ) >> 0ull]++] = J(i);
    for (index_t i = 0; i < x.size(); ++i) k[counts1[0xff & bias(x[j[i]]) >> 8ull]++] = j[i];
    return k;
}

template <class Z, class BIAS>
L<J> radix_sort1(const L<Z>& x, BIAS bias) {
    return x.size() < 1LL<< 8? radix_sort1i<uint8_t >(x, bias)
         : x.size() < 1LL<<16? radix_sort1i<uint16_t>(x, bias)
         : x.size() < 1LL<<32? radix_sort1i<uint32_t>(x, bias)
         : /* else */          radix_sort1 <index_t >(x, bias);
}

template <class CT, class Z, class BIAS>
L<J> radix_sort2i(const L<Z>& x, BIAS bias) {
    CT counts0[256] = {};
    CT counts1[256] = {};
    CT counts2[256] = {};
    for (const Z& e: x) {
        ++counts0[0xffull & bias(e) >>  0ull];
        ++counts1[0xffull & bias(e) >>  8ull];
        ++counts2[0xffull & bias(e) >> 16ull];
    }
    prev_partial_sum(counts0);
    prev_partial_sum(counts1);
    prev_partial_sum(counts2);
    L<J> r(x.size());
    int32_t* const j = reinterpret_cast<int32_t*>(r.begin());
    int32_t* const k = j + x.size();
    for (int32_t i = 0; i < x.size(); ++i) k[counts0[0xff & bias(x[i]   ) >>  0ull]++] = i;
    for (int32_t i = 0; i < x.size(); ++i) j[counts1[0xff & bias(x[k[i]]) >>  8ull]++] = k[i];
    for (int32_t i = 0; i < x.size(); ++i) k[counts2[0xff & bias(x[j[i]]) >> 16ull]++] = j[i];
    for (int32_t i = 0; i < x.size(); ++i) r[i] = J(k[i]);
    return r;
}

template <class CT, class Z, class BIAS>
L<J> radix_sort2(const L<Z>& x, BIAS bias) {
    CT counts0[256] = {};
    CT counts1[256] = {};
    CT counts2[256] = {};
    for (const Z& e: x) {
        ++counts0[0xffull & bias(e) >>  0ull];
        ++counts1[0xffull & bias(e) >>  8ull];
        ++counts2[0xffull & bias(e) >> 16ull];
    }
    prev_partial_sum(counts0);
    prev_partial_sum(counts1);
    prev_partial_sum(counts2);
    L<J> j(x.size());
    L<J> k(x.size());
    for (index_t i = 0; i < x.size(); ++i) j[counts0[0xff & bias(x[i]   ) >>  0ull]++] = J(i);
    for (index_t i = 0; i < x.size(); ++i) k[counts1[0xff & bias(x[j[i]]) >>  8ull]++] = j[i];
    for (index_t i = 0; i < x.size(); ++i) j[counts2[0xff & bias(x[k[i]]) >> 16ull]++] = k[i];
    return j;
}

template <class Z, class BIAS>
L<J> radix_sort2(const L<Z>& x, BIAS bias) {
    return x.size() < 1LL<< 8? radix_sort2i<uint8_t >(x, bias)
         : x.size() < 1LL<<16? radix_sort2i<uint16_t>(x, bias)
         : x.size() < 1LL<<32? radix_sort2i<uint32_t>(x, bias)
         : /* else */          radix_sort2 <index_t >(x, bias);
}

template <class CT, class Z, class BIAS>
L<J> radix_sort3i(const L<Z>& x, BIAS bias) {
    CT counts0[256] = {};
    CT counts1[256] = {};
    CT counts2[256] = {};
    CT counts3[256] = {};
    for (const Z& e: x) {
        ++counts0[0xffull & bias(e) >>  0ull];
        ++counts1[0xffull & bias(e) >>  8ull];
        ++counts2[0xffull & bias(e) >> 16ull];
        ++counts3[0xffull & bias(e) >> 24ull];
    }
    prev_partial_sum(counts0);
    prev_partial_sum(counts1);
    prev_partial_sum(counts2);
    prev_partial_sum(counts3);
    L<J> r(x.size());
    int32_t* const j = reinterpret_cast<int32_t*>(r.begin());
    int32_t* const k = j + x.size();
    for (int32_t i = 0; i < x.size(); ++i) j[counts0[0xff & bias(x[i]   ) >>  0ull]++] = i;
    for (int32_t i = 0; i < x.size(); ++i) k[counts1[0xff & bias(x[j[i]]) >>  8ull]++] = j[i];
    for (int32_t i = 0; i < x.size(); ++i) j[counts2[0xff & bias(x[k[i]]) >> 16ull]++] = k[i];
    for (int32_t i = 0; i < x.size(); ++i) k[counts3[0xff & bias(x[j[i]]) >> 24ull]++] = j[i];
    for (int32_t i = 0; i < x.size(); ++i) r[i] = J(k[i]);
    return r;
}

template <class CT, class Z, class BIAS>
L<J> radix_sort3(const L<Z>& x, BIAS bias) {
    CT counts0[256] = {};
    CT counts1[256] = {};
    CT counts2[256] = {};
    CT counts3[256] = {};
    for (const Z& e: x) {
        ++counts0[0xffull & bias(e) >>  0ull];
        ++counts1[0xffull & bias(e) >>  8ull];
        ++counts2[0xffull & bias(e) >> 16ull];
        ++counts3[0xffull & bias(e) >> 24ull];
    }
    prev_partial_sum(counts0);
    prev_partial_sum(counts1);
    prev_partial_sum(counts2);
    prev_partial_sum(counts3);
    L<J> j(x.size());
    L<J> k(x.size());
    for (index_t i = 0; i < x.size(); ++i) j[counts0[0xff & bias(x[i]   ) >>  0ull]++] = J(i);
    for (index_t i = 0; i < x.size(); ++i) k[counts1[0xff & bias(x[j[i]]) >>  8ull]++] = j[i];
    for (index_t i = 0; i < x.size(); ++i) j[counts2[0xff & bias(x[k[i]]) >> 16ull]++] = k[i];
    for (index_t i = 0; i < x.size(); ++i) k[counts3[0xff & bias(x[j[i]]) >> 24ull]++] = j[i];
    return k;
}

template <class Z, class BIAS>
L<J> radix_sort3(const L<Z>& x, BIAS bias) {
    return x.size() < 1LL<< 8? radix_sort3i<uint8_t >(x, bias)
         : x.size() < 1LL<<16? radix_sort3i<uint16_t>(x, bias)
         : x.size() < 1LL<<32? radix_sort3i<uint32_t>(x, bias)
         : /* else */          radix_sort3 <index_t >(x, bias);
}

template <class CT, class Z, class BIAS>
L<J> radix_sort4i(const L<Z>& x, BIAS bias) {
    CT counts0[256] = {};
    CT counts1[256] = {};
    CT counts2[256] = {};
    CT counts3[256] = {};
    CT counts4[256] = {};
    for (const Z& e: x) {
        ++counts0[0xffull & bias(e) >>  0ull];
        ++counts1[0xffull & bias(e) >>  8ull];
        ++counts2[0xffull & bias(e) >> 16ull];
        ++counts3[0xffull & bias(e) >> 24ull];
        ++counts4[0xffull & bias(e) >> 32ull];
    }
    prev_partial_sum(counts0);
    prev_partial_sum(counts1);
    prev_partial_sum(counts2);
    prev_partial_sum(counts3);
    prev_partial_sum(counts4);
    L<J> r(x.size());
    int32_t* const j = reinterpret_cast<int32_t*>(r.begin());
    int32_t* const k = j + x.size();
    for (int32_t i = 0; i < x.size(); ++i) k[counts0[0xff & bias(x[i]   ) >>  0ull]++] = i;
    for (int32_t i = 0; i < x.size(); ++i) j[counts1[0xff & bias(x[k[i]]) >>  8ull]++] = k[i];
    for (int32_t i = 0; i < x.size(); ++i) k[counts2[0xff & bias(x[j[i]]) >> 16ull]++] = j[i];
    for (int32_t i = 0; i < x.size(); ++i) j[counts3[0xff & bias(x[k[i]]) >> 24ull]++] = k[i];
    for (int32_t i = 0; i < x.size(); ++i) k[counts4[0xff & bias(x[j[i]]) >> 32ull]++] = j[i];
    for (int32_t i = 0; i < x.size(); ++i) r[i] = J(k[i]);
    return r;
}

template <class CT, class Z, class BIAS>
L<J> radix_sort4(const L<Z>& x, BIAS bias) {
    CT counts0[256] = {};
    CT counts1[256] = {};
    CT counts2[256] = {};
    CT counts3[256] = {};
    CT counts4[256] = {};
    for (const Z& e: x) {
        ++counts0[0xffull & bias(e) >>  0ull];
        ++counts1[0xffull & bias(e) >>  8ull];
        ++counts2[0xffull & bias(e) >> 16ull];
        ++counts3[0xffull & bias(e) >> 24ull];
        ++counts4[0xffull & bias(e) >> 32ull];
    }
    prev_partial_sum(counts0);
    prev_partial_sum(counts1);
    prev_partial_sum(counts2);
    prev_partial_sum(counts3);
    prev_partial_sum(counts4);
    L<J> j(x.size());
    L<J> k(x.size());
    for (index_t i = 0; i < x.size(); ++i) j[counts0[0xff & bias(x[i]   ) >>  0ull]++] = J(i);
    for (index_t i = 0; i < x.size(); ++i) k[counts1[0xff & bias(x[j[i]]) >>  8ull]++] = j[i];
    for (index_t i = 0; i < x.size(); ++i) j[counts2[0xff & bias(x[k[i]]) >> 16ull]++] = k[i];
    for (index_t i = 0; i < x.size(); ++i) k[counts3[0xff & bias(x[j[i]]) >> 24ull]++] = j[i];
    for (index_t i = 0; i < x.size(); ++i) j[counts4[0xff & bias(x[k[i]]) >> 32ull]++] = k[i];
    return j;
}

template <class Z, class BIAS>
L<J> radix_sort4(const L<Z>& x, BIAS bias) {
    return x.size() < 1LL<< 8? radix_sort4i<uint8_t >(x, bias)
         : x.size() < 1LL<<16? radix_sort4i<uint16_t>(x, bias)
         : x.size() < 1LL<<32? radix_sort4i<uint32_t>(x, bias)
         : /* else */          radix_sort4 <index_t >(x, bias);
}

template <class CT, class Z, class BIAS>
L<J> radix_sort5i(const L<Z>& x, BIAS bias) {
    // Perhaps a final rotate is better than calling bias so many times?
    CT counts0[256] = {};
    CT counts1[256] = {};
    CT counts2[256] = {};
    CT counts3[256] = {};
    CT counts4[256] = {};
    CT counts5[256] = {};
    for (const Z& e: x) {
        ++counts0[0xffull & bias(e) >>  0ull];
        ++counts1[0xffull & bias(e) >>  8ull];
        ++counts2[0xffull & bias(e) >> 16ull];
        ++counts3[0xffull & bias(e) >> 24ull];
        ++counts4[0xffull & bias(e) >> 32ull];
        ++counts5[0xffull & bias(e) >> 40ull];
    }
    prev_partial_sum(counts0);
    prev_partial_sum(counts1);
    prev_partial_sum(counts2);
    prev_partial_sum(counts3);
    prev_partial_sum(counts4);
    prev_partial_sum(counts5);
    L<J> r(x.size());
    int32_t* const j = reinterpret_cast<int32_t*>(r.begin());
    int32_t* const k = j + x.size();
    for (int32_t i = 0; i < x.size(); ++i) j[counts0[0xff & bias(x[i]   ) >>  0ull]++] = i;
    for (int32_t i = 0; i < x.size(); ++i) k[counts1[0xff & bias(x[j[i]]) >>  8ull]++] = j[i];
    for (int32_t i = 0; i < x.size(); ++i) j[counts2[0xff & bias(x[k[i]]) >> 16ull]++] = k[i];
    for (int32_t i = 0; i < x.size(); ++i) k[counts3[0xff & bias(x[j[i]]) >> 24ull]++] = j[i];
    for (int32_t i = 0; i < x.size(); ++i) j[counts4[0xff & bias(x[k[i]]) >> 32ull]++] = k[i];
    for (int32_t i = 0; i < x.size(); ++i) k[counts5[0xff & bias(x[j[i]]) >> 40ull]++] = j[i];
    for (int32_t i = 0; i < x.size(); ++i) r[i] = J(k[i]);
    return r;
}

template <class CT, class Z, class BIAS>
L<J> radix_sort5(const L<Z>& x, BIAS bias) {
    // Perhaps a final rotate is better than calling bias so many times?
    CT counts0[256] = {};
    CT counts1[256] = {};
    CT counts2[256] = {};
    CT counts3[256] = {};
    CT counts4[256] = {};
    CT counts5[256] = {};
    for (const Z& e: x) {
        ++counts0[0xffull & bias(e) >>  0ull];
        ++counts1[0xffull & bias(e) >>  8ull];
        ++counts2[0xffull & bias(e) >> 16ull];
        ++counts3[0xffull & bias(e) >> 24ull];
        ++counts4[0xffull & bias(e) >> 32ull];
        ++counts5[0xffull & bias(e) >> 40ull];
    }
    prev_partial_sum(counts0);
    prev_partial_sum(counts1);
    prev_partial_sum(counts2);
    prev_partial_sum(counts3);
    prev_partial_sum(counts4);
    prev_partial_sum(counts5);
    L<J> j(x.size()); // \ts says Arthur doesn't pay this cost.  How?
    L<J> k(x.size());
    for (index_t i = 0; i < x.size(); ++i) j[counts0[0xff & bias(x[i]   ) >>  0ull]++] = J(i);
    for (index_t i = 0; i < x.size(); ++i) k[counts1[0xff & bias(x[j[i]]) >>  8ull]++] = j[i];
    for (index_t i = 0; i < x.size(); ++i) j[counts2[0xff & bias(x[k[i]]) >> 16ull]++] = k[i];
    for (index_t i = 0; i < x.size(); ++i) k[counts3[0xff & bias(x[j[i]]) >> 24ull]++] = j[i];
    for (index_t i = 0; i < x.size(); ++i) j[counts4[0xff & bias(x[k[i]]) >> 32ull]++] = k[i];
    for (index_t i = 0; i < x.size(); ++i) k[counts5[0xff & bias(x[j[i]]) >> 40ull]++] = j[i];
    return k;
}

template <class Z, class BIAS>
L<J> radix_sort5(const L<Z>& x, BIAS bias) {
    return x.size() < 1LL<< 8? radix_sort5i<uint8_t >(x, bias)
         : x.size() < 1LL<<16? radix_sort5i<uint16_t>(x, bias)
         : x.size() < 1LL<<32? radix_sort5i<uint32_t>(x, bias)
         : /* else */          radix_sort5 <index_t >(x, bias);
}

template <class CT, class Z, class BIAS>
L<J> radix_sort6(const L<Z>& x, BIAS bias) {
    // Perhaps a final rotate is better than calling bias so many times?
    CT counts0[256] = {};
    CT counts1[256] = {};
    CT counts2[256] = {};
    CT counts3[256] = {};
    CT counts4[256] = {};
    CT counts5[256] = {};
    CT counts6[256] = {};
    for (const Z& e: x) {
        ++counts0[0xffull & bias(e) >>  0ull];
        ++counts1[0xffull & bias(e) >>  8ull];
        ++counts2[0xffull & bias(e) >> 16ull];
        ++counts3[0xffull & bias(e) >> 24ull];
        ++counts4[0xffull & bias(e) >> 32ull];
        ++counts5[0xffull & bias(e) >> 40ull];
        ++counts6[0xffull & bias(e) >> 48ull];
    }
    prev_partial_sum(counts0);
    prev_partial_sum(counts1);
    prev_partial_sum(counts2);
    prev_partial_sum(counts3);
    prev_partial_sum(counts4);
    prev_partial_sum(counts5);
    prev_partial_sum(counts6);
    L<J> j(x.size());
    L<J> k(x.size());
    for (index_t i = 0; i < x.size(); ++i) j[counts0[0xff & bias(x[i]   ) >>  0ull]++] = J(i);
    for (index_t i = 0; i < x.size(); ++i) k[counts1[0xff & bias(x[j[i]]) >>  8ull]++] = j[i];
    for (index_t i = 0; i < x.size(); ++i) j[counts2[0xff & bias(x[k[i]]) >> 16ull]++] = k[i];
    for (index_t i = 0; i < x.size(); ++i) k[counts3[0xff & bias(x[j[i]]) >> 24ull]++] = j[i];
    for (index_t i = 0; i < x.size(); ++i) j[counts4[0xff & bias(x[k[i]]) >> 32ull]++] = k[i];
    for (index_t i = 0; i < x.size(); ++i) k[counts5[0xff & bias(x[j[i]]) >> 40ull]++] = j[i];
    for (index_t i = 0; i < x.size(); ++i) j[counts6[0xff & bias(x[k[i]]) >> 48ull]++] = k[i];
    return j;
}

template <class Z, class BIAS>
L<J> radix_sort6(const L<Z>& x, BIAS bias) {
    return x.size() < 1LL<< 8? radix_sort6<uint8_t >(x, bias)
         : x.size() < 1LL<<16? radix_sort6<uint16_t>(x, bias)
         : x.size() < 1LL<<32? radix_sort6<uint32_t>(x, bias)
         : /* else */          radix_sort6<index_t >(x, bias);
}

template <class CT, class Z, class BIAS>
L<J> radix_sort7(const L<Z>& x, BIAS bias) {
    // Perhaps a final rotate is better than calling bias so many times?
    CT counts0[256] = {};
    CT counts1[256] = {};
    CT counts2[256] = {};
    CT counts3[256] = {};
    CT counts4[256] = {};
    CT counts5[256] = {};
    CT counts6[256] = {};
    CT counts7[256] = {};
    for (const Z& e: x) {
        ++counts0[0xffull & bias(e) >>  0ull];
        ++counts1[0xffull & bias(e) >>  8ull];
        ++counts2[0xffull & bias(e) >> 16ull];
        ++counts3[0xffull & bias(e) >> 24ull];
        ++counts4[0xffull & bias(e) >> 32ull];
        ++counts5[0xffull & bias(e) >> 40ull];
        ++counts6[0xffull & bias(e) >> 48ull];
        ++counts7[0xffull & bias(e) >> 56ull];
    }
    prev_partial_sum(counts0);
    prev_partial_sum(counts1);
    prev_partial_sum(counts2);
    prev_partial_sum(counts3);
    prev_partial_sum(counts4);
    prev_partial_sum(counts5);
    prev_partial_sum(counts6);
    prev_partial_sum(counts7);
    L<J> j(x.size()); // \ts says Arthur doesn't pay this cost.  How?
    L<J> k(x.size());
    for (index_t i = 0; i < x.size(); ++i) j[counts0[0xff & bias(x[i]   ) >>  0ull]++] = J(i);
    for (index_t i = 0; i < x.size(); ++i) k[counts1[0xff & bias(x[j[i]]) >>  8ull]++] = j[i];
    for (index_t i = 0; i < x.size(); ++i) j[counts2[0xff & bias(x[k[i]]) >> 16ull]++] = k[i];
    for (index_t i = 0; i < x.size(); ++i) k[counts3[0xff & bias(x[j[i]]) >> 24ull]++] = j[i];
    for (index_t i = 0; i < x.size(); ++i) j[counts4[0xff & bias(x[k[i]]) >> 32ull]++] = k[i];
    for (index_t i = 0; i < x.size(); ++i) k[counts5[0xff & bias(x[j[i]]) >> 40ull]++] = j[i];
    for (index_t i = 0; i < x.size(); ++i) j[counts6[0xff & bias(x[k[i]]) >> 48ull]++] = k[i];
    for (index_t i = 0; i < x.size(); ++i) k[counts7[0xff & bias(x[j[i]]) >> 56ull]++] = j[i];
    return k;
}

template <class Z, class BIAS>
L<J> radix_sort7(const L<Z>& x, BIAS bias) {
    return x.size() < 1LL<< 8? radix_sort7<uint8_t >(x, bias)
         : x.size() < 1LL<<16? radix_sort7<uint16_t>(x, bias)
         : x.size() < 1LL<<32? radix_sort7<uint32_t>(x, bias)
         : /* else */          radix_sort7<index_t >(x, bias);
}
