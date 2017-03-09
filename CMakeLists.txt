cmake_minimum_required(VERSION 3.5)
project(git-remote-s3 VERSION 0.0.1 LANGUAGES C CXX)

option(SIMPLE_INSTALL "" OFF) # Because "ON" makes AWS use "CMAKE_SOURCE_DIR" 
set(BUILD_ONLY "s3" CACHE STRING "")

add_subdirectory(aws-sdk-cpp)
add_subdirectory(libgit2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(git-remote-s3 main.cpp)
target_include_directories(git-remote-s3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(git-remote-s3 PUBLIC libgit2/include)
target_link_libraries(git-remote-s3 PUBLIC AWS::aws-cpp-sdk-s3 git2)

