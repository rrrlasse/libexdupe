#include <fcntl.h>
#include <iostream>
#include <vector>
#include <stdio.h>

#ifdef _WIN32
#include <io.h>
#define fseeko64 _fseeki64
#endif

#include "libexdupe/libexdupe_binding.hpp"

using namespace std;

size_t rd(vector<char>& dst, size_t len, FILE* f, size_t offset, bool exact) {
    dst.resize(len + offset);
    size_t r = fread(dst.data() + offset, 1, len, f);
    libexdupe_assert(!exact || r == len);
    dst.resize(r);
    return r;
}


// hash_size: Set to around 1 MB per 100 MB of input data
// level: 1...3 means LZ compression is done after deduplication, 0 means no LZ
// threads: more is not always faster
void compress(size_t hash_size, int threads, size_t chunk_size, int level) {
    libexdupe_assert(level >= 0 && level <= 3);
    compressor::init(threads, hash_size, level);

    while (std::cin.peek() != EOF) {
        char* in = compressor::get_buffer(chunk_size);
        size_t len = fread(in, 1, chunk_size, stdin);
        compressor::result r = compressor::compress(len);
        fwrite(r.result, 1, r.length, stdout);
    }

    compressor::result r = compressor::flush();
    fwrite(r.result, 1, r.length, stdout);
    fflush(stdout);
    compressor::uninit();
}


// Read consecutive packets. A packet can either contain a block of user payload, or it can be
// a reference that into past written data (i.e. we are at a duplicated sequence of data).
void decompress(string outfile) {
    vector<char> in;
    vector<char> out;

    decompressor::init();
    FILE* ofile = fopen(outfile.c_str(), "wb+");

    while (std::cin.peek() != EOF) {
        size_t len = rd(in, decompressor::header, stdin, 0, true);
        size_t packet = decompressor::packet_size(in);
        len = rd(in, packet - decompressor::header, stdin, decompressor::header, true);

        if (decompressor::is_reference(in)) {
            decompressor::reference r = decompressor::get_reference(in);
            fseeko64(ofile, r.position, SEEK_SET);
            rd(out, r.length, ofile, 0, true);
            fseeko64(ofile, 0, SEEK_END);
        }
        else {
            decompressor::decompress_literals(in, out);
        }

        fwrite(out.data(), out.size(), 1, ofile);
    }

    fclose(ofile);
    decompressor::uninit();
}


int main(int argc, char *argv[]) {
#ifdef _WIN32
    (void)_setmode(_fileno(stdin), _O_BINARY);
    (void)_setmode(_fileno(stdout), _O_BINARY);
#endif
   
    if(argc == 3 && std::string(argv[1]) == "-d") {
        decompress(argv[2]);
    }
    else if(argc == 2 && std::string(argv[1]) == "-c") {
        compress(60 * 1024 * 1024ull, 8, 1024 * 1024ull, 1);
    }
    else {
        cerr << "demo -c < input_file > compressed_file"
            "\ndemo -d output_file < compressed_file\n\n";
    }
    return 0;
}