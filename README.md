libexdupe is a compression library that also performs data deduplication.

It finds identical sequences of data as small as 4 KB across terabytes of input data at a speed of **5 gigabyte per second** (in-memory, 4 threads, level 0).

It has a simple C++ interface. All you need to compress from stdin to stdout is following:
```
int main() {
    size_t mb = 1 << 20;
    compressor::init(8, 128 * mb, 1);

    while (std::cin.peek() != EOF) {
        char* in = compressor::get_buffer(mb);
        size_t len = fread(in, 1, mb, stdin);
        compressor::result r = compressor::compress(len);
        fwrite(r.result, 1, r.length, stdout);
    }

    compressor::result r = compressor::flush();
    fwrite(r.result, 1, r.length, stdout);
    fflush(stdout);
    compressor::uninit();
}
```
It's currently an early preview version - use for testing only. You can try it with tar:

```tar cvf - all_my_files | ./demo -c > archive.demo```

Below is a benchmark of the backup tool [eXdupe](https://exdupe.net/) which uses libexdupe to compress a Linux virtual machine of 22.1 GB:

|                | eXdupe 3 | tar+zstd | kopia | restic | Duplicacy | zpaq64 | 7-Zip flzma2
|----------------|--------|----------|-------|--------|-----------|--------|--------------
| **Time**           | 9.76 s | 14.2 s   | 14.8 s | 24.8 s | 77.0 s    | 112 s  | 209 s        
| **Size**          | 7.34 GB| 10.6 GB  | 9.93 GB| 9.21 GB| 11.4 GB   | 8.18 GB| 9.42 GB      

Some of the performance comes from a new deduplication algorithm that reduces the number of expensive lookups in a hashtable.

Limitations: Not all kinds of data will benefit from deduplication, especially not the common compression benchmark suites. It also uses alot of memory: 1 gigabyte per 50 gigabyte of data is best.
