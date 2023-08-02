#include "util/utils.h"
#include "coders/range_coder/range_coder.h"
#include "coders/ans_coder/ans_coder.h"
#include "coders/haffman_coder/huffman_coder.h"
#include <boost/program_options.hpp>

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

    return 0;
}
