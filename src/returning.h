#pragma once

#define RETURNING(E, B4) do { const auto r(E); B4; return r; } while(false)
