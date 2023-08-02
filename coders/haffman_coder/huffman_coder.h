#include "huffman.h"

namespace coders {
    class HuffmanCoderAdapter {
    public:
        static void compress(const char* in, const char* out) {
            huffman h(in, out);
            h.create_pq();
            h.create_huffman_tree();
            h.calculate_huffman_codes();
            h.coding_save();
            cout << endl;
        }

        static void decompress(const char* in, const char* out) {
            huffman h(in, out);
            h.recreate_huffman_tree();
            h.decoding_save();
            cout << endl;
        }
    };
}