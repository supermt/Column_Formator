cmake_minimum_required(VERSION 3.8)
project(Column_Formator)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp FileReader.cpp)
add_executable(Column_Formator main.cpp untils.cpp untils.h)
add_executable(File_Reader FileReader.cpp untils.cpp untils.h)