cmake_minimum_required(VERSION 3.19)

project(qiss VERSION 0)
include(cmake/StandardProjectSettings.cmake)

find_package(FLEX REQUIRED)
flex_target(lexer src/qiss.l "${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c"
    COMPILE_FLAGS -s
    DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/lex.yy.h")

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)
target_compile_options(project_options INTERFACE
    -ferror-limit=1
    $<$<CONFIG:RELEASE>:-ffast-math>
    $<$<CONFIG:DEBUG>:-g>
# TODO fix for non-x86 targets
    $<$<CONFIG:RELEASE>:-maxv2>
    $<$<CONFIG:DEBUG>:-Og>
)

add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

set(CORE
    src/bits.cpp
    src/buddy_allocator.cpp
    src/fixed_size_allocator.cpp
    src/iaddr.cpp
    src/index.cpp
    src/l.cpp
    src/linked_list_len.cpp
    src/o.cpp
    src/object.cpp
    src/opcode.cpp
    src/prim.cpp
    src/qiss_alloc.cpp
    src/returning.cpp
    src/sym.cpp
    src/symtrie.cpp
    src/system_alloc.cpp
    src/terminal_width.cpp)

add_library(qiss ${CORE})
target_compile_definitions(qiss PUBLIC DOCTEST_CONFIG_DISABLE)
target_include_directories(qiss PUBLIC src)
target_link_libraries(qiss PRIVATE project_options project_warnings)

add_executable(ql
    src/adverb.cpp
    src/exception.cpp
    src/format.cpp
    src/indexof.cpp
    src/kv.cpp
    src/lcutil.cpp
    src/match.cpp
    src/objectio.cpp
    src/output.cpp
    src/primio.cpp
    src/prim_parse.cpp
    src/qisslex.cpp
    src/ql-main.cpp
    src/sum.cpp
    src/token.cpp
    src/ukv.cpp
    ${FLEX_lexer_OUTPUTS})
target_compile_definitions(ql PUBLIC DOCTEST_CONFIG_DISABLE)
target_include_directories(ql PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c" PROPERTIES
    COMPILE_OPTIONS "-Wno-conversion;-Wno-unused-parameter")
target_link_libraries(ql PRIVATE project_options project_warnings qiss)
target_include_directories(ql PUBLIC src)

add_executable(utest src/utest.cpp ${CORE})
target_include_directories(utest PUBLIC src)
target_link_libraries(utest PRIVATE project_options project_warnings)
