cmake_minimum_required(VERSION 3.7)
project(UnitGadget)
set(PROJECT_TARGET Gadget)

set(INCLUDES_DIR include)

file(GLOB LIST_SOURCES src/*.c)
file(GLOB LIST_HEADERS "${INCLUDES_DIR}/*.h")
set(ALL_SOURCES ${LIST_HEADERS} ${LIST_SOURCES})

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
	${CMAKE_CURRENT_SOURCE_DIR}/bin
        CACHE PATH
        "Single Directory for all"
    )

add_executable(${PROJECT_TARGET} ${ALL_SOURCES})
target_include_directories(${PROJECT_TARGET} PRIVATE ${INCLUDES_DIR})
#target_compile_options(${PROJECT_TARGET} PRIVATE -g -Wall)
target_compile_options(${PROJECT_TARGET} PRIVATE -g -Wall -DDEBUG)

#target_link_libraries(${PROJECT_TARGET} -lm)


