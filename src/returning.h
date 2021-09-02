#pragma once

#define RETURNING(E, B4) do { const auto r_(E); B4; return r_; } while(false)
