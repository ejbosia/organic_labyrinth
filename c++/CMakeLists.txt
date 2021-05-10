cmake_minimum_required(VERSION 2.8)
project( drawbot )

add_subdirectory(src)

# add_executable( drawbot main.cpp point.cpp angle.cpp line.cpp contour.cpp family.cpp intersection_strategy.cpp fill_strategy.cpp gcode.cpp)

# add_compile_definitions(DEBUG_CONTOUR DEBUG_FAMILY DEBUG_LINE)
# add_compile_definitions(DEBUG_CONTOUR)

# target_link_libraries( drawbot ${OpenCV_LIBS} )