# Fastq compress/decompress
This tool has 3 methods compress/decompress `.fastq` files:
* Asymmetric numeral systems (https://en.wikipedia.org/wiki/Asymmetric_numeral_systems)
* Range coding (https://en.wikipedia.org/wiki/Range_coding)
* Huffman coding (https://en.wikipedia.org/wiki/Huffman_coding)

Also, with this tool allows combine the 3 methods for the compression of different parts (for example, sequences or qualities) of the fastq file.

## Parameters
```sh
Allowed options:
  -i [ --input ] arg         input file
  -o [ --output ] arg        output file
  -c [ --compress ]          compress
  -d [ --decompress ]        decompress
  -m [ --method ] arg (=ans) compression method, possible values:
                             'ans' - Asymmetric numeral systems
                             'huffman' - Huffman coding
                             'range' - Range encoding
                             filename with methods for every type line
  -h [ --help ]              produce help message

```

## Usage example
Compress using one method for all file (`path_to_fq`)
```sh
./test_mgi -i path_to_fq -o path_to_fq.com -m range -c
```
Decompress using one method for all file (`path_to_fq.com`)
```sh
./test_mgi -i path_to_fq.com -o path_to_fq.uncom -m range -d
```

Compress using several methods for different parts of `path_to_fq` that are in the file `encode_methods` (examples in directory `experiments`)
```sh
./test_mgi -i path_to_fq -o path_to_fq_sep.com -m encode_methods -c
```
Decompress using several methods for different parts of `path_to_fq_sep.com` that are in the file `encode_methods`
```sh
./test_mgi -i path_to_fq_sep.com -o path_to_fq_sep.uncom -m encode_methods -d
```

## Experiments
### Compression ratio
| Range | Huffman | ANS |
| ------ | ------ | ------ |
| 0.300 | 0.440  | 0.438 |

Then I made 8 files from test-fastq-file with sizes (1/8 test-fastq-file, 2/8 test-fastq-file .. 8/8 test-fastq-file) and saw asymptotics time
### Asymptotics time compressing
![chart](https://github.com/IvanKozlov98/FastqCompressorDecompressor/assets/45848690/a1c32309-3391-4d22-a842-83f5748ea8b7)

### Asymptotics time decompressing
![chart (1)](https://github.com/IvanKozlov98/FastqCompressorDecompressor/assets/45848690/e9397da5-d1c5-4a5a-b27e-c570d3258e6b)

### Asymptotics time compressing+decompressing
![chart (2)](https://github.com/IvanKozlov98/FastqCompressorDecompressor/assets/45848690/8e6db202-9cf0-46d2-b698-c0af39d18fa5)

### Compressing separated parts (each of the 4 line types separately)
**Compression ratio table**
| | IDs |	NNs |	QUALITIES |	SEQUENCES |
| ------ | ------ | ------ | ------ | ------ |
**ANS** |	0.462	| 0.141 | 0.307 | 0.255 |
**HUFFMAN** |	0.465 |	0.125	 | 0.312 |	0.279 |
**RANGE**	| 0.283	| 0.007 |	0.298	| 0.249 |

### Discussion
Given the characteristics of fastq files, a combination of Huffman coding and Range encoding may be a reasonable choice. Huffman coding can effectively compress the DNA sequence and quality score symbols, while Range encoding can further enhance the compression efficiency for repetitive patterns and complex data. This combination can provide a good balance between compression efficiency and speed for fastq files.

ANS is also a powerful option and can potentially outperform both Huffman coding and Range encoding in terms of compression ratios. However, it may require more complex and computationally-intensive implementations, which could be a trade-off when considering compression speed.

Ultimately, the best method for compression may require experimentation and benchmarking on a representative set of fastq files to identify the most suitable method for your specific data and performance requirements.

In the considered test example, it can be observed that data compression using range coding and Huffman encoding is faster than with ANS during encoding, while during decoding, it lags significantly in speed. Ultimately, the sum of compression and decompression times for all three methods is approximately the same. Additionally, it is worth noting that the range coder showed the best compression ratio, both for compressing the entire file and for compressing it in parts. Compressing individual parts did not demonstrate significant advantages in using a combination of methods to accelerate compression or reduce compression ratio.

## Details of implementation
1) Huffman encoding algorithm based on https://github.com/cynricfu/huffman-coding
2) ANS algorithm based on https://github.com/rygorous/ryg_rans.
Top-level compression and decompression methods were written from scratch.
3) Range encoding algorithm based on Dmitry Subbotin's original carry less range coder https://gist.github.com/richgel999/d522e318bf3ad67e019eabc13137350a

## TODO
1) Exception handling
2) Tests
