cmake_minimum_required(VERSION 3.10)

project(vmasm
	VERSION 0.1
	DESCRIPTION "Virtual Machine Assembler Utility Library"
)

include(GNUInstallDirs)

find_package(BISON)
find_package(FLEX)

FLEX_TARGET( VASM_Scanner src/l.l ${CMAKE_CURRENT_BINARY_DIR}/l.c )
BISON_TARGET( VASM_Parser src/y.y ${CMAKE_CURRENT_BINARY_DIR}/y.c )
ADD_FLEX_BISON_DEPENDENCY(VASM_Scanner VASM_Parser)

add_library( ${PROJECT_NAME} SHARED
	src/asm.c
	src/labels.c
	src/parseinfo.c
 	${BISON_VASM_Parser_OUTPUTS}
	${FLEX_VASM_Scanner_OUTPUTS}
)

set_target_properties( ${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	POSITION_INDEPENDENT_CODE ON
)

set(VMASM_HEADERS inc/asm.h inc/labels.h inc/parseinfo.h)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${VMASM_HEADERS}")

target_link_libraries( ${PROJECT_NAME}
    vmcore
)

target_include_directories( ${PROJECT_NAME} PRIVATE . )

target_include_directories( ${PROJECT_NAME} PUBLIC inc )

install(TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vmasm )


