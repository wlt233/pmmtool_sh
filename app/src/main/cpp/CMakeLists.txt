
# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.22.1)

project("pmmtool_sh")
find_package(shadowhook REQUIRED CONFIG)
find_library(log-lib log)
find_library(android-lib android)
add_library(pmm SHARED main.cpp)
target_link_libraries(pmm shadowhook::shadowhook ${log-lib} ${android-lib})