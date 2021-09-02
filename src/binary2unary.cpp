#include <binary2unary.h>
#include <algorithm>
#include <iterator>
#include <opcode.h>
#include <utility>

Opcode binary2unary(Opcode op) {
    const std::pair<Opcode, Opcode> b2u[] = {
        {Opcode::add  , Opcode::flip    },
        {Opcode::at   , Opcode::type    },
        {Opcode::bang , Opcode::key     },
        {Opcode::cast , Opcode::string  },
        {Opcode::cat  , Opcode::enlist  },
        {Opcode::cut  , Opcode::floor   },
        {Opcode::dot  , Opcode::value   },
        {Opcode::eq   , Opcode::group   },
        {Opcode::fdiv , Opcode::recip   },
        {Opcode::fill , Opcode::null    },
        {Opcode::find , Opcode::distinct},
        {Opcode::great, Opcode::idesc   },
        {Opcode::less , Opcode::iasc    },
        {Opcode::match, Opcode::not_    },
        {Opcode::max  , Opcode::rev     },
        {Opcode::min  , Opcode::where   },
        {Opcode::mul  , Opcode::first   },
        {Opcode::sub  , Opcode::neg     },
        {Opcode::take , Opcode::count   },
    };
    const auto it = std::find_if(b2u, std::end(b2u),
        [=](auto x){ return x.first == op; });
    return it == std::end(b2u)? op : it->second;
}
