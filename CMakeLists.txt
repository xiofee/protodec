cmake_minimum_required(VERSION 3.8)

project(protoc LANGUAGES CXX)

set(_UNUSED ${CMAKE_C_COMPILER})

add_executable(${PROJECT_NAME} protoc.cpp)
add_executable(example example.cpp)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_features(example PRIVATE cxx_std_17)

if (MSVC)
  target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE

    _WIN32_WINNT=0x0601
  )
  target_compile_definitions(
    example
    PRIVATE

    _WIN32_WINNT=0x0601
  )
endif()

if (MSVC)
  target_compile_options(
    ${PROJECT_NAME}
    PRIVATE

    /Zc:__cplusplus
    /W3
  )
  target_compile_options(
    example
    PRIVATE

    /Zc:__cplusplus
    /W3
  )
else()
  target_compile_options(
    ${PROJECT_NAME}
    PRIVATE

    -Wall
  )
  target_compile_options(
    example
    PRIVATE

    -Wall
  )
endif()
