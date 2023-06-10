cmake_minimum_required(VERSION 3.10)

project(varvm
	VERSION 0.1
	DESCRIPTION "Virtual Machine Varserver Interface"
)

include(GNUInstallDirs)

find_library( LIB_VARSERVER varserver REQUIRED )

add_library( ${PROJECT_NAME} SHARED
	src/libvarvm.c
)

target_link_libraries( ${PROJECT_NAME}
	dl
	rt
	${LIB_VARSERVER}
)

set_target_properties( ${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	POSITION_INDEPENDENT_CODE ON
)

install(TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

