cmake_minimum_required(VERSION 3.4)

project(render)

set(TARGET render)

find_package(ASL REQUIRED)

set(SRC
    minirender/Scene.h
    minirender/io.h
    minirender/renderer.h

    Scene.cpp
    io.cpp
    renderer.cpp

    main.cpp
)

include_directories(.)

add_executable(${TARGET} ${SRC})

target_link_libraries(${TARGET} asls)



