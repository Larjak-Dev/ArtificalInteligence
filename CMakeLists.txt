cmake_minimum_required(VERSION 3.10.0)
project(SearchAlgorithm VERSION 0.1.0 LANGUAGES C CXX)

add_executable(SearchAlgorithm main.cpp TransitionModel.hpp TransitionModel.cpp MazeModel.hpp MazeModel.cpp SearchAlgorithm.hpp SearchAlgorithm.cpp)

target_include_directories(SearchAlgorithm PRIVATE include)

target_link_libraries(SearchAlgorithm PRIVATE ${CMAKE_SOURCE_DIR}/lib/libsfml-system-d.a)
target_link_libraries(SearchAlgorithm PRIVATE ${CMAKE_SOURCE_DIR}/lib/libsfml-window-d.a)
target_link_libraries(SearchAlgorithm PRIVATE ${CMAKE_SOURCE_DIR}/lib/libsfml-main-d.a)
target_link_libraries(SearchAlgorithm PRIVATE ${CMAKE_SOURCE_DIR}/lib/libsfml-graphics-d.a)



add_custom_command(TARGET SearchAlgorithm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/bin $<TARGET_FILE_DIR:SearchAlgorithm>)


