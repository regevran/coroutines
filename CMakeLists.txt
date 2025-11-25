
cmake_minimum_required(VERSION 3.30)

set (
    PROJ_NAME
    ran_coro
)
set(
    CMAKE_BUILD_TYPE
    Debug
)

project(${PROJ_NAME})


subdirs(src)


