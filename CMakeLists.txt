cmake_minimum_required(VERSION 3.5)
project(adm-engine VERSION 1.0.2 LANGUAGES C CXX)

find_package(Boost 1.57 REQUIRED)
find_package(adm REQUIRED)
find_package(ear REQUIRED)

file(GLOB HEADER_FILES ${PROJECT_SOURCE_DIR}/src/adm_engine/*.hpp)
file(GLOB SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/adm_engine/*.cpp ${HEADER_FILES})

add_library(admengine SHARED ${SOURCE_FILES})
target_link_libraries(admengine PRIVATE ear)
target_link_libraries(admengine PRIVATE adm)
set_property(TARGET admengine PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(admengine_static STATIC ${SOURCE_FILES})
target_link_libraries(admengine_static PRIVATE ear)
target_link_libraries(admengine_static PRIVATE adm)
set_property(TARGET admengine_static PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(admengine_static PROPERTIES OUTPUT_NAME admengine)

include_directories("${PROJECT_SOURCE_DIR}/src")

add_library(admengineworker SHARED ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/src/adm_worker/worker.cpp)
target_link_libraries(admengineworker PRIVATE admengine)

add_executable(adm-engine app/main.cpp)
target_link_libraries(adm-engine PRIVATE admengine_static)
target_link_libraries(adm-engine PRIVATE adm)
target_link_libraries(adm-engine PRIVATE ear)

install(TARGETS adm-engine DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(TARGETS admengine DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS admengineworker DESTINATION ${CMAKE_INSTALL_PREFIX}/worker)
install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/adm_engine)
