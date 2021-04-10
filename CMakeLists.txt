# The following lines of boilerplate have to be in the project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# Adds the bluetooth module
set(EXTRA_COMPONENT_DIRS "./bluetooth/" "./blowfan/")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(iot_pitmaster)