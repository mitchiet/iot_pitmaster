# The following lines of boilerplate have to be in the project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)

# Adds the modules
set(EXTRA_COMPONENT_DIRS "./a4988_driver/" "./bluetooth/" "./pid_control/" "./pwm/" "./max31855/" "./test/")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(iot_pitmaster)