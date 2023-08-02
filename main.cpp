#include "util/utils.h"
#include "coders/range_coder/range_coder.h"
#include "coders/ans_coder/ans_coder.h"
#include "coders/haffman_coder/huffman_coder.h"

int main() {
    std::string exp_file = "mgi_1_4";
    std::string seq_file = exp_file + "_seqs.txt";

//    separate_fastq("mgi_1_4.fq");

//    coders::RangeCoderAdapter::compress("mgi_1_4_seqs.txt", "mgi_1_4_seqs_zip.txt");
//    coders::RangeCoderAdapter::decompress("mgi_1_4_seqs_zip.txt", "mgi_1_4_seqs_unzip.txt");




//    const char* out = "mgi_1_4_huf.txt";
//    coders::HuffmanCoderAdapter::compress(seq_file.c_str(), out);
//    std::cout << "After compress " << getFileSize(out);
//    coders::HuffmanCoderAdapter::decompress(out, (seq_file + "_unhuf.txt").c_str());

//    coders::AnsCoderAdapter::compress("short_version_reads_seqs.txt", "short_version_reads_seqs_ans.txt");
//    coders::AnsCoderAdapter::decompress("short_version_reads_seqs_ans.txt", "qqqqqq.txt");

    coders::AnsCoderAdapter::compress("mgi_1_4_seqs.txt", "mgi_1_4_seqs_anszip.txt");
    coders::AnsCoderAdapter::decompress("mgi_1_4_seqs_anszip.txt", "eeeeeeeeee.txt");

    return 0;
}
