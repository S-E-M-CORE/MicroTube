cmake_minimum_required(VERSION 3.2)

set(project_name MicroTubeServer)

# Folder structure
set(MEDIA_FOLDER "${CMAKE_CURRENT_LIST_DIR}/media"      )
set(VIDEO_FOLDER "${CMAKE_CURRENT_LIST_DIR}/media/video")
set(WEB_FOLDER   "${CMAKE_CURRENT_LIST_DIR}/media/web"  )
set(TEMP_FOLDER  "${CMAKE_CURRENT_LIST_DIR}/media/temp" )
set(USER_PROFILE_PICTURE_FOLDER  "${CMAKE_CURRENT_LIST_DIR}/media/user/profile/" )
set(CHANNEL_PICTURE_FOLDER  "${CMAKE_CURRENT_LIST_DIR}/media/channel/" )

file(MAKE_DIRECTORY "${MEDIA_FOLDER}")
file(MAKE_DIRECTORY "${VIDEO_FOLDER}")
file(MAKE_DIRECTORY "${WEB_FOLDER}"  )
file(MAKE_DIRECTORY "${TEMP_FOLDER}" )
file(MAKE_DIRECTORY "${USER_PROFILE_PICTURE_FOLDER}" )
file(MAKE_DIRECTORY "${CHANNEL_PICTURE_FOLDER}" )


add_definitions(
    -DMEDIA_FOLDER="${MEDIA_FOLDER}"
    -DVIDEO_FOLDER="${VIDEO_FOLDER}"
    -DWEB_FOLDER="${WEB_FOLDER}"
    -DTEMP_FOLDER="${TEMP_FOLDER}"
    -DDATABASE_MIGRATIONS="${CMAKE_CURRENT_LIST_DIR}/src/sql"
    -DDATABASE_FILE="${CMAKE_CURRENT_SOURCE_DIR}/src/sql/database.sqlite"
    -DUSER_PROFILE_PICTURE_FOLDER="${USER_PROFILE_PICTURE_FOLDER}"
    -DCHANNEL_PICTURE_FOLDER="${CHANNEL_PICTURE_FOLDER}"
)
project(${project_name})

set(CMAKE_CXX_STANDARD 17)


add_library(${project_name}-lib
        "src/AppComponent.hpp"
        "src/controller/StaticController.hpp"
        "src/controller/UserController.hpp"
        "src/controller/VideoController.hpp"
        "src/controller/VideoController.cpp"
        "src/videos/VideoIndexer.cpp"
        "src/sql/DatabaseClient.hpp"
        "src/sql/DatabaseComponent.hpp"
     "src/controller/UserController.cpp" "src/controller/StaticController.cpp" "src/controller/ChannelController.cpp")

target_include_directories(
    ${project_name}-lib PUBLIC src
)

# Find required packages
find_package(oatpp 1.4.0 REQUIRED)
find_package(oatpp-swagger REQUIRED)
find_package(oatpp-sqlite REQUIRED)

target_link_libraries(${project_name}-lib
    oatpp::oatpp
    oatpp::oatpp-swagger
    oatpp::oatpp-sqlite
    oatpp::oatpp-test
    )

add_executable(${project_name}-exe src/App.cpp "src/videos/VideoIndexer.cpp" "src/controller/UserController.cpp" "src/controller/StaticController.cpp" "src/controller/ChannelController.cpp")

target_link_libraries(${project_name}-exe ${project_name}-lib)
add_dependencies     (${project_name}-exe ${project_name}-lib)


target_compile_definitions(${project_name}-lib
    PUBLIC OATPP_SWAGGER_RES_PATH="${oatpp-swagger_INCLUDE_DIRS}/../bin/oatpp-swagger/res"
)



add_dependencies(${project_name}-exe ${project_name}-lib)

# Set output directory for the executable
set_target_properties(${project_name}-exe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

# Set output directory for the executable
set_target_properties(${project_name}-exe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)
