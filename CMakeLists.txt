cmake_minimum_required(VERSION 3.15)

project(ltsgraphcolor)

add_subdirectory(third)

add_executable(ltsgraphcolor src/main.cpp src/color.cpp)
target_link_libraries(ltsgraphcolor tinyxml2)
