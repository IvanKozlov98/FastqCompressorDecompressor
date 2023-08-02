cmake_minimum_required(VERSION 3.25)
project(test_mgi)

set(CMAKE_CXX_STANDARD 17)

add_executable(test_mgi main.cpp coders/range_coder/range_coder.h util/utils.h coders/ans_coder/ans_coder.h coders/ans_coder/platform.h coders/ans_coder/rans_byte.h coders/haffman_coder/huffman_coder.h coders/haffman_coder/huffman.cpp)
