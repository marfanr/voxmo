cmake_minimum_required(VERSION 3.16)
project(voxmo VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${PROJECT_NAME}_lib 
  ${CMAKE_SOURCE_DIR}/src/file_loader.cc
  ${CMAKE_SOURCE_DIR}/src/builder.cc
  ${CMAKE_SOURCE_DIR}/src/manifest.cc
)

target_include_directories(${PROJECT_NAME}_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/voxmo.cc)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)
target_include_directories(${PROJECT_NAME} PRIVATE include)

add_executable(voxmo_dump ${CMAKE_SOURCE_DIR}/src/dump.cc)
target_link_libraries(voxmo_dump PRIVATE ${PROJECT_NAME}_lib)
target_include_directories(voxmo_dump PRIVATE include)


install(TARGETS ${PROJECT_NAME}_lib
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include)

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin)

install(TARGETS voxmo_dump 
        RUNTIME DESTINATION bin)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION include)

install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION lib/cmake/${PROJECT_NAME})
