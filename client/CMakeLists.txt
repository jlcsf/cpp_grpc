
find_package(Threads)


set(SOURCES
    src/main.cpp
    src/client_functions.cpp
)


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})


add_executable(client ${SOURCES})
target_link_libraries(client
    PRIVATE
        myproto
)
