cmake_minimum_required(VERSION 3.5)
project(adm-engine VERSION 1.0.0 LANGUAGES CXX)

find_package(Boost 1.57 REQUIRED)
find_package(adm REQUIRED)
find_package(ear REQUIRED)

file(GLOB HEADER_FILES ${PROJECT_SOURCE_DIR}/src/*.hpp)
file(GLOB SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp ${HEADER_FILES})

add_library(admengine STATIC ${SOURCE_FILES})
target_link_libraries(admengine PRIVATE ear)
target_link_libraries(admengine PRIVATE adm)

include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(adm-engine app/main.cpp)
target_link_libraries(adm-engine PRIVATE admengine)
target_link_libraries(adm-engine PRIVATE adm)
target_link_libraries(adm-engine PRIVATE ear)

install(TARGETS adm-engine DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(TARGETS admengine DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/admengine)
