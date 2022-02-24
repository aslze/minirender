cmake_minimum_required(VERSION 3.4)

project(minirender)

if(NOT TARGET asls)
	find_package(ASL REQUIRED)
endif()

add_subdirectory(src)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)

	add_subdirectory(samples)
	
endif()



