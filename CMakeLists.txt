cmake_minimum_required(VERSION 3.13)

project(http-server-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
# find_package(Boost REQUIRED COMPONENTS iostreams)

# include_directories(${Boost_INCLUDE_DIRS})

add_executable(server ${SOURCE_FILES})

# target_link_libraries(server PRIVATE ${Boost_LIBRARIES} Threads::Threads ZLIB::ZLIB)

target_link_libraries(server PRIVATE Threads::Threads ZLIB::ZLIB)
