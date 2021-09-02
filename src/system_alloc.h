#pragma once

#include <cstddef>

using oom_handler_t = void(*)();

void*         alloc_region              (std::size_t bytes);
void*         alloc_region_fully_aligned(std::size_t bytes);
std::size_t   bytes_allocated           ();
void          free_region               (void* p, std::size_t bytes);
oom_handler_t oom_handler               ();
oom_handler_t set_oom_handler           (oom_handler_t);
std::size_t   system_page_size          ();
