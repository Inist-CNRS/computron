cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)

project(addon)

find_package(LibXml2 REQUIRED)
find_package(LibXslt REQUIRED)
set($ENV{NODE_ADDON_API_INC} NODE_ADDON_API_INC)
include_directories(${CMAKE_JS_INC})

file(GLOB SOURCE_FILES "src/*.cpp" "src/*.h")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

target_include_directories(${PROJECT_NAME} 
  PRIVATE ${NODE_ADDON_API_INC}
  PRIVATE ${CMAKE_JS_INC} 
  PRIVATE ${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR}
)
target_link_libraries(${PROJECT_NAME} 
  PRIVATE ${CMAKE_JS_LIB}
  ${LIBXML2_LIBRARIES}
  ${LIBXSLT_LIBRARIES}
  ${LIBXSLT_EXSLT_LIBRARIES}
)
