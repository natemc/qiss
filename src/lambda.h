#pragma once

#define L1(e) [&](auto&& x){ return e; }
#define L2(e) [&](auto&& x, auto&& y){ return e; }
