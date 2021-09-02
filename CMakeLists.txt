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
# TODO fix for non-x86 targets
    $<$<CONFIG:RELEASE>:-mavx2>
)

add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)
include(cmake/StaticAnalyzers.cmake)

set(CORE
    src/adverb.cpp
    src/arith.cpp
    src/at.cpp
    src/atomic.cpp
    src/bang.cpp
    src/binary2unary.cpp
    src/bits.cpp
    src/buddy_allocator.cpp
    src/cat.cpp
    src/compile.cpp
    src/converge.cpp
    src/count.cpp
    src/detect.cpp
    src/distinct.cpp
    src/do.cpp
    src/dollar.cpp
    src/dot.cpp
    src/each.cpp
    src/enlist.cpp
    src/exception.cpp
    src/fill.cpp
    src/find.cpp
    src/first.cpp
    src/fixed_size_allocator.cpp
    src/flip.cpp
    src/floor.cpp
    src/format.cpp
    src/group.cpp
    src/grouper.cpp
    src/hash.cpp
    src/hashset.cpp
    src/iaddr.cpp
    src/iasc.cpp
    src/idesc.cpp
    src/in.cpp
    src/index.cpp
    src/indexof.cpp
    src/key.cpp
    src/kv.cpp
    src/l.cpp
    src/lcutil.cpp
    src/linked_list_len.cpp
    src/lutil.cpp
    src/match.cpp
    src/merge_dicts.cpp
    src/not.cpp
    src/null.cpp
    src/o.cpp
    src/object.cpp
    src/objectio.cpp
    src/opcode.cpp
    src/output.cpp
    src/over.cpp
    src/prim.cpp
    src/prim_parse.cpp
    src/primio.cpp
    src/prior.cpp
    src/qiss_alloc.cpp
    src/relop.cpp
    src/returning.cpp
    src/reverse.cpp
    src/scan.cpp
    src/string_.cpp
    src/sort.cpp
    src/split.cpp
    src/sum.cpp
    src/sym.cpp
    src/symtrie.cpp
    src/system_alloc.cpp
    src/take.cpp
    src/terminal_width.cpp
    src/type_pair.cpp
    src/ukv.cpp
    src/uminus.cpp
    src/under.cpp
    src/ul.cpp
    src/value.cpp
    src/vcond.cpp
    src/where.cpp)

add_library(qisscore ${CORE})
target_compile_definitions(qisscore PUBLIC DOCTEST_CONFIG_DISABLE)
target_include_directories(qisscore PUBLIC src)
target_link_libraries(qisscore PRIVATE project_options project_warnings)

add_executable(qasm
    src/qasm.cpp
    src/qasm-main.cpp
    src/read_file.cpp)
target_compile_definitions(qasm PUBLIC DOCTEST_CONFIG_DISABLE)
target_link_libraries(qasm PRIVATE project_options project_warnings qisscore)
target_include_directories(qasm PUBLIC src)

add_executable(qc
    src/compile.cpp
    src/disassemble.cpp
    src/gen.cpp
    src/getline.cpp
    src/module.cpp
    src/parse.cpp
    src/qc-main.cpp
    src/read_file.cpp
    src/token.cpp
    ${FLEX_lexer_OUTPUTS})
target_compile_definitions(qc PUBLIC DOCTEST_CONFIG_DISABLE)
target_include_directories(qc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c" PROPERTIES
    COMPILE_OPTIONS "-Wno-conversion;-Wno-unused-parameter")
target_link_libraries(qc PRIVATE project_options project_warnings qisscore)
target_include_directories(qc PUBLIC src)

add_executable(qd
    src/disassemble.cpp
    src/getline.cpp
    src/qd-main.cpp
    src/read_file.cpp)
target_compile_definitions(qd PUBLIC DOCTEST_CONFIG_DISABLE)
target_link_libraries(qd PRIVATE project_options project_warnings qisscore)
target_include_directories(qd PUBLIC src)

add_executable(qiss
    src/disassemble.cpp
    src/gen.cpp
    src/getline.cpp
    src/infix.cpp
    src/inspect.cpp
    src/interp.cpp
    src/module.cpp
    src/parse.cpp
    src/qasm.cpp
    src/qisslex.cpp
    src/qiss-main.cpp
    src/read_file.cpp
    src/token.cpp
    src/uniform.cpp
    src/vm.cpp
    ${FLEX_lexer_OUTPUTS})
target_compile_definitions(qiss PUBLIC DOCTEST_CONFIG_DISABLE)
target_include_directories(qiss PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c" PROPERTIES
    COMPILE_OPTIONS "-Wno-conversion;-Wno-unused-parameter")
target_link_libraries(qiss PRIVATE project_options project_warnings qisscore)
target_include_directories(qiss PUBLIC src)

add_executable(ql
    src/getline.cpp
    src/qisslex.cpp
    src/ql-main.cpp
    src/read_file.cpp
    src/token.cpp
    ${FLEX_lexer_OUTPUTS})
target_compile_definitions(ql PUBLIC DOCTEST_CONFIG_DISABLE)
target_include_directories(ql PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c" PROPERTIES
    COMPILE_OPTIONS "-Wno-conversion;-Wno-unused-parameter")
target_link_libraries(ql PRIVATE project_options project_warnings qisscore)
target_include_directories(ql PUBLIC src)

add_executable(qp
    src/getline.cpp
    src/parse.cpp
    src/qp-main.cpp
    src/read_file.cpp
    src/token.cpp
    ${FLEX_lexer_OUTPUTS})
target_compile_definitions(qp PUBLIC DOCTEST_CONFIG_DISABLE)
target_include_directories(qp PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c" PROPERTIES
    COMPILE_OPTIONS "-Wno-conversion;-Wno-unused-parameter")
target_link_libraries(qp PRIVATE project_options project_warnings qisscore)
target_include_directories(qp PUBLIC src)

add_executable(vm
    src/disassemble.cpp
    src/gen.cpp
    src/getline.cpp
    src/infix.cpp
    src/inspect.cpp
    src/interp.cpp
    src/module.cpp
    src/parse.cpp
    src/qasm.cpp
    src/qisslex.cpp
    src/read_file.cpp
    src/token.cpp
    src/uniform.cpp
    src/vm.cpp
    src/vm-main.cpp
    ${FLEX_lexer_OUTPUTS})
target_compile_definitions(vm PUBLIC DOCTEST_CONFIG_DISABLE)
target_include_directories(vm PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c" PROPERTIES
    COMPILE_OPTIONS "-Wno-conversion;-Wno-unused-parameter")
target_link_libraries(vm PRIVATE project_options project_warnings qisscore)
target_include_directories(vm PUBLIC src)

add_executable(utest
    src/disassemble.cpp
    src/gen.cpp
    src/interp.cpp
    src/module.cpp
    src/parse.cpp
    src/qiss-test.cpp
    src/test-main.cpp
    src/token.cpp
    src/vm.cpp
    ${CORE}
    ${FLEX_lexer_OUTPUTS})
target_include_directories(utest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c" PROPERTIES
    COMPILE_OPTIONS "-Wno-conversion;-Wno-unused-parameter")
target_include_directories(utest PUBLIC src)
target_link_libraries(utest PRIVATE project_options project_warnings)
