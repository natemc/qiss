#pragma once

#include <cstddef>

const struct LinkedListLen {
    template <class L> std::size_t operator()(const L* x) const {
        std::size_t n = 0;
        for (; x; x = x->next) ++n;
        return n;
    }
} linked_list_len;
