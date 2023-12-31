#include "platform.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "rans_byte.h"


// This is just the sample program. All the meat is in rans_byte.h.

static void panic(const char *fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    fputs("Error: ", stderr);
    vfprintf(stderr, fmt, arg);
    va_end(arg);
    fputs("\n", stderr);

    exit(1);
}

static uint8_t* read_file(char const* filename, size_t* out_size)
{
    FILE* f = fopen(filename, "rb");
    if (!f)
        panic("file not found: %s\n", filename);

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* buf = new uint8_t[size];
    if (fread(buf, size, 1, f) != 1)
        panic("read failed\n");

    fclose(f);
    if (out_size)
        *out_size = size;

    return buf;
}

// ---- Stats

struct SymbolStats
{
    static constexpr uint16_t FREQS_SIZE = 256;
    static constexpr uint16_t CUM_FREQS_SIZE = 257;
    uint64_t freqs[FREQS_SIZE];
    uint64_t cum_freqs[CUM_FREQS_SIZE];

    void count_freqs(FILE* file);
    void calc_cum_freqs();
    void normalize_freqs(uint64_t target_total);
};

void SymbolStats::count_freqs(FILE* file)
{
    for (int i=0; i < 256; i++)
        freqs[i] = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        freqs[ch]++;
    }
}

void SymbolStats::calc_cum_freqs()
{
    cum_freqs[0] = 0;
    for (int i=0; i < 256; i++)
        cum_freqs[i+1] = cum_freqs[i] + freqs[i];
}

void SymbolStats::normalize_freqs(uint64_t target_total)
{
    assert(target_total >= 256);
    
    calc_cum_freqs();
    uint32_t cur_total = cum_freqs[256];
    
    // resample distribution based on cumulative freqs
    for (int i = 1; i <= 256; i++)
        cum_freqs[i] = ((uint64_t)target_total * cum_freqs[i])/cur_total;

    // if we nuked any non-0 frequency symbol to 0, we need to steal
    // the range to make the frequency nonzero from elsewhere.
    //
    // this is not at all optimal, i'm just doing the first thing that comes to mind.
    for (int i=0; i < 256; i++) {
        if (freqs[i] && cum_freqs[i+1] == cum_freqs[i]) {
            // symbol i was set to zero freq

            // find best symbol to steal frequency from (try to steal from low-freq ones)
            uint32_t best_freq = ~0u;
            int best_steal = -1;
            for (int j=0; j < 256; j++) {
                uint32_t freq = cum_freqs[j+1] - cum_freqs[j];
                if (freq > 1 && freq < best_freq) {
                    best_freq = freq;
                    best_steal = j;
                }
            }
            assert(best_steal != -1);

            // and steal from it!
            if (best_steal < i) {
                for (int j = best_steal + 1; j <= i; j++)
                    cum_freqs[j]--;
            } else {
                assert(best_steal > i);
                for (int j = i + 1; j <= best_steal; j++)
                    cum_freqs[j]++;
            }
        }
    }

    // calculate updated freqs and make sure we didn't screw anything up
    assert(cum_freqs[0] == 0 && cum_freqs[256] == target_total);
    for (int i=0; i < 256; i++) {
        if (freqs[i] == 0)
            assert(cum_freqs[i+1] == cum_freqs[i]);
        else
            assert(cum_freqs[i+1] > cum_freqs[i]);

        // calc updated freq
        freqs[i] = cum_freqs[i+1] - cum_freqs[i];
    }
}

template <typename T>
void append_to_file(const char* filename, const T* data, size_t size) {
    // Open the file in binary mode for writing
    FILE* file = fopen(filename, "ab");

    // Check if the file was opened successfully
    if (file == NULL) {
        fprintf(stderr, "Error opening the file: %s\n", filename);
        return;
    }

    // Write the data to the file
    fwrite(data, sizeof(T), size, file);

    // Close the file
    fclose(file);
}

void create_empty_file(const char* filename) {
    FILE* file = fopen(filename, "wb"); // Open the file in binary mode for writing, creates or overwrites the file

    if (file == NULL) {
        perror("Error opening the file");
    }

    // File is created or overwritten and ready to be used

    fclose(file); // Close the file
}


