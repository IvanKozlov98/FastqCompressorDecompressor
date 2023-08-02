#include <iostream>
#include "coders/coder.h"
#include <boost/program_options.hpp>


namespace po = boost::program_options;

void printParameters(const std::string& inputFile,
                     const std::string& outputFile,
                     bool isCompress,
                     const std::string& method) {
    std::cout << "Input File: " << inputFile << std::endl;
    std::cout << "Output File: " << outputFile << std::endl;
    std::cout << "Is compression: " << std::boolalpha << isCompress << std::endl;
    std::cout << "Compression Method: " << method << std::endl;
    std::cout << "--------------------" << std::endl;
}

void process(const std::string& inputFile,
             const std::string& outputFile,
             bool isCompress,
             const std::string& method) {
    printParameters(inputFile, outputFile, isCompress, method);
    //
    bool is_separated_work = (method != "ans" && method != "huffman" && method != "range");
    if (isCompress) {
        if (is_separated_work) {
            coders::separated_compress(inputFile.c_str(), outputFile.c_str(), method);
            // Code block executed when the method is not one of the specified values
            //std::cerr << "Invalid method. Please choose 'ans', 'huffman', or 'range'." << std::endl;
            //return 1;
        } else {
            coders::compress(inputFile.c_str(), outputFile.c_str(), method, "COMPRESS ALL");
        }
    } else {
        if (is_separated_work) {
            coders::separated_decompress(inputFile.c_str(), outputFile.c_str(), method);
        } else {
            coders::decompress(inputFile.c_str(), outputFile.c_str(), method, "DECOMPRESS ALL");
        }
    }
}

int checkCdOptions(bool compress, bool decompress) {
    // Check for valid options
    if (compress && decompress) {
        std::cerr << "Error: Cannot specify both --compress and --decompress" << std::endl;
        return 1;
    }
    if (!compress && !decompress) {
        std::cerr << "Error: Either --compress or --decompress must be specified" << std::endl;
        return 1;
    }
    return 0;
}


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
                    "\n\'range\' - Range encoding"
                    "\nfilename with methods for every type line"
                    )
            ("help,h", "produce help message");

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

    if (checkCdOptions(compress, decompress)) {
        return 1;
    }

    // Run main process
    process(inputFile, outputFile, compress, method);

    return 0;
}
