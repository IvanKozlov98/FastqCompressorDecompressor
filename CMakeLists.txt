cmake_minimum_required(VERSION 3.25)
project(test_mgi)

set(CMAKE_CXX_STANDARD 17)

# Set the path to the Boost library installation (change it according to your installation path)
#set(Boost_INCLUDE_DIR /path/to/boost/include)
#set(Boost_LIBRARY_DIR /path/to/boost/lib)

# Find the Boost library components you need
find_package(Boost REQUIRED COMPONENTS program_options)

# Add the include directories for Boost
#include_directories(${Boost_INCLUDE_DIR})

add_executable(test_mgi main.cpp coders/range_coder/range_coder.h util/utils.h coders/ans_coder/ans_coder.h coders/ans_coder/platform.h coders/ans_coder/rans_byte.h coders/haffman_coder/huffman_coder.h coders/haffman_coder/huffman.cpp coders/coder.h)

# Link your executable with the Boost libraries
target_link_libraries(test_mgi PRIVATE ${Boost_LIBRARIES})
