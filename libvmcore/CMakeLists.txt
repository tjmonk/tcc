cmake_minimum_required(VERSION 3.10)

project(vmcore
	VERSION 0.1
	DESCRIPTION "Virtual Machine Core Library"
)

include(GNUInstallDirs)

add_library( ${PROJECT_NAME} SHARED
	src/strbuf.c
	src/ask.c
	src/externvars.c
	src/files.c
	src/core.c
)

set_target_properties( ${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	POSITION_INDEPENDENT_CODE ON
)

set(VMCORE_HEADERS inc/vmcore/core.h inc/vmcore/datatypes.h inc/vmcore/externvars.h)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${VMCORE_HEADERS}")

target_include_directories( ${PROJECT_NAME} PRIVATE . )

target_include_directories( ${PROJECT_NAME} PUBLIC inc )

target_link_libraries( ${PROJECT_NAME}
	dl
	rt
)

install(TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vmcore )


