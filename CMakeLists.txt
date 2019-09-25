cmake_minimum_required(VERSION 3.5)
project(adm_audio_renderer VERSION 1.0.0 LANGUAGES CXX)

find_package(Boost 1.57 REQUIRED)
find_package(adm REQUIRED)
find_package(ear REQUIRED)

file(GLOB HEADER_FILES ${PROJECT_SOURCE_DIR}/src/*.hpp)
file(GLOB SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp ${HEADER_FILES})

add_library(admrenderer STATIC ${SOURCE_FILES})
target_link_libraries(admrenderer PRIVATE ear)
target_link_libraries(admrenderer PRIVATE adm)

include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(adm_audio_renderer app/main.cpp)
target_link_libraries(adm_audio_renderer PRIVATE admrenderer)
target_link_libraries(adm_audio_renderer PRIVATE adm)
target_link_libraries(adm_audio_renderer PRIVATE ear)

install(TARGETS adm_audio_renderer DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(TARGETS admrenderer DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
