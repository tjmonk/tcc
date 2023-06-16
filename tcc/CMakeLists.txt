cmake_minimum_required(VERSION 3.10)

project(tcc
    VERSION 0.1
    DESCRIPTION "Tiny C Compiler"
)

include(GNUInstallDirs)

find_package(BISON)
find_package(FLEX)

FLEX_TARGET( TCC_Scanner src/lexan.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c )
BISON_TARGET( TCC_Parser src/tcc.y ${CMAKE_CURRENT_BINARY_DIR}/tcc.tab.c )
ADD_FLEX_BISON_DEPENDENCY(TCC_Scanner TCC_Parser)

add_executable( ${PROJECT_NAME}
    src/codegen.c
    src/typecheck.c
    src/node.c
    src/labels.c
    src/lineno.c
    src/SymbolTableManager.c
    src/Registers.c

    ${FLEX_TCC_Scanner_OUTPUTS}
     ${BISON_TCC_Parser_OUTPUTS}
)

target_include_directories( ${PROJECT_NAME} PRIVATE
    .
    inc
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/tcc 
    ${CMAKE_BINARY_DIR}/tcc/tcc )

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )

file(GLOB test_files "test/*.c")

install( FILES ${test_files}
    DESTINATION /usr/share/tcc
)
