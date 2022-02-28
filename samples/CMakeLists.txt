set(TARGET render)

set(SRC
	main.cpp
)

add_executable(${TARGET} ${SRC})

target_link_libraries(${TARGET} minirender asls)
