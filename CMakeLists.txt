cmake_minimum_required(VERSION 2.6)

project(glight)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0>=3.0.0 REQUIRED)

find_package(Boost COMPONENTS filesystem system REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_FLAGS "-O3 -Wall -DNDEBUG -march=native -std=c++11")
set(CMAKE_C_FLAGS "-O3 -Wall -DNDEBUG -march=native")

include_directories(${GTKMM_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIR})

set(APPFILES
  soundb.cpp)

add_library(soundb-object OBJECT
  ${APPFILES})

add_executable(soundb $<TARGET_OBJECTS:soundb-object>)

link_directories(${GTKMM_LIBDIR})

set(SOUNDB_LIBRARIES
  ${Boost_FILESYSTEM_LIBRARY} 
  ${Boost_SYSTEM_LIBRARY} 
  ${GTKMM_LIBRARIES} 
  ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(soundb ${SOUNDB_LIBRARIES})

install(TARGETS soundb RUNTIME DESTINATION bin)
