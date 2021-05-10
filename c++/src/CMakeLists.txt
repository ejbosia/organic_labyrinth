
find_package( OpenCV REQUIRED )


message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

if(CMAKE_VERSION VERSION_LESS "2.8.11")
    # Add OpenCV headers location to your include paths
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()

include_directories(${drawbot_SOURCE_DIR}/include)

message(STATUS ${drawbot_SOURCE_DIR})

link_directories(${drawbot_SOURCE_DIR}/src)

add_executable( drawbot main.cpp point.cpp angle.cpp line.cpp contour.cpp family.cpp intersection_strategy.cpp linear_fill_strategy.cpp contour_fill_strategy.cpp gcode.cpp)

target_link_libraries(drawbot ${OpenCV_LIBS})