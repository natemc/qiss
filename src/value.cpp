#include <value.h>
#include <exception.h>
#include <o.h>
#include <ukv.h>

O value(O x) {
    if (x.is_atom() || x.is_list()) return x;
    if (x.is_dict()) return UKV(std::move(x)).val();
    throw Exception("type: . (value)");
}
