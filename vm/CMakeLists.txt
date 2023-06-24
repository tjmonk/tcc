cmake_minimum_required(VERSION 3.10)

include(GNUInstallDirs)

project(vm
	VERSION 0.1
	DESCRIPTION "Virtual Machine Assembler"
)

add_executable( ${PROJECT_NAME}
	src/vm.c
)

target_include_directories( ${PROJECT_NAME}
	PRIVATE inc
)

target_link_libraries( ${PROJECT_NAME}
	dl
	rt
	vmcore
	vmasm
)

install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


