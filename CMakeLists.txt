cmake_minimum_required(VERSION 3.4)

project(minirender)

set(CMAKE_CXX_STANDARD 11)

if(NOT MSVC)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -O3)
endif()

if(NOT TARGET asls)
	find_package(ASL QUIET)
	if(NOT TARGET asls)
		cmake_minimum_required(VERSION 3.14...3.24)
		include(FetchContent)
		FetchContent_Declare(asl URL https://github.com/aslze/asl/archive/714a36312ec45ed68ce8b04c23a646e43892e01e.zip)
		FetchContent_MakeAvailable(asl)
	endif()
endif()

add_subdirectory(src)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	add_subdirectory(samples)
endif()
