#include <assert.h>
#include <iostream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "libexdupe.h"

// Customize your own error handling here
#define libexdupe_assert(b) if(!(b)) { \
    std::cerr << "libexdupe_assert failed: " << #b << ", file " << __FILE__ << ", line " << __LINE__ << std::endl; \
    abort(); \
}

namespace libexdupe {
    size_t m = 1 << 20;
    size_t k = 1 << 10;

    namespace compressor {
    namespace detail {
        std::vector<char> hash_table;
        std::vector<std::vector<char>> in;
        std::vector<std::vector<char>> out;
        std::vector<char> flushed;
        size_t ptr = 0;
        int threads;
    }

    struct result {
        char* result;
        size_t length;
    };

    void init(int threads, size_t hash_size, int level) {
        libexdupe_assert(threads >= 1);
        libexdupe_assert(hash_size >= m);
        libexdupe_assert(level >= 0 && level <= 3);
        
        using namespace detail;
        detail::threads = threads;
        hash_table.resize(hash_size);
        in.resize(threads + 1);
        out.resize(threads + 1);
        dup_init_compression(128 * k, 4 * k, hash_size, threads, hash_table.data(), level, false, 0, 0);
    }

    void uninit() {
        dup_uninit_compression();
        detail::flushed.clear();
        detail::flushed.shrink_to_fit();
    }

    void flush(std::vector<char>& dst) {
        using namespace detail;
        char* res;
        size_t len;
        dst.clear();
        do {
            len = dup_flush_pend_block(nullptr, &res, nullptr);
            dst.insert(dst.end(), res, res + len);
        } while (len > 0);
    }

    char* get_buffer(size_t reserve) {
        using namespace detail;
        if(in[ptr].size() < reserve) {
            in[ptr].resize(reserve);
        }
        if (out[ptr].size() < reserve + 128 * k) {
            out[ptr].resize(reserve + 128 * k);
        }
        return in[ptr].data();
    }
    
    result compress(size_t length) {
        using namespace detail;
        libexdupe_assert(length <= in[ptr].size());
        char* compressed;
        size_t ret = dup_compress(in[ptr].data(), out[ptr].data(), length, nullptr, false, &compressed, nullptr);
        ptr = (ptr + 1) % in.size();
        return { compressed, ret };
    }

    result flush() {
        using namespace detail;
        char* res;
        size_t len;

        do {
            len = dup_flush_pend_block(0, &res, nullptr);
            flushed.insert(flushed.end(), res, res + len);
        } while (len > 0);

        return { flushed.data(), flushed.size() };
    }

}

namespace decompressor {
    size_t header = DUP_HEADER_LEN;

    struct reference {
        size_t length;
        size_t position;
    };

    void init() {
        dup_init_decompression();
    }

    void uninit() {
        dup_uninit_decompression();
    }

    bool is_reference(const std::vector<char>& src) {
        int r = dup_packet_info(src.data(), nullptr, nullptr);
        libexdupe_assert(r == DUP_REFERENCE || r == DUP_LITERAL);
        return r == DUP_REFERENCE;
    }

    void decompress_literals(const std::vector<char>& src, std::vector<char>& dst) {
        libexdupe_assert(!is_reference(src));
        uint64_t pay;
        size_t len = dup_size_decompressed(src.data());
        dst.resize(len);
        int r = dup_decompress(src.data(), dst.data(), &len, &pay);
        libexdupe_assert(r == 0 || r == 1);
    }

    size_t packet_size(const std::vector<char>& src) {
        size_t r = dup_size_compressed(src.data());
        libexdupe_assert(r);
        return r;
    }

    reference get_reference(const std::vector<char>& src) {
        libexdupe_assert(is_reference(src));
        uint64_t pay;
        size_t len;
        int r = dup_packet_info(src.data(), &len, &pay);
        libexdupe_assert(r);
        return reference(len, pay);
    }
}
}