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
Compress using one method for all file (`mgi_1_8.fq`)
```sh
./test_mgi -i mgi_1_8.fq -o mgi_1_8_range.zip -m range -c
```
Decompress using one method for all file (`mgi_1_8_range.zip`)
```sh
./test_mgi -i mgi_1_8_range.zip -o mgi_1_8_range_unzip.unzip -m range -d
```

Compress using several methods for different parts of `mgi_1_8.fq` that are in the file `range_methods.txt`
```sh
./test_mgi -i mgi_1_8.fq -o mgi_1_8_range_sep.zip -m range_methods.txt -c
```
Decompress using several methods for different parts of `mgi_1_8.fq` that are in the file `range_methods.txt`
```sh
./test_mgi -i mgi_1_8_range_sep.zip -o mgi_1_8_range_sep.unzip -m range_methods.txt -d
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

### Compressing separated parts (each of the 4 line types separately)
| IDs |	NNs |	QUALITIES |	SEQUENCES |
| ------ | ------ | ------ | ------ |
**ANS** |	0.462	| 0.141 | 0.307 | 0.255 |
**HUFFMAN** |	0.465 |	0.125	 | 0.312 |	0.279 |
**RANGE**	| 0.283	| 0.007 |	0.298	| 0.249 |

## Details of implementation
1) Huffman encoding algorithm based on https://github.com/cynricfu/huffman-coding
2) ANS algorithm based on https://github.com/rygorous/ryg_rans.
Top-level compression and decompression methods were written from scratch.
3) Range encoding algorithm based on Dmitry Subbotin's original carry less range coder https://gist.github.com/richgel999/d522e318bf3ad67e019eabc13137350a

