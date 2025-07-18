file(GLOB SOURCES "./Source/*.cpp")

idf_component_register(
    SRCS ${SOURCES}
    PRIV_REQUIRES driver
    INCLUDE_DIRS "Include"
)