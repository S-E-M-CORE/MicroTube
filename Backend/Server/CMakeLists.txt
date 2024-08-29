cmake_minimum_required(VERSION 3.1)

project(MicroTubeServer)

set(SOURCES
    src/App.cpp
)

# Create a library target
add_library(MicroTubeLib ${SOURCES} "src/App.cpp")

# Find required packages
find_package(oatpp 1.3.0 REQUIRED)
find_package(oatpp-swagger 1.3.0 REQUIRED)
find_package(oatpp-sqlite 1.3.0 REQUIRED)

target_link_libraries(MicroTubeLib
    oatpp::oatpp
    oatpp::oatpp-swagger
    oatpp::oatpp-sqlite
    )

if(CMAKE_SYSTEM_NAME MATCHES Linux)
    find_package(Threads REQUIRED)
    target_link_libraries(crud-lib INTERFACE Threads::Threads ${CMAKE_DL_LIBS})
endif()

target_compile_definitions(MicroTubeLib
    PUBLIC OATPP_SWAGGER_RES_PATH="${oatpp-swagger_INCLUDE_DIRS}/../bin/oatpp-swagger/res"
    PUBLIC WEB_CONTENT_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/bin/web"
)

target_include_directories(MicroTubeLib PUBLIC src)

add_executable(MicroTubeServer src/App.cpp "src/App.cpp")

target_link_libraries(MicroTubeServer MicroTubeLib)

# Set output directory for the executable
set_target_properties(MicroTubeServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

# Set output directory for the executable
set_target_properties(MicroTubeServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)