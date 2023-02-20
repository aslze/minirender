set(TARGET minirender)

set(SRC
	../include/minirender/Scene.h
	../include/minirender/Renderer.h
	../include/minirender/io.h
	Scene.cpp
	Renderer.cpp
	io.cpp
	x3d.cpp
)

add_library(${TARGET} STATIC ${SRC})

target_link_libraries(${TARGET} asls)
target_include_directories(${TARGET} PUBLIC ../include)
