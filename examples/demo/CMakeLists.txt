# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(macchiato32_demo)

# Use SPIFFS to access Flash ROM using file handles
spiffs_create_partition_image(rom assets/m32-native FLASH_IN_PROJECT)