//
// Created by ivankozlov98 on 8/1/23.
//

#ifndef TEST_MGI_RANGE_CODER_H
#define TEST_MGI_RANGE_CODER_H


//The context compressor, author M.Smirnov.
//The source also can be found at
//http://compression.graphicon.ru/
//usage:
//c infile outfile //encoding
//d infile outfile //decoding

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

class DFile {

    FILE *f;
public:
    int ReadSymbol (void) {
        return getc(f);
    };

    int WriteSymbol (int c) {
        return putc(c, f);
    };

    FILE* GetFile (void) {
        return f;
    }

    void SetFile (FILE *file) {
        f = file;
    }

} DataFile, CompressedFile;

typedef unsigned int uint;
#define DO(n) for (int _=0; _<n; _++)
#define TOP (1<<24)

class RangeCoder {
    uint code, range, FFNum, Cache;
    __int64_t low; // Microsoft C/C++ 64-bit integer type
    FILE *f;
public:

    void StartEncode( FILE *out ) {
        low=FFNum=Cache=0; range=(uint)-1;
        f = out;
    }

    void StartDecode( FILE *in ) {
        code=0;
        range=(uint)-1;
        f = in;
        DO(5) code=(code<<8) | getc(f);
    }

    void FinishEncode( void ) {
        low+=1;
        DO(5) ShiftLow();
    }

    void FinishDecode( void ) {
    }

    void encode(uint cumFreq, uint freq, uint totFreq) {
        low += cumFreq * (range/= totFreq);
        range*= freq;
        while( range<TOP ) ShiftLow(), range<<=8;
    }

    inline void ShiftLow( void ) {
        if ( (low>>24)!=0xFF ) {
            putc ( Cache + (low>>32), f );
            int c = 0xFF+(low>>32);
            while ( FFNum ) putc(c, f), FFNum--;
            Cache = uint(low)>>24;
        } else
            FFNum++;
        low = uint(low)<<8;
    }

    uint get_freq (uint totFreq) {
        return code / (range/= totFreq);
    }

    void decode_update (uint cumFreq, uint freq, uint totFreq) {
        code -= cumFreq*range;
        range *= freq;
        while( range<TOP ) code=(code<<8)|getc(f), range<<=8;
    }

} AC;

struct ContextModel {
    int esc,
            TotFr;
    int count[256];
};

ContextModel cm[257], *stack[2];
int context [1], SP;
const int MAX_TotFr = 0x3fff;

void init_model (void) {
    for ( int j = 0; j < 256; j++ )
        cm[256].count[j] = 1;
    cm[256].TotFr = 256;
    cm[256].esc = 1;
    context[0] = SP = 0;
}

int encode_sym (ContextModel *CM, int c) {
    stack [SP++] = CM;
    if (CM->count[c]) {
        int CumFreqUnder = 0;
        for (int i = 0; i < c; i++)
            CumFreqUnder += CM->count[i];
        AC.encode (CumFreqUnder, CM->count[c], CM->TotFr + CM->esc);
        return 1;
    }
    else {
        if (CM->esc) {
            AC.encode (CM->TotFr, CM->esc, CM->TotFr +
                                           CM->esc);
        }
        return 0;
    }
}

int decode_sym (ContextModel *CM, int *c) {
    stack [SP++] = CM;
    if (!CM->esc)
        return 0;
    int cum_freq = AC.get_freq (CM->TotFr + CM->esc);
    if (cum_freq < CM->TotFr) {
        int CumFreqUnder = 0;
        int i = 0;
        for (;;) {
            if ( (CumFreqUnder + CM->count[i]) <= cum_freq)
                CumFreqUnder += CM->count[i];
            else
                break;
            i++;
        }

        AC.decode_update (CumFreqUnder, CM->count[i], CM->TotFr + CM->esc);
        *c = i;
        return 1;
    }
    else {
        AC.decode_update (CM->TotFr, CM->esc, CM->TotFr + CM->esc);
        return 0;
    }
}

void rescale (ContextModel *CM) {
    CM->TotFr = 0;
    for (int i = 0; i < 256; i++) {
        CM->count[i] -= CM->count[i] >> 1;
        CM->TotFr += CM->count[i];
    }
}

void update_model (int c) {
    while (SP) {
        SP--;
        if (stack[SP]->TotFr >= MAX_TotFr)
            rescale (stack[SP]);
        stack[SP]->TotFr += 1;
        if (!stack[SP]->count[c])
            stack[SP]->esc += 1;
        stack[SP]->count[c] += 1;
    }
}

void encode (void){
    int c, success;
    init_model ();
    AC.StartEncode (CompressedFile.GetFile());
    while (( c = DataFile.ReadSymbol() ) != EOF) {
        success = encode_sym (&cm[context[0]], c);
        if (!success)
            encode_sym (&cm[256], c);
        update_model (c);
        context [0] = c;
    }

    AC.encode (cm[context[0]].TotFr, cm[context[0]].esc, cm[context[0]].TotFr + cm[context[0]].esc);
    AC.encode (cm[256].TotFr, cm[256].esc, cm[256].TotFr + cm[256].esc);
    AC.FinishEncode();
}

void decode (void){
    int c, success;
    init_model ();
    AC.StartDecode (CompressedFile.GetFile());
    for (;;) {
        success = decode_sym (&cm[context[0]], &c);
        if (!success){
            success = decode_sym (&cm[256], &c);
            if (!success)
                break;
        }

        update_model (c);

        context [0] = c;
        DataFile.WriteSymbol (c);
    }
}

int getFileSize(char* fileName) {

    FILE* f;
    if ((f = fopen(fileName, "rb")) == NULL)
        return -1;

    fseek(f, 0, SEEK_END);
    int bytes = ftell(f);
    fclose(f);

    return bytes;
}

namespace coders {
    class RangeCoderAdapter {
    public:
        static void compress(char* inFile, char* outFile) {
            clock_t start_encoding = clock();
            FILE *inf, *outf;
            inf = fopen (inFile, "rb");
            outf = fopen (outFile, "wb");
            DataFile.SetFile (inf);
            CompressedFile.SetFile (outf);
            encode ();
            fclose (inf);
            fclose (outf);

            int data_size = getFileSize(inFile);
            int result_size = getFileSize(outFile);

            printf("Original size                                %10d bytes\n", data_size);
            printf("Actual encoded size                          %10d bytes\n\n", result_size);

            clock_t end_encoding = clock();
            printf("Time for encoding                                 %2.3f sec.\n", (double)(end_encoding - start_encoding)/CLOCKS_PER_SEC);

            double ratio = (double)result_size;
            ratio /= (double)data_size;
            printf("Compression ratio                                 %2.3f of original size.\n\n", ratio);
        }

        static void decompress(char* inFile, char* outFile){
            FILE *inf, *outf;
            int data_size = getFileSize(inFile);
            printf("Compressed file size        %10d bytes\n", data_size);

            clock_t start_decoding = clock();

            inf = fopen (inFile, "rb");
            outf = fopen (outFile, "wb");
            CompressedFile.SetFile (inf);
            DataFile.SetFile (outf);
            decode ();
            fclose (inf);
            fclose (outf);

            clock_t end_decoding = clock();
            printf("Decompression is ended, time is %2.3f sec.\n\n", (double)(end_decoding - start_decoding)/CLOCKS_PER_SEC);
        }
    };
}


#endif //TEST_MGI_RANGE_CODER_H
