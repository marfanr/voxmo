cmake_minimum_required(VERSION 3.16)

# Nama project dan versinya
project(voxmo VERSION 1.0 LANGUAGES CXX)

# Set standar C++ (misalnya C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${PROJECT_NAME}_lib 
  ${CMAKE_SOURCE_DIR}/src/file_loader.cc
  ${CMAKE_SOURCE_DIR}/src/builder.cc
)
target_include_directories(${PROJECT_NAME}_lib PRIVATE include)

add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/voxmo.cc)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)
target_include_directories(${PROJECT_NAME} PRIVATE include)