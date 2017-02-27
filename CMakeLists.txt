cmake_minimum_required(VERSION 2.6)
project(impl)


include_directories(${CMAKE_SOURCE_DIR}/include)


#set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_BUILD_TYPE Debug)
add_definitions(-Wall)

# add header files here
set(HEADER_FILES
	${CMAKE_SOURCE_DIR}/include/interpreter.hpp
	${CMAKE_SOURCE_DIR}/include/instructions.hpp)

# add required sources here
set(SOURCE_FILES
	${CMAKE_SOURCE_DIR}/source/interpreter.cpp
	${CMAKE_SOURCE_DIR}/source/instructions.cpp
	${CMAKE_SOURCE_DIR}/source/object.cpp)

set(MAIN ${CMAKE_SOURCE_DIR}/source/main.cpp)

add_executable(main ${MAIN} ${SOURCE_FILES} ${HEADER_FILES})