namespace coders {
    class AnsCoderAdapter {
    private:
        static constexpr uint32_t BLOCK_SIZE_BY_ORIGINAL = 32<<15;
    public:
        static void compress(const char* in, const char* out) {
            FILE* infile = fopen(in, "rb"); // Open the infile in binary mode
            create_empty_file(out);
            if (infile == NULL) {
                perror("Error opening the file");
                return;
            }

            SymbolStats stats;
            stats.count_freqs(infile);
            // Move the file pointer to the end of the file
            fseek(infile, 0, SEEK_END);
            // Get the size of the file
            unsigned long fileSize = ftell(infile);
            static const uint32_t prob_bits = 14;
            static const uint32_t prob_scale = 1 << prob_bits;
            stats.normalize_freqs(prob_scale);
            uint8_t cum2sym[prob_scale];
            for (int s=0; s < 256; s++)
                for (uint32_t i=stats.cum_freqs[s]; i < stats.cum_freqs[s+1]; i++)
                    cum2sym[i] = s;

            // try rANS encode
            RansEncSymbol esyms[256];
            for (int i=0; i < 256; i++) {
                RansEncSymbolInit(&esyms[i], stats.cum_freqs[i], stats.freqs[i], prob_bits);
            }

            uint32_t block_sizes[1024];
            unsigned long compress_size = 0;

            for (long i = fileSize, ind = 0; i > 0; i -= BLOCK_SIZE_BY_ORIGINAL, ++ind) {
                int qq = 0;
                RansState rans;
                RansEncInit(&rans);
                uint8_t out_buf[BLOCK_SIZE_BY_ORIGINAL];

                uint8_t* ptr = out_buf + BLOCK_SIZE_BY_ORIGINAL; // *end* of output buffer
                for (long j = i - 1; j >= i - BLOCK_SIZE_BY_ORIGINAL && j >= 0; --j) {
                    fseek(infile, j, SEEK_SET); // Move the file pointer to the desired position
                    int s = fgetc(infile); // Read the character at the current positions
                    RansEncPutSymbol(&rans, &ptr, &esyms[s]);
                    qq++;
                }
                RansEncFlush(&rans, &ptr);
                uint8_t *rans_begin;
                rans_begin = ptr;

                int batch_compress_size = (int) (out_buf + BLOCK_SIZE_BY_ORIGINAL - rans_begin);
                append_to_file(out, rans_begin, batch_compress_size);
                block_sizes[ind] = batch_compress_size;
                compress_size += batch_compress_size;
            }

            append_to_file(out, stats.freqs, SymbolStats::FREQS_SIZE);
            append_to_file(out, stats.cum_freqs, SymbolStats::CUM_FREQS_SIZE);
            append_to_file(out, block_sizes, 1024);
            append_to_file(out, &fileSize, 1);
        }

        static void decompress(const char* in, const char* out) {
            SymbolStats stats;
            create_empty_file(out);
            FILE* file = fopen(in, "rb"); // Open the file in binary mode for reading
            if (file == NULL) {
                perror("Error opening the file");
            }
            fseek(file, 0, SEEK_END);
            long fileSize = ftell(file);
            fseek(file, 0, SEEK_SET);

            // Calculate the starting position of the arrays and compress_size
            long start_pos = fileSize - sizeof(uint64_t) * 256 - sizeof(uint64_t) * 257 - sizeof(uint32_t) * 1024 - sizeof(unsigned long);

            // Set the file pointer to the starting position
            fseek(file, start_pos, SEEK_SET);

            // Read the arrays and compress_size from the file
            uint32_t block_sizes[1024];
            unsigned long originalFileSize;
            if (fread(stats.freqs, sizeof(uint64_t), 256, file) != 256 ||
                fread(stats.cum_freqs, sizeof(uint64_t), 257, file) != 257 ||
                fread(block_sizes, sizeof(uint32_t), 1024, file) != 1024 ||
                fread(&originalFileSize, sizeof(unsigned long), 1, file) != 1) {
                fprintf(stderr, "Error reading data from the file\n");
                fclose(file);
                return;
            }

            static const uint32_t prob_bits = 14;
            static const uint32_t prob_scale = 1 << prob_bits;
            uint8_t cum2sym[prob_scale];
            for (int s = 0; s < 256; s++)
                for (uint32_t i = stats.cum_freqs[s]; i < stats.cum_freqs[s + 1]; i++)
                    cum2sym[i] = s;
            RansDecSymbol dsyms[256];
            for (int i=0; i < 256; i++) {
                RansDecSymbolInit(&dsyms[i], stats.cum_freqs[i], stats.freqs[i]);
            }

            int num_blocks = 0;
            uint64_t off_blocks[1024] = {0};
            while (block_sizes[num_blocks] > 0) {
                off_blocks[num_blocks + 1] = off_blocks[num_blocks] + block_sizes[num_blocks];
                num_blocks++;
            }

            for (long blockId = num_blocks - 1; blockId >= 0; --blockId) {
                const auto cur_block_size = block_sizes[blockId];
                const auto original_block_size = num_blocks - 1 == blockId ? originalFileSize % BLOCK_SIZE_BY_ORIGINAL : BLOCK_SIZE_BY_ORIGINAL;
                uint8_t dec_bytes[original_block_size];
                uint8_t rans_begin[original_block_size];
                fseek(file, off_blocks[blockId], SEEK_SET);
                fread(rans_begin, 1, cur_block_size, file);
                RansState rans;
                uint8_t *ptr = rans_begin;
                RansDecInit(&rans, &ptr);
                //
                for (size_t j = 0; j < original_block_size; j++) {
                    uint32_t s = cum2sym[RansDecGet(&rans, prob_bits)];
                    dec_bytes[j] = (uint8_t)s;
                    RansDecAdvanceSymbol(&rans, &ptr, &dsyms[s], prob_bits);
                }
                append_to_file(out, dec_bytes, original_block_size);
            }
            fclose(file);
        }
    };
}

