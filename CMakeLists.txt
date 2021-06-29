cmake_minimum_required(VERSION 3.10)
#set(CMAKE_C_STANDARD 11)#C11
#set(CMAKE_CXX_STANDARD 17)#C17
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(juice  VERSION 1.0
               DESCRIPTION "Battery charging and monitoring"
#               LANGUAGES CXX
)

#include_directories(${PROJECT_SOURCE_DIR})
#add_compile_definitions(LV_CONF_INCLUDE_SIMPLE)
#file(GLOB_RECURSE LVGL_INCLUDES "lv_drv_epson/*.h" "lvgl/*.h"  "./*.h" )
#file(GLOB_RECURSE LVGL_SOURCES  "lv_drv_epson/*.c" "lvgl/*.c" )

add_executable(${CMAKE_PROJECT_NAME}
   ${PROJECT_SOURCE_DIR}/main.cpp
   ${PROJECT_SOURCE_DIR}/i2c.hpp
   ${PROJECT_SOURCE_DIR}/i2c.cpp
   ${PROJECT_SOURCE_DIR}/cw2015.hpp
   ${PROJECT_SOURCE_DIR}/cw2015.cpp
)

#target_compile_features(${CMAKE_PROJECT_NAME} PUBLIC cxx_std_14)
target_link_libraries(${CMAKE_PROJECT_NAME} wiringPi pthread crypt rt)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

