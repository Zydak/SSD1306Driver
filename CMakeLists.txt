cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)

file(GLOB SOURCES "./Source/*.cpp")

idf_component_register(
    SRCS ${SOURCES}
    PRIV_REQUIRES driver
    INCLUDE_DIRS "./Include"
)