#include "util/utils.h"
#include "coders/range_coder/range_coder.h"
#include "coders/ans_coder/ans_coder.h"
#include "coders/haffman_coder/huffman_coder.h"
#include <boost/program_options.hpp>
#include <time.h>


namespace po = boost::program_options;


int main(int argc, char* argv[]) {
// Define variables to store the option values
    std::string inputFile;
    std::string outputFile;
    bool compress = false;
    bool decompress = false;
    std::string method = "ans";

    // Declare the command-line options
    po::options_description desc("Allowed options");
    desc.add_options()
            ("input,i", po::value<std::string>(&inputFile)->required(), "input file")
            ("output,o", po::value<std::string>(&outputFile)->required(), "output file")
            ("compress,c", po::bool_switch(&compress), "compress")
            ("decompress,d", po::bool_switch(&decompress), "decompress")
            ("method,m", po::value<std::string>(&method)->default_value("ans"),
                    "compression method, possible values:"
                    "\n\'ans\' - Asymmetric numeral systems"
                    "\n\'huffman\' - Huffman coding"
                    "\n\'range\' - Range encoding")
            ("help,h", "produce help message");

    // Parse the command-line options
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        }

        po::notify(vm);
    } catch (const po::error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << desc << std::endl;
        return 1;
    }

    // Check for valid options
    if (compress && decompress) {
        std::cerr << "Error: Cannot specify both --compress and --decompress" << std::endl;
        return 1;
    }
    if (!compress && !decompress) {
        std::cerr << "Error: Either --compress or --decompress must be specified" << std::endl;
        return 1;
    }

    if (method != "ans" && method != "huffman" && method != "range") {
        // Code block executed when the method is not one of the specified values
        std::cerr << "Invalid method. Please choose 'ans', 'huffman', or 'range'." << std::endl;
        return 1;
    }

    // Process the options and run your compression/decompression logic
    std::cout << "Input File: " << inputFile << std::endl;
    std::cout << "Output File: " << outputFile << std::endl;
    std::cout << "Compression: " << std::boolalpha << compress << std::endl;
    std::cout << "Decompression: " << std::boolalpha << decompress << std::endl;
    std::cout << "Compression Method: " << method << std::endl;
    std::cout << "--------------------" << std::endl;

    if (compress) {
        clock_t start_decoding = clock();
        if (method == "ans") {
            coders::AnsCoderAdapter::compress(inputFile.c_str(), outputFile.c_str());
        } else if (method == "huffman") {
            coders::HuffmanCoderAdapter::compress(inputFile.c_str(), outputFile.c_str());
        } else {
            coders::RangeCoderAdapter::compress(inputFile.c_str(), outputFile.c_str());
        }
        clock_t end_decoding = clock();
        printf("Decompression is ended, time is %2.3f sec.\n", (double)(end_decoding - start_decoding)/CLOCKS_PER_SEC);
        auto data_size = getFileSize(inputFile.c_str());
        auto result_size = getFileSize(outputFile.c_str());

        printf("Original size                                %10ld bytes\n", data_size);
        printf("Actual encoded size                          %10ld bytes\n", result_size);
        double ratio = (double)result_size;
        ratio /= (double)data_size;
        printf("Compression ratio                                 %2.3f of original size.\n\n", ratio);
    } else {
        if (method == "ans") {
            coders::AnsCoderAdapter::decompress(inputFile.c_str(), outputFile.c_str());
        } else if (method == "huffman") {
            coders::HuffmanCoderAdapter::decompress(inputFile.c_str(), outputFile.c_str());
        } else {
            coders::RangeCoderAdapter::decompress(inputFile.c_str(), outputFile.c_str());
        }
    }

    return 0;
}
