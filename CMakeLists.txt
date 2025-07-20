cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES "./Source/*.cpp")

idf_component_register(
    SRCS ${SOURCES}
    PRIV_REQUIRES driver
    INCLUDE_DIRS "./Include"
)