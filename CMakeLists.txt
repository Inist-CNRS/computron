cmake_minimum_required(VERSION 2.8)

# Name of the project (will be the name of the plugin)
project(addon)

find_package(LibXml2 REQUIRED)
find_package(LibXslt REQUIRED)

include_directories(${CMAKE_JS_INC})

# Build a shared library named after the project from the files in `src/`
file(GLOB SOURCE_FILES "src/*.cpp" "src/*.h")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

# Gives our library file a .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Include N-API wrappers
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/node_modules/node-addon-api")

# Include Libxml2 and Libxslt
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES})