cmake_minimum_required(VERSION 3.10.0)
project(group-chat-app VERSION 0.1.0 LANGUAGES C)

add_executable(gc-client client.c)
add_executable(gc-server server.c)


