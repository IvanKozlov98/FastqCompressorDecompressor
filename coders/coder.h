#pragma once

#include "range_coder/range_coder.h"
#include "ans_coder/ans_coder.h"
#include "haffman_coder/huffman_coder.h"
#include "../util/utils.h"
#include <ctime>

namespace coders {
    void compress(const char* inputFile, const char* outputFile, const std::string& method, const char* label) {
        printf("%s \n", label);
        clock_t start_decoding = clock();
        if (method == "ans") {
            coders::AnsCoderAdapter::compress(inputFile, outputFile);
        } else if (method == "huffman") {
            coders::HuffmanCoderAdapter::compress(inputFile, outputFile);
        } else {
            coders::RangeCoderAdapter::compress(inputFile, outputFile);
        }
        clock_t end_decoding = clock();
        printf("Compression is ended, time is %2.3f sec.\n", (double)(end_decoding - start_decoding)/CLOCKS_PER_SEC);
        auto data_size = getFileSize(inputFile);
        auto result_size = getFileSize(outputFile);

        printf("Original size                                %10ld bytes\n", data_size);
        printf("Actual encoded size                          %10ld bytes\n", result_size);
        auto ratio = (double)result_size;
        ratio /= (double)data_size;
        printf("Compression ratio                                 %2.3f of original size.\n\n", ratio);
    }

    void separated_compress(const char* inputFile, const char* outputFile, const std::string& method) {
        const auto& methods = getEncodeMethods(method.c_str());
        const auto& input_separated_names = SeparatedNames::getNames(inputFile);
        const auto& output_separated_names = SeparatedNames::getNames(outputFile);
        separate_fastq(inputFile, input_separated_names);
        //
        coders::compress(input_separated_names.ids.c_str(), output_separated_names.ids.c_str(), methods[0], ("COMPRESS IDs by " + methods[0]).c_str());
        coders::compress(input_separated_names.nn.c_str(), output_separated_names.nn.c_str(), methods[1], ("COMPRESS NNs by " + methods[1]).c_str());
        coders::compress(input_separated_names.qs.c_str(), output_separated_names.qs.c_str(), methods[2], ("COMPRESS QUALITIES by " + methods[2]).c_str());
        coders::compress(input_separated_names.seqs.c_str(), output_separated_names.seqs.c_str(), methods[3], ("COMPRESS SEQUENCES by " + methods[3]).c_str());
    }

    void decompress(const char* inputFile, const char* outputFile, const std::string& method, const char* label) {
        printf("%s \n", label);
        clock_t start_decoding = clock();
        if (method == "ans") {
            coders::AnsCoderAdapter::decompress(inputFile, outputFile);
        } else if (method == "huffman") {
            coders::HuffmanCoderAdapter::decompress(inputFile, outputFile);
        } else {
            coders::RangeCoderAdapter::decompress(inputFile, outputFile);
        }
        clock_t end_decoding = clock();
        printf("Decompression is ended, time is %2.3f sec.\n", (double)(end_decoding - start_decoding)/CLOCKS_PER_SEC);
    }

    void separated_decompress(const char* inputFile, const char* outputFile, const std::string& method) {
        const auto& methods = getEncodeMethods(method.c_str());
        const auto& input_separated_names = SeparatedNames::getNames(inputFile);
        const auto& output_separated_names = SeparatedNames::getNames(outputFile);
        coders::decompress(input_separated_names.ids.c_str(), output_separated_names.ids.c_str(), methods[0], ("DECOMPRESS IDs by "+ methods[0]).c_str());
        coders::decompress(input_separated_names.nn.c_str(), output_separated_names.nn.c_str(), methods[1], ("DECOMPRESS NNs by " + methods[1]).c_str());
        coders::decompress(input_separated_names.qs.c_str(), output_separated_names.qs.c_str(), methods[2], ("DECOMPRESS QUALITIES by " + methods[2]).c_str());
        coders::decompress(input_separated_names.seqs.c_str(), output_separated_names.seqs.c_str(), methods[3], ("DECOMPRESS SEQUENCES by " + methods[3]).c_str());
        mergePartsFastq(outputFile, output_separated_names);
    }
}

