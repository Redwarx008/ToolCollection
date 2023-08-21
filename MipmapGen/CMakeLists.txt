cmake_minimum_required(VERSION 3.16)

project(MipmapGen LANGUAGES CXX)

#add_library(lodePng   lodepng/lodepng.cpp
#                      lodepng/lodepng.h
#                      lodepng/lodepng_util.cpp
#                      lodepng/lodepng_util.h)

#target_compile_features(lodePng PUBLIC cxx_std_11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(MipmapGen main.cpp "TiledTexture.cpp")

target_compile_features(MipmapGen PUBLIC cxx_std_17)

target_include_directories(MipmapGen PUBLIC include)

#target_link_libraries(MipmapGen PUBLIC lodePng)