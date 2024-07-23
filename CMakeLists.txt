cmake_minimum_required(VERSION 3.28)
project(ServerClientProject)

set(CMAKE_CXX_STANDARD 11)


# 添加 server 可执行文件
add_executable(server
        nio_socket_example/server.cpp
        nio_socket_example/NIOSocketSenderReceiver/NIOSocketSenderReceiver.hpp
)
# 链接 ws2_32 库到 server
target_link_libraries(server ws2_32)

# 添加 client 可执行文件
add_executable(client
        nio_socket_example/client.cpp
        nio_socket_example/NIOSocketSenderReceiver/NIOSocketSenderReceiver.hpp
)
# 链接 ws2_32 库到 client
target_link_libraries(client ws2_32)


# Boost.Asio 项目所在目录
set(INCLUDE_DIRS include)
include_directories(${INCLUDE_DIRS})
# 如果是WIN32，要链接 ws2_32 库 以及 mswsock 库
if (WIN32)
    set(PLATFORM_LIBS ws2_32 Mswsock)
else()
    set(PLATFORM_LIBS)
endif()

add_executable(asio_server asio_example/asio_server.cpp)
target_link_libraries(asio_server ${PLATFORM_LIBS})

add_executable(asio_client asio_example/asio_client.cpp)
target_link_libraries(asio_client ${PLATFORM_LIBS})
