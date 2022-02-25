cmake_minimum_required(VERSION 3.14)

project(minirender)


if(NOT TARGET asls)
	include(FetchContent)
	FetchContent_Declare(asl URL https://github.com/aslze/asl/archive/1.10.4.zip)
	FetchContent_MakeAvailable(asl)
endif()

add_subdirectory(src)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)

	add_subdirectory(samples)
	
endif()
