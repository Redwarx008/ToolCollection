cmake_minimum_required(VERSION 3.16)

project(MipmapGen LANGUAGES CXX)

#  add_library(lodepng STATIC 
#                      lodepng/lodepng.cpp
#                      lodepng/lodepng.h
#                      lodepng/lodepng_util.cpp
#                      lodepng/lodepng_util.h)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(MipmapGen main.cpp)

target_compile_features(MipmapGen PUBLIC cxx_std_11)

target_include_directories(MipmapGen PUBLIC include)

# target_link_libraries(MipmapGen PUBLIC lodepng)