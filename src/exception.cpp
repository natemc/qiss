#include <exception.h>
#include <cstddef>

Exception::Exception(const char* s) {
    std::size_t i = 0;
    for (; i < sizeof err - 1 && s[i]; ++i) err[i] = s[i];
    err[i] = '\0';
}

Exception::Exception(const char* first, const char* last) {
    std::size_t i = 0;
    for (; first != last && i < sizeof err - 1; ++first, ++i) err[i] = *first;
    err[i] = '\0';
}

const char* Exception::what() const {
    return err;
}
