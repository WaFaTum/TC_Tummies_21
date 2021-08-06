
# cmake minimum version
cmake_minimum_required(VERSION 3.10)

# set the project name
project(TC_TUMMIES21)

# group source files togother
set(SRC_FILES src/main.cpp src/open62541_server.cpp)


# add folder to include path
include_directories(include)

# add library
add_compile_options(-lopen62541 -I include)

# output
add_executable(myServer ${SRC_FILES})
