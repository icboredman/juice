cmake_minimum_required(VERSION 3.10)

project(juice  VERSION 1.0
               DESCRIPTION "Battery charging and monitoring"
)

#include_directories(${PROJECT_SOURCE_DIR})

add_executable(${CMAKE_PROJECT_NAME}
   ${PROJECT_SOURCE_DIR}/main.cpp
   ${PROJECT_SOURCE_DIR}/i2c.hpp
   ${PROJECT_SOURCE_DIR}/i2c.cpp
   ${PROJECT_SOURCE_DIR}/cw2015.hpp
   ${PROJECT_SOURCE_DIR}/cw2015.cpp
   ${PROJECT_SOURCE_DIR}/bq25703a.hpp
   ${PROJECT_SOURCE_DIR}/bq25703a.cpp
)

find_package(OpenSSL REQUIRED)
find_library(paho-mqttpp3 NAMES libpaho-mqttpp3.a REQUIRED)
find_library(paho-mqtt3a NAMES libpaho-mqtt3a.so REQUIRED)

target_link_libraries(${CMAKE_PROJECT_NAME} pthread crypt rt paho-mqtt3a paho-mqttpp3)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

