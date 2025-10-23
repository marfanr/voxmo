#ifndef __METADATA_HH__
#define __METADATA_HH__

#include <cstdint>
struct  metadata_file {
    uint64_t next_offset;
    uint32_t metadata_length;
    uint32_t size;
    uint16_t nama_file_len;
    // char name[nama_file_len]
};

struct metadata_header {
    uint32_t magic;
    uint16_t version;
    uint32_t header_len;
    uint16_t nama_module_len;
    // char nama_module[nama_module_len]
};

#endif // __METADATA__HH__