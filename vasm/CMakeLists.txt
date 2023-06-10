cmake_minimum_required(VERSION 3.10)

include(GNUInstallDirs)

project(vasm
	VERSION 0.1
	DESCRIPTION "Virtual Machine Assembler"
)

find_library( LIB_VMCORE vmcore REQUIRED )
find_library( LIB_VMASM vmasm REQUIRED )

add_executable( ${PROJECT_NAME}
	src/vasm.c
)

target_include_directories( ${PROJECT_NAME}
	PRIVATE inc
)

target_link_libraries( ${PROJECT_NAME}
	dl
	rt
	${LIB_VMCORE}
	${LIB_VMASM}
)

install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
