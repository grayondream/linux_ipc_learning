cmake_minimum_required(VERSION 3.5)
project(ipc)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-stack-protector")

add_definitions(-w)

include_directories(include)
aux_source_directory(. MAIN_DIR)
aux_source_directory(src SOURCE_DIR)

add_executable(main ${MAIN_DIR} ${SOURCE_DIR})
target_link_libraries(main rt pthread)
