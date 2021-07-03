cmake_minimum_required(VERSION 3.10)

project(juice  VERSION 1.0
               DESCRIPTION "Battery charging and monitoring"
)

add_executable(${CMAKE_PROJECT_NAME}
   ${PROJECT_SOURCE_DIR}/main.cpp
   ${PROJECT_SOURCE_DIR}/i2c.hpp
   ${PROJECT_SOURCE_DIR}/i2c.cpp
   ${PROJECT_SOURCE_DIR}/cw2015.hpp
   ${PROJECT_SOURCE_DIR}/cw2015.cpp
   ${PROJECT_SOURCE_DIR}/bq25703a.hpp
   ${PROJECT_SOURCE_DIR}/bq25703a.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME} wiringPi pthread crypt rt)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

