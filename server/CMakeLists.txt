cmake_minimum_required(VERSION 3.10)

project(YourProjectName)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


include_directories(service)
include_directories(src)
include_directories(src/service) 

set(SOURCES
    src/main.cpp
    src/grpc/grpc_genop.cpp
    src/grpc/grpc_image.cpp
    src/grpc/grpc_torch.cpp
    src/grpc/grpc_resource.cpp
    src/grpc/grpc_session.cpp
    src/grpc/grpc_tensor.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

# find_package(OpenCV REQUIRED)
# include_directories(${OpenCV_INCLUDE_DIRS})

find_package(Threads REQUIRED)

# Add the executable
add_executable(server ${SOURCES})

# target_link_libraries(server
#     PRIVATE
#         myproto
#         vaccel
#         ${OpenCV_LIBS}
# )


target_link_libraries(server
    PRIVATE
        myproto
        vaccel
)
