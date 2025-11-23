

add_executable(
    ${PROJ_NAME}
    main.cc
)

set(CMAKE_BUILD_TYPE Debug)

set_property(
    TARGET 
        ${PROJ_NAME}
    PROPERTY 
        CXX_STANDARD 23
)

