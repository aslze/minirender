set(TARGET render)

set(SRC
	main.cpp
)

add_executable(${TARGET} ${SRC})

target_link_libraries(${TARGET} minirender asls)


set(TARGET minirender-bench)
add_executable(${TARGET} bench.cpp)
target_link_libraries(${TARGET} minirender asls)

