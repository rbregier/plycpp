cmake_minimum_required(VERSION 2.8)



get_filename_component(MODELS_DIRECTORY
                       "${CMAKE_CURRENT_LIST_DIR}/models"
                       ABSOLUTE)


add_library(plycpp src/plycpp.cpp)
target_include_directories(plycpp PUBLIC ${CMAKE_CURRENT_LIST_DIR}/hdr)
					   
add_executable(ply_io_example src/example.cpp)
target_link_libraries(ply_io_example plycpp)
target_compile_definitions(ply_io_example PRIVATE MODELS_DIRECTORY="${MODELS_DIRECTORY}")

