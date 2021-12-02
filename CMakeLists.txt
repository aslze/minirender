cmake_minimum_required(VERSION 3.4)

project(minirender)

if(NOT TARGET asls)
	find_package(ASL REQUIRED)
endif()

set(TARGET minirender)

set(SRC
    minirender/Scene.h
    minirender/io.h
    minirender/renderer.h
    Scene.cpp
    io.cpp
    renderer.cpp
)

add_library(${TARGET} STATIC ${SRC})

target_link_libraries(${TARGET} asls)
target_include_directories(${TARGET} PUBLIC .)


if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)

	set(TARGET render)

	set(SRC
		main.cpp
	)

	add_executable(${TARGET} ${SRC})

	target_link_libraries(${TARGET} minirender asls)

endif()



