
include_directories(${labyrinth_SOURCE_DIR}/include)

message(STATUS ${labyrinth_SOURCE_DIR})

link_directories(${labyrinth_SOURCE_DIR}/src)

add_executable( labyrinth main.cpp point.cpp angle.cpp maze.cpp config.cpp)

target_link_libraries(labyrinth)