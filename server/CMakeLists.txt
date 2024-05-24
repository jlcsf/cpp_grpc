find_package(Threads)


set(SOURCES
    src/main.cpp
    src/service_functions.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(server ${SOURCES})
target_link_libraries(server
    PRIVATE
        myproto
        vaccel
        ${OpenCV_LIBS}
)
