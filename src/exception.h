#pragma once

struct Exception {
    explicit Exception(const char* s);
    explicit Exception(const char* first, const char* last);
    const char* what() const;
private:
    char err[256];
};
