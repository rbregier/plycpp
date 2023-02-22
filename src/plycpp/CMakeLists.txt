cmake_minimum_required(VERSION 3.8)
project(plycpp)

add_compile_options(-Wall -Wextra -Wpedantic)

# find dependencies
find_package(ament_cmake REQUIRED)

get_filename_component(MODELS_DIRECTORY
  "${CMAKE_CURRENT_SOURCE_DIR}/models"
  ABSOLUTE)

include_directories(
  PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_library(plycpp SHARED src/plycpp.cpp)

install(
  TARGETS plycpp
  EXPORT plycpp)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
install(
  DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include
)
add_executable(plycpp_example src/example.cpp)
target_link_libraries(plycpp_example plycpp)
target_compile_definitions(plycpp_example PRIVATE MODELS_DIRECTORY="${MODELS_DIRECTORY}")

if(BUILD_TESTING)
find_package(ament_lint_auto REQUIRED)
ament_lint_auto_find_test_dependencies()
endif()

ament_export_libraries(plycpp)
ament_package()
