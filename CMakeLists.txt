cmake_minimum_required(VERSION 3.5.0)
project(multi-downloader VERSION 0.1.0 LANGUAGES C CXX)

add_executable(multi-downloader main.cpp)

# Find the CURL package
find_package(CURL REQUIRED)
# Find the indicators package
find_package(indicators CONFIG REQUIRED)

# Link the libraries to the target
target_link_libraries(multi-downloader PRIVATE CURL::libcurl indicators::indicators)

