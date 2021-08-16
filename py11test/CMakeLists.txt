cmake_minimum_required (VERSION 3.16)

project (myprogram)

set (CMAKE_BUILD_TYPE "Release")

add_subdirectory(pybind11)
pybind11_add_module(kf_cpp pylib.cpp)

#add_executable (mytest pylib.cpp)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../ val3)
## add_subdirectory (/home/elvis/code/val3dity/ build)
target_link_libraries (kf_cpp LINK_PUBLIC val3dity)


#  USE
# cmake .. -DLIBRARY=true
# -- Building val3dity library