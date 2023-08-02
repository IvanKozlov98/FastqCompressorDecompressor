//
// Created by ivankozlov98 on 8/1/23.
//

#ifndef TEST_MGI_UTILS_H
#define TEST_MGI_UTILS_H
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>


struct SeparatedNames {
    std::string ids;
    std::string seqs;
    std::string nn;
    std::string qs;

    static SeparatedNames getNames(const std::string& filename) {
        std::string prefix = filename.substr(0, filename.find_last_of('.'));
        SeparatedNames result{};
        result.ids = prefix + "_ids.txt";
        result.seqs = prefix + "_seqs.txt";
        result.nn = prefix + "_nn.txt";
        result.qs = prefix + "_qs.txt";
        return result;
    }
};

SeparatedNames separate_fastq(const std::string& filename, const SeparatedNames& separated_names) {
    std::ifstream fin(filename);
    std::ofstream fout_ids(separated_names.ids);
    std::ofstream fout_seqs(separated_names.seqs);
    std::ofstream fout_nn(separated_names.nn);
    std::ofstream fout_qs(separated_names.qs);

    std::string cur_str;
    while(!fin.eof()) {
        // read & write identifiers
        std::getline(fin, cur_str);
        fout_ids << cur_str << std::endl;
        // read & write sequences
        std::getline(fin, cur_str);
        fout_seqs << cur_str << std::endl;
        // read & write nn_ids
        std::getline(fin, cur_str);
        fout_nn << cur_str << std::endl;
        // read & write qualities
        std::getline(fin, cur_str);
        fout_qs << cur_str << std::endl;
    }
    fout_ids.close();
    fout_seqs.close();
    fout_nn.close();
    fout_qs.close();
    return separated_names;
}

long getFileSize(const char* filename) {
    FILE* file = fopen(filename, "rb"); // Open the file in binary mode

    if (file == NULL) {
        perror("Error opening the file");
        return -1; // Return -1 to indicate an error
    }

    fseek(file, 0, SEEK_END); // Move the file pointer to the end of the file
    long size = ftell(file); // Get the current position of the file pointer (which is the file size)
    fclose(file); // Close the file

    return size;
}

std::vector<std::string> getEncodeMethods(const char* filename) {
    std::ifstream fin(filename);
    std::vector<std::string> res;
    std::string method;
    while(!fin.eof()) {
        // read & write identifiers
        std::getline(fin, method);
        res.push_back(method);
    }
    return res;
}

void mergePartsFastq(const std::string& filename, const SeparatedNames& separated_names) {
    std::ofstream fout(filename);
    std::ifstream fin_ids(separated_names.ids);
    std::ifstream fin_seqs(separated_names.seqs);
    std::ifstream fin_nn(separated_names.nn);
    std::ifstream fin_qs(separated_names.qs);

    std::string cur_str;
    while(!fin_ids.eof()) {
        // read & write identifiers
        std::getline(fin_ids, cur_str);
        fout << cur_str << std::endl;
        // read & write sequences
        std::getline(fin_seqs, cur_str);
        fout << cur_str << std::endl;
        // read & write nn_ids
        std::getline(fin_nn, cur_str);
        fout << cur_str << std::endl;
        // read & write qualities
        std::getline(fin_qs, cur_str);
        fout << cur_str << std::endl;
    }
    std::remove(separated_names.ids.c_str());
    std::remove(separated_names.seqs.c_str());
    std::remove(separated_names.nn.c_str());
    std::remove(separated_names.qs.c_str());
    fout.close();
}


#endif //TEST_MGI_UTILS_H
