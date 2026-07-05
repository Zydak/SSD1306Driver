cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES "./Source/*.cpp")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "./Include"
    PRIV_REQUIRES driver esp_driver_i2c
)