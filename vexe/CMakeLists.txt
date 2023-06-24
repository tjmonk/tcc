cmake_minimum_required(VERSION 3.10)

include(GNUInstallDirs)

project(vexe
	VERSION 0.1
	DESCRIPTION "Virtual Machine Executor"
)

add_executable( ${PROJECT_NAME}
	src/vexe.c
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
